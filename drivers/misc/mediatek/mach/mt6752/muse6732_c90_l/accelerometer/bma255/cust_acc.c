#include <linux/types.h>
#include <cust_acc.h>
#include <mach/mt_pm_ldo.h>

/*                                                                           */
int cust_acc_power(struct acc_hw *hw, unsigned int on, char* devname)
{
#ifndef FPGA_EARLY_PORTING
    if (hw->power_id == MT65XX_POWER_NONE)
        return 0;
    if (on)
        return hwPowerOn(hw->power_id, hw->power_vol, devname);
    else
        return hwPowerDown(hw->power_id, devname);
#else
    return 0;
#endif
}
/*                                                                           */
static struct acc_hw cust_acc_hw = {
    .i2c_num = 1,
    .direction = 6,
    .power_id = MT65XX_POWER_NONE,  /*                   */
    .power_vol= VOL_DEFAULT,        /*                   */
    .firlen = 16,                   /*                                 */
    .power = cust_acc_power,
//                              
};
/*                                                                           */
struct acc_hw* get_cust_acc_hw(void)
{
    return &cust_acc_hw;
}
