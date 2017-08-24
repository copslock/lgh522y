#ifndef _GPIO_CONST_H_
#define _GPIO_CONST_H_

//                             

#define GPIO_MODE_BITS                 4
#define MAX_GPIO_MODE_PER_REG          8
#define MAX_GPIO_REG_BITS              32

#if 0
#define GPIO_BASE                      0x10005000
#define IOCFG_L_BASE                   0x10002000
#define IOCFG_B_BASE                   0x10002400
#define IOCFG_R_BASE                   0x10002800
#define IOCFG_T_BASE                   0x10002C00
#define MIPI_TX0_BASE                  0x10215000
#define MIPI_RX_ANA_CSI0_BASE          0x10217000
#define MIPI_RX_ANA_CSI1_BASE          0x10218000
#endif

#define GPIO_BASE_1                      gpio_vbase.gpio_regs
#define IOCFG_L_BASE_1                   gpio_vbase.IOCFG_L_regs
#define IOCFG_B_BASE_1                   gpio_vbase.IOCFG_B_regs
#define IOCFG_R_BASE_1                   gpio_vbase.IOCFG_R_regs
#define IOCFG_T_BASE_1                   gpio_vbase.IOCFG_T_regs
#define MIPI_TX0_BASE_1                  gpio_vbase.MIPI_TX0_regs
#define MIPI_RX_ANA_CSI0_BASE_1          gpio_vbase.MIPI_RX_CSI0_regs
#define MIPI_RX_ANA_CSI1_BASE_1          gpio_vbase.MIPI_RX_CSI1_regs

struct mt_gpio_vbase {
	void __iomem *gpio_regs;
	void __iomem *IOCFG_L_regs;
	void __iomem *IOCFG_B_regs;
	void __iomem *IOCFG_R_regs;
	void __iomem *IOCFG_T_regs;
	void __iomem *MIPI_TX0_regs;
	void __iomem *MIPI_RX_CSI0_regs;
	void __iomem *MIPI_RX_CSI1_regs;
};

extern struct mt_gpio_vbase gpio_vbase;

/*                                                                             
                          
                                                                             */
typedef enum GPIO_PIN
{    
    GPIO_UNSUPPORTED = -1,    
    
    GPIO0  , GPIO1  , GPIO2  , GPIO3  , GPIO4  , GPIO5  , GPIO6  , GPIO7  ,
    GPIO8  , GPIO9  , GPIO10 , GPIO11 , GPIO12 , GPIO13 , GPIO14 , GPIO15 ,
    GPIO16 , GPIO17 , GPIO18 , GPIO19 , GPIO20 , GPIO21 , GPIO22 , GPIO23 ,
    GPIO24 , GPIO25 , GPIO26 , GPIO27 , GPIO28 , GPIO29 , GPIO30 , GPIO31 ,
    GPIO32 , GPIO33 , GPIO34 , GPIO35 , GPIO36 , GPIO37 , GPIO38 , GPIO39 ,
    GPIO40 , GPIO41 , GPIO42 , GPIO43 , GPIO44 , GPIO45 , GPIO46 , GPIO47 ,
    GPIO48 , GPIO49 , GPIO50 , GPIO51 , GPIO52 , GPIO53 , GPIO54 , GPIO55 ,
    GPIO56 , GPIO57 , GPIO58 , GPIO59 , GPIO60 , GPIO61 , GPIO62 , GPIO63 ,
    GPIO64 , GPIO65 , GPIO66 , GPIO67 , GPIO68 , GPIO69 , GPIO70 , GPIO71 ,
    GPIO72 , GPIO73 , GPIO74 , GPIO75 , GPIO76 , GPIO77 , GPIO78 , GPIO79 ,
    GPIO80 , GPIO81 , GPIO82 , GPIO83 , GPIO84 , GPIO85 , GPIO86 , GPIO87 ,
    GPIO88 , GPIO89 , GPIO90 , GPIO91 , GPIO92 , GPIO93 , GPIO94 , GPIO95 ,
    GPIO96 , GPIO97 , GPIO98 , GPIO99 , GPIO100, GPIO101, GPIO102, GPIO103,
    GPIO104, GPIO105, GPIO106, GPIO107, GPIO108, GPIO109, GPIO110, GPIO111,
    GPIO112, GPIO113, GPIO114, GPIO115, GPIO116, GPIO117, GPIO118, GPIO119,
    GPIO120, GPIO121, GPIO122, GPIO123, GPIO124, GPIO125, GPIO126, GPIO127,
    GPIO128, GPIO129, GPIO130, GPIO131, GPIO132, GPIO133, GPIO134, GPIO135,
    GPIO136, GPIO137, GPIO138, GPIO139, GPIO140, GPIO141, GPIO142, GPIO143,
    GPIO144, GPIO145, GPIO146, GPIO147, GPIO148, GPIO149, GPIO150, GPIO151,
    GPIO152, GPIO153, GPIO154, GPIO155, GPIO156, GPIO157, GPIO158, GPIO159,
    GPIO160, GPIO161, GPIO162, GPIO163, GPIO164, GPIO165, GPIO166, GPIO167,
    GPIO168, GPIO169, GPIO170, GPIO171, GPIO172, GPIO173, GPIO174, GPIO175,
    GPIO176, GPIO177, GPIO178, GPIO179, GPIO180, GPIO181, GPIO182, GPIO183,
    GPIO184, GPIO185, GPIO186, MT_GPIO_BASE_MAX
}GPIO_PIN;    
#define MT_GPIO_BASE_START GPIO0
#define MT_GPIO_EXT_START  MT_GPIO_BASE_MAX

typedef enum GPIO_PIN_EXT
{    
    MT_GPIO_EXT_MAX = MT_GPIO_EXT_START
}GPIO_PIN_EXT;    
#define MT_GPIO_MAX_PIN MT_GPIO_EXT_MAX

#endif //              
