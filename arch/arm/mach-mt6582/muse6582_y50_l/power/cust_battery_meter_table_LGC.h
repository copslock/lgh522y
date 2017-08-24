#ifndef _CUST_BATTERY_METER_TABLE_LGC_H
#define _CUST_BATTERY_METER_TABLE_LGC_H

#define Q_MAX_POS_50_LGC		1838
#define Q_MAX_POS_25_LGC		1848
#define Q_MAX_POS_0_LGC			1701
#define Q_MAX_NEG_10_LGC		1490

#define Q_MAX_POS_50_H_CURRENT_LGC	1824
#define Q_MAX_POS_25_H_CURRENT_LGC	1809
#define Q_MAX_POS_0_H_CURRENT_LGC	1297
#define Q_MAX_NEG_10_H_CURRENT_LGC	602

/*                                  */

#define BATTERY_PROFILE_SIZE_LGC sizeof(battery_profile_t2_LGC) / sizeof(BATTERY_PROFILE_STRUC);
//        
BATTERY_PROFILE_STRUC battery_profile_t0_LGC[] =
{
	{0	,	4275},	//              
	{2	,	4246},	//               
	{5	,	4220},	//               
	{7	,	4196},	//                
	{10	,	4172},	//                
	{12	,	4152},	//                
	{14	,	4127},	//                
	{17	,	4098},	//                
	{19	,	4066},	//                
	{22	,	4030},	//                
	{24	,	3996},	//                
	{27	,	3973},	//                
	{29	,	3958},	//                
	{31	,	3941},	//                
	{34	,	3922},	//                
	{36	,	3903},	//                
	{39	,	3883},	//                
	{41	,	3864},	//                
	{43	,	3850},	//                
	{46	,	3836},	//                
	{48	,	3827},	//                
	{51	,	3819},	//                
	{53	,	3810},	//                
	{56	,	3805},	//                
	{58	,	3800},	//                
	{60	,	3796},	//                
	{63	,	3794},	//                
	{65	,	3792},	//                
	{68	,	3789},	//                 
	{70	,	3784},	//                 
	{72	,	3779},	//                 
	{75	,	3771},	//                 
	{77	,	3763},	//                 
	{80	,	3752},	//                 
	{82	,	3741},	//                 
	{84	,	3726},	//                 
	{87	,	3714},	//                 
	{89	,	3706},	//                 
	{90	,	3700},	//                 
	{91	,	3698},	//                 
	{92	,	3696},	//                 
	{93	,	3693},	//                 
	{93	,	3693},	//                 
	{94	,	3692},	//                 
	{94	,	3692},	//                 
	{95	,	3689},	//                 
	{95	,	3689},	//                 
	{95	,	3688},	//                 
	{96	,	3687},	//                 
	{96	,	3686},	//                 
	{96	,	3685},	//                 
	{96	,	3684},	//                 
	{97	,	3684},	//                 
	{97	,	3681},	//                 
	{97	,	3680},	//                 
	{97	,	3679},	//                 
	{98	,	3677},	//                 
	{98	,	3677},	//                 
	{98	,	3675},	//                 
	{98	,	3674},	//                 
	{98	,	3671},	//                 
	{98	,	3671},	//                 
	{98	,	3667},	//                 
	{99	,	3665},	//                 
	{99	,	3663},	//                 
	{99	,	3660},	//                 
	{99	,	3657},	//                 
	{99	,	3655},	//                 
	{99	,	3653},	//                 
	{99	,	3650},	//                 
	{99	,	3646},	//                 
	{100	,	3644},	//                 
	{100	,	3640},	//                 
	{100	,	3637},	//                 
	{100	,	3635},	//                 
	{100	,	3631},	//                 
	{100	,	3628},	//                 
};

//      
BATTERY_PROFILE_STRUC battery_profile_t1_LGC[] =
{
	{0	,	4281},	//              
	{2	,	4237},	//               
	{4	,	4209},	//               
	{6	,	4184},	//                
	{8	,	4161},	//                
	{11	,	4140},	//                
	{13	,	4118},	//                
	{15	,	4098},	//                
	{17	,	4081},	//                
	{19	,	4060},	//                
	{21	,	4038},	//                
	{23	,	4019},	//                
	{25	,	4002},	//                
	{27	,	3984},	//                
	{30	,	3970},	//                
	{32	,	3957},	//                
	{34	,	3943},	//                
	{36	,	3925},	//                
	{38	,	3908},	//                
	{40	,	3888},	//                
	{42	,	3869},	//                
	{44	,	3852},	//                
	{47	,	3840},	//                
	{49	,	3831},	//                
	{51	,	3821},	//                
	{53	,	3813},	//                
	{55	,	3804},	//                
	{57	,	3799},	//                
	{59	,	3794},	//                 
	{61	,	3788},	//                 
	{63	,	3786},	//                 
	{65	,	3785},	//                 
	{68	,	3781},	//                 
	{70	,	3779},	//                 
	{72	,	3776},	//                 
	{74	,	3773},	//                 
	{76	,	3769},	//                 
	{78	,	3761},	//                 
	{80	,	3751},	//                 
	{82	,	3736},	//                 
	{85	,	3719},	//                 
	{87	,	3702},	//                 
	{89	,	3691},	//                 
	{91	,	3685},	//                 
	{93	,	3678},	//                 
	{95	,	3670},	//                 
	{96	,	3664},	//                 
	{96	,	3656},	//                 
	{97	,	3647},	//                 
	{97	,	3635},	//                 
	{97	,	3626},	//                 
	{98	,	3613},	//                 
	{98	,	3604},	//                 
	{98	,	3594},	//                 
	{98	,	3587},	//                 
	{98	,	3580},	//                 
	{99	,	3573},	//                 
	{99	,	3565},	//                 
	{99	,	3559},	//                 
	{99	,	3553},	//                 
	{99	,	3547},	//                 
	{99	,	3543},	//                 
	{99	,	3537},	//                 
	{99	,	3532},	//                 
	{99	,	3527},	//                 
	{99	,	3523},	//                 
	{99	,	3518},	//                 
	{100	,	3513},	//                 
	{100	,	3508},	//                 
	{100	,	3504},	//                 
	{100	,	3500},	//                 
	{100	,	3496},	//                 
	{100	,	3491},	//                 
	{100	,	3488},	//                 
	{100	,	3484},	//                 
	{100	,	3480},	//                 
	{100	,	3475},	//                 
};

//       
BATTERY_PROFILE_STRUC battery_profile_t2_LGC[] =
{
	{0	,	4321},	//              
	{2	,	4291},	//               
	{4	,	4267},	//               
	{6	,	4244},	//                
	{8	,	4221},	//                
	{10	,	4200},	//                
	{12	,	4179},	//                
	{14	,	4159},	//                
	{16	,	4138},	//                
	{18	,	4119},	//                
	{19	,	4098},	//                
	{21	,	4080},	//                
	{23	,	4066},	//                
	{25	,	4046},	//                
	{27	,	4021},	//                
	{29	,	4002},	//                
	{31	,	3989},	//                
	{33	,	3977},	//                
	{35	,	3963},	//                
	{37	,	3948},	//                
	{39	,	3936},	//                
	{41	,	3920},	//                
	{43	,	3906},	//                
	{45	,	3885},	//                
	{47	,	3866},	//                
	{49	,	3850},	//                
	{51	,	3839},	//                
	{53	,	3829},	//                
	{54	,	3820},	//                 
	{56	,	3814},	//                 
	{58	,	3806},	//                 
	{60	,	3800},	//                 
	{62	,	3795},	//                 
	{64	,	3789},	//                 
	{66	,	3785},	//                 
	{68	,	3780},	//                 
	{70	,	3777},	//                 
	{72	,	3774},	//                 
	{74	,	3773},	//                 
	{76	,	3769},	//                 
	{78	,	3762},	//                 
	{80	,	3753},	//                 
	{82	,	3746},	//                 
	{84	,	3733},	//                 
	{86	,	3717},	//                 
	{88	,	3697},	//                 
	{90	,	3690},	//                 
	{91	,	3689},	//                 
	{93	,	3686},	//                 
	{95	,	3677},	//                 
	{97	,	3621},	//                 
	{99	,	3493},	//                 
	{100	,	3302},	//                 
	{100	,	3231},	//                 
	{100	,	3187},	//                 
	{100	,	3159},	//                 
	{100	,	3139},	//                 
	{100	,	3123},	//                 
	{100	,	3114},	//                 
	{100	,	3106},	//                 
	{100	,	3103},	//                 
	{100	,	3096},	//                 
	{100	,	3093},	//                 
	{100	,	3085},	//                 
	{100	,	3086},	//                 
	{100	,	3087},	//                 
	{100	,	3087},	//                 
	{100	,	3086},	//                 
	{100	,	3080},	//                 
	{100	,	3075},	//                 
	{100	,	3075},	//                 
	{100	,	3071},	//                 
	{100	,	3070},	//                 
	{100	,	3070},	//                 
	{100	,	3067},	//                 
	{100	,	3064},	//                 
	{100	,	3063},	//                 
};

//       
BATTERY_PROFILE_STRUC battery_profile_t3_LGC[] =
{
	{0	,	4335},	//              
	{2	,	4307},	//               
	{4	,	4281},	//               
	{6	,	4257},	//                
	{8	,	4233},	//                
	{10	,	4210},	//                
	{12	,	4187},	//                
	{14	,	4166},	//                
	{16	,	4145},	//                
	{18	,	4123},	//                
	{20	,	4104},	//                
	{21	,	4083},	//                
	{23	,	4064},	//                
	{25	,	4046},	//                
	{27	,	4028},	//                
	{29	,	4009},	//                
	{31	,	3993},	//                
	{33	,	3976},	//                
	{35	,	3963},	//                
	{37	,	3948},	//                
	{39	,	3934},	//                
	{41	,	3921},	//                
	{43	,	3908},	//                
	{45	,	3893},	//                
	{47	,	3875},	//                
	{49	,	3852},	//                
	{51	,	3837},	//                
	{53	,	3827},	//                
	{55	,	3818},	//                 
	{57	,	3810},	//                 
	{59	,	3804},	//                 
	{61	,	3795},	//                 
	{63	,	3790},	//                 
	{65	,	3784},	//                 
	{66	,	3780},	//                 
	{68	,	3776},	//                 
	{70	,	3772},	//                 
	{72	,	3764},	//                 
	{74	,	3752},	//                 
	{76	,	3747},	//                 
	{78	,	3741},	//                 
	{80	,	3731},	//                 
	{82	,	3721},	//                 
	{84	,	3712},	//                 
	{86	,	3694},	//                 
	{88	,	3673},	//                 
	{90	,	3666},	//                 
	{92	,	3663},	//                 
	{94	,	3657},	//                 
	{96	,	3645},	//                 
	{98	,	3572},	//                 
	{100	,	3440},	//                 
	{100	,	3167},	//                 
	{100	,	3111},	//                 
	{100	,	3084},	//                 
	{100	,	3071},	//                 
	{100	,	3065},	//                 
	{100	,	3061},	//                 
	{100	,	3059},	//                 
	{100	,	3056},	//                 
	{100	,	3052},	//                 
	{100	,	3050},	//                 
	{100	,	3047},	//                 
	{100	,	3048},	//                 
	{100	,	3047},	//                 
	{100	,	3042},	//                 
	{100	,	3043},	//                 
	{100	,	3043},	//                 
	{100	,	3042},	//                 
	{100	,	3041},	//                 
	{100	,	3038},	//                 
	{100	,	3036},	//                 
	{100	,	3032},	//                 
	{100	,	3030},	//                 
	{100	,	3028},	//                 
	{100	,	3027},	//                 
	{100	,	3025},	//                 
};

//                                                                                     
BATTERY_PROFILE_STRUC battery_profile_temperature_LGC[] =
{
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
};

#define R_PROFILE_SIZE_LGC sizeof(r_profile_t2_LGC) / sizeof(R_PROFILE_STRUC);
//                                                             
//                              
//                                                             
//        
R_PROFILE_STRUC r_profile_t0_LGC[] =
{
	{360	,	4275},		//              
	{635	,	4246},		//               
	{655	,	4220},		//               
	{665	,	4196},		//                
	{668	,	4172},		//                
	{695	,	4152},		//                
	{750	,	4127},		//                
	{763	,	4098},		//                
	{788	,	4066},		//                
	{858	,	4030},		//                
	{1010	,	3996},		//                
	{1048	,	3973},		//                
	{1078	,	3958},		//                
	{1093	,	3941},		//                
	{1158	,	3922},		//                
	{1148	,	3903},		//                
	{1123	,	3883},		//                
	{1188	,	3864},		//                
	{1130	,	3850},		//                
	{1175	,	3836},		//                
	{1203	,	3827},		//                
	{1170	,	3819},		//                
	{1205	,	3810},		//                
	{1243	,	3805},		//                
	{1243	,	3800},		//                
	{1275	,	3796},		//                
	{1345	,	3794},		//                
	{1395	,	3792},		//                
	{1450	,	3789},		//                 
	{1433	,	3784},		//                 
	{1480	,	3779},		//                 
	{1583	,	3771},		//                 
	{1648	,	3763},		//                 
	{1645	,	3752},		//                 
	{1668	,	3741},		//                 
	{1728	,	3726},		//                 
	{1765	,	3714},		//                 
	{1765	,	3706},		//                 
	{1750	,	3700},		//                 
	{1748	,	3698},		//                 
	{1743	,	3696},		//                 
	{1735	,	3693},		//                 
	{1740	,	3693},		//                 
	{1735	,	3692},		//                 
	{1733	,	3692},		//                 
	{1725	,	3689},		//                 
	{1725	,	3689},		//                 
	{1723	,	3688},		//                 
	{1720	,	3687},		//                 
	{1718	,	3686},		//                 
	{1723	,	3685},		//                 
	{1710	,	3684},		//                 
	{1715	,	3684},		//                 
	{1708	,	3681},		//                 
	{1703	,	3680},		//                 
	{1705	,	3679},		//                 
	{1698	,	3677},		//                 
	{1698	,	3677},		//                 
	{1695	,	3675},		//                 
	{1693	,	3674},		//                 
	{1683	,	3671},		//                 
	{1693	,	3671},		//                 
	{1683	,	3667},		//                 
	{1665	,	3665},		//                 
	{1665	,	3663},		//                 
	{1650	,	3660},		//                 
	{1663	,	3657},		//                 
	{1655	,	3655},		//                 
	{1650	,	3653},		//                 
	{1630	,	3650},		//                 
	{1630	,	3646},		//                 
	{1623	,	3644},		//                 
	{1620	,	3640},		//                 
	{1610	,	3637},		//                 
	{1603	,	3635},		//                 
	{1578	,	3631},		//                 
	{1598	,	3628},		//                 
};

//      
R_PROFILE_STRUC r_profile_t1_LGC[] =
{
	{268	,	4281},		//              
	{475	,	4237},		//               
	{488	,	4209},		//               
	{495	,	4184},		//                
	{513	,	4161},		//                
	{518	,	4140},		//                
	{513	,	4118},		//                
	{518	,	4098},		//                
	{538	,	4081},		//                
	{558	,	4060},		//                
	{568	,	4038},		//                
	{578	,	4019},		//                
	{568	,	4002},		//                
	{570	,	3984},		//                
	{575	,	3970},		//                
	{593	,	3957},		//                
	{608	,	3943},		//                
	{610	,	3925},		//                
	{610	,	3908},		//                
	{598	,	3888},		//                
	{585	,	3869},		//                
	{570	,	3852},		//                
	{573	,	3840},		//                
	{588	,	3831},		//                
	{593	,	3821},		//                
	{605	,	3813},		//                
	{615	,	3804},		//                
	{625	,	3799},		//                
	{643	,	3794},		//                 
	{660	,	3788},		//                 
	{683	,	3786},		//                 
	{708	,	3785},		//                 
	{728	,	3781},		//                 
	{765	,	3779},		//                 
	{808	,	3776},		//                 
	{865	,	3773},		//                 
	{918	,	3769},		//                 
	{958	,	3761},		//                 
	{1010	,	3751},		//                 
	{1053	,	3736},		//                 
	{1113	,	3719},		//                 
	{1180	,	3702},		//                 
	{1273	,	3691},		//                 
	{1405	,	3685},		//                 
	{1543	,	3678},		//                 
	{1678	,	3670},		//                 
	{1663	,	3664},		//                 
	{1643	,	3656},		//                 
	{1620	,	3647},		//                 
	{1598	,	3635},		//                 
	{1570	,	3626},		//                 
	{1540	,	3613},		//                 
	{1525	,	3604},		//                 
	{1498	,	3594},		//                 
	{1470	,	3587},		//                 
	{1455	,	3580},		//                 
	{1450	,	3573},		//                 
	{1413	,	3565},		//                 
	{1430	,	3559},		//                 
	{1403	,	3553},		//                 
	{1378	,	3547},		//                 
	{1365	,	3543},		//                 
	{1378	,	3537},		//                 
	{1343	,	3532},		//                 
	{1328	,	3527},		//                 
	{1333	,	3523},		//                 
	{1310	,	3518},		//                 
	{1308	,	3513},		//                 
	{1278	,	3508},		//                 
	{1303	,	3504},		//                 
	{1305	,	3500},		//                 
	{1243	,	3496},		//                 
	{1240	,	3491},		//                 
	{1280	,	3488},		//                 
	{1248	,	3484},		//                 
	{1205	,	3480},		//                 
	{1213	,	3475},		//                 
};

//       
R_PROFILE_STRUC r_profile_t2_LGC[] =
{
	{150	,	4321},		//              
	{188	,	4291},		//               
	{188	,	4267},		//               
	{190	,	4244},		//                
	{193	,	4221},		//                
	{195	,	4200},		//                
	{198	,	4179},		//                
	{203	,	4159},		//                
	{205	,	4138},		//                
	{205	,	4119},		//                
	{208	,	4098},		//                
	{213	,	4080},		//                
	{225	,	4066},		//                
	{225	,	4046},		//                
	{223	,	4021},		//                
	{230	,	4002},		//                
	{235	,	3989},		//                
	{240	,	3977},		//                
	{245	,	3963},		//                
	{243	,	3948},		//                
	{250	,	3936},		//                
	{245	,	3920},		//                
	{243	,	3906},		//                
	{220	,	3885},		//                
	{205	,	3866},		//                
	{193	,	3850},		//                
	{190	,	3839},		//                
	{193	,	3829},		//                
	{195	,	3820},		//                 
	{198	,	3814},		//                 
	{198	,	3806},		//                 
	{203	,	3800},		//                 
	{208	,	3795},		//                 
	{205	,	3789},		//                 
	{215	,	3785},		//                 
	{215	,	3780},		//                 
	{220	,	3777},		//                 
	{223	,	3774},		//                 
	{233	,	3773},		//                 
	{233	,	3769},		//                 
	{225	,	3762},		//                 
	{220	,	3753},		//                 
	{230	,	3746},		//                 
	{228	,	3733},		//                 
	{230	,	3717},		//                 
	{230	,	3697},		//                 
	{235	,	3690},		//                 
	{268	,	3689},		//                 
	{310	,	3686},		//                 
	{360	,	3677},		//                 
	{408	,	3621},		//                 
	{530	,	3493},		//                 
	{760	,	3302},		//                 
	{580	,	3231},		//                 
	{470	,	3187},		//                 
	{405	,	3159},		//                 
	{348	,	3139},		//                 
	{320	,	3123},		//                 
	{285	,	3114},		//                 
	{273	,	3106},		//                 
	{270	,	3103},		//                 
	{255	,	3096},		//                 
	{250	,	3093},		//                 
	{253	,	3085},		//                 
	{238	,	3086},		//                 
	{233	,	3087},		//                 
	{233	,	3087},		//                 
	{225	,	3086},		//                 
	{233	,	3080},		//                 
	{225	,	3075},		//                 
	{215	,	3075},		//                 
	{228	,	3071},		//                 
	{223	,	3070},		//                 
	{193	,	3070},		//                 
	{230	,	3067},		//                 
	{213	,	3064},		//                 
	{230	,	3063},		//                 
};

//       
R_PROFILE_STRUC r_profile_t3_LGC[] =
{
	{150	,	4335},		//              
	{115	,	4307},		//               
	{113	,	4281},		//               
	{113	,	4257},		//                
	{115	,	4233},		//                
	{115	,	4210},		//                
	{115	,	4187},		//                
	{120	,	4166},		//                
	{123	,	4145},		//                
	{120	,	4123},		//                
	{130	,	4104},		//                
	{125	,	4083},		//                
	{130	,	4064},		//                
	{138	,	4046},		//                
	{138	,	4028},		//                
	{135	,	4009},		//                
	{140	,	3993},		//                
	{143	,	3976},		//                
	{150	,	3963},		//                
	{158	,	3948},		//                
	{158	,	3934},		//                
	{163	,	3921},		//                
	{170	,	3908},		//                
	{168	,	3893},		//                
	{155	,	3875},		//                
	{130	,	3852},		//                
	{120	,	3837},		//                
	{123	,	3827},		//                
	{123	,	3818},		//                 
	{123	,	3810},		//                 
	{133	,	3804},		//                 
	{125	,	3795},		//                 
	{135	,	3790},		//                 
	{138	,	3784},		//                 
	{143	,	3780},		//                 
	{145	,	3776},		//                 
	{148	,	3772},		//                 
	{140	,	3764},		//                 
	{125	,	3752},		//                 
	{130	,	3747},		//                 
	{133	,	3741},		//                 
	{133	,	3731},		//                 
	{130	,	3721},		//                 
	{135	,	3712},		//                 
	{135	,	3694},		//                 
	{130	,	3673},		//                 
	{128	,	3666},		//                 
	{143	,	3663},		//                 
	{150	,	3657},		//                 
	{165	,	3645},		//                 
	{163	,	3572},		//                 
	{185	,	3440},		//                 
	{328	,	3167},		//                 
	{283	,	3111},		//                 
	{218	,	3084},		//                 
	{180	,	3071},		//                 
	{165	,	3065},		//                 
	{158	,	3061},		//                 
	{153	,	3059},		//                 
	{150	,	3056},		//                 
	{145	,	3052},		//                 
	{138	,	3050},		//                 
	{135	,	3047},		//                 
	{128	,	3048},		//                 
	{138	,	3047},		//                 
	{133	,	3042},		//                 
	{120	,	3043},		//                 
	{128	,	3043},		//                 
	{130	,	3042},		//                 
	{123	,	3041},		//                 
	{130	,	3038},		//                 
	{125	,	3036},		//                 
	{130	,	3032},		//                 
	{135	,	3030},		//                 
	{130	,	3028},		//                 
	{123	,	3027},		//                 
	{123	,	3025},		//                 
};

//                                                                                     
R_PROFILE_STRUC r_profile_temperature_LGC[] =
{
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
};

#endif /*                                 */
