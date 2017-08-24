/*
 * 
 * Copyright (C) 2011 Goodix, Inc.
 * 
 * Author: kuuga
 * Date: 2012.03.26
 */

#ifdef CREATE_WR_NODE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#define IC_TYPE_NAME        "GT818"
#define DATA_LENGTH_UINT    512
#define CMD_HEAD_LENGTH     (sizeof(st_cmd_head) - sizeof(u8*))
#define GOODIX_ENTRY_NAME   "goodix_tool"


#if 1
#define DEBUG(fmt, arg...) printk("<--GT-DEBUG-->"fmt, ##arg)
#else#define DEBUG(fmt, arg...)
#endif

#if 1
#define NOTICE(fmt, arg...) printk("<--GT-NOTICE-->"fmt, ##arg)
#else#define NOTICE(fmt, arg...)
#endif

#if 1
#define WARNING(fmt, arg...) printk("<--GT-WARNING-->"fmt, ##arg)
#else#define WARNING(fmt, arg...)
#endif

#if 1
#define DEBUG_MSG(fmt, arg...) printk("<--GT msg-->"fmt, ##arg)
#else#define DEBUG_MSG(fmt, arg...)
#endif

#if 1
#define DEBUG_UPDATE(fmt, arg...) printk("<--GT update-->"fmt, ##arg)
#else
#define DEBUG_UPDATE(fmt, arg...)
#endif

#if 1
#define DEBUG_COOR(fmt, arg...) printk(fmt, ##arg)
#define DEBUG_COORD
#else
#define DEBUG_COOR(fmt, arg...)
#endif

#if 1
#define DEBUG_ARRAY(array, num)   do{\
	int i;\
	u8* a = array;\
	for (i = 0; i < (num); i++)\
		{\
		printk("%02x   ", (a)[i]);\
		if ((i + 1 ) %10 == 0)\
			{\
			printk("\n");\
			}\
			}\
			printk("\n");\
			}while(0)
#else
#define DEBUG_ARRAY(array, num) 
#endif 

#define ADDR_MAX_LENGTH     2
#define fail    0
#define success 1

#define false   0
#define true    1

#pragma pack(1)
typedef struct{
    u8  wr;         //                              
    u8  flag;       //                                             
    u8 flag_addr[ADDR_MAX_LENGTH];  //          
    u8  flag_val;   //        
    u8  flag_relation;  //                                                            
    u16 circle;     //        
    u8  times;      //        
    u8  retry;      //               
    u16 delay;      //                  
    u16 data_len;   //        
    u8  addr_len;   //        
    u8  addr[ADDR_MAX_LENGTH];    //    
    u8  res[3];     //    
    u8* data;       //        
}st_cmd_head;
#pragma pack()
st_cmd_head cmd_head;

struct i2c_client *gt_client = NULL;

static struct proc_dir_entry *goodix_proc_entry;

static s32 goodix_tool_write(struct file *filp, const char __user *buff, unsigned long len, void *data);
static s32 goodix_tool_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static s32 (*tool_i2c_read)(u8 *, u16);
static s32 (*tool_i2c_write)(u8 *, u16);

s32 DATA_LENGTH = 0;
s8 IC_TYPE[16] = IC_TYPE_NAME;

static s32 tool_i2c_read_no_extra(u8* buf, u16 len)
{
    s32 ret = -1;
    s32 i = 0;
    struct i2c_msg msgs[2];
    
//                         
//                                              

    //          
    msgs[0].flags = !I2C_M_RD; //      
    msgs[0].addr  = gt_client->addr;
    msgs[0].len   = cmd_head.addr_len;
    msgs[0].buf   = &buf[0];
    
    //        
    msgs[1].flags = I2C_M_RD;//      
    msgs[1].addr  = gt_client->addr;
    msgs[1].len   = len;
    msgs[1].buf   = &buf[ADDR_MAX_LENGTH];

    for (i = 0; i < cmd_head.retry; i++)
    {
        ret=i2c_transfer(gt_client->adapter, msgs, 2);
        if (ret > 0)
        {
            break;
        }
    }
    return ret;
}

static s32 tool_i2c_write_no_extra(u8* buf, u16 len)
{
    s32 ret = -1;
    s32 i = 0;
    struct i2c_msg msg;

//                          
//                          
    //            
    msg.flags = !I2C_M_RD;//      
    msg.addr  = gt_client->addr;
    msg.len   = len;
    msg.buf   = buf;

    for (i = 0; i < cmd_head.retry; i++)
    {
        ret=i2c_transfer(gt_client->adapter, &msg, 1);
        if (ret > 0)
        {
            break;
        }
    }
    return ret;
}

static s32 tool_i2c_read_with_extra(u8* buf, u16 len)
{
    s32 ret = -1;
    u8 pre[2] = {0x0f, 0xff};
    u8 end[2] = {0x80, 0x00};

    tool_i2c_write_no_extra(pre, 2);
    ret = tool_i2c_read_no_extra(buf, len);
    tool_i2c_write_no_extra(end, 2);

    return ret;
}

static s32 tool_i2c_write_with_extra(u8* buf, u16 len)
{
    s32 ret = -1;
    u8 pre[2] = {0x0f, 0xff};
    u8 end[2] = {0x80, 0x00};

    tool_i2c_write_no_extra(pre, 2);
    ret = tool_i2c_write_no_extra(buf, len);
    tool_i2c_write_no_extra(end, 2);

    return ret;
}

static void register_i2c_func(void)
{
    if (!strncmp(IC_TYPE,"GT818", 5) || !strncmp(IC_TYPE, "GT816", 5) 
        || !strncmp(IC_TYPE,"GT827", 5) || !strncmp(IC_TYPE,"GT828", 5)
        || !strncmp(IC_TYPE,"GT813", 5))
    {
        tool_i2c_read = tool_i2c_read_with_extra;
        tool_i2c_write = tool_i2c_write_with_extra;
        NOTICE("I2C function: with pre and end cmd!\n");
    }
    else
    {
        tool_i2c_read = tool_i2c_read_no_extra;
        tool_i2c_write = tool_i2c_write_no_extra;
        NOTICE("I2C function: without pre and end cmd!\n");
    }
}

static void unregister_i2c_func(void)
{
    tool_i2c_read = NULL;
    tool_i2c_write = NULL;
    NOTICE("I2C function: unregister i2c transfer function!\n");
}


s32 init_wr_node(struct i2c_client *client)
{
    s32 i;

    gt_client = client;
    memset(&cmd_head, 0, sizeof(cmd_head));
    cmd_head.data = NULL;

    i = 5;
    while ((!cmd_head.data) && i)
    {
        cmd_head.data = kzalloc(i * DATA_LENGTH_UINT, GFP_KERNEL);
        if (NULL != cmd_head.data)
        {
            break;
        }
        i--;
    }
    if (i)
    {
        DATA_LENGTH = i * DATA_LENGTH_UINT + ADDR_MAX_LENGTH;
        NOTICE("Applied memory size:%d.\n", DATA_LENGTH);
    }
    else
    {
        WARNING("Apply for memory failed.\n");
        return fail;
    }

    cmd_head.addr_len = 2;
    cmd_head.retry = 5;

    register_i2c_func();

    goodix_proc_entry = create_proc_entry(GOODIX_ENTRY_NAME, 0666, NULL);
    if (goodix_proc_entry == NULL)
    {
        WARNING("Couldn't create proc entry!\n");
        return fail;
    }
    else
    {
        NOTICE("Create proc entry success!\n");
        goodix_proc_entry->write_proc = goodix_tool_write;
        goodix_proc_entry->read_proc = goodix_tool_read;
        //                                      
    }

    return success;
}

void uninit_wr_node(void)
{
    kfree(cmd_head.data);
    cmd_head.data = NULL;
    unregister_i2c_func();
    remove_proc_entry(GOODIX_ENTRY_NAME, NULL);
}

static u8 relation(u8 src, u8 dst, u8 rlt)
{
    u8 ret = 0;
    
    switch (rlt)
    {
    case 0:
        ret = (src != dst) ? true : false;
        break;

    case 1:
        ret = (src == dst) ? true : false;
        DEBUG("equal:src:0x%02x   dst:0x%02x   ret:%d\n", src, dst, (s32)ret);
        break;

    case 2:
        ret = (src > dst) ? true : false;
        break;

    case 3:
        ret = (src < dst) ? true : false;
        break;

    case 4:
        ret = (src & dst) ? true : false;
        break;

    case 5:
        ret = (!(src | dst)) ? true : false;
        break;

    default:
        ret = false;
        break;    
    }

    return ret;
}

static u8 comfirm(void)
{
    s32 i = 0;
    u8 buf[32];
    
//                                                                                              
    memcpy(buf, &cmd_head.flag_addr, cmd_head.addr_len);
    
    for (i = 0; i < cmd_head.times; i++)
    {
        if (tool_i2c_read(buf, 1) <= 0)
        {
            WARNING("Read flag data failed!\n");
            return fail;
        }
        if (true == relation(buf[ADDR_MAX_LENGTH], cmd_head.flag_val, cmd_head.flag_relation))
        {
            DEBUG("value at flag addr:0x%02x\n", buf[ADDR_MAX_LENGTH]);
            DEBUG("flag value:0x%02x\n", cmd_head.flag_val);
            break;
        }

        msleep(cmd_head.circle);
    }

    if (i >= cmd_head.times)
    {
        WARNING("Didn't get the flag to continue!\n");
        return fail;
    }

    return success;
}

static s32 goodix_tool_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
{
    DEBUG_ARRAY((u8*)buff, len);
    
    copy_from_user(&cmd_head, buff, CMD_HEAD_LENGTH);

    DEBUG("wr  :0x%02x\n", cmd_head.wr);
    DEBUG("flag:0x%02x\n", cmd_head.flag);
    DEBUG("flag addr:0x%02x%02x\n", cmd_head.flag_addr[0], cmd_head.flag_addr[1]);
    DEBUG("flag val:0x%02x\n", cmd_head.flag_val);
    DEBUG("flag rel:0x%02x\n", cmd_head.flag_relation);
    DEBUG("circle  :%d\n", (s32)cmd_head.circle);
    DEBUG("times   :%d\n", (s32)cmd_head.times);
    DEBUG("retry   :%d\n", (s32)cmd_head.retry);
    DEBUG("delay   :%d\n", (s32)cmd_head.delay);
    DEBUG("data len:%d\n", (s32)cmd_head.data_len);
    DEBUG("addr len:%d\n", (s32)cmd_head.addr_len);
    DEBUG("addr:0x%02x%02x\n", cmd_head.addr[0], cmd_head.addr[1]);
    DEBUG("len:%d\n", (s32)len);
    DEBUG("buf[20]:0x%02x\n", buff[CMD_HEAD_LENGTH]);
    
    if (1 == cmd_head.wr)
    {
      //                                                                                               
        copy_from_user(&cmd_head.data[ADDR_MAX_LENGTH], &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
        memcpy(&cmd_head.data[ADDR_MAX_LENGTH - cmd_head.addr_len], cmd_head.addr, cmd_head.addr_len);

        DEBUG_ARRAY(cmd_head.data, cmd_head.data_len + cmd_head.addr_len);
        DEBUG_ARRAY((u8*)&buff[CMD_HEAD_LENGTH], cmd_head.data_len);
        
        if (1 == cmd_head.flag)
        {
            if (fail == comfirm())
            {
                WARNING("[WRITE]Comfirm fail!\n");
                return fail;
            }
        }
        else if (2 == cmd_head.flag)
        {
            //               
        }
        if (tool_i2c_write(&cmd_head.data[ADDR_MAX_LENGTH - cmd_head.addr_len],
            cmd_head.data_len + cmd_head.addr_len) <= 0)
        {
            WARNING("[WRITE]Write data failed!\n");
            return fail;
        }

        DEBUG_ARRAY(&cmd_head.data[ADDR_MAX_LENGTH - cmd_head.addr_len],cmd_head.data_len + cmd_head.addr_len);
        if (cmd_head.delay)
        {
            msleep(cmd_head.delay);
        }

        return cmd_head.data_len + CMD_HEAD_LENGTH;
    }
    else if (3 == cmd_head.wr)
    {
        memcpy(IC_TYPE, cmd_head.data, cmd_head.data_len);
        register_i2c_func();

        return cmd_head.data_len + CMD_HEAD_LENGTH;
    }
    else if (5 == cmd_head.wr)
    {
        //                                                  

        return cmd_head.data_len + CMD_HEAD_LENGTH;
    }

    return CMD_HEAD_LENGTH;
}

static s32 goodix_tool_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
    if (2 == cmd_head.wr)
    {
        //                                         
        memcpy(page, IC_TYPE, sizeof(IC_TYPE_NAME));
        page[sizeof(IC_TYPE_NAME)] = 0;

        DEBUG("Return ic type:%s len:%d\n", page, (s32)cmd_head.data_len);
        return cmd_head.data_len;
        //                            
    }
    else if (cmd_head.wr % 2)
    {
        return fail;
    }
    else if (!cmd_head.wr)
    {
        u16 len = 0;
        s16 data_len = 0;
        u16 loc = 0;
        
        if (1 == cmd_head.flag)
        {
            if (fail == comfirm())
            {
                WARNING("[READ]Comfirm fail!\n");
                return fail;
            }
        }
        else if (2 == cmd_head.flag)
        {
            //               
        }

        memcpy(cmd_head.data, cmd_head.addr, cmd_head.addr_len);

        DEBUG("[CMD HEAD DATA] ADDR:0x%02x%02x\n", cmd_head.data[0], cmd_head.data[1]);
        DEBUG("[CMD HEAD ADDR] ADDR:0x%02x%02x\n", cmd_head.addr[0], cmd_head.addr[1]);
        
        if (cmd_head.delay)
        {
            msleep(cmd_head.delay);
        }

        data_len = cmd_head.data_len;
        while(data_len > 0)
        {
            if (data_len > DATA_LENGTH)
            {
                len = DATA_LENGTH;
            }
            else
            {
                len = data_len;
            }
            data_len -= DATA_LENGTH;

            if (tool_i2c_read(cmd_head.data, len) <= 0)
            {
                WARNING("[READ]Read data failed!\n");
                return fail;
            }

            memcpy(&page[loc], &cmd_head.data[ADDR_MAX_LENGTH], len);
            loc += len;

            //                                                  
            DEBUG_ARRAY(page, len);
        }
    }

    return cmd_head.data_len;
}

#endif
