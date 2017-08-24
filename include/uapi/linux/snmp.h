/*
                       
  
                                                      
 */

#ifndef _LINUX_SNMP_H
#define _LINUX_SNMP_H

/*                         */
/*
                    
                                          
                                  
                                     
                                                                          
 */
enum
{
	IPSTATS_MIB_NUM = 0,
/*                                                                 */
	IPSTATS_MIB_INPKTS,			/*            */
	IPSTATS_MIB_INOCTETS,			/*          */
	IPSTATS_MIB_INDELIVERS,			/*            */
	IPSTATS_MIB_OUTFORWDATAGRAMS,		/*                  */
	IPSTATS_MIB_OUTPKTS,			/*             */
	IPSTATS_MIB_OUTOCTETS,			/*           */
/*              */
	IPSTATS_MIB_INHDRERRORS,		/*             */
	IPSTATS_MIB_INTOOBIGERRORS,		/*                */
	IPSTATS_MIB_INNOROUTES,			/*            */
	IPSTATS_MIB_INADDRERRORS,		/*              */
	IPSTATS_MIB_INUNKNOWNPROTOS,		/*                 */
	IPSTATS_MIB_INTRUNCATEDPKTS,		/*                 */
	IPSTATS_MIB_INDISCARDS,			/*            */
	IPSTATS_MIB_OUTDISCARDS,		/*             */
	IPSTATS_MIB_OUTNOROUTES,		/*             */
	IPSTATS_MIB_REASMTIMEOUT,		/*              */
	IPSTATS_MIB_REASMREQDS,			/*            */
	IPSTATS_MIB_REASMOKS,			/*          */
	IPSTATS_MIB_REASMFAILS,			/*            */
	IPSTATS_MIB_FRAGOKS,			/*         */
	IPSTATS_MIB_FRAGFAILS,			/*           */
	IPSTATS_MIB_FRAGCREATES,		/*             */
	IPSTATS_MIB_INMCASTPKTS,		/*             */
	IPSTATS_MIB_OUTMCASTPKTS,		/*              */
	IPSTATS_MIB_INBCASTPKTS,		/*             */
	IPSTATS_MIB_OUTBCASTPKTS,		/*              */
	IPSTATS_MIB_INMCASTOCTETS,		/*               */
	IPSTATS_MIB_OUTMCASTOCTETS,		/*                */
	IPSTATS_MIB_INBCASTOCTETS,		/*               */
	IPSTATS_MIB_OUTBCASTOCTETS,		/*                */
	IPSTATS_MIB_CSUMERRORS,			/*              */
	__IPSTATS_MIB_MAX
};

/*                      */
/*
                               
                                                          
 */
enum
{
	ICMP_MIB_NUM = 0,
	ICMP_MIB_INMSGS,			/*        */
	ICMP_MIB_INERRORS,			/*          */
	ICMP_MIB_INDESTUNREACHS,		/*                */
	ICMP_MIB_INTIMEEXCDS,			/*             */
	ICMP_MIB_INPARMPROBS,			/*             */
	ICMP_MIB_INSRCQUENCHS,			/*              */
	ICMP_MIB_INREDIRECTS,			/*             */
	ICMP_MIB_INECHOS,			/*         */
	ICMP_MIB_INECHOREPS,			/*            */
	ICMP_MIB_INTIMESTAMPS,			/*              */
	ICMP_MIB_INTIMESTAMPREPS,		/*                 */
	ICMP_MIB_INADDRMASKS,			/*             */
	ICMP_MIB_INADDRMASKREPS,		/*                */
	ICMP_MIB_OUTMSGS,			/*         */
	ICMP_MIB_OUTERRORS,			/*           */
	ICMP_MIB_OUTDESTUNREACHS,		/*                 */
	ICMP_MIB_OUTTIMEEXCDS,			/*              */
	ICMP_MIB_OUTPARMPROBS,			/*              */
	ICMP_MIB_OUTSRCQUENCHS,			/*               */
	ICMP_MIB_OUTREDIRECTS,			/*              */
	ICMP_MIB_OUTECHOS,			/*          */
	ICMP_MIB_OUTECHOREPS,			/*             */
	ICMP_MIB_OUTTIMESTAMPS,			/*               */
	ICMP_MIB_OUTTIMESTAMPREPS,		/*                  */
	ICMP_MIB_OUTADDRMASKS,			/*              */
	ICMP_MIB_OUTADDRMASKREPS,		/*                 */
	ICMP_MIB_CSUMERRORS,			/*              */
	__ICMP_MIB_MAX
};

#define __ICMPMSG_MIB_MAX 512	/*                                 */

/*                       */
/*
                        
 */
enum
{
	ICMP6_MIB_NUM = 0,
	ICMP6_MIB_INMSGS,			/*        */
	ICMP6_MIB_INERRORS,			/*          */
	ICMP6_MIB_OUTMSGS,			/*         */
	ICMP6_MIB_OUTERRORS,			/*           */
	ICMP6_MIB_CSUMERRORS,			/*              */
	__ICMP6_MIB_MAX
};

#define __ICMP6MSG_MIB_MAX 512 /*                                   */

/*                     */
/*
                              
                                           
 */
enum
{
	TCP_MIB_NUM = 0,
	TCP_MIB_RTOALGORITHM,			/*              */
	TCP_MIB_RTOMIN,				/*        */
	TCP_MIB_RTOMAX,				/*        */
	TCP_MIB_MAXCONN,			/*         */
	TCP_MIB_ACTIVEOPENS,			/*             */
	TCP_MIB_PASSIVEOPENS,			/*              */
	TCP_MIB_ATTEMPTFAILS,			/*              */
	TCP_MIB_ESTABRESETS,			/*             */
	TCP_MIB_CURRESTAB,			/*           */
	TCP_MIB_INSEGS,				/*        */
	TCP_MIB_OUTSEGS,			/*         */
	TCP_MIB_RETRANSSEGS,			/*             */
	TCP_MIB_INERRS,				/*        */
	TCP_MIB_OUTRSTS,			/*         */
	TCP_MIB_CSUMERRORS,			/*              */
	__TCP_MIB_MAX
};

/*                     */
/*
                              
                                           
 */
enum
{
	UDP_MIB_NUM = 0,
	UDP_MIB_INDATAGRAMS,			/*             */
	UDP_MIB_NOPORTS,			/*         */
	UDP_MIB_INERRORS,			/*          */
	UDP_MIB_OUTDATAGRAMS,			/*              */
	UDP_MIB_RCVBUFERRORS,			/*              */
	UDP_MIB_SNDBUFERRORS,			/*              */
	UDP_MIB_CSUMERRORS,			/*              */
	__UDP_MIB_MAX
};

/*                       */
enum
{
	LINUX_MIB_NUM = 0,
	LINUX_MIB_SYNCOOKIESSENT,		/*                */
	LINUX_MIB_SYNCOOKIESRECV,		/*                */
	LINUX_MIB_SYNCOOKIESFAILED,		/*                  */
	LINUX_MIB_EMBRYONICRSTS,		/*               */
	LINUX_MIB_PRUNECALLED,			/*             */
	LINUX_MIB_RCVPRUNED,			/*           */
	LINUX_MIB_OFOPRUNED,			/*           */
	LINUX_MIB_OUTOFWINDOWICMPS,		/*                  */
	LINUX_MIB_LOCKDROPPEDICMPS,		/*                  */
	LINUX_MIB_ARPFILTER,			/*           */
	LINUX_MIB_TIMEWAITED,			/*            */
	LINUX_MIB_TIMEWAITRECYCLED,		/*                  */
	LINUX_MIB_TIMEWAITKILLED,		/*                */
	LINUX_MIB_PAWSPASSIVEREJECTED,		/*                     */
	LINUX_MIB_PAWSACTIVEREJECTED,		/*                    */
	LINUX_MIB_PAWSESTABREJECTED,		/*                   */
	LINUX_MIB_DELAYEDACKS,			/*             */
	LINUX_MIB_DELAYEDACKLOCKED,		/*                  */
	LINUX_MIB_DELAYEDACKLOST,		/*                */
	LINUX_MIB_LISTENOVERFLOWS,		/*                 */
	LINUX_MIB_LISTENDROPS,			/*             */
	LINUX_MIB_TCPPREQUEUED,			/*              */
	LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG,	/*                          */
	LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE,	/*                           */
	LINUX_MIB_TCPPREQUEUEDROPPED,		/*                    */
	LINUX_MIB_TCPHPHITS,			/*           */
	LINUX_MIB_TCPHPHITSTOUSER,		/*                 */
	LINUX_MIB_TCPPUREACKS,			/*             */
	LINUX_MIB_TCPHPACKS,			/*           */
	LINUX_MIB_TCPRENORECOVERY,		/*                 */
	LINUX_MIB_TCPSACKRECOVERY,		/*                 */
	LINUX_MIB_TCPSACKRENEGING,		/*                 */
	LINUX_MIB_TCPFACKREORDER,		/*                */
	LINUX_MIB_TCPSACKREORDER,		/*                */
	LINUX_MIB_TCPRENOREORDER,		/*                */
	LINUX_MIB_TCPTSREORDER,			/*              */
	LINUX_MIB_TCPFULLUNDO,			/*             */
	LINUX_MIB_TCPPARTIALUNDO,		/*                */
	LINUX_MIB_TCPDSACKUNDO,			/*              */
	LINUX_MIB_TCPLOSSUNDO,			/*             */
	LINUX_MIB_TCPLOSTRETRANSMIT,		/*                   */
	LINUX_MIB_TCPRENOFAILURES,		/*                 */
	LINUX_MIB_TCPSACKFAILURES,		/*                 */
	LINUX_MIB_TCPLOSSFAILURES,		/*                 */
	LINUX_MIB_TCPFASTRETRANS,		/*                */
	LINUX_MIB_TCPFORWARDRETRANS,		/*                   */
	LINUX_MIB_TCPSLOWSTARTRETRANS,		/*                     */
	LINUX_MIB_TCPTIMEOUTS,			/*             */
	LINUX_MIB_TCPLOSSPROBES,		/*               */
	LINUX_MIB_TCPLOSSPROBERECOVERY,		/*                      */
	LINUX_MIB_TCPRENORECOVERYFAIL,		/*                     */
	LINUX_MIB_TCPSACKRECOVERYFAIL,		/*                     */
	LINUX_MIB_TCPSCHEDULERFAILED,		/*                    */
	LINUX_MIB_TCPRCVCOLLAPSED,		/*                 */
	LINUX_MIB_TCPDSACKOLDSENT,		/*                 */
	LINUX_MIB_TCPDSACKOFOSENT,		/*                 */
	LINUX_MIB_TCPDSACKRECV,			/*              */
	LINUX_MIB_TCPDSACKOFORECV,		/*                 */
	LINUX_MIB_TCPABORTONDATA,		/*                */
	LINUX_MIB_TCPABORTONCLOSE,		/*                 */
	LINUX_MIB_TCPABORTONMEMORY,		/*                  */
	LINUX_MIB_TCPABORTONTIMEOUT,		/*                   */
	LINUX_MIB_TCPABORTONLINGER,		/*                  */
	LINUX_MIB_TCPABORTFAILED,		/*                */
	LINUX_MIB_TCPMEMORYPRESSURES,		/*                    */
	LINUX_MIB_TCPSACKDISCARD,		/*                */
	LINUX_MIB_TCPDSACKIGNOREDOLD,		/*                   */
	LINUX_MIB_TCPDSACKIGNOREDNOUNDO,	/*                      */
	LINUX_MIB_TCPSPURIOUSRTOS,		/*                 */
	LINUX_MIB_TCPMD5NOTFOUND,		/*                */
	LINUX_MIB_TCPMD5UNEXPECTED,		/*                  */
	LINUX_MIB_SACKSHIFTED,
	LINUX_MIB_SACKMERGED,
	LINUX_MIB_SACKSHIFTFALLBACK,
	LINUX_MIB_TCPBACKLOGDROP,
	LINUX_MIB_TCPMINTTLDROP, /*          */
	LINUX_MIB_TCPDEFERACCEPTDROP,
	LINUX_MIB_IPRPFILTER, /*                                    */
	LINUX_MIB_TCPTIMEWAITOVERFLOW,		/*                     */
	LINUX_MIB_TCPREQQFULLDOCOOKIES,		/*                      */
	LINUX_MIB_TCPREQQFULLDROP,		/*                 */
	LINUX_MIB_TCPRETRANSFAIL,		/*                */
	LINUX_MIB_TCPRCVCOALESCE,		/*                */
	LINUX_MIB_TCPOFOQUEUE,			/*             */
	LINUX_MIB_TCPOFODROP,			/*            */
	LINUX_MIB_TCPOFOMERGE,			/*             */
	LINUX_MIB_TCPCHALLENGEACK,		/*                 */
	LINUX_MIB_TCPSYNCHALLENGE,		/*                 */
	LINUX_MIB_TCPFASTOPENACTIVE,		/*                   */
	LINUX_MIB_TCPFASTOPENPASSIVE,		/*                   */
	LINUX_MIB_TCPFASTOPENPASSIVEFAIL,	/*                        */
	LINUX_MIB_TCPFASTOPENLISTENOVERFLOW,	/*                           */
	LINUX_MIB_TCPFASTOPENCOOKIEREQD,	/*                       */
	LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES, /*                          */
	__LINUX_MIB_MAX
};

/*                            */
enum
{
	LINUX_MIB_XFRMNUM = 0,
	LINUX_MIB_XFRMINERROR,			/*             */
	LINUX_MIB_XFRMINBUFFERERROR,		/*                   */
	LINUX_MIB_XFRMINHDRERROR,		/*                */
	LINUX_MIB_XFRMINNOSTATES,		/*                */
	LINUX_MIB_XFRMINSTATEPROTOERROR,	/*                       */
	LINUX_MIB_XFRMINSTATEMODEERROR,		/*                      */
	LINUX_MIB_XFRMINSTATESEQERROR,		/*                     */
	LINUX_MIB_XFRMINSTATEEXPIRED,		/*                    */
	LINUX_MIB_XFRMINSTATEMISMATCH,		/*                     */
	LINUX_MIB_XFRMINSTATEINVALID,		/*                    */
	LINUX_MIB_XFRMINTMPLMISMATCH,		/*                    */
	LINUX_MIB_XFRMINNOPOLS,			/*              */
	LINUX_MIB_XFRMINPOLBLOCK,		/*                */
	LINUX_MIB_XFRMINPOLERROR,		/*                */
	LINUX_MIB_XFRMOUTERROR,			/*              */
	LINUX_MIB_XFRMOUTBUNDLEGENERROR,	/*                       */
	LINUX_MIB_XFRMOUTBUNDLECHECKERROR,	/*                         */
	LINUX_MIB_XFRMOUTNOSTATES,		/*                 */
	LINUX_MIB_XFRMOUTSTATEPROTOERROR,	/*                        */
	LINUX_MIB_XFRMOUTSTATEMODEERROR,	/*                       */
	LINUX_MIB_XFRMOUTSTATESEQERROR,		/*                      */
	LINUX_MIB_XFRMOUTSTATEEXPIRED,		/*                     */
	LINUX_MIB_XFRMOUTPOLBLOCK,		/*                 */
	LINUX_MIB_XFRMOUTPOLDEAD,		/*                */
	LINUX_MIB_XFRMOUTPOLERROR,		/*                 */
	LINUX_MIB_XFRMFWDHDRERROR,		/*                */
	LINUX_MIB_XFRMOUTSTATEINVALID,		/*                     */
	__LINUX_MIB_XFRMMAX
};

#endif	/*               */
