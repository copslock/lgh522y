#ifndef TARGET_CORE_RD_H
#define TARGET_CORE_RD_H

#define RD_HBA_VERSION		"v4.0"
#define RD_MCP_VERSION		"4.0"

/*                                              */
#define RD_MAX_ALLOCATION_SIZE	65536
#define RD_DEVICE_QUEUE_DEPTH	32
#define RD_MAX_DEVICE_QUEUE_DEPTH 128
#define RD_BLOCKSIZE		512

/*                                                              */
int __init rd_module_init(void);
void rd_module_exit(void);

struct rd_dev_sg_table {
	u32		page_start_offset;
	u32		page_end_offset;
	u32		rd_sg_count;
	struct scatterlist *sg_table;
} ____cacheline_aligned;

#define RDF_HAS_PAGE_COUNT	0x01
#define RDF_NULLIO		0x02

struct rd_dev {
	struct se_device dev;
	u32		rd_flags;
	/*                                         */
	u32		rd_dev_id;
	/*                                     */
	u32		rd_page_count;
	/*                                       */
	u32		sg_table_count;
	/*                                                    */
	struct rd_dev_sg_table *sg_table_array;
	/*                                    */
	struct rd_host *rd_host;
} ____cacheline_aligned;

struct rd_host {
	u32		rd_host_dev_id_count;
	u32		rd_host_id;		/*                        */
} ____cacheline_aligned;

#endif /*                  */
