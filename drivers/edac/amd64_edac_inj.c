#include "amd64_edac.h"

static ssize_t amd64_inject_section_show(struct device *dev,
					 struct device_attribute *mattr,
					 char *buf)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	return sprintf(buf, "0x%x\n", pvt->injection.section);
}

/*
                                                                                
                             
  
              
 */
static ssize_t amd64_inject_section_store(struct device *dev,
					  struct device_attribute *mattr,
					  const char *data, size_t count)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	unsigned long value;
	int ret;

	ret = strict_strtoul(data, 10, &value);
	if (ret < 0)
		return ret;

	if (value > 3) {
		amd64_warn("%s: invalid section 0x%lx\n", __func__, value);
		return -EINVAL;
	}

	pvt->injection.section = (u32) value;
	return count;
}

static ssize_t amd64_inject_word_show(struct device *dev,
					struct device_attribute *mattr,
					char *buf)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	return sprintf(buf, "0x%x\n", pvt->injection.word);
}

/*
                                                                               
                                       
  
              
 */
static ssize_t amd64_inject_word_store(struct device *dev,
				       struct device_attribute *mattr,
				       const char *data, size_t count)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	unsigned long value;
	int ret;

	ret = strict_strtoul(data, 10, &value);
	if (ret < 0)
		return ret;

	if (value > 8) {
		amd64_warn("%s: invalid word 0x%lx\n", __func__, value);
		return -EINVAL;
	}

	pvt->injection.word = (u32) value;
	return count;
}

static ssize_t amd64_inject_ecc_vector_show(struct device *dev,
					    struct device_attribute *mattr,
					    char *buf)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	return sprintf(buf, "0x%x\n", pvt->injection.bit_map);
}

/*
                                                                            
                                                                              
                                                                    
 */
static ssize_t amd64_inject_ecc_vector_store(struct device *dev,
				       struct device_attribute *mattr,
				       const char *data, size_t count)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	unsigned long value;
	int ret;

	ret = strict_strtoul(data, 16, &value);
	if (ret < 0)
		return ret;

	if (value & 0xFFFF0000) {
		amd64_warn("%s: invalid EccVector: 0x%lx\n", __func__, value);
		return -EINVAL;
	}

	pvt->injection.bit_map = (u32) value;
	return count;
}

/*
                                                                          
                                                                            
 */
static ssize_t amd64_inject_read_store(struct device *dev,
				       struct device_attribute *mattr,
				       const char *data, size_t count)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	unsigned long value;
	u32 section, word_bits;
	int ret;

	ret = strict_strtoul(data, 10, &value);
	if (ret < 0)
		return ret;

	/*                                                   */
	section = F10_NB_ARRAY_DRAM | SET_NB_ARRAY_ADDR(pvt->injection.section);

	amd64_write_pci_cfg(pvt->F3, F10_NB_ARRAY_ADDR, section);

	word_bits = SET_NB_DRAM_INJECTION_READ(pvt->injection);

	/*                                                    */
	amd64_write_pci_cfg(pvt->F3, F10_NB_ARRAY_DATA, word_bits);

	edac_dbg(0, "section=0x%x word_bits=0x%x\n", section, word_bits);

	return count;
}

/*
                                                                              
                                            
 */
static ssize_t amd64_inject_write_store(struct device *dev,
					struct device_attribute *mattr,
					const char *data, size_t count)
{
	struct mem_ctl_info *mci = to_mci(dev);
	struct amd64_pvt *pvt = mci->pvt_info;
	u32 section, word_bits, tmp;
	unsigned long value;
	int ret;

	ret = strict_strtoul(data, 10, &value);
	if (ret < 0)
		return ret;

	/*                                                   */
	section = F10_NB_ARRAY_DRAM | SET_NB_ARRAY_ADDR(pvt->injection.section);

	amd64_write_pci_cfg(pvt->F3, F10_NB_ARRAY_ADDR, section);

	word_bits = SET_NB_DRAM_INJECTION_WRITE(pvt->injection);

	pr_notice_once("Don't forget to decrease MCE polling interval in\n"
			"/sys/bus/machinecheck/devices/machinecheck<CPUNUM>/check_interval\n"
			"so that you can get the error report faster.\n");

	on_each_cpu(disable_caches, NULL, 1);

	/*                                                    */
	amd64_write_pci_cfg(pvt->F3, F10_NB_ARRAY_DATA, word_bits);

 retry:
	/*                              */
	amd64_read_pci_cfg(pvt->F3, F10_NB_ARRAY_DATA, &tmp);
	if (tmp & F10_NB_ARR_ECC_WR_REQ) {
		cpu_relax();
		goto retry;
	}

	on_each_cpu(enable_caches, NULL, 1);

	edac_dbg(0, "section=0x%x word_bits=0x%x\n", section, word_bits);

	return count;
}

/*
                                                   
 */

static DEVICE_ATTR(inject_section, S_IRUGO | S_IWUSR,
		   amd64_inject_section_show, amd64_inject_section_store);
static DEVICE_ATTR(inject_word, S_IRUGO | S_IWUSR,
		   amd64_inject_word_show, amd64_inject_word_store);
static DEVICE_ATTR(inject_ecc_vector, S_IRUGO | S_IWUSR,
		   amd64_inject_ecc_vector_show, amd64_inject_ecc_vector_store);
static DEVICE_ATTR(inject_write, S_IWUSR,
		   NULL, amd64_inject_write_store);
static DEVICE_ATTR(inject_read,  S_IWUSR,
		   NULL, amd64_inject_read_store);


int amd64_create_sysfs_inject_files(struct mem_ctl_info *mci)
{
	int rc;

	rc = device_create_file(&mci->dev, &dev_attr_inject_section);
	if (rc < 0)
		return rc;
	rc = device_create_file(&mci->dev, &dev_attr_inject_word);
	if (rc < 0)
		return rc;
	rc = device_create_file(&mci->dev, &dev_attr_inject_ecc_vector);
	if (rc < 0)
		return rc;
	rc = device_create_file(&mci->dev, &dev_attr_inject_write);
	if (rc < 0)
		return rc;
	rc = device_create_file(&mci->dev, &dev_attr_inject_read);
	if (rc < 0)
		return rc;

	return 0;
}

void amd64_remove_sysfs_inject_files(struct mem_ctl_info *mci)
{
	device_remove_file(&mci->dev, &dev_attr_inject_section);
	device_remove_file(&mci->dev, &dev_attr_inject_word);
	device_remove_file(&mci->dev, &dev_attr_inject_ecc_vector);
	device_remove_file(&mci->dev, &dev_attr_inject_write);
	device_remove_file(&mci->dev, &dev_attr_inject_read);
}
