/*
                                                                                    
*/

/*                   
                                         

                                                                                 
                          
*/



/*
** $Log: gl_init.c $
**
** 11 15 2012 cp.wu
** [ALPS00382763] N820_JB:[WIFI]N820JB WLAN �K???,����?�y�\�Ӥj
** do not try reconnecting when being disconnected by the peer
 *
 * 07 17 2012 yuche.tsai
 * NULL
 * Fix compile error.
 *
 * 07 17 2012 yuche.tsai
 * NULL
 * Fix compile error for JB.
 *
 * 07 17 2012 yuche.tsai
 * NULL
 * Let netdev bring up.
 *
 * 07 17 2012 yuche.tsai
 * NULL
 * Compile no error before trial run.
 *
 * 06 13 2012 yuche.tsai
 * NULL
 * Update maintrunk driver.
 * Add support for driver compose assoc request frame.
 *
 * 05 25 2012 yuche.tsai
 * NULL
 * Fix reset KE issue.
 *
 * 05 11 2012 cp.wu
 * [WCXRP00001237] [MT6620 Wi-Fi][Driver] Show MAC address and MAC address source for ACS's convenience
 * show MAC address & source while initiliazation
 *
 * 03 02 2012 terry.wu
 * NULL
 * EXPORT_SYMBOL(rsnParseCheckForWFAInfoElem);.
 *
 * 03 02 2012 terry.wu
 * NULL
 * Snc CFG80211 modification for ICS migration from branch 2.2.
 *
 * 03 02 2012 terry.wu
 * NULL
 * Sync CFG80211 modification from branch 2,2.
 *
 * 03 02 2012 terry.wu
 * NULL
 * Enable CFG80211 Support.
 *
 * 12 22 2011 george.huang
 * [WCXRP00000905] [MT6628 Wi-Fi][FW] Code refinement for ROM/ RAM module dependency
 * using global variable instead of stack for setting wlanoidSetNetworkAddress(), due to buffer may be released before TX thread handling
 *
 * 11 18 2011 yuche.tsai
 * NULL
 * CONFIG P2P support RSSI query, default turned off.
 *
 * 11 14 2011 yuche.tsai
 * [WCXRP00001107] [Volunteer Patch][Driver] Large Network Type index assert in FW issue.
 * Fix large network type index assert in FW issue.
 *
 * 11 14 2011 cm.chang
 * NULL
 * Fix compiling warning
 *
 * 11 11 2011 yuche.tsai
 * NULL
 * Fix work thread cancel issue.
 *
 * 11 10 2011 cp.wu
 * [WCXRP00001098] [MT6620 Wi-Fi][Driver] Replace printk by DBG LOG macros in linux porting layer
 * 1. eliminaite direct calls to printk in porting layer.
 * 2. replaced by DBGLOG, which would be XLOG on ALPS platforms.
 *
 * 10 06 2011 eddie.chen
 * [WCXRP00001027] [MT6628 Wi-Fi][Firmware/Driver] Tx fragmentation
 * Add rlmDomainGetChnlList symbol.
 *
 * 09 22 2011 cm.chang
 * NULL
 * Safer writng stype to avoid unitialized regitry structure
 *
 * 09 21 2011 cm.chang
 * [WCXRP00000969] [MT6620 Wi-Fi][Driver][FW] Channel list for 5G band based on country code
 * Avoid possible structure alignment problem
 *
 * 09 20 2011 chinglan.wang
 * [WCXRP00000989] [WiFi Direct] [Driver] Add a new io control API to start the formation for the sigma test.
 * .
 *
 * 09 08 2011 cm.chang
 * [WCXRP00000969] [MT6620 Wi-Fi][Driver][FW] Channel list for 5G band based on country code
 * Use new fields ucChannelListMap and ucChannelListIndex in NVRAM
 *
 * 08 31 2011 cm.chang
 * [WCXRP00000969] [MT6620 Wi-Fi][Driver][FW] Channel list for 5G band based on country code
 * .
 *
 * 08 11 2011 cp.wu
 * [WCXRP00000830] [MT6620 Wi-Fi][Firmware] Use MDRDY counter to detect empty channel for shortening scan time
 * expose scnQuerySparseChannel() for P2P-FSM.
 *
 * 08 11 2011 cp.wu
 * [WCXRP00000830] [MT6620 Wi-Fi][Firmware] Use MDRDY counter to detect empty channel for shortening scan time
 * sparse channel detection:
 * driver: collect sparse channel information with scan-done event
 *
 * 08 02 2011 yuche.tsai
 * [WCXRP00000896] [Volunteer Patch][WiFi Direct][Driver] GO with multiple client, TX deauth to a disconnecting device issue.
 * Fix GO send deauth frame issue.
 *
 * 07 07 2011 wh.su
 * [WCXRP00000839] [MT6620 Wi-Fi][Driver] Add the dumpMemory8 and dumpMemory32 EXPORT_SYMBOL
 * Add the dumpMemory8 symbol export for debug mode.
 *
 * 07 06 2011 terry.wu
 * [WCXRP00000735] [MT6620 Wi-Fi][BoW][FW/Driver] Protect BoW connection establishment
 * Improve BoW connection establishment speed.
 *
 * 07 05 2011 yuche.tsai
 * [WCXRP00000821] [Volunteer Patch][WiFi Direct][Driver] WiFi Direct Connection Speed Issue
 * Export one symbol for enhancement.
 *
 * 06 13 2011 eddie.chen
 * [WCXRP00000779] [MT6620 Wi-Fi][DRV]  Add tx rx statistics in linux and use netif_rx_ni
 * Add tx rx statistics and netif_rx_ni.
 *
 * 05 27 2011 cp.wu
 * [WCXRP00000749] [MT6620 Wi-Fi][Driver] Add band edge tx power control to Wi-Fi NVRAM
 * invoke CMD_ID_SET_EDGE_TXPWR_LIMIT when there is valid data exist in NVRAM content.
 *
 * 05 18 2011 cp.wu
 * [WCXRP00000734] [MT6620 Wi-Fi][Driver] Pass PHY_PARAM in NVRAM to firmware domain
 * pass PHY_PARAM in NVRAM from driver to firmware.
 *
 * 05 09 2011 jeffrey.chang
 * [WCXRP00000710] [MT6620 Wi-Fi] Support pattern filter update function on IP address change
 * support ARP filter through kernel notifier
 *
 * 05 03 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Use kalMemAlloc to allocate event buffer for kalIndicateBOWEvent.
 *
 * 04 27 2011 george.huang
 * [WCXRP00000684] [MT6620 Wi-Fi][Driver] Support P2P setting ARP filter
 * Support P2P ARP filter setting on early suspend/ late resume
 *
 * 04 18 2011 terry.wu
 * [WCXRP00000660] [MT6620 Wi-Fi][Driver] Remove flag CFG_WIFI_DIRECT_MOVED
 * Remove flag CFG_WIFI_DIRECT_MOVED.
 *
 * 04 15 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Add BOW short range mode.
 *
 * 04 14 2011 yuche.tsai
 * [WCXRP00000646] [Volunteer Patch][MT6620][FW/Driver] Sigma Test Modification for some test case.
 * Modify some driver connection flow or behavior to pass Sigma test more easier..
 *
 * 04 12 2011 cm.chang
 * [WCXRP00000634] [MT6620 Wi-Fi][Driver][FW] 2nd BSS will not support 40MHz bandwidth for concurrency
 * .
 *
 * 04 11 2011 george.huang
 * [WCXRP00000621] [MT6620 Wi-Fi][Driver] Support P2P supplicant to set power mode
 * export wlan functions to p2p
 *
 * 04 08 2011 pat.lu
 * [WCXRP00000623] [MT6620 Wi-Fi][Driver] use ARCH define to distinguish PC Linux driver
 * Use CONFIG_X86 instead of PC_LINUX_DRIVER_USE option to have proper compile settting for PC Linux driver
 *
 * 04 08 2011 cp.wu
 * [WCXRP00000540] [MT5931][Driver] Add eHPI8/eHPI16 support to Linux Glue Layer
 * glBusFreeIrq() should use the same pvCookie as glBusSetIrq() or request_irq()/free_irq() won't work as a pair.
 *
 * 04 08 2011 eddie.chen
 * [WCXRP00000617] [MT6620 Wi-Fi][DRV/FW] Fix for sigma
 * Fix for sigma
 *
 * 04 06 2011 cp.wu
 * [WCXRP00000540] [MT5931][Driver] Add eHPI8/eHPI16 support to Linux Glue Layer
 * 1. do not check for pvData inside wlanNetCreate() due to it is NULL for eHPI  port
 * 2. update perm_addr as well for MAC address
 * 3. not calling check_mem_region() anymore for eHPI
 * 4. correct MSC_CS macro for 0-based notation
 *
 * 03 29 2011 cp.wu
 * [WCXRP00000598] [MT6620 Wi-Fi][Driver] Implementation of interface for communicating with user space process for RESET_START and RESET_END events
 * fix typo.
 *
 * 03 29 2011 cp.wu
 * [WCXRP00000598] [MT6620 Wi-Fi][Driver] Implementation of interface for communicating with user space process for RESET_START and RESET_END events
 * implement kernel-to-userspace communication via generic netlink socket for whole-chip resetting mechanism
 *
 * 03 23 2011 cp.wu
 * [WCXRP00000540] [MT5931][Driver] Add eHPI8/eHPI16 support to Linux Glue Layer
 * apply multi-queue operation only for linux kernel > 2.6.26
 *
 * 03 22 2011 pat.lu
 * [WCXRP00000592] [MT6620 Wi-Fi][Driver] Support PC Linux Environment Driver Build
 * Add a compiler option "PC_LINUX_DRIVER_USE" for building driver in PC Linux environment.
 *
 * 03 21 2011 cp.wu
 * [WCXRP00000540] [MT5931][Driver] Add eHPI8/eHPI16 support to Linux Glue Layer
 * portability for compatible with linux 2.6.12.
 *
 * 03 21 2011 cp.wu
 * [WCXRP00000540] [MT5931][Driver] Add eHPI8/eHPI16 support to Linux Glue Layer
 * improve portability for awareness of early version of linux kernel and wireless extension.
 *
 * 03 21 2011 cp.wu
 * [WCXRP00000540] [MT5931][Driver] Add eHPI8/eHPI16 support to Linux Glue Layer
 * portability improvement
 *
 * 03 18 2011 jeffrey.chang
 * [WCXRP00000512] [MT6620 Wi-Fi][Driver] modify the net device relative functions to support the H/W multiple queue
 * remove early suspend functions
 *
 * 03 17 2011 cp.wu
 * [WCXRP00000562] [MT6620 Wi-Fi][Driver] I/O buffer pre-allocation to avoid physically continuous memory shortage after system running for a long period
 * reverse order to prevent probing racing.
 *
 * 03 16 2011 cp.wu
 * [WCXRP00000562] [MT6620 Wi-Fi][Driver] I/O buffer pre-allocation to avoid physically continuous memory shortage after system running for a long period
 * 1. pre-allocate physical continuous buffer while module is being loaded
 * 2. use pre-allocated physical continuous buffer for TX/RX DMA transfer
 *
 * The windows part remained the same as before, but added similiar APIs to hide the difference.
 *
 * 03 15 2011 jeffrey.chang
 * [WCXRP00000558] [MT6620 Wi-Fi][MT6620 Wi-Fi][Driver] refine the queue selection algorithm for WMM
 * refine the queue_select function
 *
 * 03 10 2011 cp.wu
 * [WCXRP00000532] [MT6620 Wi-Fi][Driver] Migrate NVRAM configuration procedures from MT6620 E2 to MT6620 E3
 * deprecate configuration used by MT6620 E2
 *
 * 03 10 2011 terry.wu
 * [WCXRP00000505] [MT6620 Wi-Fi][Driver/FW] WiFi Direct Integration
 * Remove unnecessary assert and message.
 *
 * 03 08 2011 terry.wu
 * [WCXRP00000505] [MT6620 Wi-Fi][Driver/FW] WiFi Direct Integration
 * Export nicQmUpdateWmmParms.
 *
 * 03 03 2011 jeffrey.chang
 * [WCXRP00000512] [MT6620 Wi-Fi][Driver] modify the net device relative functions to support the H/W multiple queue
 * support concurrent network
 *
 * 03 03 2011 jeffrey.chang
 * [WCXRP00000512] [MT6620 Wi-Fi][Driver] modify the net device relative functions to support the H/W multiple queue
 * modify net device relative functions to support multiple H/W queues
 *
 * 02 24 2011 george.huang
 * [WCXRP00000495] [MT6620 Wi-Fi][FW] Support pattern filter for unwanted ARP frames
 * Support ARP filter during suspended
 *
 * 02 21 2011 cp.wu
 * [WCXRP00000482] [MT6620 Wi-Fi][Driver] Simplify logic for checking NVRAM existence in driver domain
 * simplify logic for checking NVRAM existence only once.
 *
 * 02 17 2011 terry.wu
 * [WCXRP00000459] [MT6620 Wi-Fi][Driver] Fix deference null pointer problem in wlanRemove
 * Fix deference a null pointer problem in wlanRemove.
 *
 * 02 16 2011 jeffrey.chang
 * NULL
 * fix compilig error
 *
 * 02 16 2011 jeffrey.chang
 * NULL
 * Add query ipv4 and ipv6 address during early suspend and late resume
 *
 * 02 15 2011 jeffrey.chang
 * NULL
 * to support early suspend in android
 *
 * 02 11 2011 yuche.tsai
 * [WCXRP00000431] [Volunteer Patch][MT6620][Driver] Add MLME support for deauthentication under AP(Hot-Spot) mode.
 * Add one more export symbol.
 *
 * 02 10 2011 yuche.tsai
 * [WCXRP00000431] [Volunteer Patch][MT6620][Driver] Add MLME support for deauthentication under AP(Hot-Spot) mode.
 * Add RX deauthentication & disassociation process under Hot-Spot mode.
 *
 * 02 09 2011 terry.wu
 * [WCXRP00000383] [MT6620 Wi-Fi][Driver] Separate WiFi and P2P driver into two modules
 * Halt p2p module init and exit until TxThread finished p2p register and unregister.
 *
 * 02 08 2011 george.huang
 * [WCXRP00000422] [MT6620 Wi-Fi][Driver] support query power mode OID handler
 * Support querying power mode OID.
 *
 * 02 08 2011 yuche.tsai
 * [WCXRP00000421] [Volunteer Patch][MT6620][Driver] Fix incorrect SSID length Issue
 * Export Deactivation Network.
 *
 * 02 01 2011 jeffrey.chang
 * [WCXRP00000414] KAL Timer is not unregistered when driver not loaded
 * Unregister the KAL timer during driver unloading
 *
 * 01 26 2011 cm.chang
 * [WCXRP00000395] [MT6620 Wi-Fi][Driver][FW] Search STA_REC with additional net type index argument
 * Allocate system RAM if fixed message or mgmt buffer is not available
 *
 * 01 19 2011 cp.wu
 * [WCXRP00000371] [MT6620 Wi-Fi][Driver] make linux glue layer portable for Android 2.3.1 with Linux 2.6.35.7
 * add compile option to check linux version 2.6.35 for different usage of system API to improve portability
 *
 * 01 12 2011 cp.wu
 * [WCXRP00000357] [MT6620 Wi-Fi][Driver][Bluetooth over Wi-Fi] add another net device interface for BT AMP
 * implementation of separate BT_OVER_WIFI data path.
 *
 * 01 10 2011 cp.wu
 * [WCXRP00000349] [MT6620 Wi-Fi][Driver] make kalIoctl() of linux port as a thread safe API to avoid potential issues due to multiple access
 * use mutex to protect kalIoctl() for thread safe.
 *
 * 01 04 2011 cp.wu
 * [WCXRP00000338] [MT6620 Wi-Fi][Driver] Separate kalMemAlloc into kmalloc and vmalloc implementations to ease physically continous memory demands
 * separate kalMemAlloc() into virtually-continous and physically-continous type to ease slab system pressure
 *
 * 12 15 2010 cp.wu
 * [WCXRP00000265] [MT6620 Wi-Fi][Driver] Remove set_mac_address routine from legacy Wi-Fi Android driver
 * remove set MAC address. MAC address is always loaded from NVRAM instead.
 *
 * 12 10 2010 kevin.huang
 * [WCXRP00000128] [MT6620 Wi-Fi][Driver] Add proc support to Android Driver for debug and driver status check
 * Add Linux Proc Support
 *
 * 11 01 2010 yarco.yang
 * [WCXRP00000149] [MT6620 WI-Fi][Driver]Fine tune performance on MT6516 platform
 * Add GPIO debug function
 *
 * 11 01 2010 cp.wu
 * [WCXRP00000056] [MT6620 Wi-Fi][Driver] NVRAM implementation with Version Check[WCXRP00000150] [MT6620 Wi-Fi][Driver] Add implementation for querying current TX rate from firmware auto rate module
 * 1) Query link speed (TX rate) from firmware directly with buffering mechanism to reduce overhead
 * 2) Remove CNM CH-RECOVER event handling
 * 3) cfg read/write API renamed with kal prefix for unified naming rules.
 *
 * 10 26 2010 cp.wu
 * [WCXRP00000056] [MT6620 Wi-Fi][Driver] NVRAM implementation with Version Check[WCXRP00000137] [MT6620 Wi-Fi] [FW] Support NIC capability query command
 * 1) update NVRAM content template to ver 1.02
 * 2) add compile option for querying NIC capability (default: off)
 * 3) modify AIS 5GHz support to run-time option, which could be turned on by registry or NVRAM setting
 * 4) correct auto-rate compiler error under linux (treat warning as error)
 * 5) simplify usage of NVRAM and REG_INFO_T
 * 6) add version checking between driver and firmware
 *
 * 10 21 2010 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * .
 *
 * 10 19 2010 jeffrey.chang
 * [WCXRP00000120] [MT6620 Wi-Fi][Driver] Refine linux kernel module to the license of MTK propietary and enable MTK HIF by default
 * Refine linux kernel module to the license of MTK and enable MTK HIF
 *
 * 10 18 2010 jeffrey.chang
 * [WCXRP00000106] [MT6620 Wi-Fi][Driver] Enable setting multicast  callback in Android
 * .
 *
 * 10 18 2010 cp.wu
 * [WCXRP00000056] [MT6620 Wi-Fi][Driver] NVRAM implementation with Version Check[WCXRP00000086] [MT6620 Wi-Fi][Driver] The mac address is all zero at android
 * complete implementation of Android NVRAM access
 *
 * 09 27 2010 chinghwa.yu
 * [WCXRP00000063] Update BCM CoEx design and settings[WCXRP00000065] Update BoW design and settings
 * Update BCM/BoW design and settings.
 *
 * 09 23 2010 cp.wu
 * [WCXRP00000051] [MT6620 Wi-Fi][Driver] WHQL test fail in MAC address changed item
 * use firmware reported mac address right after wlanAdapterStart() as permanent address
 *
 * 09 21 2010 kevin.huang
 * [WCXRP00000052] [MT6620 Wi-Fi][Driver] Eliminate Linux Compile Warning
 * Eliminate Linux Compile Warning
 *
 * 09 03 2010 kevin.huang
 * NULL
 * Refine #include sequence and solve recursive/nested #include issue
 *
 * 09 01 2010 wh.su
 * NULL
 * adding the wapi support for integration test.
 *
 * 08 18 2010 yarco.yang
 * NULL
 * 1. Fixed HW checksum offload function not work under Linux issue.
 * 2. Add debug message.
 *
 * 08 16 2010 yarco.yang
 * NULL
 * Support Linux x86
 *
 * 08 02 2010 jeffrey.chang
 * NULL
 * 1) modify tx service thread to avoid busy looping
 * 2) add spin lock declartion for linux build
 *
 * 07 29 2010 jeffrey.chang
 * NULL
 * fix memory leak for module unloading
 *
 * 07 28 2010 jeffrey.chang
 * NULL
 * 1) remove unused spinlocks
 * 2) enable encyption ioctls
 * 3) fix scan ioctl which may cause supplicant to hang
 *
 * 07 23 2010 jeffrey.chang
 *
 * bug fix: allocate regInfo when disabling firmware download
 *
 * 07 23 2010 jeffrey.chang
 *
 * use glue layer api to decrease or increase counter atomically
 *
 * 07 22 2010 jeffrey.chang
 *
 * add new spinlock
 *
 * 07 19 2010 jeffrey.chang
 *
 * modify cmd/data path for new design
 *
 * 07 08 2010 cp.wu
 *
 * [WPD00003833] [MT6620 and MT5931] Driver migration - move to new repository.
 *
 * 06 06 2010 kevin.huang
 * [WPD00003832][MT6620 5931] Create driver base
 * [MT6620 5931] Create driver base
 *
 * 05 26 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * 1) Modify set mac address code
 * 2) remove power managment macro
 *
 * 05 10 2010 cp.wu
 * [WPD00003831][MT6620 Wi-Fi] Add framework for Wi-Fi Direct support
 * implement basic wi-fi direct framework
 *
 * 05 07 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * prevent supplicant accessing driver during resume
 *
 * 05 07 2010 cp.wu
 * [WPD00003831][MT6620 Wi-Fi] Add framework for Wi-Fi Direct support
 * add basic framework for implementating P2P driver hook.
 *
 * 04 27 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * 1) fix firmware download bug
 * 2) remove query statistics for acelerating firmware download
 *
 * 04 27 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * follow Linux's firmware framework, and remove unused kal API
 *
 * 04 21 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * add for private ioctl support
 *
 * 04 19 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * Query statistics from firmware
 *
 * 04 19 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * modify tcp/ip checksum offload flags
 *
 * 04 16 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * fix tcp/ip checksum offload bug
 *
 * 04 13 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * add framework for BT-over-Wi-Fi support.
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * 1) prPendingCmdInfo is replaced by queue for multiple handler capability
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * 2) command sequence number is now increased atomically
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * 3) private data could be hold and taken use for other purpose
 *
 * 04 09 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * fix spinlock usage
 *
 * 04 07 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * Set MAC address from firmware
 *
 * 04 07 2010 cp.wu
 * [WPD00001943]Create WiFi test driver framework on WinXP
 * rWlanInfo should be placed at adapter rather than glue due to most operations
 *  *  *  *  *  * are done in adapter layer.
 *
 * 04 07 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * (1)improve none-glue code portability
 *  * (2) disable set Multicast address during atomic context
 *
 * 04 06 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * adding debug module
 *
 * 03 31 2010 wh.su
 * [WPD00003816][MT6620 Wi-Fi] Adding the security support
 * modify the wapi related code for new driver's design.
 *
 * 03 30 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * emulate NDIS Pending OID facility
 *
 * 03 26 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * fix f/w download start and load address by using config.h
 *
 * 03 26 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * [WPD00003826] Initial import for Linux port
 * adding firmware download support
 *
 * 03 24 2010 jeffrey.chang
 * [WPD00003826]Initial import for Linux port
 * initial import for Linux port
**  \main\maintrunk.MT5921\52 2009-10-27 22:49:59 GMT mtk01090
**  Fix compile error for Linux EHPI driver
**  \main\maintrunk.MT5921\51 2009-10-20 17:38:22 GMT mtk01090
**  Refine driver unloading and clean up procedure. Block requests, stop main thread and clean up queued requests, and then stop hw.
**  \main\maintrunk.MT5921\50 2009-10-08 10:33:11 GMT mtk01090
**  Avoid accessing private data of net_device directly. Replace with netdev_priv(). Add more checking for input parameters and pointers.
**  \main\maintrunk.MT5921\49 2009-09-28 20:19:05 GMT mtk01090
**  Add private ioctl to carry OID structures. Restructure public/private ioctl interfaces to Linux kernel.
**  \main\maintrunk.MT5921\48 2009-09-03 13:58:46 GMT mtk01088
**  remove non-used code
**  \main\maintrunk.MT5921\47 2009-09-03 11:40:25 GMT mtk01088
**  adding the module parameter for wapi
**  \main\maintrunk.MT5921\46 2009-08-18 22:56:41 GMT mtk01090
**  Add Linux SDIO (with mmc core) support.
**  Add Linux 2.6.21, 2.6.25, 2.6.26.
**  Fix compile warning in Linux.
**  \main\maintrunk.MT5921\45 2009-07-06 20:53:00 GMT mtk01088
**  adding the code to check the wapi 1x frame
**  \main\maintrunk.MT5921\44 2009-06-23 23:18:55 GMT mtk01090
**  Add build option BUILD_USE_EEPROM and compile option CFG_SUPPORT_EXT_CONFIG for NVRAM support
**  \main\maintrunk.MT5921\43 2009-02-16 23:46:51 GMT mtk01461
**  Revise the order of increasing u4TxPendingFrameNum because of  CFG_TX_RET_TX_CTRL_EARLY
**  \main\maintrunk.MT5921\42 2009-01-22 13:11:59 GMT mtk01088
**  set the tid and 1x value at same packet reserved field
**  \main\maintrunk.MT5921\41 2008-10-20 22:43:53 GMT mtk01104
**  Fix wrong variable name "prDev" in wlanStop()
**  \main\maintrunk.MT5921\40 2008-10-16 15:37:10 GMT mtk01461
**  add handle WLAN_STATUS_SUCCESS in wlanHardStartXmit() for CFG_TX_RET_TX_CTRL_EARLY
**  \main\maintrunk.MT5921\39 2008-09-25 15:56:21 GMT mtk01461
**  Update driver for Code review
**  \main\maintrunk.MT5921\38 2008-09-05 17:25:07 GMT mtk01461
**  Update Driver for Code Review
**  \main\maintrunk.MT5921\37 2008-09-02 10:57:06 GMT mtk01461
**  Update driver for code review
**  \main\maintrunk.MT5921\36 2008-08-05 01:53:28 GMT mtk01461
**  Add support for linux statistics
**  \main\maintrunk.MT5921\35 2008-08-04 16:52:58 GMT mtk01461
**  Fix ASSERT if removing module in BG_SSID_SCAN state
**  \main\maintrunk.MT5921\34 2008-06-13 22:52:24 GMT mtk01461
**  Revise status code handling in wlanHardStartXmit() for WLAN_STATUS_SUCCESS
**  \main\maintrunk.MT5921\33 2008-05-30 18:56:53 GMT mtk01461
**  Not use wlanoidSetCurrentAddrForLinux()
**  \main\maintrunk.MT5921\32 2008-05-30 14:39:40 GMT mtk01461
**  Remove WMM Assoc Flag
**  \main\maintrunk.MT5921\31 2008-05-23 10:26:40 GMT mtk01084
**  modify wlanISR interface
**  \main\maintrunk.MT5921\30 2008-05-03 18:52:36 GMT mtk01461
**  Fix Unset Broadcast filter when setMulticast
**  \main\maintrunk.MT5921\29 2008-05-03 15:17:26 GMT mtk01461
**  Move Query Media Status to GLUE
**  \main\maintrunk.MT5921\28 2008-04-24 22:48:21 GMT mtk01461
**  Revise set multicast function by using windows oid style for LP own back
**  \main\maintrunk.MT5921\27 2008-04-24 12:00:08 GMT mtk01461
**  Fix multicast setting in Linux and add comment
**  \main\maintrunk.MT5921\26 2008-03-28 10:40:22 GMT mtk01461
**  Fix set mac address func in Linux
**  \main\maintrunk.MT5921\25 2008-03-26 15:37:26 GMT mtk01461
**  Add set MAC Address
**  \main\maintrunk.MT5921\24 2008-03-26 14:24:53 GMT mtk01461
**  For Linux, set net_device has feature with checksum offload by default
**  \main\maintrunk.MT5921\23 2008-03-11 14:50:52 GMT mtk01461
**  Fix typo
**  \main\maintrunk.MT5921\22 2008-02-29 15:35:20 GMT mtk01088
**  add 1x decide code for sw port control
**  \main\maintrunk.MT5921\21 2008-02-21 15:01:54 GMT mtk01461
**  Rearrange the set off place of GLUE spin lock in HardStartXmit
**  \main\maintrunk.MT5921\20 2008-02-12 23:26:50 GMT mtk01461
**  Add debug option - Packet Order for Linux and add debug level - Event
**  \main\maintrunk.MT5921\19 2007-12-11 00:11:12 GMT mtk01461
**  Fix SPIN_LOCK protection
**  \main\maintrunk.MT5921\18 2007-11-30 17:02:25 GMT mtk01425
**  1. Set Rx multicast packets mode before setting the address list
**  \main\maintrunk.MT5921\17 2007-11-26 19:44:24 GMT mtk01461
**  Add OS_TIMESTAMP to packet
**  \main\maintrunk.MT5921\16 2007-11-21 15:47:20 GMT mtk01088
**  fixed the unload module issue
**  \main\maintrunk.MT5921\15 2007-11-07 18:37:38 GMT mtk01461
**  Fix compile warnning
**  \main\maintrunk.MT5921\14 2007-11-02 01:03:19 GMT mtk01461
**  Unify TX Path for Normal and IBSS Power Save + IBSS neighbor learning
**  \main\maintrunk.MT5921\13 2007-10-30 10:42:33 GMT mtk01425
**  1. Refine for multicast list
**  \main\maintrunk.MT5921\12 2007-10-25 18:08:13 GMT mtk01461
**  Add VOIP SCAN Support  & Refine Roaming
** Revision 1.4  2007/07/05 07:25:33  MTK01461
** Add Linux initial code, modify doc, add 11BB, RF init code
**
** Revision 1.3  2007/06/27 02:18:50  MTK01461
** Update SCAN_FSM, Initial(Can Load Module), Proc(Can do Reg R/W), TX API
**
** Revision 1.2  2007/06/25 06:16:24  MTK01461
** Update illustrations, gl_init.c, gl_kal.c, gl_kal.h, gl_os.h and RX API
**
*/

/*                                                                              
                                                     
                                                                                
*/

/*                                                                              
                                                          
                                                                                
*/
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "gl_cfg80211.h"
#include "precomp.h"
#if defined(MTK_TC1_FEATURE_TEMP)
#include <tc1_partition.h>	/*       */
#endif
/*                                                                              
                                                
                                                                                
*/
/*                            */


#define FIX_ALPS00409409406
#undef TEST_FOR_NET_RCU_LOCK


#ifdef FIX_ALPS00409409406
atomic_t fgIsUnderEarlierSuspend = {.counter = 0 };
#else
BOOLEAN fgIsUnderEarlierSuspend = false;
#endif

struct semaphore g_halt_sem;
int g_u4HaltFlag = 0;

#if defined(MTK_PACKET_FILTERING_SUPPORT)
UINT_8 g_packet_switch = 0;
#endif

/*                                                                              
                                                 
                                                                                
*/
/*                                                                
                                                                    
                                                                               
 */
typedef struct _WLANDEV_INFO_T {
	struct net_device *prDev;
} WLANDEV_INFO_T, *P_WLANDEV_INFO_T;

/*                                                                              
                                                  
                                                                                
*/

MODULE_AUTHOR(NIC_AUTHOR);
MODULE_DESCRIPTION(NIC_DESC);
MODULE_SUPPORTED_DEVICE(NIC_NAME);

#if 0
MODULE_LICENSE("MTK Propietary");
#else
MODULE_LICENSE("GPL");
#endif

#define NIC_INF_NAME    "wlan%d"	/*                */

/*                                                 */
UINT_8 aucDebugModule[DBG_MODULE_NUM];
UINT_32 u4DebugModule = 0;


/*                                                                                         */
static WLANDEV_INFO_T arWlanDevInfo[CFG_MAX_WLAN_DEVICES] = { {0} };

static UINT_32 u4WlanDevNum;	/*                           */

/*                                                                              
                                                   
                                                                                
*/
#if CFG_ENABLE_WIFI_DIRECT
static SUB_MODULE_HANDLER rSubModHandler[SUB_MODULE_NUM] = { {NULL} };
#endif

#define CHAN2G(_channel, _freq, _flags)         \
    {                                           \
    .band               = IEEE80211_BAND_2GHZ,  \
    .center_freq        = (_freq),              \
    .hw_value           = (_channel),           \
    .flags              = (_flags),             \
    .max_antenna_gain   = 0,                    \
    .max_power          = 30,                   \
    }
static struct ieee80211_channel mtk_2ghz_channels[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

#define CHAN5G(_channel, _flags)                    \
    {                                               \
    .band               = IEEE80211_BAND_5GHZ,      \
    .center_freq        = 5000 + (5 * (_channel)),  \
    .hw_value           = (_channel),               \
    .flags              = (_flags),                 \
    .max_antenna_gain   = 0,                        \
    .max_power          = 30,                       \
    }
static struct ieee80211_channel mtk_5ghz_channels[] = {
	CHAN5G(34, 0), CHAN5G(36, 0),
	CHAN5G(38, 0), CHAN5G(40, 0),
	CHAN5G(42, 0), CHAN5G(44, 0),
	CHAN5G(46, 0), CHAN5G(48, 0),
	CHAN5G(52, 0), CHAN5G(56, 0),
	CHAN5G(60, 0), CHAN5G(64, 0),
	CHAN5G(100, 0), CHAN5G(104, 0),
	CHAN5G(108, 0), CHAN5G(112, 0),
	CHAN5G(116, 0), CHAN5G(120, 0),
	CHAN5G(124, 0), CHAN5G(128, 0),
	CHAN5G(132, 0), CHAN5G(136, 0),
	CHAN5G(140, 0), CHAN5G(149, 0),
	CHAN5G(153, 0), CHAN5G(157, 0),
	CHAN5G(161, 0), CHAN5G(165, 0),
	CHAN5G(169, 0), CHAN5G(173, 0),
	CHAN5G(184, 0), CHAN5G(188, 0),
	CHAN5G(192, 0), CHAN5G(196, 0),
	CHAN5G(200, 0), CHAN5G(204, 0),
	CHAN5G(208, 0), CHAN5G(212, 0),
	CHAN5G(216, 0),
};

/*                           */
static struct ieee80211_rate mtk_rates[] = {
	RATETAB_ENT(10, 0x1000, 0),
	RATETAB_ENT(20, 0x1001, 0),
	RATETAB_ENT(55, 0x1002, 0),
	RATETAB_ENT(110, 0x1003, 0),	/*         */
	RATETAB_ENT(60, 0x2000, 0),
	RATETAB_ENT(90, 0x2001, 0),
	RATETAB_ENT(120, 0x2002, 0),
	RATETAB_ENT(180, 0x2003, 0),
	RATETAB_ENT(240, 0x2004, 0),
	RATETAB_ENT(360, 0x2005, 0),
	RATETAB_ENT(480, 0x2006, 0),
	RATETAB_ENT(540, 0x2007, 0),	/*           */
};

#define mtk_a_rates         (mtk_rates + 4)
#define mtk_a_rates_size    (sizeof(mtk_rates) / sizeof(mtk_rates[0]) - 4)
#define mtk_g_rates         (mtk_rates + 0)
#define mtk_g_rates_size    (sizeof(mtk_rates) / sizeof(mtk_rates[0]) - 0)

#define MT6620_MCS_INFO                                 \
{                                                       \
    .rx_mask        = {0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0},\
    .rx_highest     = 0,                                \
    .tx_params      = IEEE80211_HT_MCS_TX_DEFINED,      \
}

#define MT6620_HT_CAP                                   \
{                                                       \
    .ht_supported   = true,                             \
    .cap            = IEEE80211_HT_CAP_SUP_WIDTH_20_40  \
		    | IEEE80211_HT_CAP_SM_PS            \
		    | IEEE80211_HT_CAP_GRN_FLD          \
		    | IEEE80211_HT_CAP_SGI_20           \
		    | IEEE80211_HT_CAP_SGI_40,          \
    .ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,       \
    .ampdu_density  = IEEE80211_HT_MPDU_DENSITY_NONE,   \
    .mcs            = MT6620_MCS_INFO,                  \
}

/*                                           */
struct ieee80211_supported_band mtk_band_2ghz = {
	.band = IEEE80211_BAND_2GHZ,
	.channels = mtk_2ghz_channels,
	.n_channels = ARRAY_SIZE(mtk_2ghz_channels),
	.bitrates = mtk_g_rates,
	.n_bitrates = mtk_g_rates_size,
	.ht_cap = MT6620_HT_CAP,
};

/*                                           */
struct ieee80211_supported_band mtk_band_5ghz = {
	.band = IEEE80211_BAND_5GHZ,
	.channels = mtk_5ghz_channels,
	.n_channels = ARRAY_SIZE(mtk_5ghz_channels),
	.bitrates = mtk_a_rates,
	.n_bitrates = mtk_a_rates_size,
	.ht_cap = MT6620_HT_CAP,
};

static const UINT_32 mtk_cipher_suites[] = {
	/*                                         */
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,

	/*                                   */
	WLAN_CIPHER_SUITE_AES_CMAC
};

static struct cfg80211_ops mtk_wlan_ops = {
	.change_virtual_intf = mtk_cfg80211_change_iface,
	.add_key = mtk_cfg80211_add_key,
	.get_key = mtk_cfg80211_get_key,
	.del_key = mtk_cfg80211_del_key,
	.set_default_key = mtk_cfg80211_set_default_key,
	.get_station = mtk_cfg80211_get_station,
	.scan = mtk_cfg80211_scan,
	.connect = mtk_cfg80211_connect,
	.disconnect = mtk_cfg80211_disconnect,
	.join_ibss = mtk_cfg80211_join_ibss,
	.leave_ibss = mtk_cfg80211_leave_ibss,
	.set_power_mgmt = mtk_cfg80211_set_power_mgmt,
	.set_pmksa = mtk_cfg80211_set_pmksa,
	.del_pmksa = mtk_cfg80211_del_pmksa,
	.flush_pmksa = mtk_cfg80211_flush_pmksa,

	/*                    */
	.remain_on_channel = mtk_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = mtk_cfg80211_cancel_remain_on_channel,
	.mgmt_tx = mtk_cfg80211_mgmt_tx,
	.mgmt_tx_cancel_wait = mtk_cfg80211_mgmt_tx_cancel_wait,
#ifdef CONFIG_NL80211_TESTMODE
	.testmode_cmd = mtk_cfg80211_testmode_cmd,
#endif
};

/*                                                                              
                                             
                                                                                
*/

/*                                                                              
                                                             
                                                                                
*/

#if defined(CONFIG_HAS_EARLYSUSPEND)
extern int glRegisterEarlySuspend(struct early_suspend *prDesc,
				  early_suspend_callback wlanSuspend,
				  late_resume_callback wlanResume);

extern int glUnregisterEarlySuspend(struct early_suspend *prDesc);
#endif

/*                                                                              
                                                
                                                                                
*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
/*                                                                            */
/* 
                                                    
 
                                             
                                           
 
                
*/
/*                                                                            */
unsigned int _cfg80211_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp = 0;

	/*                                                    
                                                          
                                                          
        
  */

	if (skb->priority >= 256 && skb->priority <= 263) {
		return skb->priority - 256;
	}
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	}
	return dscp >> 5;
}


static const UINT_16 au16Wlan1dToQueueIdx[8] = { 1, 0, 0, 1, 2, 2, 3, 3 };

static UINT_16 wlanSelectQueue(struct net_device *dev, struct sk_buff *skb)
{
	skb->priority = _cfg80211_classify8021d(skb);

	return au16Wlan1dToQueueIdx[skb->priority];
}
#endif


/*                                                                            */
/* 
                                                         
 
                                                      
                                                     
 
                
*/
/*                                                                            */
static void glLoadNvram(IN P_GLUE_INFO_T prGlueInfo, OUT P_REG_INFO_T prRegInfo)
{
	UINT_32 i, j;
	UINT_8 aucTmp[2];
	PUINT_8 pucDest;

	ASSERT(prGlueInfo);
	ASSERT(prRegInfo);

	if ((!prGlueInfo) || (!prRegInfo)) {
		return;
	}

	if (kalCfgDataRead16(prGlueInfo,
			     sizeof(WIFI_CFG_PARAM_STRUCT) - sizeof(UINT_16),
			     (PUINT_16) aucTmp) == TRUE) {
		prGlueInfo->fgNvramAvailable = TRUE;

		/*                  */
#if !defined(MTK_TC1_FEATURE_TEMP)
		for (i = 0; i < sizeof(PARAM_MAC_ADDR_LEN); i += sizeof(UINT_16)) {
			kalCfgDataRead16(prGlueInfo,
					 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, aucMacAddress) + i,
					 (PUINT_16) (((PUINT_8) prRegInfo->aucMacAddr) + i));
		}
#else
		TC1_FAC_NAME(FacReadWifiMacAddr) ((unsigned char *)prRegInfo->aucMacAddr);
#endif

		/*                   */
		kalCfgDataRead16(prGlueInfo,
				 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, aucCountryCode[0]),
				 (PUINT_16) aucTmp);

		/*                         */
		prRegInfo->au2CountryCode[0] = (UINT_16) aucTmp[0];
		prRegInfo->au2CountryCode[1] = (UINT_16) aucTmp[1];

		/*                              */
		for (i = 0; i < sizeof(TX_PWR_PARAM_T); i += sizeof(UINT_16)) {
			kalCfgDataRead16(prGlueInfo,
					 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, rTxPwr) + i,
					 (PUINT_16) (((PUINT_8) &(prRegInfo->rTxPwr)) + i));
		}

		/*                    */
		kalCfgDataRead16(prGlueInfo,
				 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, ucTxPwrValid), (PUINT_16) aucTmp);
		prRegInfo->ucTxPwrValid = aucTmp[0];
		prRegInfo->ucSupport5GBand = aucTmp[1];

		kalCfgDataRead16(prGlueInfo,
				 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, uc2G4BwFixed20M),
				 (PUINT_16) aucTmp);
		prRegInfo->uc2G4BwFixed20M = aucTmp[0];
		prRegInfo->uc5GBwFixed20M = aucTmp[1];

		kalCfgDataRead16(prGlueInfo,
				 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, ucEnable5GBand),
				 (PUINT_16) aucTmp);
		prRegInfo->ucEnable5GBand = aucTmp[0];

		/*                            */
		for (i = 0; i < sizeof(prRegInfo->aucEFUSE); i += sizeof(UINT_16)) {
			kalCfgDataRead16(prGlueInfo,
					 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, aucEFUSE) + i,
					 (PUINT_16) (((PUINT_8) &(prRegInfo->aucEFUSE)) + i));
		}

		/*                                 */
		kalCfgDataRead16(prGlueInfo,
				 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, fg2G4BandEdgePwrUsed),
				 (PUINT_16) aucTmp);
		prRegInfo->fg2G4BandEdgePwrUsed = (BOOLEAN) aucTmp[0];
		if (aucTmp[0]) {
			prRegInfo->cBandEdgeMaxPwrCCK = (INT_8) aucTmp[1];

			kalCfgDataRead16(prGlueInfo,
					 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, cBandEdgeMaxPwrOFDM20),
					 (PUINT_16) aucTmp);
			prRegInfo->cBandEdgeMaxPwrOFDM20 = (INT_8) aucTmp[0];
			prRegInfo->cBandEdgeMaxPwrOFDM40 = (INT_8) aucTmp[1];
		}

		/*                          */
		kalCfgDataRead16(prGlueInfo,
				 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, ucRegChannelListMap),
				 (PUINT_16) aucTmp);
		prRegInfo->eRegChannelListMap = (ENUM_REG_CH_MAP_T) aucTmp[0];
		prRegInfo->ucRegChannelListIndex = aucTmp[1];

		if (prRegInfo->eRegChannelListMap == REG_CH_MAP_CUSTOMIZED) {
			for (i = 0; i < MAX_SUBBAND_NUM; i++) {
				pucDest = (PUINT_8) &prRegInfo->rDomainInfo.rSubBand[i];
				for (j = 0; j < 6; j += sizeof(UINT_16)) {
					kalCfgDataRead16(prGlueInfo,
							 OFFSET_OF(WIFI_CFG_PARAM_STRUCT,
								   aucRegSubbandInfo)
							 + (i * 6 + j), (PUINT_16) aucTmp);

					*pucDest++ = aucTmp[0];
					*pucDest++ = aucTmp[1];
				}
			}
		}
	} else {
		prGlueInfo->fgNvramAvailable = FALSE;
	}

	return;
}


#if CFG_ENABLE_WIFI_DIRECT
/*                                                                            */
/* 
                                                         
 
                                                      
 
                
*/
/*                                                                            */
VOID wlanSubModRunInit(P_GLUE_INFO_T prGlueInfo)
{
	/*                             */
	if (rSubModHandler[P2P_MODULE].fgIsInited == FALSE) {
		rSubModHandler[P2P_MODULE].subModInit(prGlueInfo);
		rSubModHandler[P2P_MODULE].fgIsInited = TRUE;
	}

}

/*                                                                            */
/* 
                                                         
 
                                                      
 
                
*/
/*                                                                            */
VOID wlanSubModRunExit(P_GLUE_INFO_T prGlueInfo)
{
	/*                             */
	if (rSubModHandler[P2P_MODULE].fgIsInited == TRUE) {
		rSubModHandler[P2P_MODULE].subModExit(prGlueInfo);
		rSubModHandler[P2P_MODULE].fgIsInited = FALSE;
	}
}

/*                                                                            */
/* 
                                                                       
 
                                                      
 
                
*/
/*                                                                            */
BOOLEAN wlanSubModInit(P_GLUE_INFO_T prGlueInfo)
{
	/*                                                        */
	prGlueInfo->u4Flag |= GLUE_FLAG_SUB_MOD_INIT;
	/*                     */
	wake_up_interruptible(&prGlueInfo->waitq);
	/*                                          */
	wait_for_completion_interruptible(&prGlueInfo->rSubModComp);

#if 0
	if (prGlueInfo->prAdapter->fgIsP2PRegistered) {
		p2pNetRegister(prGlueInfo);
	}
#endif

	return TRUE;
}

/*                                                                            */
/* 
                                                                       
 
                                                      
 
                
*/
/*                                                                            */
BOOLEAN wlanSubModExit(P_GLUE_INFO_T prGlueInfo)
{
#if 0
	if (prGlueInfo->prAdapter->fgIsP2PRegistered) {
		p2pNetUnregister(prGlueInfo);
	}
#endif

	/*                                                        */
	prGlueInfo->u4Flag |= GLUE_FLAG_SUB_MOD_EXIT;
	/*                     */
	wake_up_interruptible(&prGlueInfo->waitq);
	/*                                         */
	wait_for_completion_interruptible(&prGlueInfo->rSubModComp);

	return TRUE;
}


/*                                                                            */
/* 
                                                                   
 
                                                                             
                                                                              
                                           
 
                
*/
/*                                                                            */
VOID
wlanSubModRegisterInitExit(SUB_MODULE_INIT rSubModInit,
			   SUB_MODULE_EXIT rSubModExit, ENUM_SUB_MODULE_IDX_T eSubModIdx)
{
	rSubModHandler[eSubModIdx].subModInit = rSubModInit;
	rSubModHandler[eSubModIdx].subModExit = rSubModExit;
	rSubModHandler[eSubModIdx].fgIsInited = FALSE;
}

#if 0
/*                                                                            */
/* 
                                      
 
                    
 
                                       
                                            
*/
/*                                                                            */
BOOLEAN wlanIsLaunched(VOID)
{
	struct net_device *prDev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;

	/*                    */
	ASSERT(u4WlanDevNum <= CFG_MAX_WLAN_DEVICES);
	if (0 == u4WlanDevNum) {
		return FALSE;
	}

	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;

	ASSERT(prDev);
	if (NULL == prDev) {
		return FALSE;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);
	if (NULL == prGlueInfo) {
		return FALSE;
	}

	return prGlueInfo->prAdapter->fgIsWlanLaunched;
}

#endif

/*                                                                            */
/* 
                                                      
 
                                                      
 
                                                 
                                           
*/
/*                                                                           */
BOOLEAN wlanExportGlueInfo(P_GLUE_INFO_T *prGlueInfoExpAddr)
{
	struct net_device *prDev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;

	if (0 == u4WlanDevNum) {
		return FALSE;
	}

	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	if (NULL == prDev) {
		return FALSE;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	if (NULL == prGlueInfo) {
		return FALSE;
	}

	if (FALSE == prGlueInfo->prAdapter->fgIsWlanLaunched) {
		return FALSE;
	}

	*prGlueInfoExpAddr = prGlueInfo;
	return TRUE;
}

#endif


/*                                                                            */
/* 
                                                                                
 
                                                
 
                
*/
/*                                                                            */
static void wlanClearDevIdx(struct net_device *prDev)
{
	int i;

	ASSERT(prDev);

	for (i = 0; i < CFG_MAX_WLAN_DEVICES; i++) {
		if (arWlanDevInfo[i].prDev == prDev) {
			arWlanDevInfo[i].prDev = NULL;
			u4WlanDevNum--;
		}
	}

	return;
}				/*                          */


/*                                                                            */
/* 
                                                                                
                                                                           
                                      
 
                                                
 
                                      
                                      
*/
/*                                                                            */
static int wlanGetDevIdx(struct net_device *prDev)
{
	int i;

	ASSERT(prDev);

	for (i = 0; i < CFG_MAX_WLAN_DEVICES; i++) {
		if (arWlanDevInfo[i].prDev == (struct net_device *)NULL) {
			/*                                            
                                        
    */
			arWlanDevInfo[i].prDev = prDev;
			u4WlanDevNum++;
			return i;
		}
	}

	return -1;
}				/*                        */

/*                                                                            */
/* 
                                                                               
                                                                          
                                                                                
                                                         
 
                                              
 
                                                                                  
                                                                  
 
                                  
 
                                                                         
                                                         
*/
/*                                                                            */
int wlanDoIOCTL(struct net_device *prDev, struct ifreq *prIFReq, int i4Cmd)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	int ret = 0;

	/*                                                     */
	ASSERT(prDev && prIFReq);
	if (!prDev || !prIFReq) {
		DBGLOG(INIT, WARN, ("%s Invalid input data\n", __func__));
		return -EINVAL;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(INIT, WARN, ("%s No glue info\n", __func__));
		return -EFAULT;
	}

	if (prGlueInfo->u4ReadyFlag == 0) {
		return -EINVAL;
	}
	/*                               */

	if (i4Cmd == SIOCGIWPRIV) {
		/*                                 */
		ret = wext_get_priv(prDev, prIFReq);
	} else if ((i4Cmd >= SIOCIWFIRST) && (i4Cmd < SIOCIWFIRSTPRIV)) {
		/*                                            */
		ret = wext_support_ioctl(prDev, prIFReq, i4Cmd);
	} else if ((i4Cmd >= SIOCIWFIRSTPRIV) && (i4Cmd < SIOCIWLASTPRIV)) {
		/*                                       */
		ret = priv_support_ioctl(prDev, prIFReq, i4Cmd);
	} else {
		DBGLOG(INIT, WARN, ("Unexpected ioctl command: 0x%04x\n", i4Cmd));
		/*                    */
	}

	return ret;
}				/*                      */

/*                                                                            */
/* 
                                                                
 
                                                
 
                
*/
/*                                                                            */

static struct delayed_work workq;
static struct net_device *gPrDev;

static void wlanSetMulticastList(struct net_device *prDev)
{
	gPrDev = prDev;
	schedule_delayed_work(&workq, 0);
}

/*                                                                     
                                                         
                                                   */

static void wlanSetMulticastListWorkQueue(struct work_struct *work)
{

	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_32 u4PacketFilter = 0;
	UINT_32 u4SetInfoLen;
	struct net_device *prDev = gPrDev;

	down(&g_halt_sem);
	if (g_u4HaltFlag) {
		up(&g_halt_sem);
		return;
	}

	prGlueInfo = (NULL != prDev) ? *((P_GLUE_INFO_T *) netdev_priv(prDev)) : NULL;
	ASSERT(prDev);
	ASSERT(prGlueInfo);
	if (!prDev || !prGlueInfo) {
		DBGLOG(INIT, WARN, ("abnormal dev or skb: prDev(0x%p), prGlueInfo(0x%p)\n",
				    prDev, prGlueInfo));
		up(&g_halt_sem);
		return;
	}

	if (prDev->flags & IFF_PROMISC) {
		u4PacketFilter |= PARAM_PACKET_FILTER_PROMISCUOUS;
	}

	if (prDev->flags & IFF_BROADCAST) {
		u4PacketFilter |= PARAM_PACKET_FILTER_BROADCAST;
		printk("u4PacketFilter |= PARAM_PACKET_FILTER_BROADCAST\n");
	}

	if (prDev->flags & IFF_MULTICAST) {
		if ((prDev->flags & IFF_ALLMULTI) ||
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
		    (netdev_mc_count(prDev) > MAX_NUM_GROUP_ADDR)) {
#else
		    (prDev->mc_count > MAX_NUM_GROUP_ADDR)) {
#endif

			u4PacketFilter |= PARAM_PACKET_FILTER_ALL_MULTICAST;
			printk("u4PacketFilter |= PARAM_PACKET_FILTER_ALL_MULTICAST\n");
		}
		else {
			u4PacketFilter |= PARAM_PACKET_FILTER_MULTICAST;
			printk("u4PacketFilter |= PARAM_PACKET_FILTER_MULTICAST\n");
		}
	}

	up(&g_halt_sem);

	if (kalIoctl(prGlueInfo,
		     wlanoidSetCurrentPacketFilter,
		     &u4PacketFilter,
		     sizeof(u4PacketFilter),
		     FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen) != WLAN_STATUS_SUCCESS) {
		return;
	}


	if (u4PacketFilter & PARAM_PACKET_FILTER_MULTICAST) {
		/*                                */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
		struct netdev_hw_addr *ha;
#else
		struct dev_mc_list *prMcList;
#endif
		PUINT_8 prMCAddrList = NULL;
		UINT_32 i = 0;

		down(&g_halt_sem);
		if (g_u4HaltFlag) {
			up(&g_halt_sem);
			return;
		}

		prMCAddrList = kalMemAlloc(MAX_NUM_GROUP_ADDR * ETH_ALEN, VIR_MEM_TYPE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
		netdev_for_each_mc_addr(ha, prDev) {
			if (i < MAX_NUM_GROUP_ADDR) {
				memcpy((prMCAddrList + i * ETH_ALEN), ha->addr, ETH_ALEN);
				i++;
			}
		}
#else
		for (i = 0, prMcList = prDev->mc_list;
		     (prMcList) && (i < prDev->mc_count) && (i < MAX_NUM_GROUP_ADDR);
		     i++, prMcList = prMcList->next) {
			memcpy((prMCAddrList + i * ETH_ALEN), prMcList->dmi_addr, ETH_ALEN);
		}
#endif

		up(&g_halt_sem);

		kalIoctl(prGlueInfo,
			 wlanoidSetMulticastList,
			 prMCAddrList, (i * ETH_ALEN), FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

		kalMemFree(prMCAddrList, VIR_MEM_TYPE, MAX_NUM_GROUP_ADDR * ETH_ALEN);
	}

	return;
}				/*                               */


/*                                                                     
                                                         
                                                   */

void p2pSetMulticastListWorkQueueWrapper(P_GLUE_INFO_T prGlueInfo)
{

	ASSERT(prGlueInfo);

	if (!prGlueInfo) {
		DBGLOG(INIT, WARN, ("abnormal dev or skb: prGlueInfo(0x%p)\n", prGlueInfo));
		return;
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (prGlueInfo->prAdapter->fgIsP2PRegistered) {
		mtk_p2p_wext_set_Multicastlist(prGlueInfo);
	}
#endif

	return;
}				/*                                              */



/*                                                                            */
/* 
                                                       
 
                                                     
                                                
 
                                    
                                                                               
*/
/*                                                                            */
int wlanHardStartXmit(struct sk_buff *prSkb, struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));

	P_QUE_ENTRY_T prQueueEntry = NULL;
	P_QUE_T prTxQueue = NULL;
	UINT_16 u2QueueIdx = 0;

#if CFG_BOW_TEST
	UINT_32 i;
#endif

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prSkb);
	ASSERT(prDev);
	ASSERT(prGlueInfo);

	if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
		DBGLOG(INIT, INFO, ("GLUE_FLAG_HALT skip tx\n"));
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}

	prQueueEntry = (P_QUE_ENTRY_T) GLUE_GET_PKT_QUEUE_ENTRY(prSkb);
	prTxQueue = &prGlueInfo->rTxQueue;

#if CFG_BOW_TEST
	DBGLOG(BOW, TRACE, ("sk_buff->len: %d\n", prSkb->len));
	DBGLOG(BOW, TRACE, ("sk_buff->data_len: %d\n", prSkb->data_len));
	DBGLOG(BOW, TRACE, ("sk_buff->data:\n"));

	for (i = 0; i < prSkb->len; i++) {
		DBGLOG(BOW, TRACE, ("%4x", prSkb->data[i]));

		if ((i + 1) % 16 == 0) {
			DBGLOG(BOW, TRACE, ("\n"));
		}
	}

	DBGLOG(BOW, TRACE, ("\n"));
#endif

	if (wlanProcessSecurityFrame(prGlueInfo->prAdapter, (P_NATIVE_PACKET) prSkb) == FALSE) {

#if CFG_DBG_GPIO_PINS
		{
			/*                    */
			mtk_wcn_stp_debug_gpio_assert(IDX_TX_REQ, DBG_TIE_LOW);
			kalUdelay(1);
			mtk_wcn_stp_debug_gpio_assert(IDX_TX_REQ, DBG_TIE_HIGH);
		}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 26)
		u2QueueIdx = skb_get_queue_mapping(prSkb);
		ASSERT(u2QueueIdx < CFG_MAX_TXQ_NUM);
#endif

		GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
		QUEUE_INSERT_TAIL(prTxQueue, prQueueEntry);
		GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);

		GLUE_INC_REF_CNT(prGlueInfo->i4TxPendingFrameNum);
		GLUE_INC_REF_CNT(prGlueInfo->
				 ai4TxPendingFrameNumPerQueue[NETWORK_TYPE_AIS_INDEX][u2QueueIdx]);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 26)
		if (prGlueInfo->ai4TxPendingFrameNumPerQueue[NETWORK_TYPE_AIS_INDEX][u2QueueIdx] >=
		    CFG_TX_STOP_NETIF_PER_QUEUE_THRESHOLD) {
			netif_stop_subqueue(prDev, u2QueueIdx);
		}
#else
		if (prGlueInfo->i4TxPendingFrameNum >= CFG_TX_STOP_NETIF_QUEUE_THRESHOLD) {
			netif_stop_queue(prDev);
		}
#endif
	} else {
		/*                                */

		GLUE_INC_REF_CNT(prGlueInfo->i4TxPendingSecurityFrameNum);
	}

	DBGLOG(TX, EVENT,
	       ("\n+++++ pending frame %ld len = %d +++++\n", prGlueInfo->i4TxPendingFrameNum,
		prSkb->len));
	prGlueInfo->rNetDevStats.tx_bytes += prSkb->len;
	prGlueInfo->rNetDevStats.tx_packets++;

	/*                                */
	/*                                            */
	kalSetEvent(prGlueInfo);


	/*                                                                                */
	return NETDEV_TX_OK;
}				/*                            */


/*                                                                            */
/* 
                                                                                
                     
 
                                                                                
                                                                           
 
                                                     
 
                                          
*/
/*                                                                            */
struct net_device_stats *wlanGetStats(IN struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));

#if 0
	WLAN_STATUS rStatus;
	UINT_32 u4XmitError = 0;
	UINT_32 u4XmitOk = 0;
	UINT_32 u4RecvError = 0;
	UINT_32 u4RecvOk = 0;
	UINT_32 u4BufLen;

	ASSERT(prDev);

	/*                                           */


	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryXmitError,
			   &u4XmitError, sizeof(UINT_32), TRUE, TRUE, TRUE, &u4BufLen);

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryXmitOk,
			   &u4XmitOk, sizeof(UINT_32), TRUE, TRUE, TRUE, &u4BufLen);
	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryRcvOk,
			   &u4RecvOk, sizeof(UINT_32), TRUE, TRUE, TRUE, &u4BufLen);
	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryRcvError,
			   &u4RecvError, sizeof(UINT_32), TRUE, TRUE, TRUE, &u4BufLen);
	prGlueInfo->rNetDevStats.rx_packets = u4RecvOk;
	prGlueInfo->rNetDevStats.tx_packets = u4XmitOk;
	prGlueInfo->rNetDevStats.tx_errors = u4XmitError;
	prGlueInfo->rNetDevStats.rx_errors = u4RecvError;
	/*                                                                     */
	/*                                                                     */
	/*                                                                      */
	/*                                                                       */
#endif
	/*                                          */
	/*                                          */
	prGlueInfo->rNetDevStats.tx_errors = 0;
	prGlueInfo->rNetDevStats.rx_errors = 0;
	/*                                          */
	/*                                          */
	prGlueInfo->rNetDevStats.rx_errors = 0;
	prGlueInfo->rNetDevStats.multicast = 0;

	return &prGlueInfo->rNetDevStats;

}				/*                       */

/*                                                                            */
/* 
                                   
 
                                                     
 
                                                       
                                   
*/
/*                                                                            */
static int wlanInit(struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = NULL;

	if (!prDev) {
		return -ENXIO;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12)
	INIT_DELAYED_WORK(&workq, wlanSetMulticastListWorkQueue);
#else
	INIT_DELAYED_WORK(&workq, wlanSetMulticastListWorkQueue, NULL);
#endif

	return 0;		/*         */
}				/*                   */


/*                                                                            */
/* 
                                     
 
                                                     
 
                
*/
/*                                                                            */
static void wlanUninit(struct net_device *prDev)
{
	return;
}				/*                     */


/*                                                                            */
/* 
                                   
 
                                                     
 
                                                   
                                                 
*/
/*                                                                            */
static int wlanOpen(struct net_device *prDev)
{
	ASSERT(prDev);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 26)
	netif_tx_start_all_queues(prDev);
#else
	netif_start_queue(prDev);
#endif

	return 0;		/*         */
}				/*                   */


/*                                                                            */
/* 
                                   
 
                                                     
 
                                                   
                                                 
*/
/*                                                                            */
static int wlanStop(struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	struct cfg80211_scan_request *prScanRequest = NULL;
	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prDev);

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));

	/*               */
	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);
	if (prGlueInfo->prScanRequest != NULL) {
		prScanRequest = prGlueInfo->prScanRequest;
		prGlueInfo->prScanRequest = NULL;
	}
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);

	if (prScanRequest) {
		cfg80211_scan_done(prScanRequest, TRUE);
	}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 26)
	netif_tx_stop_all_queues(prDev);
#else
	netif_stop_queue(prDev);
#endif

	return 0;		/*         */
}				/*                   */


/*                                                                            */
/* 
                                                                                          
  
                                                  
  
                 
 */
/*                                                                            */
VOID wlanUpdateChannelTable(P_GLUE_INFO_T prGlueInfo)
{
	UINT_8 i, j;
	UINT_8 ucNumOfChannel;
	RF_CHANNEL_INFO_T aucChannelList[ARRAY_SIZE(mtk_2ghz_channels) +
					 ARRAY_SIZE(mtk_5ghz_channels)];

	/*                        */
	for (i = 0; i < ARRAY_SIZE(mtk_2ghz_channels); i++) {
		mtk_2ghz_channels[i].flags |= IEEE80211_CHAN_DISABLED;
	}

	for (i = 0; i < ARRAY_SIZE(mtk_5ghz_channels); i++) {
		mtk_5ghz_channels[i].flags |= IEEE80211_CHAN_DISABLED;
	}

	/*                                    */
	rlmDomainGetChnlList(prGlueInfo->prAdapter,
			     BAND_NULL,
			     ARRAY_SIZE(mtk_2ghz_channels) + ARRAY_SIZE(mtk_5ghz_channels),
			     &ucNumOfChannel, aucChannelList);

	/*                                                         */
	for (i = 0; i < ucNumOfChannel; i++) {
		switch (aucChannelList[i].eBand) {
		case BAND_2G4:
			for (j = 0; j < ARRAY_SIZE(mtk_2ghz_channels); j++) {
				if (mtk_2ghz_channels[j].hw_value == aucChannelList[i].ucChannelNum) {
					mtk_2ghz_channels[j].flags &= ~IEEE80211_CHAN_DISABLED;
					break;
				}
			}
			break;

		case BAND_5G:
			for (j = 0; j < ARRAY_SIZE(mtk_5ghz_channels); j++) {
				if (mtk_5ghz_channels[j].hw_value == aucChannelList[i].ucChannelNum) {
					mtk_5ghz_channels[j].flags &= ~IEEE80211_CHAN_DISABLED;
					break;
				}
			}
			break;

		default:
			break;
		}
	}

	return;
}


/*                                                                            */
/* 
                                                                
 
                                                     
 
                                                          
                                                        
*/
/*                                                                            */
static INT_32 wlanNetRegister(struct wireless_dev *prWdev)
{
	P_GLUE_INFO_T prGlueInfo;
	INT_32 i4DevIdx = -1;

	ASSERT(prWdev);


	do {
		if (!prWdev) {
			break;
		}

		prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);

		if ((i4DevIdx = wlanGetDevIdx(prWdev->netdev)) < 0) {
			DBGLOG(INIT, ERROR, ("wlanNetRegister: net_device number exceeds.\n"));
			break;
		}

		/*                               */
		wlanUpdateChannelTable((P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy));

		if (wiphy_register(prWdev->wiphy) < 0) {
			DBGLOG(INIT, ERROR,
			       ("wlanNetRegister: wiphy context is not registered.\n"));
			wlanClearDevIdx(prWdev->netdev);
			i4DevIdx = -1;
		}

		if (register_netdev(prWdev->netdev) < 0) {
			DBGLOG(INIT, ERROR,
			       ("wlanNetRegister: net_device context is not registered.\n"));

			wiphy_unregister(prWdev->wiphy);
			wlanClearDevIdx(prWdev->netdev);
			i4DevIdx = -1;
		}

		if (i4DevIdx != -1) {
			prGlueInfo->fgIsRegistered = TRUE;
		}
	}
	while (FALSE);

	return i4DevIdx;	/*         */
}				/*                          */


/*                                                                            */
/* 
                                              
 
                                                      
 
                
*/
/*                                                                            */
static VOID wlanNetUnregister(struct wireless_dev *prWdev)
{
	P_GLUE_INFO_T prGlueInfo;

	if (!prWdev) {
		DBGLOG(INIT, ERROR, ("wlanNetUnregister: The device context is NULL\n"));
		return;
	}

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);

	wlanClearDevIdx(prWdev->netdev);
	unregister_netdev(prWdev->netdev);
	wiphy_unregister(prWdev->wiphy);

	prGlueInfo->fgIsRegistered = FALSE;

	DBGLOG(INIT, INFO, ("unregister wireless_dev(0x%p)\n", prWdev));

	return;
}				/*                            */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
static const struct net_device_ops wlan_netdev_ops = {
	.ndo_open = wlanOpen,
	.ndo_stop = wlanStop,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
	.ndo_set_rx_mode = wlanSetMulticastList,
#else
	.ndo_set_multicast_list = wlanSetMulticastList,
#endif
	.ndo_get_stats = wlanGetStats,
	.ndo_do_ioctl = wlanDoIOCTL,
	.ndo_start_xmit = wlanHardStartXmit,
	.ndo_init = wlanInit,
	.ndo_uninit = wlanUninit,
	.ndo_select_queue = wlanSelectQueue,
};
#endif

#ifdef CONFIG_PM
static const struct wiphy_wowlan_support wlan_wowlan_support = {
	.flags = WIPHY_WOWLAN_DISCONNECT | WIPHY_WOWLAN_ANY,
};
#endif

/*                                                                            */
/* 
                                                                          
                                                                                 
                                                             
 
                                                     
 
                                                
                                                          
*/
/*                                                                            */
static struct lock_class_key rSpinKey[SPIN_LOCK_NUM];
static struct wireless_dev *wlanNetCreate(PVOID pvData)
{
	struct wireless_dev *prWdev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	UINT_32 i;
	struct device *prDev;

	/*                             */
	prWdev = kzalloc(sizeof(struct wireless_dev), GFP_KERNEL);
	DBGLOG(INIT, INFO, ("wireless_dev prWdev(0x%p) allocated\n", prWdev));
	if (!prWdev) {
		DBGLOG(INIT, ERROR, ("Allocating memory to wireless_dev context failed\n"));
		return NULL;
	}
	/*                      */
	prWdev->wiphy = wiphy_new(&mtk_wlan_ops, sizeof(GLUE_INFO_T));
	DBGLOG(INIT, INFO, ("wiphy (0x%p) allocated\n", prWdev->wiphy));
	if (!prWdev->wiphy) {
		DBGLOG(INIT, ERROR, ("Allocating memory to wiphy device failed\n"));
		kfree(prWdev);
		return NULL;
	}
	/*                                 */
#if MTK_WCN_HIF_SDIO
	mtk_wcn_hif_sdio_get_dev(*((MTK_WCN_HIF_SDIO_CLTCTX *) pvData), &prDev);
#else
	prDev = ((struct sdio_func *)pvData)->dev;
#endif
	if (!prDev) {
		printk(KERN_ALERT DRV_NAME "unable to get struct dev for wlan\n");
	}
	set_wiphy_dev(prWdev->wiphy, prDev);

	/*                                        */
	prWdev->iftype = NL80211_IFTYPE_STATION;
	prWdev->wiphy->max_scan_ssids = 1;	/*                       */
	prWdev->wiphy->max_scan_ie_len = 512;
	prWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_ADHOC);
	prWdev->wiphy->bands[IEEE80211_BAND_2GHZ] = &mtk_band_2ghz;
	/*                                        */
	/*                                                             */
	prWdev->wiphy->bands[IEEE80211_BAND_5GHZ] = NULL;
	prWdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	prWdev->wiphy->cipher_suites = (const u32 *)mtk_cipher_suites;
	prWdev->wiphy->n_cipher_suites = ARRAY_SIZE(mtk_cipher_suites);
	prWdev->wiphy->flags = WIPHY_FLAG_CUSTOM_REGULATORY | WIPHY_FLAG_SUPPORTS_FW_ROAM;
#ifdef CONFIG_PM
	kalMemCopy(&prWdev->wiphy->wowlan, &wlan_wowlan_support,
		   sizeof(struct wiphy_wowlan_support));
#endif
	/*                             */
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);
	if (!prGlueInfo) {
		DBGLOG(INIT, ERROR, ("Allocating memory to glue layer failed\n"));
		goto netcreate_err;
	}
	/*                              */
	/*                           */
	prGlueInfo->prDevHandler =
	    alloc_netdev_mq(sizeof(P_GLUE_INFO_T), NIC_INF_NAME, ether_setup, CFG_MAX_TXQ_NUM);

	DBGLOG(INIT, INFO, ("net_device prDev(0x%p) allocated\n", prGlueInfo->prDevHandler));
	if (!prGlueInfo->prDevHandler) {
		DBGLOG(INIT, ERROR, ("Allocating memory to net_device context failed\n"));
		goto netcreate_err;
	}
	/*                                            */
	*((P_GLUE_INFO_T *) netdev_priv(prGlueInfo->prDevHandler)) = prGlueInfo;

	prGlueInfo->prDevHandler->netdev_ops = &wlan_netdev_ops;
#ifdef CONFIG_WIRELESS_EXT
	prGlueInfo->prDevHandler->wireless_handlers = &wext_handler_def;
#endif
	netif_carrier_off(prGlueInfo->prDevHandler);
	netif_tx_stop_all_queues(prGlueInfo->prDevHandler);

	/*                                                 */
	prGlueInfo->prDevHandler->ieee80211_ptr = prWdev;
#if CFG_TCP_IP_CHKSUM_OFFLOAD
	prGlueInfo->prDevHandler->features = NETIF_F_HW_CSUM;
#endif
	prWdev->netdev = prGlueInfo->prDevHandler;

	/*                                        */
	SET_NETDEV_DEV(prGlueInfo->prDevHandler, wiphy_dev(prWdev->wiphy));

	/*                                 */
	prGlueInfo->eParamMediaStateIndicated = PARAM_MEDIA_STATE_DISCONNECTED;
	prGlueInfo->ePowerState = ParamDeviceStateD0;
	prGlueInfo->fgIsMacAddrOverride = FALSE;
	prGlueInfo->fgIsRegistered = FALSE;
	prGlueInfo->prScanRequest = NULL;

	init_completion(&prGlueInfo->rScanComp);
	init_completion(&prGlueInfo->rHaltComp);
	init_completion(&prGlueInfo->rPendComp);
#if CFG_ENABLE_WIFI_DIRECT
	init_completion(&prGlueInfo->rSubModComp);
#endif

	/*                                          */
	kalOsTimerInitialize(prGlueInfo, kalTimeoutHandler);

	for (i = 0; i < SPIN_LOCK_NUM; i++) {
		spin_lock_init(&prGlueInfo->rSpinLock[i]);
		lockdep_set_class(&prGlueInfo->rSpinLock[i], &rSpinKey[i]);
	}

	/*                                */
	sema_init(&prGlueInfo->ioctl_sem, 1);

	/*                                */
	sema_init(&g_halt_sem, 1);
	g_u4HaltFlag = 0;

	/*                                */
	prAdapter = (P_ADAPTER_T) wlanAdapterCreate(prGlueInfo);

	if (!prAdapter) {
		DBGLOG(INIT, ERROR, ("Allocating memory to adapter failed\n"));
		goto netcreate_err;
	}

	prGlueInfo->prAdapter = prAdapter;

#ifdef CONFIG_CFG80211_WEXT
	/*                                               */
	prWdev->wiphy->wext = &wext_handler_def;
#endif

	goto netcreate_done;

 netcreate_err:
	if (NULL != prAdapter) {
		wlanAdapterDestroy(prAdapter);
		prAdapter = NULL;
	}

	if (NULL != prGlueInfo->prDevHandler) {
		free_netdev(prGlueInfo->prDevHandler);
		prGlueInfo->prDevHandler = NULL;
	}

	if (NULL != prWdev->wiphy) {
		wiphy_free(prWdev->wiphy);
		prWdev->wiphy = NULL;
	}

	if (NULL != prWdev) {
		/*                                                         
                    
   */
		kfree(prWdev);
		prWdev = NULL;
	}

 netcreate_done:

	return prWdev;
}				/*                        */


/*                                                                            */
/* 
                                                                      
 
                                                        
 
                
*/
/*                                                                            */
static VOID wlanNetDestroy(struct wireless_dev *prWdev)
{
	P_GLUE_INFO_T prGlueInfo = NULL;

	ASSERT(prWdev);

	if (!prWdev) {
		DBGLOG(INIT, ERROR, ("wlanNetDestroy: The device context is NULL\n"));
		return;
	}

	/*                                         */
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);
	ASSERT(prGlueInfo);

	/*                      */
	kalCancelTimer(prGlueInfo);

	glClearHifInfo(prGlueInfo);

	wlanAdapterDestroy(prGlueInfo->prAdapter);
	prGlueInfo->prAdapter = NULL;

	/*                                                                         
  */
	free_netdev(prWdev->netdev);
	wiphy_free(prWdev->wiphy);

	kfree(prWdev);

	return;
}				/*                         */

#ifndef CONFIG_X86
UINT_8 g_aucBufIpAddr[32] = { 0 };

static void wlanEarlySuspend(void)
{
	struct in_device *in_dev;	/*                 */
	struct net_device *prDev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_8 ip[4] = { 0 };
	UINT_32 u4NumIPv4 = 0;
#if defined(CONFIG_IPV6) && defined(ENABLE_IPV6_WLAN)
	UINT_8 ip6[16] = { 0 };	/*                                                 */
#endif
	UINT_32 u4NumIPv6 = 0;
	UINT_32 i;
	P_PARAM_NETWORK_ADDRESS_IP prParamIpAddr;

#if defined(MTK_PACKET_FILTERING_SUPPORT)
	UINT_32 u4PacketFilter = 0, u4SetInfoLen = 0, u4QueryInfoLen = 0;
	WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
#endif


	DBGLOG(INIT, INFO, ("*********wlanEarlySuspend************\n"));

	/*                                             */
	ASSERT(u4WlanDevNum <= CFG_MAX_WLAN_DEVICES);
	if (u4WlanDevNum == 0) {
		DBGLOG(INIT, ERROR, ("wlanEarlySuspend u4WlanDevNum==0 invalid!!\n"));
		return;
	}
	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	ASSERT(prDev);

	/*                            */
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);

#ifdef FIX_ALPS00409409406
	atomic_set(&fgIsUnderEarlierSuspend, 1);
#else
	fgIsUnderEarlierSuspend = true;
#endif

	/*                 */
#ifdef FIX_ALPS00409409406
	/*                          */
	in_dev = in_dev_get(prDev);
	if (!in_dev)
		return;

	/*              */
	if (!in_dev->ifa_list || !in_dev->ifa_list->ifa_local) {
		in_dev_put(in_dev);
		/*                */
		DBGLOG(INIT, INFO, ("ip is not avaliable.\n"));
		return;
	}
	/*                           */
	kalMemCopy(ip, &(in_dev->ifa_list->ifa_local), sizeof(ip));
	/*                */
	in_dev_put(in_dev);

	DBGLOG(INIT, INFO, ("ip is %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]));

#else

	/*                          */
	if (!prDev || !(prDev->ip_ptr) ||
	    !((struct in_device *)(prDev->ip_ptr))->ifa_list ||
	    !(&(((struct in_device *)(prDev->ip_ptr))->ifa_list->ifa_local))) {
		DBGLOG(INIT, INFO, ("ip is not avaliable.\n"));
		return;
	}
	/*                           */
	kalMemCopy(ip, &(((struct in_device *)(prDev->ip_ptr))->ifa_list->ifa_local), sizeof(ip));
	DBGLOG(INIT, INFO, ("ip is %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]));
#endif


	/*                       */
#if defined(MTK_PACKET_FILTERING_SUPPORT)
	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryCurrentPacketFilter,
			   &u4PacketFilter,
			   sizeof(u4PacketFilter), FALSE, FALSE, TRUE, FALSE, &u4QueryInfoLen);


	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("wlanoidQueryCurrentPacketFilter fail 0x%lx\n", rStatus));
	} else {
		DBGLOG(INIT, INFO, ("wlanoidQueryCurrentPacketFilter 0x%lx\n", u4PacketFilter));
	}

/*                                                                                              */
	u4PacketFilter &= ~PARAM_PACKET_FILTER_P2P_MASK;
	u4PacketFilter &= ~PARAM_PACKET_FILTER_MULTICAST;

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidSetCurrentPacketFilter,
			   &u4PacketFilter,
			   sizeof(u4PacketFilter), FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("wlanoidSetCurrentPacketFilter fail 0x%lx\n", rStatus));
	} else {
		DBGLOG(INIT, INFO, ("wlanoidSetCurrentPacketFilter 0x%lx\n", u4PacketFilter));
	}

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryCurrentPacketFilter,
			   &u4PacketFilter,
			   sizeof(u4PacketFilter), FALSE, FALSE, TRUE, FALSE, &u4QueryInfoLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("wlanoidQueryCurrentPacketFilter fail 0x%lx\n", rStatus));
	} else {
		DBGLOG(INIT, INFO, ("wlanoidQueryCurrentPacketFilter 0x%lx\n", u4PacketFilter));
	}

	g_packet_switch = 1;
#endif

	/*                                                                  */
	if (!((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0))) {
		u4NumIPv4++;
	}
#if defined(CONFIG_IPV6) && defined(ENABLE_IPV6_WLAN)
#if 0
	if (!prDev || !(prDev->ip6_ptr) ||
	    !((struct in_device *)(prDev->ip6_ptr))->ifa_list ||
	    !(&(((struct in_device *)(prDev->ip6_ptr))->ifa_list->ifa_local))) {
		DBGLOG(INIT, INFO, ("ipv6 is not avaliable.\n"));
		return;
	}
	/*                           */
	kalMemCopy(ip6, &(((struct in_device *)(prDev->ip6_ptr))->ifa_list->ifa_local),
		   sizeof(ip6));
	DBGLOG(INIT, INFO,
	       ("ipv6 is %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\n", ip6[0], ip6[1], ip6[2],
		ip6[3], ip6[4], ip6[5], ip6[6], ip6[7], ip6[8], ip6[9], ip6[10], ip6[11], ip6[12],
		ip6[13], ip6[14], ip6[15]
	       ));

	/*                                                                  */
	if (!((ip6[0] == 0) &&
	      (ip6[1] == 0) && (ip6[2] == 0) && (ip6[3] == 0) && (ip6[4] == 0) && (ip6[5] == 0))) {
		/*              */
	}
#else
	{
		struct inet6_dev *in6_dev = NULL;

		in6_dev = in6_dev_get(prDev);
		if (!in6_dev)
			return;

		/*              */
		if (!in6_dev->ac_list || !in6_dev->ac_list->aca_addr.s6_addr16) {
			/*                */
			in6_dev_put(in6_dev);
			DBGLOG(INIT, INFO, ("ipv6 is not avaliable.\n"));
			return;
		}
		/*                           */
		kalMemCopy(ip6, in6_dev->ac_list->aca_addr.s6_addr16, sizeof(ip6));
		/*                */
		in6_dev_put(in6_dev);

		DBGLOG(INIT, INFO, ("ipv6 is %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\n",
				    ip6[0], ip6[1], ip6[2], ip6[3],
				    ip6[4], ip6[5], ip6[6], ip6[7],
				    ip6[8], ip6[9], ip6[10], ip6[11],
				    ip6[12], ip6[13], ip6[14], ip6[15]
		       ));

		/*                                                                  */
		if (!((ip6[0] == 0) &&
		      (ip6[1] == 0) &&
		      (ip6[2] == 0) && (ip6[3] == 0) && (ip6[4] == 0) && (ip6[5] == 0))) {
			u4NumIPv6++;
		}

	}
#endif


#endif

	/*                           */
	{
		WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
		UINT_32 u4SetInfoLen = 0;
/*                          */
		UINT_32 u4Len = OFFSET_OF(PARAM_NETWORK_ADDRESS_LIST, arAddress);
		P_PARAM_NETWORK_ADDRESS_LIST prParamNetAddrList = (P_PARAM_NETWORK_ADDRESS_LIST) g_aucBufIpAddr;	/*         */
		P_PARAM_NETWORK_ADDRESS prParamNetAddr = prParamNetAddrList->arAddress;

		kalMemZero(g_aucBufIpAddr, sizeof(g_aucBufIpAddr));

		prParamNetAddrList->u4AddressCount = u4NumIPv4 + u4NumIPv6;
		prParamNetAddrList->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;
		for (i = 0; i < u4NumIPv4; i++) {
			prParamNetAddr->u2AddressLength = sizeof(PARAM_NETWORK_ADDRESS_IP);	/*     */
			prParamNetAddr->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;
#if 0
			kalMemCopy(prParamNetAddr->aucAddress, ip, sizeof(ip));
			prParamNetAddr =
			    (P_PARAM_NETWORK_ADDRESS) ((UINT_32) prParamNetAddr + sizeof(ip));
			u4Len += OFFSET_OF(PARAM_NETWORK_ADDRESS, aucAddress) + sizeof(ip);
#else
			prParamIpAddr = (P_PARAM_NETWORK_ADDRESS_IP) prParamNetAddr->aucAddress;
			kalMemCopy(&prParamIpAddr->in_addr, ip, sizeof(ip));
			prParamNetAddr =
			    (P_PARAM_NETWORK_ADDRESS) ((UINT_32) prParamNetAddr +
						       sizeof(PARAM_NETWORK_ADDRESS));
			u4Len +=
			    OFFSET_OF(PARAM_NETWORK_ADDRESS,
				      aucAddress) + sizeof(PARAM_NETWORK_ADDRESS);
#endif
		}
#if defined(CONFIG_IPV6) && defined(ENABLE_IPV6_WLAN)
		for (i = 0; i < u4NumIPv6; i++) {
			prParamNetAddr->u2AddressLength = 6;
			prParamNetAddr->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;
			kalMemCopy(prParamNetAddr->aucAddress, ip6, sizeof(ip6));
			prParamNetAddr =
			    (P_PARAM_NETWORK_ADDRESS) ((UINT_32) prParamNetAddr + sizeof(ip6));
			u4Len += OFFSET_OF(PARAM_NETWORK_ADDRESS, aucAddress) + sizeof(ip6);
		}
#endif
		ASSERT(u4Len <= sizeof(g_aucBufIpAddr /*       */));

		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetNetworkAddress,
				   (PVOID) prParamNetAddrList,
				   u4Len, FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, INFO, ("set HW pattern filter fail 0x%lx\n", rStatus));
		}
	}
}

static void wlanLateResume(void)
{
	struct in_device *in_dev;	/*                 */
	struct net_device *prDev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
#ifdef TEST_FOR_NET_RCU_LOCK
	UINT_8 ip[4] = { 0 };
#endif
#ifdef CONFIG_IPV6
	UINT_8 ip6[16] = { 0 };	/*                                                 */
#endif
#if defined(MTK_PACKET_FILTERING_SUPPORT)
	UINT_32 u4PacketFilter = 0, u4SetInfoLen = 0, u4QueryInfoLen = 0;
	WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
#endif
	DBGLOG(INIT, INFO, ("*********wlanLateResume************\n"));

	/*                                             */
	ASSERT(u4WlanDevNum <= CFG_MAX_WLAN_DEVICES);
	if (u4WlanDevNum == 0) {
		DBGLOG(INIT, ERROR, ("wlanLateResume u4WlanDevNum==0 invalid!!\n"));
		return;
	}
	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	if (NULL == prDev)
		return;
	ASSERT(prDev);

	/*                            */
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);

#ifdef FIX_ALPS00409409406
	atomic_set(&fgIsUnderEarlierSuspend, 0);
#else
	fgIsUnderEarlierSuspend = false;
#endif

	/*                          */
	/*                 */
#ifdef FIX_ALPS00409409406
	in_dev = in_dev_get(prDev);
	if (!in_dev)
		return;

	/*              */
	if (!in_dev->ifa_list || !in_dev->ifa_list->ifa_local) {
		in_dev_put(in_dev);
		/*                */
		DBGLOG(INIT, INFO, ("ip is not avaliable.\n"));
		return;
	}
#ifdef TEST_FOR_NET_RCU_LOCK
	{
		int i;
		for (i = 0; i < 1000; i++) {
			volatile struct in_device *inDev = in_dev;
			volatile struct in_ifaddr *inIfaddr = inDev;
			kalMemCopy(ip, &(inIfaddr->ifa_local), sizeof(ip));
			DBGLOG(INIT, INFO, ("ip is %d,%d%d,%d\n", ip[0], ip[1], ip[2], ip[3]));
		}
	}
#endif
	in_dev_put(in_dev);
	/*                */

#else
	if (!prDev || !(prDev->ip_ptr) ||
	    !((struct in_device *)(prDev->ip_ptr))->ifa_list ||
	    !(&(((struct in_device *)(prDev->ip_ptr))->ifa_list->ifa_local))) {
		DBGLOG(INIT, INFO, ("ip is not avaliable.\n"));
		return;
	}
#endif


#if defined(CONFIG_IPV6) && defined(ENABLE_IPV6_WLAN)

#if 0
	/*                          */
	if (!prDev || !(prDev->ip6_ptr) ||
	    !((struct in_device *)(prDev->ip6_ptr))->ifa_list ||
	    !(&(((struct in_device *)(prDev->ip6_ptr))->ifa_list->ifa_local))) {
		DBGLOG(INIT, INFO, ("ipv6 is not avaliable.\n"));
		return;
	}
	/*                           */
	kalMemCopy(ip6, &(((struct in_device *)(prDev->ip6_ptr))->ifa_list->ifa_local),
		   sizeof(ip6));
	DBGLOG(INIT, INFO,
	       ("ipv6 is %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\n", ip6[0], ip6[1], ip6[2],
		ip6[3], ip6[4], ip6[5], ip6[6], ip6[7], ip6[8], ip6[9], ip6[10], ip6[11], ip6[12],
		ip6[13], ip6[14], ip6[15]
	       ));
#else
	{
		struct inet6_dev *in6_dev = NULL;

		in6_dev = in6_dev_get(prDev);
		if (!in6_dev)
			return;

		/*              */
		if (!in6_dev->ac_list || !in6_dev->ac_list->aca_addr.s6_addr16) {
			/*                */
			in6_dev_put(in6_dev);
			DBGLOG(INIT, INFO, ("ipv6 is not avaliable.\n"));
			return;
		}
		/*                */
		in6_dev_put(in6_dev);
	}
#endif

#endif

	/*                         */
#if defined(MTK_PACKET_FILTERING_SUPPORT)
	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryCurrentPacketFilter,
			   &u4PacketFilter,
			   sizeof(u4PacketFilter), FALSE, FALSE, TRUE, FALSE, &u4QueryInfoLen);

/*                                                                                */
	u4PacketFilter &= ~PARAM_PACKET_FILTER_P2P_MASK;
	u4PacketFilter |= PARAM_PACKET_FILTER_MULTICAST;	/*                               */

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("wlanoidQueryCurrentPacketFilter fail 0x%lx\n", rStatus));
	} else {
		DBGLOG(INIT, INFO, ("wlanoidQueryCurrentPacketFilter 0x%lx\n", u4PacketFilter));
	}

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidSetCurrentPacketFilter,
			   &u4PacketFilter,
			   sizeof(u4PacketFilter), FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("wlanoidSetCurrentPacketFilter fail 0x%lx\n", rStatus));
	} else {
		DBGLOG(INIT, INFO, ("wlanoidSetCurrentPacketFilter  0x%lx\n", u4PacketFilter));
	}

	g_packet_switch = 0;
#endif

#ifdef FIX_ALPS00409409406
#ifdef TEST_FOR_NET_RCU_LOCK
	in_dev = in_dev_get(prDev);
	if (in_dev) {
		int i;
		for (i = 0; i < 1000; i++) {
			volatile struct in_device *inDev = in_dev;
			volatile struct in_ifaddr *inIfaddr = inDev;
			kalMemCopy(ip, &(inIfaddr->ifa_local), sizeof(ip));
			DBGLOG(INIT, INFO, ("ip is %d,%d%d,%d\n", ip[0], ip[1], ip[2], ip[3]));
		}
		in_dev_put(in_dev);
	}
#endif
#endif


	/*                          */
	{
		WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
		UINT_32 u4SetInfoLen = 0;
/*                          */
		UINT_32 u4Len = sizeof(PARAM_NETWORK_ADDRESS_LIST);
		P_PARAM_NETWORK_ADDRESS_LIST prParamNetAddrList = (P_PARAM_NETWORK_ADDRESS_LIST) g_aucBufIpAddr;	/*         */

		kalMemZero(g_aucBufIpAddr, sizeof(g_aucBufIpAddr));

		prParamNetAddrList->u4AddressCount = 0;
		prParamNetAddrList->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;

		ASSERT(u4Len <= sizeof(g_aucBufIpAddr /*       */));
		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetNetworkAddress,
				   (PVOID) prParamNetAddrList,
				   u4Len, FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, INFO, ("set HW pattern filter fail 0x%lx\n", rStatus));
		}
	}
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
static struct early_suspend mt6620_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
};

static void wlan_early_suspend(struct early_suspend *h)
{
	DBGLOG(INIT, INFO, ("*********wlan_early_suspend************\n"));
	wlanEarlySuspend();
}

static void wlan_late_resume(struct early_suspend *h)
{
	DBGLOG(INIT, INFO, ("*********wlan_late_resume************\n"));
	wlanLateResume();
}
#endif				/*                                  */
#endif				/*              */

extern void wlanRegisterNotifier(void);
extern void wlanUnregisterNotifier(void);


typedef int (*set_p2p_mode) (struct net_device *netdev, PARAM_CUSTOM_P2P_SET_STRUC_T p2pmode);
typedef void (*set_dbg_level) (unsigned char modules[DBG_MODULE_NUM]);

extern void register_set_p2p_mode_handler(set_p2p_mode handler);
extern void register_set_dbg_level_handler(set_dbg_level handler);

static int set_p2p_mode_handler(struct net_device *netdev, PARAM_CUSTOM_P2P_SET_STRUC_T p2pmode)
{
	extern BOOLEAN fgIsResetting;
	P_GLUE_INFO_T prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(netdev));
	PARAM_CUSTOM_P2P_SET_STRUC_T rSetP2P;
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;
	UINT_32 u4BufLen = 0;

	rSetP2P.u4Enable = p2pmode.u4Enable;
	rSetP2P.u4Mode = p2pmode.u4Mode;

	if ((!rSetP2P.u4Enable) && (fgIsResetting == FALSE)) {
		p2pNetUnregister(prGlueInfo, TRUE);
	}

	rWlanStatus = kalIoctl(prGlueInfo,
			       wlanoidSetP2pMode,
			       (PVOID) &rSetP2P,
			       sizeof(PARAM_CUSTOM_P2P_SET_STRUC_T),
			       FALSE, FALSE, TRUE, FALSE, &u4BufLen);

	DBGLOG(INIT, INFO, ("set_p2p_mode_handler ret = 0x%08lx\n", (UINT_32) rWlanStatus));

	/*                                                              
                                                  
                                          */
	if ((rSetP2P.u4Enable) && (prGlueInfo->prAdapter->fgIsP2PRegistered) &&
	    (fgIsResetting == FALSE)) {
		p2pNetRegister(prGlueInfo, TRUE);
	}

	return 0;
}

static void set_dbg_level_handler(unsigned char dbg_lvl[DBG_MODULE_NUM])
{
	kalMemCopy(aucDebugModule, dbg_lvl, sizeof(aucDebugModule));
	kalPrint("[wlan] change debug level");
}

/*                                                                            */
/* 
                                                                              
 
                                                               
                                           
                                                             
 
                   
                               
*/
/*                                                                            */
static INT_32 wlanProbe(PVOID pvData)
{
	struct wireless_dev *prWdev = NULL;
	P_WLANDEV_INFO_T prWlandevInfo = NULL;
	INT_32 i4DevIdx = 0;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	INT_32 i4Status = 0;
	BOOL bRet = FALSE;


	do {
		/*                                               */
		/*                                                      
                                                              
                                     
                                 
   */

		bRet = glBusInit(pvData);

		/*                                      */
		if (FALSE == bRet) {
			DBGLOG(INIT, ERROR, (KERN_ALERT "wlanProbe: glBusInit() fail\n"));
			i4Status = -EIO;
			break;
		}
		/*                                                                     */
		if ((prWdev = wlanNetCreate(pvData)) == NULL) {
			DBGLOG(INIT, ERROR, ("wlanProbe: No memory for dev and its private\n"));
			i4Status = -ENOMEM;
			break;
		}
		/*                                    */
		prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);
		gPrDev = prGlueInfo->prDevHandler;
		glSetHifInfo(prGlueInfo, (UINT_32) pvData);


		/*                                         */
		init_waitqueue_head(&prGlueInfo->waitq);
		/*  */

		QUEUE_INITIALIZE(&prGlueInfo->rCmdQueue);
		QUEUE_INITIALIZE(&prGlueInfo->rTxQueue);


		/*                                                                                          */

		/*                 */
		prWlandevInfo = &arWlanDevInfo[i4DevIdx];

		/*                                                                                                 */

		if (i4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, ("wlanProbe: Set IRQ error\n"));
			break;
		}

		prGlueInfo->i4DevIdx = i4DevIdx;

		prAdapter = prGlueInfo->prAdapter;

		prGlueInfo->u4ReadyFlag = 0;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
		prAdapter->u4CSUMFlags =
		    (CSUM_OFFLOAD_EN_TX_TCP | CSUM_OFFLOAD_EN_TX_UDP | CSUM_OFFLOAD_EN_TX_IP);
#endif

		/*                    */
		/*  */
#if CFG_ENABLE_FW_DOWNLOAD
		/*                                                         */
		{
			UINT_32 u4FwSize = 0;
			PVOID prFwBuffer = NULL;
			P_REG_INFO_T prRegInfo = &prGlueInfo->rRegInfo;

			/*                                                                                  */
			kalMemSet(prRegInfo, 0, sizeof(REG_INFO_T));
			prRegInfo->u4StartAddress = CFG_FW_START_ADDRESS;
			prRegInfo->u4LoadAddress = CFG_FW_LOAD_ADDRESS;

			/*                                  */
			glLoadNvram(prGlueInfo, prRegInfo);

			/*                                                                   */

			prRegInfo->u4PowerMode = CFG_INIT_POWER_SAVE_PROF;
			prRegInfo->fgEnArpFilter = TRUE;

			if (kalFirmwareImageMapping(prGlueInfo, &prFwBuffer, &u4FwSize) == NULL) {
				i4Status = -EIO;
				goto bailout;
			} else {
				if (wlanAdapterStart(prAdapter, prRegInfo, prFwBuffer, u4FwSize) !=
				    WLAN_STATUS_SUCCESS) {
					i4Status = -EIO;
				}
			}

			kalFirmwareImageUnmapping(prGlueInfo, NULL, prFwBuffer);

 bailout:
			/*                   */

			if (i4Status < 0) {
				break;
			}
		}
#else
		/*                                                                                  */
		kalMemSet(&prGlueInfo->rRegInfo, 0, sizeof(REG_INFO_T));
		P_REG_INFO_T prRegInfo = &prGlueInfo->rRegInfo;

		/*                                  */
		glLoadNvram(prGlueInfo, prRegInfo);

		prRegInfo->u4PowerMode = CFG_INIT_POWER_SAVE_PROF;

		if (wlanAdapterStart(prAdapter, prRegInfo, NULL, 0) != WLAN_STATUS_SUCCESS) {
			i4Status = -EIO;
			break;
		}
#endif
		if (TRUE == prAdapter->fgEnable5GBand)
			prWdev->wiphy->bands[IEEE80211_BAND_5GHZ] = &mtk_band_5ghz;

		prGlueInfo->main_thread =
		    kthread_run(tx_thread, prGlueInfo->prDevHandler, "tx_thread");

		/*                 */
		{
			WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
			struct sockaddr MacAddr;
			UINT_32 u4SetInfoLen = 0;

			rStatus = kalIoctl(prGlueInfo,
					   wlanoidQueryCurrentAddr,
					   &MacAddr.sa_data,
					   PARAM_MAC_ADDR_LEN,
					   TRUE, TRUE, TRUE, FALSE, &u4SetInfoLen);

			if (rStatus != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, WARN, ("set MAC addr fail 0x%lx\n", rStatus));
				prGlueInfo->u4ReadyFlag = 0;
			} else {
				memcpy(prGlueInfo->prDevHandler->dev_addr, &MacAddr.sa_data,
				       ETH_ALEN);
				memcpy(prGlueInfo->prDevHandler->perm_addr,
				       prGlueInfo->prDevHandler->dev_addr, ETH_ALEN);

				/*               */
				prGlueInfo->u4ReadyFlag = 1;
#if CFG_SHOW_MACADDR_SOURCE
				DBGLOG(INIT, INFO,
				       ("MAC address: " MACSTR, MAC2STR(&MacAddr.sa_data)));
#endif
			}
		}


#if CFG_TCP_IP_CHKSUM_OFFLOAD
		/*                         */
		{
			WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
			UINT_32 u4CSUMFlags = CSUM_OFFLOAD_EN_ALL;
			UINT_32 u4SetInfoLen = 0;

			rStatus = kalIoctl(prGlueInfo,
					   wlanoidSetCSUMOffload,
					   (PVOID) &u4CSUMFlags,
					   sizeof(UINT_32),
					   FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

			if (rStatus != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, WARN,
				       ("set HW checksum offload fail 0x%lx\n", rStatus));
			}
		}
#endif

		/*                         */
		if ((i4DevIdx = wlanNetRegister(prWdev)) < 0) {
			i4Status = -ENXIO;
			DBGLOG(INIT, ERROR,
			       ("wlanProbe: Cannot register the net_device context to the kernel\n"));
			break;
		}
#if defined(CONFIG_HAS_EARLYSUSPEND)
		glRegisterEarlySuspend(&mt6620_early_suspend_desc, wlan_early_suspend,
				       wlan_late_resume);
		wlanRegisterNotifier();
#endif

		/*                                   */
#ifdef WLAN_INCLUDE_PROC
		if ((i4Status = procInitProcfs(prDev, NIC_DEVICE_ID_LOW)) < 0) {
			DBGLOG(INIT, ERROR, ("wlanProbe: init procfs failed\n"));
			break;
		}
#endif				/*                   */

#if CFG_ENABLE_BT_OVER_WIFI
		prGlueInfo->rBowInfo.fgIsNetRegistered = FALSE;
		prGlueInfo->rBowInfo.fgIsRegistered = FALSE;
		glRegisterAmpc(prGlueInfo);
#endif

#if CFG_ENABLE_WIFI_DIRECT
		/*                  */
		prGlueInfo->prAdapter->fgIsWlanLaunched = TRUE;
		/*                                                                 */
		if (rSubModHandler[P2P_MODULE].subModInit) {
			wlanSubModInit(prGlueInfo);
		}
		/*                                               */
		register_set_p2p_mode_handler(set_p2p_mode_handler);
#endif
	}
	while (FALSE);

	return i4Status;
}				/*                    */


/*                                                                            */
/* 
                                                                               
                                                                         
 
                
*/
/*                                                                            */
static VOID wlanRemove(VOID)
{
	struct net_device *prDev = NULL;
	P_WLANDEV_INFO_T prWlandevInfo = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;

	DBGLOG(INIT, INFO, ("Remove wlan!\n"));


	/*                    */
	ASSERT(u4WlanDevNum <= CFG_MAX_WLAN_DEVICES);
	if (0 == u4WlanDevNum) {
		DBGLOG(INIT, INFO, ("0 == u4WlanDevNum\n"));
		return;
	}
	/*                                                 */
	register_set_p2p_mode_handler(NULL);

	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	prWlandevInfo = &arWlanDevInfo[u4WlanDevNum - 1];

	ASSERT(prDev);
	if (NULL == prDev) {
		DBGLOG(INIT, INFO, ("NULL == prDev\n"));
		return;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);
	if (NULL == prGlueInfo) {
		DBGLOG(INIT, INFO, ("NULL == prGlueInfo\n"));
		free_netdev(prDev);
		return;
	}

#if CFG_ENABLE_BT_OVER_WIFI
	if (prGlueInfo->rBowInfo.fgIsNetRegistered) {
		bowNotifyAllLinkDisconnected(prGlueInfo->prAdapter);
		/*                                          */
		kalMsleep(300);
	}
#endif

	/*                                                */
	glBusFreeIrq(prDev, *((P_GLUE_INFO_T *) netdev_priv(prDev)));

	kalMemSet(&(prGlueInfo->prAdapter->rWlanInfo), 0, sizeof(WLAN_INFO_T));

	flush_delayed_work(&workq); /*                                       */

	down(&g_halt_sem);
	g_u4HaltFlag = 1;

	/*                                                                           */
	prGlueInfo->u4Flag |= GLUE_FLAG_HALT;
	/*                     */
	wake_up_interruptible(&prGlueInfo->waitq);
	/*                        */
	wait_for_completion_interruptible(&prGlueInfo->rHaltComp);

	DBGLOG(INIT, INFO, ("mtk_sdiod stopped\n"));

	/*                                          */
	prGlueInfo->main_thread = NULL;

#if CFG_ENABLE_BT_OVER_WIFI
	if (prGlueInfo->rBowInfo.fgIsRegistered) {
		glUnregisterAmpc(prGlueInfo);
	}
#endif

	/*                                */
#ifdef WLAN_INCLUDE_PROC
	procRemoveProcfs(prDev, NIC_DEVICE_ID_LOW);
#endif				/*                   */

	/*                       */
	prAdapter = prGlueInfo->prAdapter;

	wlanAdapterStop(prAdapter);
	DBGLOG(INIT, INFO, ("Number of Stalled Packets = %ld\n", prGlueInfo->i4TxPendingFrameNum));

#if CFG_ENABLE_WIFI_DIRECT
	prGlueInfo->prAdapter->fgIsWlanLaunched = FALSE;
	if (prGlueInfo->prAdapter->fgIsP2PRegistered) {
		DBGLOG(INIT, INFO, ("p2pNetUnregister...\n"));
		p2pNetUnregister(prGlueInfo, FALSE);
		DBGLOG(INIT, INFO, ("p2pRemove...\n"));
		p2pRemove(prGlueInfo);
	}
#endif

	/*                       */
	glBusRelease(prDev);

	up(&g_halt_sem);

	/*                           */
	wlanNetUnregister(prDev->ieee80211_ptr);

	/*                          */
	wlanNetDestroy(prDev->ieee80211_ptr);
	prDev = NULL;

#if defined(CONFIG_HAS_EARLYSUSPEND)
	glUnregisterEarlySuspend(&mt6620_early_suspend_desc);
#endif
	wlanUnregisterNotifier();

	return;
}				/*                     */

/*                                                                            */
/* 
                                                                                
                                                                       
                                         
 
                       
*/
/*                                                                            */
/*                      */
static int initWlan(void)
{
	int ret = 0, i;

	DBGLOG(INIT, INFO, ("initWlan\n"));

	/*                       */
	kalInitIOBuffer();

	ret = ((glRegisterBus(wlanProbe, wlanRemove) == WLAN_STATUS_SUCCESS) ? 0 : -EIO);

	if (ret == -EIO) {
		kalUninitIOBuffer();
		return ret;
	}

#if (CFG_CHIP_RESET_SUPPORT)
	glResetInit();
#endif

	/*                                                */
	register_set_dbg_level_handler(set_dbg_level_handler);

	/*                                            */
#if DBG
	for (i = 0; i < DBG_MODULE_NUM; i++) {
		aucDebugModule[i] = DBG_CLASS_MASK;	/*            */
	}
#else
	/*                           */
	for (i = 0; i < DBG_MODULE_NUM; i++) {
		aucDebugModule[i] = DBG_CLASS_ERROR |
		    DBG_CLASS_WARN |
		    DBG_CLASS_STATE | DBG_CLASS_EVENT | DBG_CLASS_TRACE | DBG_CLASS_INFO;
	}
	aucDebugModule[DBG_TX_IDX] &= ~(DBG_CLASS_EVENT | DBG_CLASS_TRACE | DBG_CLASS_INFO);
	aucDebugModule[DBG_RX_IDX] &= ~(DBG_CLASS_EVENT | DBG_CLASS_TRACE | DBG_CLASS_INFO);
	aucDebugModule[DBG_REQ_IDX] &= ~(DBG_CLASS_EVENT | DBG_CLASS_TRACE | DBG_CLASS_INFO);
	aucDebugModule[DBG_INTR_IDX] = 0;
	aucDebugModule[DBG_MEM_IDX] = 0;
#endif				/*     */

	return ret;
}				/*                   */


/*                                                                            */
/* 
                                                                               
                                                                              
                                                             
 
                
*/
/*                                                                            */
/*                      */
static VOID exitWlan(void)
{
	DBGLOG(INIT, INFO, ("exitWlan\n"));

	/*                                                  */
	register_set_dbg_level_handler(NULL);

#if CFG_CHIP_RESET_SUPPORT
	glResetUninit();
#endif

	glUnregisterBus(wlanRemove);

	/*                           */
	kalUninitIOBuffer();

	DBGLOG(INIT, INFO, ("exitWlan\n"));

	return;
}				/*                   */


#ifdef MTK_WCN_REMOVE_KERNEL_MODULE
int mtk_wcn_wlan_6620_init(void)
{
	return initWlan();
}

void mtk_wcn_wlan_6620_exit(void)
{
	return exitWlan();
}
EXPORT_SYMBOL(mtk_wcn_wlan_6620_init);
EXPORT_SYMBOL(mtk_wcn_wlan_6620_exit);
#else
module_init(initWlan);
module_exit(exitWlan);
#endif
