/*
 *  Copyright (c) 2005 The University of Waikato, Hamilton, New Zealand.
 *  Copyright (c) 2005 Ian McDonald <ian.mcdonald@jandi.co.nz>
 *  Copyright (c) 2005 Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *  Copyright (c) 2003 Nils-Erik Mattsson, Joacim Haggmark, Magnus Erixzon
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <linux/module.h>
#include "../../dccp.h"
#include "tfrc.h"

#define TFRC_CALC_X_ARRSIZE 500
#define TFRC_CALC_X_SPLIT   50000	/*                               */
#define TFRC_SMALLEST_P	    (TFRC_CALC_X_SPLIT/TFRC_CALC_X_ARRSIZE)

/*
                                                         

                                                                               
                                     

          
                                                                           
                                                                    

        
                                            
                                   
                                         
                                                                        
                                                                 
                                                          
                                                                 

                                                                        

          
                                                                    
                                                             

                               

         
                     
              

                                        

                                                        

                                                                              
                                                                          
                    
                            
                            
                                                                            

                                

                                                                               
                                                                                
                                                                              
                                                                               
                                                                     

                                                                

                                   
                                            
                                                         
                                                                   
     

                                                                        
                                                          
                                                     
                                                             
                                                           

                                                                      
                                                  
                                                   
                                                                            
                                                   
 */
static const u32 tfrc_calc_x_lookup[TFRC_CALC_X_ARRSIZE][2] = {
	{     37172,   8172 },
	{     53499,  11567 },
	{     66664,  14180 },
	{     78298,  16388 },
	{     89021,  18339 },
	{     99147,  20108 },
	{    108858,  21738 },
	{    118273,  23260 },
	{    127474,  24693 },
	{    136520,  26052 },
	{    145456,  27348 },
	{    154316,  28589 },
	{    163130,  29783 },
	{    171919,  30935 },
	{    180704,  32049 },
	{    189502,  33130 },
	{    198328,  34180 },
	{    207194,  35202 },
	{    216114,  36198 },
	{    225097,  37172 },
	{    234153,  38123 },
	{    243294,  39055 },
	{    252527,  39968 },
	{    261861,  40864 },
	{    271305,  41743 },
	{    280866,  42607 },
	{    290553,  43457 },
	{    300372,  44293 },
	{    310333,  45117 },
	{    320441,  45929 },
	{    330705,  46729 },
	{    341131,  47518 },
	{    351728,  48297 },
	{    362501,  49066 },
	{    373460,  49826 },
	{    384609,  50577 },
	{    395958,  51320 },
	{    407513,  52054 },
	{    419281,  52780 },
	{    431270,  53499 },
	{    443487,  54211 },
	{    455940,  54916 },
	{    468635,  55614 },
	{    481581,  56306 },
	{    494785,  56991 },
	{    508254,  57671 },
	{    521996,  58345 },
	{    536019,  59014 },
	{    550331,  59677 },
	{    564939,  60335 },
	{    579851,  60988 },
	{    595075,  61636 },
	{    610619,  62279 },
	{    626491,  62918 },
	{    642700,  63553 },
	{    659253,  64183 },
	{    676158,  64809 },
	{    693424,  65431 },
	{    711060,  66050 },
	{    729073,  66664 },
	{    747472,  67275 },
	{    766266,  67882 },
	{    785464,  68486 },
	{    805073,  69087 },
	{    825103,  69684 },
	{    845562,  70278 },
	{    866460,  70868 },
	{    887805,  71456 },
	{    909606,  72041 },
	{    931873,  72623 },
	{    954614,  73202 },
	{    977839,  73778 },
	{   1001557,  74352 },
	{   1025777,  74923 },
	{   1050508,  75492 },
	{   1075761,  76058 },
	{   1101544,  76621 },
	{   1127867,  77183 },
	{   1154739,  77741 },
	{   1182172,  78298 },
	{   1210173,  78852 },
	{   1238753,  79405 },
	{   1267922,  79955 },
	{   1297689,  80503 },
	{   1328066,  81049 },
	{   1359060,  81593 },
	{   1390684,  82135 },
	{   1422947,  82675 },
	{   1455859,  83213 },
	{   1489430,  83750 },
	{   1523671,  84284 },
	{   1558593,  84817 },
	{   1594205,  85348 },
	{   1630518,  85878 },
	{   1667543,  86406 },
	{   1705290,  86932 },
	{   1743770,  87457 },
	{   1782994,  87980 },
	{   1822973,  88501 },
	{   1863717,  89021 },
	{   1905237,  89540 },
	{   1947545,  90057 },
	{   1990650,  90573 },
	{   2034566,  91087 },
	{   2079301,  91600 },
	{   2124869,  92111 },
	{   2171279,  92622 },
	{   2218543,  93131 },
	{   2266673,  93639 },
	{   2315680,  94145 },
	{   2365575,  94650 },
	{   2416371,  95154 },
	{   2468077,  95657 },
	{   2520707,  96159 },
	{   2574271,  96660 },
	{   2628782,  97159 },
	{   2684250,  97658 },
	{   2740689,  98155 },
	{   2798110,  98651 },
	{   2856524,  99147 },
	{   2915944,  99641 },
	{   2976382, 100134 },
	{   3037850, 100626 },
	{   3100360, 101117 },
	{   3163924, 101608 },
	{   3228554, 102097 },
	{   3294263, 102586 },
	{   3361063, 103073 },
	{   3428966, 103560 },
	{   3497984, 104045 },
	{   3568131, 104530 },
	{   3639419, 105014 },
	{   3711860, 105498 },
	{   3785467, 105980 },
	{   3860253, 106462 },
	{   3936229, 106942 },
	{   4013410, 107422 },
	{   4091808, 107902 },
	{   4171435, 108380 },
	{   4252306, 108858 },
	{   4334431, 109335 },
	{   4417825, 109811 },
	{   4502501, 110287 },
	{   4588472, 110762 },
	{   4675750, 111236 },
	{   4764349, 111709 },
	{   4854283, 112182 },
	{   4945564, 112654 },
	{   5038206, 113126 },
	{   5132223, 113597 },
	{   5227627, 114067 },
	{   5324432, 114537 },
	{   5422652, 115006 },
	{   5522299, 115474 },
	{   5623389, 115942 },
	{   5725934, 116409 },
	{   5829948, 116876 },
	{   5935446, 117342 },
	{   6042439, 117808 },
	{   6150943, 118273 },
	{   6260972, 118738 },
	{   6372538, 119202 },
	{   6485657, 119665 },
	{   6600342, 120128 },
	{   6716607, 120591 },
	{   6834467, 121053 },
	{   6953935, 121514 },
	{   7075025, 121976 },
	{   7197752, 122436 },
	{   7322131, 122896 },
	{   7448175, 123356 },
	{   7575898, 123815 },
	{   7705316, 124274 },
	{   7836442, 124733 },
	{   7969291, 125191 },
	{   8103877, 125648 },
	{   8240216, 126105 },
	{   8378321, 126562 },
	{   8518208, 127018 },
	{   8659890, 127474 },
	{   8803384, 127930 },
	{   8948702, 128385 },
	{   9095861, 128840 },
	{   9244875, 129294 },
	{   9395760, 129748 },
	{   9548529, 130202 },
	{   9703198, 130655 },
	{   9859782, 131108 },
	{  10018296, 131561 },
	{  10178755, 132014 },
	{  10341174, 132466 },
	{  10505569, 132917 },
	{  10671954, 133369 },
	{  10840345, 133820 },
	{  11010757, 134271 },
	{  11183206, 134721 },
	{  11357706, 135171 },
	{  11534274, 135621 },
	{  11712924, 136071 },
	{  11893673, 136520 },
	{  12076536, 136969 },
	{  12261527, 137418 },
	{  12448664, 137867 },
	{  12637961, 138315 },
	{  12829435, 138763 },
	{  13023101, 139211 },
	{  13218974, 139658 },
	{  13417071, 140106 },
	{  13617407, 140553 },
	{  13819999, 140999 },
	{  14024862, 141446 },
	{  14232012, 141892 },
	{  14441465, 142339 },
	{  14653238, 142785 },
	{  14867346, 143230 },
	{  15083805, 143676 },
	{  15302632, 144121 },
	{  15523842, 144566 },
	{  15747453, 145011 },
	{  15973479, 145456 },
	{  16201939, 145900 },
	{  16432847, 146345 },
	{  16666221, 146789 },
	{  16902076, 147233 },
	{  17140429, 147677 },
	{  17381297, 148121 },
	{  17624696, 148564 },
	{  17870643, 149007 },
	{  18119154, 149451 },
	{  18370247, 149894 },
	{  18623936, 150336 },
	{  18880241, 150779 },
	{  19139176, 151222 },
	{  19400759, 151664 },
	{  19665007, 152107 },
	{  19931936, 152549 },
	{  20201564, 152991 },
	{  20473907, 153433 },
	{  20748982, 153875 },
	{  21026807, 154316 },
	{  21307399, 154758 },
	{  21590773, 155199 },
	{  21876949, 155641 },
	{  22165941, 156082 },
	{  22457769, 156523 },
	{  22752449, 156964 },
	{  23049999, 157405 },
	{  23350435, 157846 },
	{  23653774, 158287 },
	{  23960036, 158727 },
	{  24269236, 159168 },
	{  24581392, 159608 },
	{  24896521, 160049 },
	{  25214642, 160489 },
	{  25535772, 160929 },
	{  25859927, 161370 },
	{  26187127, 161810 },
	{  26517388, 162250 },
	{  26850728, 162690 },
	{  27187165, 163130 },
	{  27526716, 163569 },
	{  27869400, 164009 },
	{  28215234, 164449 },
	{  28564236, 164889 },
	{  28916423, 165328 },
	{  29271815, 165768 },
	{  29630428, 166208 },
	{  29992281, 166647 },
	{  30357392, 167087 },
	{  30725779, 167526 },
	{  31097459, 167965 },
	{  31472452, 168405 },
	{  31850774, 168844 },
	{  32232445, 169283 },
	{  32617482, 169723 },
	{  33005904, 170162 },
	{  33397730, 170601 },
	{  33792976, 171041 },
	{  34191663, 171480 },
	{  34593807, 171919 },
	{  34999428, 172358 },
	{  35408544, 172797 },
	{  35821174, 173237 },
	{  36237335, 173676 },
	{  36657047, 174115 },
	{  37080329, 174554 },
	{  37507197, 174993 },
	{  37937673, 175433 },
	{  38371773, 175872 },
	{  38809517, 176311 },
	{  39250924, 176750 },
	{  39696012, 177190 },
	{  40144800, 177629 },
	{  40597308, 178068 },
	{  41053553, 178507 },
	{  41513554, 178947 },
	{  41977332, 179386 },
	{  42444904, 179825 },
	{  42916290, 180265 },
	{  43391509, 180704 },
	{  43870579, 181144 },
	{  44353520, 181583 },
	{  44840352, 182023 },
	{  45331092, 182462 },
	{  45825761, 182902 },
	{  46324378, 183342 },
	{  46826961, 183781 },
	{  47333531, 184221 },
	{  47844106, 184661 },
	{  48358706, 185101 },
	{  48877350, 185541 },
	{  49400058, 185981 },
	{  49926849, 186421 },
	{  50457743, 186861 },
	{  50992759, 187301 },
	{  51531916, 187741 },
	{  52075235, 188181 },
	{  52622735, 188622 },
	{  53174435, 189062 },
	{  53730355, 189502 },
	{  54290515, 189943 },
	{  54854935, 190383 },
	{  55423634, 190824 },
	{  55996633, 191265 },
	{  56573950, 191706 },
	{  57155606, 192146 },
	{  57741621, 192587 },
	{  58332014, 193028 },
	{  58926806, 193470 },
	{  59526017, 193911 },
	{  60129666, 194352 },
	{  60737774, 194793 },
	{  61350361, 195235 },
	{  61967446, 195677 },
	{  62589050, 196118 },
	{  63215194, 196560 },
	{  63845897, 197002 },
	{  64481179, 197444 },
	{  65121061, 197886 },
	{  65765563, 198328 },
	{  66414705, 198770 },
	{  67068508, 199213 },
	{  67726992, 199655 },
	{  68390177, 200098 },
	{  69058085, 200540 },
	{  69730735, 200983 },
	{  70408147, 201426 },
	{  71090343, 201869 },
	{  71777343, 202312 },
	{  72469168, 202755 },
	{  73165837, 203199 },
	{  73867373, 203642 },
	{  74573795, 204086 },
	{  75285124, 204529 },
	{  76001380, 204973 },
	{  76722586, 205417 },
	{  77448761, 205861 },
	{  78179926, 206306 },
	{  78916102, 206750 },
	{  79657310, 207194 },
	{  80403571, 207639 },
	{  81154906, 208084 },
	{  81911335, 208529 },
	{  82672880, 208974 },
	{  83439562, 209419 },
	{  84211402, 209864 },
	{  84988421, 210309 },
	{  85770640, 210755 },
	{  86558080, 211201 },
	{  87350762, 211647 },
	{  88148708, 212093 },
	{  88951938, 212539 },
	{  89760475, 212985 },
	{  90574339, 213432 },
	{  91393551, 213878 },
	{  92218133, 214325 },
	{  93048107, 214772 },
	{  93883493, 215219 },
	{  94724314, 215666 },
	{  95570590, 216114 },
	{  96422343, 216561 },
	{  97279594, 217009 },
	{  98142366, 217457 },
	{  99010679, 217905 },
	{  99884556, 218353 },
	{ 100764018, 218801 },
	{ 101649086, 219250 },
	{ 102539782, 219698 },
	{ 103436128, 220147 },
	{ 104338146, 220596 },
	{ 105245857, 221046 },
	{ 106159284, 221495 },
	{ 107078448, 221945 },
	{ 108003370, 222394 },
	{ 108934074, 222844 },
	{ 109870580, 223294 },
	{ 110812910, 223745 },
	{ 111761087, 224195 },
	{ 112715133, 224646 },
	{ 113675069, 225097 },
	{ 114640918, 225548 },
	{ 115612702, 225999 },
	{ 116590442, 226450 },
	{ 117574162, 226902 },
	{ 118563882, 227353 },
	{ 119559626, 227805 },
	{ 120561415, 228258 },
	{ 121569272, 228710 },
	{ 122583219, 229162 },
	{ 123603278, 229615 },
	{ 124629471, 230068 },
	{ 125661822, 230521 },
	{ 126700352, 230974 },
	{ 127745083, 231428 },
	{ 128796039, 231882 },
	{ 129853241, 232336 },
	{ 130916713, 232790 },
	{ 131986475, 233244 },
	{ 133062553, 233699 },
	{ 134144966, 234153 },
	{ 135233739, 234608 },
	{ 136328894, 235064 },
	{ 137430453, 235519 },
	{ 138538440, 235975 },
	{ 139652876, 236430 },
	{ 140773786, 236886 },
	{ 141901190, 237343 },
	{ 143035113, 237799 },
	{ 144175576, 238256 },
	{ 145322604, 238713 },
	{ 146476218, 239170 },
	{ 147636442, 239627 },
	{ 148803298, 240085 },
	{ 149976809, 240542 },
	{ 151156999, 241000 },
	{ 152343890, 241459 },
	{ 153537506, 241917 },
	{ 154737869, 242376 },
	{ 155945002, 242835 },
	{ 157158929, 243294 },
	{ 158379673, 243753 },
	{ 159607257, 244213 },
	{ 160841704, 244673 },
	{ 162083037, 245133 },
	{ 163331279, 245593 },
	{ 164586455, 246054 },
	{ 165848586, 246514 },
	{ 167117696, 246975 },
	{ 168393810, 247437 },
	{ 169676949, 247898 },
	{ 170967138, 248360 },
	{ 172264399, 248822 },
	{ 173568757, 249284 },
	{ 174880235, 249747 },
	{ 176198856, 250209 },
	{ 177524643, 250672 },
	{ 178857621, 251136 },
	{ 180197813, 251599 },
	{ 181545242, 252063 },
	{ 182899933, 252527 },
	{ 184261908, 252991 },
	{ 185631191, 253456 },
	{ 187007807, 253920 },
	{ 188391778, 254385 },
	{ 189783129, 254851 },
	{ 191181884, 255316 },
	{ 192588065, 255782 },
	{ 194001698, 256248 },
	{ 195422805, 256714 },
	{ 196851411, 257181 },
	{ 198287540, 257648 },
	{ 199731215, 258115 },
	{ 201182461, 258582 },
	{ 202641302, 259050 },
	{ 204107760, 259518 },
	{ 205581862, 259986 },
	{ 207063630, 260454 },
	{ 208553088, 260923 },
	{ 210050262, 261392 },
	{ 211555174, 261861 },
	{ 213067849, 262331 },
	{ 214588312, 262800 },
	{ 216116586, 263270 },
	{ 217652696, 263741 },
	{ 219196666, 264211 },
	{ 220748520, 264682 },
	{ 222308282, 265153 },
	{ 223875978, 265625 },
	{ 225451630, 266097 },
	{ 227035265, 266569 },
	{ 228626905, 267041 },
	{ 230226576, 267514 },
	{ 231834302, 267986 },
	{ 233450107, 268460 },
	{ 235074016, 268933 },
	{ 236706054, 269407 },
	{ 238346244, 269881 },
	{ 239994613, 270355 },
	{ 241651183, 270830 },
	{ 243315981, 271305 }
};

/*                                                           */
static inline u32 tfrc_binsearch(u32 fval, u8 small)
{
	u32 try, low = 0, high = TFRC_CALC_X_ARRSIZE - 1;

	while (low < high) {
		try = (low + high) / 2;
		if (fval <= tfrc_calc_x_lookup[try][small])
			high = try;
		else
			low  = try + 1;
	}
	return high;
}

/* 
                                                                      
                                    
                                                                    
                                             
  
                                                             
 */
u32 tfrc_calc_x(u16 s, u32 R, u32 p)
{
	u16 index;
	u32 f;
	u64 result;

	/*                                                       */
	BUG_ON(p >  1000000);		/*                          */
	BUG_ON(p == 0);			/*                          */
	if (R == 0) {			/*                          */
		DCCP_CRIT("WARNING: RTT is 0, returning maximum X_calc.");
		return ~0U;
	}

	if (p <= TFRC_CALC_X_SPLIT)		{     /*                      */
		if (p < TFRC_SMALLEST_P) {	      /*                      */
			DCCP_WARN("Value of p (%d) below resolution. "
				  "Substituting %d\n", p, TFRC_SMALLEST_P);
			index = 0;
		} else				      /*                      */
			index =  p/TFRC_SMALLEST_P - 1;

		f = tfrc_calc_x_lookup[index][1];

	} else {				      /*                      */
		index = p/(1000000/TFRC_CALC_X_ARRSIZE) - 1;

		f = tfrc_calc_x_lookup[index][0];
	}

	/*
                                               
                                                                       
                                                                       
                                                                        
                                                                     
                                                                     
                                                                         
  */
	result = scaled_div(s, R);
	return scaled_div32(result, f);
}

/* 
                                                           
                                                       
  
                                                       
 */
u32 tfrc_calc_x_reverse_lookup(u32 fvalue)
{
	int index;

	if (fvalue == 0)	/*                           */
		return 0;

	/*              */
	if (fvalue < tfrc_calc_x_lookup[0][1]) {
		DCCP_WARN("fvalue %u smaller than resolution\n", fvalue);
		return TFRC_SMALLEST_P;
	}
	if (fvalue > tfrc_calc_x_lookup[TFRC_CALC_X_ARRSIZE - 1][0]) {
		DCCP_WARN("fvalue %u exceeds bounds!\n", fvalue);
		return 1000000;
	}

	if (fvalue <= tfrc_calc_x_lookup[TFRC_CALC_X_ARRSIZE - 1][1]) {
		index = tfrc_binsearch(fvalue, 1);
		return (index + 1) * TFRC_CALC_X_SPLIT / TFRC_CALC_X_ARRSIZE;
	}

	/*                                                  */
	index = tfrc_binsearch(fvalue, 0);
	return (index + 1) * 1000000 / TFRC_CALC_X_ARRSIZE;
}

/* 
                                                                             
                                                                              
                                                                                
 */
u32 tfrc_invert_loss_event_rate(u32 loss_event_rate)
{
	if (loss_event_rate == UINT_MAX)		/*                   */
		return 0;
	if (unlikely(loss_event_rate == 0))		/*                   */
		return 1000000;
	return max_t(u32, scaled_div(1, loss_event_rate), TFRC_SMALLEST_P);
}
