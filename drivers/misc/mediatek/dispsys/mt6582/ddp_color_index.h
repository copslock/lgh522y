
#include "ddp_drv.h"

static DISPLAY_PQ_T pqindex =
{
GLOBAL_SAT   :
{0x80,0x84,0x88,0x8c,0x90,0x94,0x98,0x9c,0xa0,0xa5}, //   

PURP_TONE_S  :
{//       
{//           
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	}, 

	{// 
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80}
	},

	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80}
	},

	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80}
	},

	{//   
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x14, 0x14, 0x14},
		{0x32, 0x32, 0x32}
	}, 

	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1a, 0x1e, 0x18},
		{0x36, 0x3c, 0x30}
	},
	
	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x14, 0x14, 0x14},
		{0x32, 0x32, 0x32}
	},

	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x18, 0x1e, 0x13},
		{0x30, 0x3c, 0x26}
	},
	
	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x18, 0x1e, 0x13},
		{0x30, 0x3c, 0x26}
	},
	
	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x18, 0x1e, 0x13},
		{0x30, 0x3c, 0x26}
	},

	{//  
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x18, 0x1e, 0x13},
		{0x30, 0x3c, 0x26}
	},
        
	{//   
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80}
	},
	
	{//   
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80}
	}
},
SKIN_TONE_S:
{//           
	{//           
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x14, 0x14,  0x14, 0x14,  0x14,  0x14,  0x14, 0x14},
		{0x32, 0x32,  0x32, 0x32,  0x32,  0x32,  0x32, 0x32}
	},

	{// 
		{0x89, 0x89,  0x89, 0x89,  0x89,  0x89,  0x89, 0x80},
		{0x86, 0x86,  0x86, 0x86,  0x86,  0x86,  0x86, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{// 
		{0x8b, 0x8e,  0x8e, 0x8e,  0x8e,  0x8e,  0x8e, 0x80},
		{0x8b, 0x8c,  0x8c, 0x8c,  0x8c,  0x8c,  0x8c, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{// 
		{0x8d, 0x92,  0x92, 0x92,  0x92,  0x92,  0x92, 0x80},
		{0x8b, 0x8e,  0x8e, 0x8e,  0x8e,  0x8e,  0x8e, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},

	{// 
		{0x8f, 0x93,  0x97, 0x97,  0x97,  0x97,  0x97, 0x80},
		{0x8b, 0x8e,  0x8f, 0x8f,  0x8f,  0x8f,  0x8f, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{// 
		{0x8f, 0x97,  0x9d, 0x9d,  0x9d,  0x9d,  0x98, 0x80},
		{0x8b, 0x90,  0x92, 0x92,  0x92,  0x92,  0x92, 0x80},
		{0x80, 0x80,  0x80, 0x80,  0x80,  0x80,  0x80, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	{// 
		{0x95, 0x9e,  0xa3, 0xa3,  0xa3,  0xa3,  0xa0, 0x80},
		{0x8b, 0x93,  0x95, 0x95,  0x95,  0x95,  0x95, 0x80},
		{0x78, 0x76,  0x76, 0x76,  0x76,  0x76,  0x77, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{// 
		{0x99, 0xa3,  0xa8, 0xa8,  0xa8,  0xa8,  0xa6, 0x80},
		{0x90, 0x97,  0x99, 0x99,  0x99,  0x99,  0x99, 0x80},
		{0x75, 0x72,  0x72, 0x73,  0x73,  0x73,  0x73, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{// 
		{0x9c, 0xaa,  0xae, 0xae,  0xae,  0xae,  0xa7, 0x80},
		{0x92, 0x99,  0x9d, 0x9d,  0x9d,  0x9d,  0x9a, 0x80},
		{0x75, 0x6f,  0x6f, 0x6f,  0x6f,  0x6f,  0x6f, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{// 
		{0x9e, 0xab,  0xb4, 0xb4,  0xb4,  0xb4,  0xa9, 0x80},
		{0x92, 0x9b,  0xa0, 0xa0,  0xa0,  0xa0,  0x9a, 0x80},
		{0x73, 0x6c,  0x6b, 0x6b,  0x6b,  0x6b,  0x6b, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},

	{//  
		{0x9f, 0xae,  0xb9, 0xb9,  0xb9,  0xb9,  0xab, 0x80},
		{0x92, 0x9d,  0xa3, 0xa3,  0xa3,  0xa3,  0x9c, 0x80},
		{0x71, 0x69,  0x68, 0x68,  0x68,  0x68,  0x68, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{//  
		{0xa1, 0xb2,  0xbf, 0xbf,  0xbf,  0xbf,  0xae, 0x80},
		{0x95, 0xa0,  0xa7, 0xa7,  0xa7,  0xa7,  0x9e, 0x80},
		{0x6f, 0x67,  0x66, 0x66,  0x66,  0x66,  0x66, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{//  
		{0xa1, 0xb2,  0xc5, 0xc5,  0xc5,  0xc5,  0xae, 0x80},
		{0x98, 0xa6,  0xab, 0xab,  0xab,  0xab,  0xa3, 0x80},
		{0x6c, 0x65,  0x63, 0x63,  0x63,  0x63,  0x63, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},

	{//  
		{0xa1, 0xb8,  0xcb, 0xcb,  0xcb,  0xcb,  0xb0, 0x80},
		{0x9b, 0xa9,  0xaf, 0xaf,  0xaf,  0xaf,  0xa5, 0x80},
		{0x69, 0x62,  0x60, 0x60,  0x60,  0x60,  0x60, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{//  
		{0xa6, 0xbd,  0xd0, 0xd0,  0xd0,  0xd0,  0xbb, 0x80},
		{0xa0, 0xab,  0xb1, 0xb1,  0xb1,  0xb1,  0xa7, 0x80},
		{0x67, 0x60,  0x5e, 0x5e,  0x5e,  0x5e,  0x5e, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	{//  
		{0xa7, 0xc0,  0xd6, 0xd6,  0xd6,  0xd6,  0xbb, 0x80},
		{0xa0, 0xb0,  0xb5, 0xb5,  0xb5,  0xb5,  0xa7, 0x80},
		{0x66, 0x5f,  0x5c, 0x5c,  0x5c,  0x5c,  0x5c, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{//  
		{0xa7, 0xc3,  0xdc, 0xdc,  0xdc,  0xdc,  0xbb, 0x80},
		{0xa3, 0xb3,  0xb9, 0xb9,  0xb9,  0xb9,  0xa9, 0x80},
		{0x60, 0x5a,  0x5a, 0x5a,  0x5a,  0x5a,  0x5a, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{//  
		{0xa7, 0xc3,  0xe2, 0xe2,  0xe2,  0xe2,  0xbb, 0x80},
		{0xa7, 0xb6,  0xbd, 0xbd,  0xbd,  0xbd,  0xa9, 0x80},
		{0x5f, 0x57,  0x57, 0x57,  0x57,  0x57,  0x57, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	},
	
	{//  
		{0xa7, 0xc3,  0xe7, 0xe7,  0xe7,  0xe7,  0xbb, 0x80},
		{0xa7, 0xb6,  0xc0, 0xc0,  0xc0,  0xc0,  0xa9, 0x80},
		{0x5f, 0x55,  0x55, 0x55,  0x55,  0x55,  0x55, 0x80},
		{0x1e, 0x1e,  0x1e, 0x1e,  0x1e,  0x1e,  0x1e, 0x1e},
		{0x3c, 0x3c,  0x3c, 0x3c,  0x3c,  0x3c,  0x3c, 0x3c}
	}
},
GRASS_TONE_S:
{//            
	{//  
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x89, 0x89, 0x89, 0x89, 0x89, 0x80},
		{0x86, 0x86, 0x86, 0x86, 0x86, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x8e, 0x8e, 0x8e, 0x8e, 0x8b, 0x80},
		{0x8c, 0x8c, 0x8c, 0x8c, 0x8b, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x92, 0x92, 0x92, 0x92, 0x8c, 0x80},
		{0x8e, 0x8e, 0x8e, 0x8e, 0x8b, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x97, 0x97, 0x97, 0x97, 0x90, 0x80},
		{0x8f, 0x8f, 0x8f, 0x8f, 0x8b, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x9d, 0x9d, 0x9d, 0x99, 0x91, 0x80},
		{0x92, 0x92, 0x92, 0x92, 0x8d, 0x80},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0xa0, 0xa3, 0xa3, 0x9e, 0x95, 0x80},
		{0x95, 0x95, 0x95, 0x95, 0x8f, 0x80},
		{0x76, 0x76, 0x77, 0x78, 0x79, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{// 
		{0xa6, 0xa8, 0xa8, 0xa3, 0x9b, 0x80},
		{0x99, 0x99, 0x99, 0x99, 0x91, 0x80},
		{0x74, 0x72, 0x72, 0x72, 0x79, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0xaa, 0xae, 0xae, 0xa8, 0x9b, 0x80},
		{0x99, 0x9d, 0x9d, 0x9d, 0x94, 0x80},
		{0x71, 0x6f, 0x6f, 0x71, 0x77, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0xab, 0xb4, 0xb4, 0xab, 0x9b, 0x80},
		{0x9e, 0xa0, 0xa0, 0x9d, 0x95, 0x80},
		{0x6c, 0x6b, 0x6b, 0x71, 0x76, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	{//  
		{0xad, 0xb9, 0xb9, 0xad, 0x9c, 0x80},
		{0x9f, 0xa3, 0xa3, 0x9f, 0x97, 0x80},
		{0x69, 0x68, 0x68, 0x6f, 0x74, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xb0, 0xbf, 0xbf, 0xb2, 0xa0, 0x80},
		{0xa0, 0xa7, 0xa7, 0xa2, 0x9b, 0x80},
		{0x67, 0x66, 0x66, 0x6d, 0x72, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xb8, 0xc5, 0xc5, 0xb8, 0xa3, 0x80},
		{0xa3, 0xab, 0xab, 0xa5, 0x9b, 0x80},
		{0x67, 0x63, 0x63, 0x6a, 0x71, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xbd, 0xcb, 0xcb, 0xbf, 0xa9, 0x80},
		{0xa7, 0xaf, 0xaf, 0xaa, 0x9e, 0x80},
		{0x63, 0x60, 0x60, 0x67, 0x6e, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xc1, 0xd0, 0xd0, 0xc5, 0xb2, 0x80},
		{0xa9, 0xb1, 0xb1, 0xac, 0xa0, 0x80},
		{0x62, 0x5e, 0x5e, 0x65, 0x6c, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xc5, 0xd6, 0xd6, 0xcd, 0xba, 0x80},
		{0xad, 0xb5, 0xb5, 0xb1, 0xa4, 0x80},
		{0x60, 0x5c, 0x5c, 0x63, 0x6c, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xc6, 0xdc, 0xdc, 0xd6, 0xbd, 0x80},
		{0xb0, 0xb9, 0xb9, 0xb3, 0xa7, 0x80},
		{0x5d, 0x5a, 0x5a, 0x61, 0x6a, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xc6, 0xe2, 0xe2, 0xdb, 0xbd, 0x80},
		{0xb2, 0xbd, 0xbd, 0xb6, 0xab, 0x80},
		{0x5c, 0x57, 0x57, 0x5f, 0x6a, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xc6, 0xe7, 0xe7, 0xdb, 0xbd, 0x80},
		{0xb2, 0xc0, 0xc0, 0xb6, 0xab, 0x80},
		{0x5a, 0x55, 0x55, 0x5f, 0x6a, 0x80},
		{0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c}
	}
},
SKY_TONE_S:
{//           
	{//         
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x89, 0x89, 0x89},
		{0x86, 0x86, 0x86},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	{// 
		{0x8e, 0x8e, 0x8e},
		{0x8a, 0x8c, 0x8a},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x8f, 0x92, 0x8f},
		{0x8b, 0x8e, 0x8b},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	{// 
		{0x92, 0x97, 0x91},
		{0x8b, 0x8f, 0x8b},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{// 
		{0x95, 0x9d, 0x95},
		{0x8d, 0x92, 0x8d},
		{0x80, 0x80, 0x80},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x97, 0xa3, 0x97},
		{0x8f, 0x95, 0x8f},
		{0x7b, 0x76, 0x7c},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x9c, 0xa8, 0x9c},
		{0x92, 0x99, 0x94},
		{0x79, 0x73, 0x7b},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0x9e, 0xae, 0x9e},
		{0x95, 0x9d, 0x95},
		{0x78, 0x6f, 0x76},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{// 
		{0xa1, 0xb4, 0xa1},
		{0x98, 0xa0, 0x98},
		{0x73, 0x6b, 0x73},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	{//   
		{0xa4, 0xb9, 0xa4},
		{0x9b, 0xa3, 0x9b},
		{0x70, 0x68, 0x70},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xa8, 0xbf, 0xa8},
		{0x9a, 0xa7, 0x9e},
		{0x6e, 0x66, 0x6e},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	{//  
		{0xab, 0xc5, 0xa9},
		{0x9d, 0xab, 0x9d},
		{0x6c, 0x63, 0x6c},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xb2, 0xcb, 0xb2},
		{0xa1, 0xaf, 0xa1},
		{0x69, 0x60, 0x69},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	{//  
		{0xb8, 0xd0, 0xb6},
		{0xa3, 0xb1, 0xa3},
		{0x67, 0x5e, 0x67},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},

	{//  
		{0xc0, 0xd6, 0xc0},
		{0xa8, 0xb5, 0xa8},
		{0x65, 0x5c, 0x65},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xc4, 0xdc, 0xc4},
		{0xad, 0xb9, 0xaf},
		{0x63, 0x5a, 0x63},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xc8, 0xe2, 0xc8},
		{0xb2, 0xbd, 0xb4},
		{0x60, 0x57, 0x60},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	},
	
	{//  
		{0xca, 0xe7, 0xca},
		{0xb2, 0xc0, 0xb4},
		{0x5f, 0x55, 0x5f},
		{0x1e, 0x1e, 0x1e},
		{0x3c, 0x3c, 0x3c}
	}
},

PURP_TONE_H :
{ 
//        
  {0x80, 0x80, 0x80},//    
	{0x80, 0x80, 0x80},//    
	{0x80, 0x80, 0x80},//    
  {0x80, 0x80, 0x80},//    
  {0x80, 0x80, 0x80},//    
  {0x80, 0x80, 0x80},//    
  {0x80, 0x80, 0x80},//      
  {0x80, 0x80, 0x80},//     
  {0x80, 0x80, 0x80},//    
  {0x80, 0x80, 0x80},//      
  {0x80, 0x80, 0x80},//     
	{0x80, 0x80, 0x80},//     
	{0x80, 0x80, 0x80},//     
  {0x80, 0x80, 0x80},//     
  {0x80, 0x80, 0x80},//     
  {0x80, 0x80, 0x80},//     
  {0x80, 0x80, 0x80},//      
  {0x80, 0x80, 0x80},//      
  {0x80, 0x80, 0x80}//     
},
	
SKIN_TONE_H:
{
//        
	{0x80, 0x80, 0x74, 0x6a, 0x63,0x67,  0x71, 0x80},//        
	{0x80, 0x80, 0x74, 0x69, 0x66,0x69,  0x71, 0x80},//         
	{0x80, 0x80, 0x75, 0x6b, 0x69,0x6b,  0x72, 0x80},//        
	{0x80, 0x80, 0x76, 0x6e, 0x6c,0x6d,  0x73, 0x80},//      
	{0x80, 0x80, 0x77, 0x70, 0x70,0x70,  0x76, 0x80},//        
	{0x80, 0x80, 0x79, 0x73, 0x73,0x73,  0x77, 0x80},//        
	{0x80, 0x80, 0x7a, 0x76, 0x76,0x76,  0x79, 0x80},//        
	{0x80, 0x80, 0x7b, 0x79, 0x79,0x79,  0x7a, 0x80},//      
  {0x80, 0x80, 0x7c, 0x7c, 0x7c,0x7c,  0x7d, 0x80},//      
  {0x80, 0x80, 0x80, 0x80, 0x80,0x80,  0x80, 0x80},//      
  {0x83, 0x83, 0x83, 0x83, 0x83,0x83,  0x83, 0x80},//          
	{0x84, 0x85, 0x86, 0x86, 0x86,0x86,  0x86, 0x80},//          
	{0x84, 0x87, 0x8a, 0x8a, 0x8a,0x8a,  0x8a, 0x80},//          
	{0x86, 0x8a, 0x8d, 0x8d, 0x8d,0x8d,  0x8b, 0x80},//      
	{0x87, 0x8c, 0x90, 0x90, 0x90,0x90,  0x8d, 0x80},//          
	{0x89, 0x90, 0x93, 0x93, 0x93,0x93,  0x8f, 0x80},//          
	{0x89, 0x90, 0x96, 0x96, 0x96,0x96,  0x8f, 0x80},//         
	{0x89, 0x90, 0x99, 0x99, 0x99,0x99,  0x90, 0x80},//     
  {0x8a, 0x90, 0x9c, 0x9c, 0x9c,0x99,  0x90, 0x80}//     
},

	
GRASS_TONE_H :
{
//          
	{0x74,0x69, 0x64, 0x68, 0x6f, 0x80},//    
	{0x76,0x6b, 0x66, 0x6b, 0x73, 0x80},//    
	{0x78,0x6d, 0x69, 0x6d, 0x75, 0x80},//    
	{0x79,0x6f, 0x6d, 0x70, 0x76, 0x80},//    
	{0x79,0x71, 0x70, 0x71, 0x77, 0x80},//    
	{0x7a,0x73, 0x73, 0x73, 0x78, 0x80},//    
	{0x7b,0x76, 0x76, 0x76, 0x7a, 0x80},//    
	{0x7c,0x7a, 0x7a, 0x7a, 0x7b, 0x80},//    
	{0x7d,0x7d, 0x7d, 0x7d, 0x7d, 0x80},//     
	{0x80,0x80, 0x80, 0x80, 0x80, 0x80},//     
	{0x83,0x83, 0x83, 0x83, 0x83, 0x80},//     
	{0x86,0x86, 0x86, 0x86, 0x85, 0x80},//     
	{0x8a,0x8a, 0x8a, 0x8a, 0x86, 0x80},//     
	{0x8d,0x8d, 0x8d, 0x8d, 0x88, 0x80},//     
	{0x8d,0x90, 0x90, 0x8f, 0x8a, 0x80},//     
	{0x8f,0x93, 0x93, 0x91, 0x8b, 0x80},//     
	{0x90,0x96, 0x96, 0x93, 0x8d, 0x80},//     
	{0x92,0x99, 0x99, 0x96, 0x8d, 0x80},//     
	{0x92,0x9c, 0x9c, 0x98, 0x8d, 0x80}//      
},

SKY_TONE_H:
{//       
  {0x70, 0x60, 0x70},//       
	{0x70, 0x63, 0x70},//       
	{0x73, 0x69, 0x72},//       
	{0x75, 0x6d, 0x74},//       
	{0x77, 0x70, 0x76},//       
	{0x7a, 0x73, 0x79},//       
	{0x7b, 0x76, 0x7b},//       
	{0x7c, 0x79, 0x7c},//       
	{0x7e, 0x7c, 0x7e},//       
	{0x80, 0x80, 0x80},//       
	{0x83, 0x83, 0x83},//       
	{0x85, 0x86, 0x85},//       
	{0x86, 0x8a, 0x86},//       
	{0x89, 0x8d, 0x89},//       
	{0x8b, 0x90, 0x8b},//       
	{0x8e, 0x93, 0x8e},//       
	{0x91, 0x96, 0x91},//       
	{0x93, 0x99, 0x93},//       
	{0x95, 0x9c, 0x95}//        
}

};

const DISPLAY_GAMMA_T gammaindex = 
{
entry:
{
    {
            0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,
        100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
        200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296,
        300, 304, 308, 312, 316, 320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392, 396,
        400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496,
        500, 504, 508, 512, 516, 520, 524, 528, 532, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596,
        600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640, 644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692, 696,
        700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744, 748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
        800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848, 852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896,
        900, 904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952, 956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996,
        1000, 1004, 1008, 1012, 1016, 1020, 1023
    },
    {
            0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,
        100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
        200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296,
        300, 304, 308, 312, 316, 320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392, 396,
        400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496,
        500, 504, 508, 512, 516, 520, 524, 528, 532, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596,
        600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640, 644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692, 696,
        700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744, 748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
        800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848, 852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896,
        900, 904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952, 956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996,
        1000, 1004, 1008, 1012, 1016, 1020, 1023
    },
    {
            0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,
        100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
        200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296,
        300, 304, 308, 312, 316, 320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392, 396,
        400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496,
        500, 504, 508, 512, 516, 520, 524, 528, 532, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596,
        600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640, 644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692, 696,
        700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744, 748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
        800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848, 852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896,
        900, 904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952, 956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996,
        1000, 1004, 1008, 1012, 1016, 1020, 1023
    }
}
};
