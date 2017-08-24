#define QLA_MODEL_NAMES		0x5C

/*
                                        
 */
static char *qla2x00_model_name[QLA_MODEL_NAMES*2] = {
	"QLA2340",	"133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"QLA2342",	"133MHz PCI-X to 2Gb FC, Dual Channel",		/*       */
	"QLA2344",	"133MHz PCI-X to 2Gb FC, Quad Channel",		/*       */
	"QCP2342",	"cPCI to 2Gb FC, Dual Channel",			/*       */
	"QSB2340",	"SBUS to 2Gb FC, Single Channel",		/*       */
	"QSB2342",	"SBUS to 2Gb FC, Dual Channel",			/*       */
	"QLA2310",	"Sun 66MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"QLA2332",	"Sun 66MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"QCP2332",	"Sun cPCI to 2Gb FC, Dual Channel",		/*       */
	"QCP2340",	"cPCI to 2Gb FC, Single Channel",		/*       */
	"QLA2342",	"Sun 133MHz PCI-X to 2Gb FC, Dual Channel",	/*       */
	"QCP2342",	"Sun - cPCI to 2Gb FC, Dual Channel",		/*       */
	"QLA2350",	"133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"QLA2352",	"133MHz PCI-X to 2Gb FC, Dual Channel",		/*       */
	"QLA2352",	"Sun 133MHz PCI-X to 2Gb FC, Dual Channel",	/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QLA2360",	"133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"QLA2362",	"133MHz PCI-X to 2Gb FC, Dual Channel",		/*       */
	"QLE2360",	"PCI-Express to 2Gb FC, Single Channel",	/*       */
	"QLE2362",	"PCI-Express to 2Gb FC, Dual Channel",		/*       */
	"QLA200",	"133MHz PCI-X to 2Gb FC Optical",		/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QLA200P",	"133MHz PCI-X to 2Gb FC SFP",			/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QLA210",	"133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"EMC 250",	"133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"HP A7538A",	"HP 1p2g PCI-X to 2Gb FC, Single Channel",	/*       */
	"QLA210",	"Sun 133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	"QLA2460",	"PCI-X 2.0 to 4Gb FC, Single Channel",		/*       */
	"QLA2462",	"PCI-X 2.0 to 4Gb FC, Dual Channel",		/*       */
	"QMC2462",	"IBM eServer BC 4Gb FC Expansion Card",		/*       */
	"QMC2462S",	"IBM eServer BC 4Gb FC Expansion Card SFF",	/*       */
	"QLE2460",	"PCI-Express to 4Gb FC, Single Channel",	/*       */
	"QLE2462",	"PCI-Express to 4Gb FC, Dual Channel",		/*       */
	"QME2462",	"Dell BS PCI-Express to 4Gb FC, Dual Channel",	/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QEM2462",	"Sun Server I/O Module 4Gb FC, Dual Channel",	/*       */
	"QLE210",	"PCI-Express to 2Gb FC, Single Channel",	/*       */
	"QLE220",	"PCI-Express to 4Gb FC, Single Channel",	/*       */
	"QLA2460",	"Sun PCI-X 2.0 to 4Gb FC, Single Channel",	/*       */
	"QLA2462",	"Sun PCI-X 2.0 to 4Gb FC, Dual Channel",	/*       */
	"QLE2460",	"Sun PCI-Express to 2Gb FC, Single Channel",	/*       */
	"QLE2462",	"Sun PCI-Express to 4Gb FC, Single Channel",	/*       */
	"QEM2462",	"Server I/O Module 4Gb FC, Dual Channel",	/*       */
	"QLE2440",	"PCI-Express to 4Gb FC, Single Channel",	/*       */
	"QLE2464",	"PCI-Express to 4Gb FC, Quad Channel",		/*       */
	"QLA2440",	"PCI-X 2.0 to 4Gb FC, Single Channel",		/*       */
	"HP AE369A",	"PCI-X 2.0 to 4Gb FC, Dual Channel",		/*       */
	"QLA2340",	"Sun 133MHz PCI-X to 2Gb FC, Single Channel",	/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QMC2432M",	"IBM eServer BC 4Gb FC Expansion Card CFFE",	/*       */
	"QMC2422M",	"IBM eServer BC 4Gb FC Expansion Card CFFX",	/*       */
	"QLE220",	"Sun PCI-Express to 4Gb FC, Single Channel",	/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QME2462",	"PCI-Express to 4Gb FC, Dual Channel Mezz HBA",	/*       */
	"QMH2462",	"PCI-Express to 4Gb FC, Dual Channel Mezz HBA",	/*       */
	" ",		" ",						/*       */
	"QLE220",	"PCI-Express to 4Gb FC, Single Channel",	/*       */
	"QLE220",	"PCI-Express to 4Gb FC, Single Channel",	/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	" ",		" ",						/*       */
	"QME2472",	"Dell BS PCI-Express to 4Gb FC, Dual Channel",	/*       */
};
