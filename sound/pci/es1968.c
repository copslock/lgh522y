/*
 *  Driver for ESS Maestro 1/2/2E Sound Card (started 21.8.99)
 *  Copyright (c) by Matze Braun <MatzeBraun@gmx.de>.
 *                   Takashi Iwai <tiwai@suse.de>
 *                  
 *  Most of the driver code comes from Zach Brown(zab@redhat.com)
 *	Alan Cox OSS Driver
 *  Rewritted from card-es1938.c source.
 *
 *  TODO:
 *   Perhaps Synth
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *
 *  Notes from Zach Brown about the driver code
 *
 *  Hardware Description
 *
 *	A working Maestro setup contains the Maestro chip wired to a 
 *	codec or 2.  In the Maestro we have the APUs, the ASSP, and the
 *	Wavecache.  The APUs can be though of as virtual audio routing
 *	channels.  They can take data from a number of sources and perform
 *	basic encodings of the data.  The wavecache is a storehouse for
 *	PCM data.  Typically it deals with PCI and interracts with the
 *	APUs.  The ASSP is a wacky DSP like device that ESS is loth
 *	to release docs on.  Thankfully it isn't required on the Maestro
 *	until you start doing insane things like FM emulation and surround
 *	encoding.  The codecs are almost always AC-97 compliant codecs, 
 *	but it appears that early Maestros may have had PT101 (an ESS
 *	part?) wired to them.  The only real difference in the Maestro
 *	families is external goop like docking capability, memory for
 *	the ASSP, and initialization differences.
 *
 *  Driver Operation
 *
 *	We only drive the APU/Wavecache as typical DACs and drive the
 *	mixers in the codecs.  There are 64 APUs.  We assign 6 to each
 *	/dev/dsp? device.  2 channels for output, and 4 channels for
 *	input.
 *
 *	Each APU can do a number of things, but we only really use
 *	3 basic functions.  For playback we use them to convert PCM
 *	data fetched over PCI by the wavecahche into analog data that
 *	is handed to the codec.  One APU for mono, and a pair for stereo.
 *	When in stereo, the combination of smarts in the APU and Wavecache
 *	decide which wavecache gets the left or right channel.
 *
 *	For record we still use the old overly mono system.  For each in
 *	coming channel the data comes in from the codec, through a 'input'
 *	APU, through another rate converter APU, and then into memory via
 *	the wavecache and PCI.  If its stereo, we mash it back into LRLR in
 *	software.  The pass between the 2 APUs is supposedly what requires us
 *	to have a 512 byte buffer sitting around in wavecache/memory.
 *
 *	The wavecache makes our life even more fun.  First off, it can
 *	only address the first 28 bits of PCI address space, making it
 *	useless on quite a few architectures.  Secondly, its insane.
 *	It claims to fetch from 4 regions of PCI space, each 4 meg in length.
 *	But that doesn't really work.  You can only use 1 region.  So all our
 *	allocations have to be in 4meg of each other.  Booo.  Hiss.
 *	So we have a module parameter, dsps_order, that is the order of
 *	the number of dsps to provide.  All their buffer space is allocated
 *	on open time.  The sonicvibes OSS routines we inherited really want
 *	power of 2 buffers, so we have all those next to each other, then
 *	512 byte regions for the recording wavecaches.  This ends up
 *	wasting quite a bit of memory.  The only fixes I can see would be 
 *	getting a kernel allocator that could work in zones, or figuring out
 *	just how to coerce the WP into doing what we want.
 *
 *	The indirection of the various registers means we have to spinlock
 *	nearly all register accesses.  We have the main register indirection
 *	like the wave cache, maestro registers, etc.  Then we have beasts
 *	like the APU interface that is indirect registers gotten at through
 *	the main maestro indirection.  Ouch.  We spinlock around the actual
 *	ports on a per card basis.  This means spinlock activity at each IO
 *	operation, but the only IO operation clusters are in non critical 
 *	paths and it makes the code far easier to follow.  Interrupts are
 *	blocked while holding the locks because the int handler has to
 *	get at some of them :(.  The mixer interface doesn't, however.
 *	We also have an OSS state lock that is thrown around in a few
 *	places.
 */

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/gameport.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/input.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/mpu401.h>
#include <sound/ac97_codec.h>
#include <sound/initval.h>

#ifdef CONFIG_SND_ES1968_RADIO
#include <sound/tea575x-tuner.h>
#endif

#define CARD_NAME "ESS Maestro1/2"
#define DRIVER_NAME "ES1968"

MODULE_DESCRIPTION("ESS Maestro");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ESS,Maestro 2e},"
		"{ESS,Maestro 2},"
		"{ESS,Maestro 1},"
		"{TerraTec,DMX}}");

#if defined(CONFIG_GAMEPORT) || (defined(MODULE) && defined(CONFIG_GAMEPORT_MODULE))
#define SUPPORT_JOYSTICK 1
#endif

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/*             */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/*                  */
static bool enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;	/*                  */
static int total_bufsize[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 1024 };
static int pcm_substreams_p[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 4 };
static int pcm_substreams_c[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 1 };
static int clock[SNDRV_CARDS];
static int use_pm[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 2};
static int enable_mpu[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 2};
#ifdef SUPPORT_JOYSTICK
static bool joystick[SNDRV_CARDS];
#endif
static int radio_nr[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = -1};

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for " CARD_NAME " soundcard.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for " CARD_NAME " soundcard.");
module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable " CARD_NAME " soundcard.");
module_param_array(total_bufsize, int, NULL, 0444);
MODULE_PARM_DESC(total_bufsize, "Total buffer size in kB.");
module_param_array(pcm_substreams_p, int, NULL, 0444);
MODULE_PARM_DESC(pcm_substreams_p, "PCM Playback substreams for " CARD_NAME " soundcard.");
module_param_array(pcm_substreams_c, int, NULL, 0444);
MODULE_PARM_DESC(pcm_substreams_c, "PCM Capture substreams for " CARD_NAME " soundcard.");
module_param_array(clock, int, NULL, 0444);
MODULE_PARM_DESC(clock, "Clock on " CARD_NAME " soundcard.  (0 = auto-detect)");
module_param_array(use_pm, int, NULL, 0444);
MODULE_PARM_DESC(use_pm, "Toggle power-management.  (0 = off, 1 = on, 2 = auto)");
module_param_array(enable_mpu, int, NULL, 0444);
MODULE_PARM_DESC(enable_mpu, "Enable MPU401.  (0 = off, 1 = on, 2 = auto)");
#ifdef SUPPORT_JOYSTICK
module_param_array(joystick, bool, NULL, 0444);
MODULE_PARM_DESC(joystick, "Enable joystick.");
#endif
module_param_array(radio_nr, int, NULL, 0444);
MODULE_PARM_DESC(radio_nr, "Radio device numbers");



#define NR_APUS			64
#define NR_APU_REGS		16

/*              */
#define NEC_VERSA_SUBID1	0x80581033
#define NEC_VERSA_SUBID2	0x803c1033

/*            */
#define ESS_FMT_STEREO     	0x01
#define ESS_FMT_16BIT      	0x02

#define DAC_RUNNING		1
#define ADC_RUNNING		2

/*                                         */

#define ESS_DISABLE_AUDIO	0x8000
#define ESS_ENABLE_SERIAL_IRQ	0x4000
#define IO_ADRESS_ALIAS		0x0020
#define MPU401_IRQ_ENABLE	0x0010
#define MPU401_IO_ENABLE	0x0008
#define GAME_IO_ENABLE		0x0004
#define FM_IO_ENABLE		0x0002
#define SB_IO_ENABLE		0x0001

/*                             */

#define PIC_SNOOP1		0x4000
#define PIC_SNOOP2		0x2000
#define SAFEGUARD		0x0800
#define DMA_CLEAR		0x0700
#define DMA_DDMA		0x0000
#define DMA_TDMA		0x0100
#define DMA_PCPCI		0x0200
#define POST_WRITE		0x0080
#define PCI_TIMING		0x0040
#define SWAP_LR			0x0020
#define SUBTR_DECODE		0x0002

/*                             */

#define SPDIF_CONFB		0x0100
#define HWV_CONFB		0x0080
#define DEBOUNCE		0x0040
#define GPIO_CONFB		0x0020
#define CHI_CONFB		0x0010
#define IDMA_CONFB		0x0008	/*      */
#define MIDI_FIX		0x0004	/*      */
#define IRQ_TO_ISA		0x0001	/*      */

/*                               */
#define	RINGB_2CODEC_ID_MASK	0x0003
#define RINGB_DIS_VALIDATION	0x0008
#define RINGB_EN_SPDIF		0x0010
#define	RINGB_EN_2CODEC		0x0020
#define RINGB_SING_BIT_DUAL	0x0040

/*                        */

/*                */
#define ESM_INDEX		0x02
#define ESM_DATA		0x00

/*                  */
#define ESM_AC97_INDEX		0x30
#define	ESM_AC97_DATA		0x32
#define ESM_RING_BUS_DEST	0x34
#define ESM_RING_BUS_CONTR_A	0x36
#define ESM_RING_BUS_CONTR_B	0x38
#define ESM_RING_BUS_SDO	0x3A

/*            */
#define WC_INDEX		0x10
#define WC_DATA			0x12
#define WC_CONTROL		0x14

/*       */
#define ASSP_INDEX		0x80
#define ASSP_MEMORY		0x82
#define ASSP_DATA		0x84
#define ASSP_CONTROL_A		0xA2
#define ASSP_CONTROL_B		0xA4
#define ASSP_CONTROL_C		0xA6
#define ASSP_HOSTW_INDEX	0xA8
#define ASSP_HOSTW_DATA		0xAA
#define ASSP_HOSTW_IRQ		0xAC
/*      */
#define ESM_MPU401_PORT		0x98
/*        */
#define ESM_PORT_HOST_IRQ	0x18

#define IDR0_DATA_PORT		0x00
#define IDR1_CRAM_POINTER	0x01
#define IDR2_CRAM_DATA		0x02
#define IDR3_WAVE_DATA		0x03
#define IDR4_WAVE_PTR_LOW	0x04
#define IDR5_WAVE_PTR_HI	0x05
#define IDR6_TIMER_CTRL		0x06
#define IDR7_WAVE_ROMRAM	0x07

#define WRITEABLE_MAP		0xEFFFFF
#define READABLE_MAP		0x64003F

/*              */

#define ESM_LEGACY_AUDIO_CONTROL 0x40
#define ESM_ACPI_COMMAND	0x54
#define ESM_CONFIG_A		0x50
#define ESM_CONFIG_B		0x52
#define ESM_DDMA		0x60

/*          */
#define ESM_BOB_ENABLE		0x0001
#define ESM_BOB_START		0x0001

/*                       */
#define ESM_RESET_MAESTRO	0x8000
#define ESM_RESET_DIRECTSOUND   0x4000
#define ESM_HIRQ_ClkRun		0x0100
#define ESM_HIRQ_HW_VOLUME	0x0040
#define ESM_HIRQ_HARPO		0x0030	/*              */
#define ESM_HIRQ_ASSP		0x0010
#define	ESM_HIRQ_DSIE		0x0004
#define ESM_HIRQ_MPU401		0x0002
#define ESM_HIRQ_SB		0x0001

/*                      */
#define ESM_MPU401_IRQ		0x02
#define ESM_SB_IRQ		0x01
#define ESM_SOUND_IRQ		0x04
#define	ESM_ASSP_IRQ		0x10
#define ESM_HWVOL_IRQ		0x40

#define ESS_SYSCLK		50000000
#define ESM_BOB_FREQ 		200
#define ESM_BOB_FREQ_MAX	800

#define ESM_FREQ_ESM1  		(49152000L / 1024L)	/*                    */
#define ESM_FREQ_ESM2  		(50000000L / 1024L)

/*                              */
#define ESM_APU_MODE_SHIFT	4
#define ESM_APU_MODE_MASK	(0xf << 4)
#define	ESM_APU_OFF		0x00
#define	ESM_APU_16BITLINEAR	0x01	/*                             */
#define	ESM_APU_16BITSTEREO	0x02	/*                             */
#define	ESM_APU_8BITLINEAR	0x03	/*                            */
#define	ESM_APU_8BITSTEREO	0x04	/*                            */
#define	ESM_APU_8BITDIFF	0x05	/*                                   */
#define	ESM_APU_DIGITALDELAY	0x06	/*                    */
#define	ESM_APU_DUALTAP		0x07	/*                 */
#define	ESM_APU_CORRELATOR	0x08	/*            */
#define	ESM_APU_INPUTMIXER	0x09	/*             */
#define	ESM_APU_WAVETABLE	0x0A	/*                 */
#define	ESM_APU_SRCONVERTOR	0x0B	/*                       */
#define	ESM_APU_16BITPINGPONG	0x0C	/*                                */
#define	ESM_APU_RESERVED1	0x0D	/*            */
#define	ESM_APU_RESERVED2	0x0E	/*            */
#define	ESM_APU_RESERVED3	0x0F	/*            */

/*          */
#define ESM_APU_FILTER_Q_SHIFT		0
#define ESM_APU_FILTER_Q_MASK		(3 << 0)
/*                      */
#define ESM_APU_FILTER_LESSQ	0x00
#define ESM_APU_FILTER_MOREQ	0x03

#define ESM_APU_FILTER_TYPE_SHIFT	2
#define ESM_APU_FILTER_TYPE_MASK	(3 << 2)
#define ESM_APU_ENV_TYPE_SHIFT		8
#define ESM_APU_ENV_TYPE_MASK		(3 << 8)
#define ESM_APU_ENV_STATE_SHIFT		10
#define ESM_APU_ENV_STATE_MASK		(3 << 10)
#define ESM_APU_END_CURVE		(1 << 12)
#define ESM_APU_INT_ON_LOOP		(1 << 13)
#define ESM_APU_DMA_ENABLE		(1 << 14)

/*          */
#define ESM_APU_SUBMIX_GROUP_SHIRT	0
#define ESM_APU_SUBMIX_GROUP_MASK	(7 << 0)
#define ESM_APU_SUBMIX_MODE		(1 << 3)
#define ESM_APU_6dB			(1 << 4)
#define ESM_APU_DUAL_EFFECT		(1 << 5)
#define ESM_APU_EFFECT_CHANNELS_SHIFT	6
#define ESM_APU_EFFECT_CHANNELS_MASK	(3 << 6)

/*          */
#define ESM_APU_STEP_SIZE_MASK		0x0fff

/*          */
#define ESM_APU_PHASE_SHIFT		0
#define ESM_APU_PHASE_MASK		(0xff << 0)
#define ESM_APU_WAVE64K_PAGE_SHIFT	8	/*                                */
#define ESM_APU_WAVE64K_PAGE_MASK	(0xff << 8)

/*                              */
/*                            */
/*                             */

/*          */
#define ESM_APU_EFFECT_GAIN_SHIFT	0
#define ESM_APU_EFFECT_GAIN_MASK	(0xff << 0)
#define ESM_APU_TREMOLO_DEPTH_SHIFT	8
#define ESM_APU_TREMOLO_DEPTH_MASK	(0xf << 8)
#define ESM_APU_TREMOLO_RATE_SHIFT	12
#define ESM_APU_TREMOLO_RATE_MASK	(0xf << 12)

/*          */
/*                         */
#define ESM_APU_AMPLITUDE_NOW_SHIFT	8
#define ESM_APU_AMPLITUDE_NOW_MASK	(0xff << 8)

/*          */
#define ESM_APU_POLAR_PAN_SHIFT		0
#define ESM_APU_POLAR_PAN_MASK		(0x3f << 0)
/*                   */
#define	ESM_APU_PAN_CENTER_CIRCLE		0x00
#define	ESM_APU_PAN_MIDDLE_RADIUS		0x01
#define	ESM_APU_PAN_OUTSIDE_RADIUS		0x02

#define ESM_APU_FILTER_TUNING_SHIFT	8
#define ESM_APU_FILTER_TUNING_MASK	(0xff << 8)

/*          */
#define ESM_APU_DATA_SRC_A_SHIFT	0
#define ESM_APU_DATA_SRC_A_MASK		(0x7f << 0)
#define ESM_APU_INV_POL_A		(1 << 7)
#define ESM_APU_DATA_SRC_B_SHIFT	8
#define ESM_APU_DATA_SRC_B_MASK		(0x7f << 8)
#define ESM_APU_INV_POL_B		(1 << 15)

#define ESM_APU_VIBRATO_RATE_SHIFT	0
#define ESM_APU_VIBRATO_RATE_MASK	(0xf << 0)
#define ESM_APU_VIBRATO_DEPTH_SHIFT	4
#define ESM_APU_VIBRATO_DEPTH_MASK	(0xf << 4)
#define ESM_APU_VIBRATO_PHASE_SHIFT	8
#define ESM_APU_VIBRATO_PHASE_MASK	(0xff << 8)

/*          */
#define ESM_APU_RADIUS_SELECT		(1 << 6)

/*                    */
#define	ESM_APU_FILTER_2POLE_LOPASS	0x00
#define	ESM_APU_FILTER_2POLE_BANDPASS	0x01
#define	ESM_APU_FILTER_2POLE_HIPASS	0x02
#define	ESM_APU_FILTER_1POLE_LOPASS	0x03
#define	ESM_APU_FILTER_1POLE_HIPASS	0x04
#define	ESM_APU_FILTER_OFF		0x05

/*               */
#define	ESM_APU_ATFP_AMPLITUDE			0x00
#define	ESM_APU_ATFP_TREMELO			0x01
#define	ESM_APU_ATFP_FILTER			0x02
#define	ESM_APU_ATFP_PAN			0x03

/*                */
#define	ESM_APU_ATFP_FLG_OFF			0x00
#define	ESM_APU_ATFP_FLG_WAIT			0x01
#define	ESM_APU_ATFP_FLG_DONE			0x02
#define	ESM_APU_ATFP_FLG_INPROCESS		0x03


/*                            */
#define ESM_MEM_ALIGN		0x1000
#define ESM_MIXBUF_SIZE		0x400

#define ESM_MODE_PLAY		0
#define ESM_MODE_CAPTURE	1


/*                       */
enum snd_enum_apu_type {
	ESM_APU_PCM_PLAY,
	ESM_APU_PCM_CAPTURE,
	ESM_APU_PCM_RATECONV,
	ESM_APU_FREE
};

/*           */
enum {
	TYPE_MAESTRO, TYPE_MAESTRO2, TYPE_MAESTRO2E
};

/*           */
struct esm_memory {
	struct snd_dma_buffer buf;
	int empty;	/*        */
	struct list_head list;
};

/*                  */
struct esschan {
	int running;

	u8 apu[4];
	u8 apu_mode[4];

	/*                             */
	struct esm_memory *memory;
	/*                      */
	struct esm_memory *mixbuf;

	unsigned int hwptr;	/*                             */
	unsigned int count;	/*                         */
	unsigned int dma_size;	/*                            */
	unsigned int frag_size;	/*                      */
	unsigned int wav_shift;
	u16 base[4];		/*                */

	/*                   */
	unsigned char fmt;
	int mode;	/*                    */

	int bob_freq;	/*                          */

	struct snd_pcm_substream *substream;

	/*             */
	struct list_head list;

#ifdef CONFIG_PM_SLEEP
	u16 wc_map[4];
#endif
};

struct es1968 {
	/*               */
	int total_bufsize;			/*          */

	int playback_streams, capture_streams;

	unsigned int clock;		/*       */
	/*                       */
	unsigned int in_measurement: 1;
	unsigned int measure_apu;
	unsigned int measure_lastpos;
	unsigned int measure_count;

	/*        */
	struct snd_dma_buffer dma;

	/*              */
	int irq;
	unsigned long io_port;
	int type;
	struct pci_dev *pci;
	struct snd_card *card;
	struct snd_pcm *pcm;
	int do_pm;		/*                          */

	/*                  */
	struct list_head buf_list;

	/*            */
	struct snd_ac97 *ac97;
	struct snd_rawmidi *rmidi;

	spinlock_t reg_lock;
	unsigned int in_suspend;

	/*               */
	u16 maestro_map[32];
	int bobclient;		/*                       */
	int bob_freq;		/*                 */
	struct mutex memory_mutex;	/*             */

	/*            */
	unsigned char apu[NR_APUS];

	/*                   */
	struct list_head substream_list;
	spinlock_t substream_lock;

#ifdef CONFIG_PM_SLEEP
	u16 apu_map[NR_APUS][NR_APU_REGS];
#endif

#ifdef SUPPORT_JOYSTICK
	struct gameport *gameport;
#endif

#ifdef CONFIG_SND_ES1968_INPUT
	struct input_dev *input_dev;
	char phys[64];			/*                      */
#else
	struct snd_kcontrol *master_switch; /*                        */
	struct snd_kcontrol *master_volume;
#endif
	struct work_struct hwvol_work;

#ifdef CONFIG_SND_ES1968_RADIO
	struct v4l2_device v4l2_dev;
	struct snd_tea575x tea;
#endif
};

static irqreturn_t snd_es1968_interrupt(int irq, void *dev_id);

static DEFINE_PCI_DEVICE_TABLE(snd_es1968_ids) = {
	/*           */
        { 0x1285, 0x0100, PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_MULTIMEDIA_AUDIO << 8, 0xffff00, TYPE_MAESTRO },
	/*           */
	{ 0x125d, 0x1968, PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_MULTIMEDIA_AUDIO << 8, 0xffff00, TYPE_MAESTRO2 },
	/*            */
        { 0x125d, 0x1978, PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_MULTIMEDIA_AUDIO << 8, 0xffff00, TYPE_MAESTRO2E },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, snd_es1968_ids);

/*                      
                        
                       */

/*             */
static void __maestro_write(struct es1968 *chip, u16 reg, u16 data)
{
	outw(reg, chip->io_port + ESM_INDEX);
	outw(data, chip->io_port + ESM_DATA);
	chip->maestro_map[reg] = data;
}

static inline void maestro_write(struct es1968 *chip, u16 reg, u16 data)
{
	unsigned long flags;
	spin_lock_irqsave(&chip->reg_lock, flags);
	__maestro_write(chip, reg, data);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

/*             */
static u16 __maestro_read(struct es1968 *chip, u16 reg)
{
	if (READABLE_MAP & (1 << reg)) {
		outw(reg, chip->io_port + ESM_INDEX);
		chip->maestro_map[reg] = inw(chip->io_port + ESM_DATA);
	}
	return chip->maestro_map[reg];
}

static inline u16 maestro_read(struct es1968 *chip, u16 reg)
{
	unsigned long flags;
	u16 result;
	spin_lock_irqsave(&chip->reg_lock, flags);
	result = __maestro_read(chip, reg);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
	return result;
}

/*                                   */
static int snd_es1968_ac97_wait(struct es1968 *chip)
{
	int timeout = 100000;

	while (timeout-- > 0) {
		if (!(inb(chip->io_port + ESM_AC97_INDEX) & 1))
			return 0;
		cond_resched();
	}
	snd_printd("es1968: ac97 timeout\n");
	return 1; /*         */
}

static int snd_es1968_ac97_wait_poll(struct es1968 *chip)
{
	int timeout = 100000;

	while (timeout-- > 0) {
		if (!(inb(chip->io_port + ESM_AC97_INDEX) & 1))
			return 0;
	}
	snd_printd("es1968: ac97 timeout\n");
	return 1; /*         */
}

static void snd_es1968_ac97_write(struct snd_ac97 *ac97, unsigned short reg, unsigned short val)
{
	struct es1968 *chip = ac97->private_data;

	snd_es1968_ac97_wait(chip);

	/*               */
	outw(val, chip->io_port + ESM_AC97_DATA);
	/*          */
	outb(reg, chip->io_port + ESM_AC97_INDEX);
	/*          */
}

static unsigned short snd_es1968_ac97_read(struct snd_ac97 *ac97, unsigned short reg)
{
	u16 data = 0;
	struct es1968 *chip = ac97->private_data;

	snd_es1968_ac97_wait(chip);

	outb(reg | 0x80, chip->io_port + ESM_AC97_INDEX);
	/*          */

	if (!snd_es1968_ac97_wait_poll(chip)) {
		data = inw(chip->io_port + ESM_AC97_DATA);
		/*          */
	}

	return data;
}

/*             */
static void apu_index_set(struct es1968 *chip, u16 index)
{
	int i;
	__maestro_write(chip, IDR1_CRAM_POINTER, index);
	for (i = 0; i < 1000; i++)
		if (__maestro_read(chip, IDR1_CRAM_POINTER) == index)
			return;
	snd_printd("es1968: APU register select failed. (Timeout)\n");
}

/*             */
static void apu_data_set(struct es1968 *chip, u16 data)
{
	int i;
	for (i = 0; i < 1000; i++) {
		if (__maestro_read(chip, IDR0_DATA_PORT) == data)
			return;
		__maestro_write(chip, IDR0_DATA_PORT, data);
	}
	snd_printd("es1968: APU register set probably failed (Timeout)!\n");
}

/*             */
static void __apu_set_register(struct es1968 *chip, u16 channel, u8 reg, u16 data)
{
	if (snd_BUG_ON(channel >= NR_APUS))
		return;
#ifdef CONFIG_PM_SLEEP
	chip->apu_map[channel][reg] = data;
#endif
	reg |= (channel << 4);
	apu_index_set(chip, reg);
	apu_data_set(chip, data);
}

static void apu_set_register(struct es1968 *chip, u16 channel, u8 reg, u16 data)
{
	unsigned long flags;
	spin_lock_irqsave(&chip->reg_lock, flags);
	__apu_set_register(chip, channel, reg, data);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

static u16 __apu_get_register(struct es1968 *chip, u16 channel, u8 reg)
{
	if (snd_BUG_ON(channel >= NR_APUS))
		return 0;
	reg |= (channel << 4);
	apu_index_set(chip, reg);
	return __maestro_read(chip, IDR0_DATA_PORT);
}

static u16 apu_get_register(struct es1968 *chip, u16 channel, u8 reg)
{
	unsigned long flags;
	u16 v;
	spin_lock_irqsave(&chip->reg_lock, flags);
	v = __apu_get_register(chip, channel, reg);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
	return v;
}

#if 0 /*                       */

static void assp_set_register(struct es1968 *chip, u32 reg, u32 value)
{
	unsigned long flags;

	spin_lock_irqsave(&chip->reg_lock, flags);
	outl(reg, chip->io_port + ASSP_INDEX);
	outl(value, chip->io_port + ASSP_DATA);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

static u32 assp_get_register(struct es1968 *chip, u32 reg)
{
	unsigned long flags;
	u32 value;

	spin_lock_irqsave(&chip->reg_lock, flags);
	outl(reg, chip->io_port + ASSP_INDEX);
	value = inl(chip->io_port + ASSP_DATA);
	spin_unlock_irqrestore(&chip->reg_lock, flags);

	return value;
}

#endif

static void wave_set_register(struct es1968 *chip, u16 reg, u16 value)
{
	unsigned long flags;

	spin_lock_irqsave(&chip->reg_lock, flags);
	outw(reg, chip->io_port + WC_INDEX);
	outw(value, chip->io_port + WC_DATA);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

static u16 wave_get_register(struct es1968 *chip, u16 reg)
{
	unsigned long flags;
	u16 value;

	spin_lock_irqsave(&chip->reg_lock, flags);
	outw(reg, chip->io_port + WC_INDEX);
	value = inw(chip->io_port + WC_DATA);
	spin_unlock_irqrestore(&chip->reg_lock, flags);

	return value;
}

/*                    
                      
                     */

static void snd_es1968_bob_stop(struct es1968 *chip)
{
	u16 reg;

	reg = __maestro_read(chip, 0x11);
	reg &= ~ESM_BOB_ENABLE;
	__maestro_write(chip, 0x11, reg);
	reg = __maestro_read(chip, 0x17);
	reg &= ~ESM_BOB_START;
	__maestro_write(chip, 0x17, reg);
}

static void snd_es1968_bob_start(struct es1968 *chip)
{
	int prescale;
	int divide;

	/*                                                               */
	/*                                                */
	for (prescale = 5; prescale < 12; prescale++)
		if (chip->bob_freq > (ESS_SYSCLK >> (prescale + 9)))
			break;

	/*                                                                    */
	divide = 1;
	while ((prescale > 5) && (divide < 32)) {
		prescale--;
		divide <<= 1;
	}
	divide >>= 1;

	/*                                          */
	for (; divide < 31; divide++)
		if (chip->bob_freq >
		    ((ESS_SYSCLK >> (prescale + 9)) / (divide + 1))) break;

	/*                                                    */
	if (divide == 0) {
		divide++;
		if (prescale > 5)
			prescale--;
	} else if (divide > 1)
		divide--;

	__maestro_write(chip, 6, 0x9000 | (prescale << 5) | divide);	/*         */

	/*                   */
	__maestro_write(chip, 0x11, __maestro_read(chip, 0x11) | 1);
	__maestro_write(chip, 0x17, __maestro_read(chip, 0x17) | 1);
}

/*                              */
static void snd_es1968_bob_inc(struct es1968 *chip, int freq)
{
	chip->bobclient++;
	if (chip->bobclient == 1) {
		chip->bob_freq = freq;
		snd_es1968_bob_start(chip);
	} else if (chip->bob_freq < freq) {
		snd_es1968_bob_stop(chip);
		chip->bob_freq = freq;
		snd_es1968_bob_start(chip);
	}
}

/*                              */
static void snd_es1968_bob_dec(struct es1968 *chip)
{
	chip->bobclient--;
	if (chip->bobclient <= 0)
		snd_es1968_bob_stop(chip);
	else if (chip->bob_freq > ESM_BOB_FREQ) {
		/*                                    */
		int max_freq = ESM_BOB_FREQ;
		struct esschan *es;
		list_for_each_entry(es, &chip->substream_list, list) {
			if (max_freq < es->bob_freq)
				max_freq = es->bob_freq;
		}
		if (max_freq != chip->bob_freq) {
			snd_es1968_bob_stop(chip);
			chip->bob_freq = max_freq;
			snd_es1968_bob_start(chip);
		}
	}
}

static int
snd_es1968_calc_bob_rate(struct es1968 *chip, struct esschan *es,
			 struct snd_pcm_runtime *runtime)
{
	/*                                                          */
	int freq = runtime->rate * 4;
	if (es->fmt & ESS_FMT_STEREO)
		freq <<= 1;
	if (es->fmt & ESS_FMT_16BIT)
		freq <<= 1;
	freq /= es->frag_size;
	if (freq < ESM_BOB_FREQ)
		freq = ESM_BOB_FREQ;
	else if (freq > ESM_BOB_FREQ_MAX)
		freq = ESM_BOB_FREQ_MAX;
	return freq;
}


/*            
              
             */

static u32 snd_es1968_compute_rate(struct es1968 *chip, u32 freq)
{
	u32 rate = (freq << 16) / chip->clock;
#if 0 /*                       */ 
	if (rate > 0x10000)
		rate = 0x10000;
#endif
	return rate;
}

/*                     */
static inline unsigned int
snd_es1968_get_dma_ptr(struct es1968 *chip, struct esschan *es)
{
	unsigned int offset;

	offset = apu_get_register(chip, es->apu[0], 5);

	offset -= es->base[0];

	return (offset & 0xFFFE);	/*                      */
}

static void snd_es1968_apu_set_freq(struct es1968 *chip, int apu, int freq)
{
	apu_set_register(chip, apu, 2,
			   (apu_get_register(chip, apu, 2) & 0x00FF) |
			   ((freq & 0xff) << 8) | 0x10);
	apu_set_register(chip, apu, 3, freq >> 8);
}

/*                */
static inline void snd_es1968_trigger_apu(struct es1968 *esm, int apu, int mode)
{
	/*                  */
	__apu_set_register(esm, apu, 0,
			   (__apu_get_register(esm, apu, 0) & 0xff0f) |
			   (mode << 4));
}

static void snd_es1968_pcm_start(struct es1968 *chip, struct esschan *es)
{
	spin_lock(&chip->reg_lock);
	__apu_set_register(chip, es->apu[0], 5, es->base[0]);
	snd_es1968_trigger_apu(chip, es->apu[0], es->apu_mode[0]);
	if (es->mode == ESM_MODE_CAPTURE) {
		__apu_set_register(chip, es->apu[2], 5, es->base[2]);
		snd_es1968_trigger_apu(chip, es->apu[2], es->apu_mode[2]);
	}
	if (es->fmt & ESS_FMT_STEREO) {
		__apu_set_register(chip, es->apu[1], 5, es->base[1]);
		snd_es1968_trigger_apu(chip, es->apu[1], es->apu_mode[1]);
		if (es->mode == ESM_MODE_CAPTURE) {
			__apu_set_register(chip, es->apu[3], 5, es->base[3]);
			snd_es1968_trigger_apu(chip, es->apu[3], es->apu_mode[3]);
		}
	}
	spin_unlock(&chip->reg_lock);
}

static void snd_es1968_pcm_stop(struct es1968 *chip, struct esschan *es)
{
	spin_lock(&chip->reg_lock);
	snd_es1968_trigger_apu(chip, es->apu[0], 0);
	snd_es1968_trigger_apu(chip, es->apu[1], 0);
	if (es->mode == ESM_MODE_CAPTURE) {
		snd_es1968_trigger_apu(chip, es->apu[2], 0);
		snd_es1968_trigger_apu(chip, es->apu[3], 0);
	}
	spin_unlock(&chip->reg_lock);
}

/*                               */
static void snd_es1968_program_wavecache(struct es1968 *chip, struct esschan *es,
					 int channel, u32 addr, int capture)
{
	u32 tmpval = (addr - 0x10) & 0xFFF8;

	if (! capture) {
		if (!(es->fmt & ESS_FMT_16BIT))
			tmpval |= 4;	/*      */
		if (es->fmt & ESS_FMT_STEREO)
			tmpval |= 2;	/*        */
	}

	/*                               */
	wave_set_register(chip, es->apu[channel] << 3, tmpval);

#ifdef CONFIG_PM_SLEEP
	es->wc_map[channel] = tmpval;
#endif
}


static void snd_es1968_playback_setup(struct es1968 *chip, struct esschan *es,
				      struct snd_pcm_runtime *runtime)
{
	u32 pa;
	int high_apu = 0;
	int channel, apu;
	int i, size;
	unsigned long flags;
	u32 freq;

	size = es->dma_size >> es->wav_shift;

	if (es->fmt & ESS_FMT_STEREO)
		high_apu++;

	for (channel = 0; channel <= high_apu; channel++) {
		apu = es->apu[channel];

		snd_es1968_program_wavecache(chip, es, channel, es->memory->buf.addr, 0);

		/*                  */
		pa = es->memory->buf.addr;
		pa -= chip->dma.addr;
		pa >>= 1;	/*       */

		pa |= 0x00400000;	/*                     */

		if (es->fmt & ESS_FMT_STEREO) {
			/*               */
			if (channel)
				pa |= 0x00800000;	/*          */
			if (es->fmt & ESS_FMT_16BIT)
				pa >>= 1;
		}

		/*                                                  
                      */
		es->base[channel] = pa & 0xFFFF;

		for (i = 0; i < 16; i++)
			apu_set_register(chip, apu, i, 0x0000);

		/*                                      */
		apu_set_register(chip, apu, 4, ((pa >> 16) & 0xFF) << 8);
		apu_set_register(chip, apu, 5, pa & 0xFFFF);
		apu_set_register(chip, apu, 6, (pa + size) & 0xFFFF);
		/*                            */
		apu_set_register(chip, apu, 7, size);

		/*                     */
		apu_set_register(chip, apu, 8, 0x0000);
		/*                                                        */
		apu_set_register(chip, apu, 9, 0xD000);

		/*                     */
		apu_set_register(chip, apu, 11, 0x0000);
		/*                                         */
		apu_set_register(chip, apu, 0, 0x400F);

		if (es->fmt & ESS_FMT_16BIT)
			es->apu_mode[channel] = ESM_APU_16BITLINEAR;
		else
			es->apu_mode[channel] = ESM_APU_8BITLINEAR;

		if (es->fmt & ESS_FMT_STEREO) {
			/*                            */
			/*                                                      
                                                          
                                                        
                                */
			apu_set_register(chip, apu, 10,
					 0x8F00 | (channel ? 0 : 0x10));
			es->apu_mode[channel] += 1;	/*        */
		} else
			apu_set_register(chip, apu, 10, 0x8F08);
	}

	spin_lock_irqsave(&chip->reg_lock, flags);
	/*                     */
	outw(1, chip->io_port + 0x04);
	/*                */
	outw(inw(chip->io_port + ESM_PORT_HOST_IRQ) | ESM_HIRQ_DSIE, chip->io_port + ESM_PORT_HOST_IRQ);
	spin_unlock_irqrestore(&chip->reg_lock, flags);

	freq = runtime->rate;
	/*               */
	if (freq > 48000)
		freq = 48000;
	if (freq < 4000)
		freq = 4000;

	/*        */
	if (!(es->fmt & ESS_FMT_16BIT) && !(es->fmt & ESS_FMT_STEREO))
		freq >>= 1;

	freq = snd_es1968_compute_rate(chip, freq);

	/*                                 */
	snd_es1968_apu_set_freq(chip, es->apu[0], freq);
	snd_es1968_apu_set_freq(chip, es->apu[1], freq);
}


static void init_capture_apu(struct es1968 *chip, struct esschan *es, int channel,
			     unsigned int pa, unsigned int bsize,
			     int mode, int route)
{
	int i, apu = es->apu[channel];

	es->apu_mode[channel] = mode;

	/*                               */
	snd_es1968_program_wavecache(chip, es, channel, pa, 1);

	/*                  */
	pa -= chip->dma.addr;
	pa >>= 1;	/*       */

	/*                                                  
                     */
	es->base[channel] = pa & 0xFFFF;
	pa |= 0x00400000;	/*                      */

	/*                       */
	for (i = 0; i < 16; i++)
		apu_set_register(chip, apu, i, 0x0000);

	/*                                                  
                                                     */
	apu_set_register(chip, apu, 2, 0x8);

	/*                                      */
	apu_set_register(chip, apu, 4, ((pa >> 16) & 0xFF) << 8);
	apu_set_register(chip, apu, 5, pa & 0xFFFF);
	apu_set_register(chip, apu, 6, (pa + bsize) & 0xFFFF);
	apu_set_register(chip, apu, 7, bsize);
	/*                     */
	apu_set_register(chip, apu, 8, 0x00F0);
	/*                                  */
	apu_set_register(chip, apu, 9, 0x0000);
	/*                                    */
	apu_set_register(chip, apu, 10, 0x8F08);
	/*             */
	apu_set_register(chip, apu, 11, route);
	/*                                         */
	apu_set_register(chip, apu, 0, 0x400F);
}

static void snd_es1968_capture_setup(struct es1968 *chip, struct esschan *es,
				     struct snd_pcm_runtime *runtime)
{
	int size;
	u32 freq;
	unsigned long flags;

	size = es->dma_size >> es->wav_shift;

	/*                 
                     
                 
                             
                         
 */
	/*                                                       
                                                      
                                               */

	/*                         */
	/*                                              */
	init_capture_apu(chip, es, 2,
			 es->mixbuf->buf.addr, ESM_MIXBUF_SIZE/4, /*          */
			 ESM_APU_INPUTMIXER, 0x14);
	/*                                              */
	init_capture_apu(chip, es, 0, es->memory->buf.addr, size,
			 ESM_APU_SRCONVERTOR, es->apu[2]);
	if (es->fmt & ESS_FMT_STEREO) {
		/*                     */
		init_capture_apu(chip, es, 3,
				 es->mixbuf->buf.addr + ESM_MIXBUF_SIZE/2,
				 ESM_MIXBUF_SIZE/4, /*          */
				 ESM_APU_INPUTMIXER, 0x15);
		/*             */
		init_capture_apu(chip, es, 1,
				 es->memory->buf.addr + size*2, size,
				 ESM_APU_SRCONVERTOR, es->apu[3]);
	}

	freq = runtime->rate;
	/*                                                               */
	if (freq > 47999)
		freq = 47999;
	if (freq < 4000)
		freq = 4000;

	freq = snd_es1968_compute_rate(chip, freq);

	/*                                 */
	snd_es1968_apu_set_freq(chip, es->apu[0], freq);
	snd_es1968_apu_set_freq(chip, es->apu[1], freq);

	/*                                                      */
	freq = 0x10000;
	snd_es1968_apu_set_freq(chip, es->apu[2], freq);
	snd_es1968_apu_set_freq(chip, es->apu[3], freq);

	spin_lock_irqsave(&chip->reg_lock, flags);
	/*                     */
	outw(1, chip->io_port + 0x04);
	/*                */
	outw(inw(chip->io_port + ESM_PORT_HOST_IRQ) | ESM_HIRQ_DSIE, chip->io_port + ESM_PORT_HOST_IRQ);
	spin_unlock_irqrestore(&chip->reg_lock, flags);
}

/*                  
                    
                   */

static int snd_es1968_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct esschan *es = runtime->private_data;

	es->dma_size = snd_pcm_lib_buffer_bytes(substream);
	es->frag_size = snd_pcm_lib_period_bytes(substream);

	es->wav_shift = 1; /*                              */
	es->fmt = 0;
	if (snd_pcm_format_width(runtime->format) == 16)
		es->fmt |= ESS_FMT_16BIT;
	if (runtime->channels > 1) {
		es->fmt |= ESS_FMT_STEREO;
		if (es->fmt & ESS_FMT_16BIT) /*                              */
			es->wav_shift++;
	}
	es->bob_freq = snd_es1968_calc_bob_rate(chip, es, runtime);

	switch (es->mode) {
	case ESM_MODE_PLAY:
		snd_es1968_playback_setup(chip, es, runtime);
		break;
	case ESM_MODE_CAPTURE:
		snd_es1968_capture_setup(chip, es, runtime);
		break;
	}

	return 0;
}

static int snd_es1968_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct esschan *es = substream->runtime->private_data;

	spin_lock(&chip->substream_lock);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		if (es->running)
			break;
		snd_es1968_bob_inc(chip, es->bob_freq);
		es->count = 0;
		es->hwptr = 0;
		snd_es1968_pcm_start(chip, es);
		es->running = 1;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		if (! es->running)
			break;
		snd_es1968_pcm_stop(chip, es);
		es->running = 0;
		snd_es1968_bob_dec(chip);
		break;
	}
	spin_unlock(&chip->substream_lock);
	return 0;
}

static snd_pcm_uframes_t snd_es1968_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct esschan *es = substream->runtime->private_data;
	unsigned int ptr;

	ptr = snd_es1968_get_dma_ptr(chip, es) << es->wav_shift;
	
	return bytes_to_frames(substream->runtime, ptr % es->dma_size);
}

static struct snd_pcm_hardware snd_es1968_playback = {
	.info =			(SNDRV_PCM_INFO_MMAP |
               		         SNDRV_PCM_INFO_MMAP_VALID |
				 SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_BLOCK_TRANSFER |
				 /*                      */
				 SNDRV_PCM_INFO_RESUME),
	.formats =		SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE,
	.rates =		SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000,
	.rate_min =		4000,
	.rate_max =		48000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =	65536,
	.period_bytes_min =	256,
	.period_bytes_max =	65536,
	.periods_min =		1,
	.periods_max =		1024,
	.fifo_size =		0,
};

static struct snd_pcm_hardware snd_es1968_capture = {
	.info =			(SNDRV_PCM_INFO_NONINTERLEAVED |
				 SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_MMAP_VALID |
				 SNDRV_PCM_INFO_BLOCK_TRANSFER |
				 /*                      */
				 SNDRV_PCM_INFO_RESUME),
	.formats =		/*                     */ SNDRV_PCM_FMTBIT_S16_LE,
	.rates =		SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000,
	.rate_min =		4000,
	.rate_max =		48000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =	65536,
	.period_bytes_min =	256,
	.period_bytes_max =	65536,
	.periods_min =		1,
	.periods_max =		1024,
	.fifo_size =		0,
};

/*                          
                            
                           */

/*                                                                             
               */

static int calc_available_memory_size(struct es1968 *chip)
{
	int max_size = 0;
	struct esm_memory *buf;

	mutex_lock(&chip->memory_mutex);
	list_for_each_entry(buf, &chip->buf_list, list) {
		if (buf->empty && buf->buf.bytes > max_size)
			max_size = buf->buf.bytes;
	}
	mutex_unlock(&chip->memory_mutex);
	if (max_size >= 128*1024)
		max_size = 127*1024;
	return max_size;
}

/*                                                     */
static struct esm_memory *snd_es1968_new_memory(struct es1968 *chip, int size)
{
	struct esm_memory *buf;

	size = ALIGN(size, ESM_MEM_ALIGN);
	mutex_lock(&chip->memory_mutex);
	list_for_each_entry(buf, &chip->buf_list, list) {
		if (buf->empty && buf->buf.bytes >= size)
			goto __found;
	}
	mutex_unlock(&chip->memory_mutex);
	return NULL;

__found:
	if (buf->buf.bytes > size) {
		struct esm_memory *chunk = kmalloc(sizeof(*chunk), GFP_KERNEL);
		if (chunk == NULL) {
			mutex_unlock(&chip->memory_mutex);
			return NULL;
		}
		chunk->buf = buf->buf;
		chunk->buf.bytes -= size;
		chunk->buf.area += size;
		chunk->buf.addr += size;
		chunk->empty = 1;
		buf->buf.bytes = size;
		list_add(&chunk->list, &buf->list);
	}
	buf->empty = 0;
	mutex_unlock(&chip->memory_mutex);
	return buf;
}

/*                     */
static void snd_es1968_free_memory(struct es1968 *chip, struct esm_memory *buf)
{
	struct esm_memory *chunk;

	mutex_lock(&chip->memory_mutex);
	buf->empty = 1;
	if (buf->list.prev != &chip->buf_list) {
		chunk = list_entry(buf->list.prev, struct esm_memory, list);
		if (chunk->empty) {
			chunk->buf.bytes += buf->buf.bytes;
			list_del(&buf->list);
			kfree(buf);
			buf = chunk;
		}
	}
	if (buf->list.next != &chip->buf_list) {
		chunk = list_entry(buf->list.next, struct esm_memory, list);
		if (chunk->empty) {
			buf->buf.bytes += chunk->buf.bytes;
			list_del(&chunk->list);
			kfree(chunk);
		}
	}
	mutex_unlock(&chip->memory_mutex);
}

static void snd_es1968_free_dmabuf(struct es1968 *chip)
{
	struct list_head *p;

	if (! chip->dma.area)
		return;
	snd_dma_reserve_buf(&chip->dma, snd_dma_pci_buf_id(chip->pci));
	while ((p = chip->buf_list.next) != &chip->buf_list) {
		struct esm_memory *chunk = list_entry(p, struct esm_memory, list);
		list_del(p);
		kfree(chunk);
	}
}

static int
snd_es1968_init_dmabuf(struct es1968 *chip)
{
	int err;
	struct esm_memory *chunk;

	chip->dma.dev.type = SNDRV_DMA_TYPE_DEV;
	chip->dma.dev.dev = snd_dma_pci_data(chip->pci);
	if (! snd_dma_get_reserved_buf(&chip->dma, snd_dma_pci_buf_id(chip->pci))) {
		err = snd_dma_alloc_pages_fallback(SNDRV_DMA_TYPE_DEV,
						   snd_dma_pci_data(chip->pci),
						   chip->total_bufsize, &chip->dma);
		if (err < 0 || ! chip->dma.area) {
			snd_printk(KERN_ERR "es1968: can't allocate dma pages for size %d\n",
				   chip->total_bufsize);
			return -ENOMEM;
		}
		if ((chip->dma.addr + chip->dma.bytes - 1) & ~((1 << 28) - 1)) {
			snd_dma_free_pages(&chip->dma);
			snd_printk(KERN_ERR "es1968: DMA buffer beyond 256MB.\n");
			return -ENOMEM;
		}
	}

	INIT_LIST_HEAD(&chip->buf_list);
	/*                         */
	chunk = kmalloc(sizeof(*chunk), GFP_KERNEL);
	if (chunk == NULL) {
		snd_es1968_free_dmabuf(chip);
		return -ENOMEM;
	}
	memset(chip->dma.area, 0, ESM_MEM_ALIGN);
	chunk->buf = chip->dma;
	chunk->buf.area += ESM_MEM_ALIGN;
	chunk->buf.addr += ESM_MEM_ALIGN;
	chunk->buf.bytes -= ESM_MEM_ALIGN;
	chunk->empty = 1;
	list_add(&chunk->list, &chip->buf_list);

	return 0;
}

/*                     */
/*                                                         */
static int snd_es1968_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *hw_params)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct esschan *chan = runtime->private_data;
	int size = params_buffer_bytes(hw_params);

	if (chan->memory) {
		if (chan->memory->buf.bytes >= size) {
			runtime->dma_bytes = size;
			return 0;
		}
		snd_es1968_free_memory(chip, chan->memory);
	}
	chan->memory = snd_es1968_new_memory(chip, size);
	if (chan->memory == NULL) {
		//                                                             
		return -ENOMEM;
	}
	snd_pcm_set_runtime_buffer(substream, &chan->memory->buf);
	return 1; /*                  */
}

/*                               */
static int snd_es1968_hw_free(struct snd_pcm_substream *substream)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct esschan *chan;
	
	if (runtime->private_data == NULL)
		return 0;
	chan = runtime->private_data;
	if (chan->memory) {
		snd_es1968_free_memory(chip, chan->memory);
		chan->memory = NULL;
	}
	return 0;
}


/*
                    
 */
static int snd_es1968_alloc_apu_pair(struct es1968 *chip, int type)
{
	int apu;

	for (apu = 0; apu < NR_APUS; apu += 2) {
		if (chip->apu[apu] == ESM_APU_FREE &&
		    chip->apu[apu + 1] == ESM_APU_FREE) {
			chip->apu[apu] = chip->apu[apu + 1] = type;
			return apu;
		}
	}
	return -EBUSY;
}

/*
                   
 */
static void snd_es1968_free_apu_pair(struct es1968 *chip, int apu)
{
	chip->apu[apu] = chip->apu[apu + 1] = ESM_APU_FREE;
}


/*                 
                   
                  */

static int snd_es1968_playback_open(struct snd_pcm_substream *substream)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct esschan *es;
	int apu1;

	/*               */
	apu1 = snd_es1968_alloc_apu_pair(chip, ESM_APU_PCM_PLAY);
	if (apu1 < 0)
		return apu1;

	es = kzalloc(sizeof(*es), GFP_KERNEL);
	if (!es) {
		snd_es1968_free_apu_pair(chip, apu1);
		return -ENOMEM;
	}

	es->apu[0] = apu1;
	es->apu[1] = apu1 + 1;
	es->apu_mode[0] = 0;
	es->apu_mode[1] = 0;
	es->running = 0;
	es->substream = substream;
	es->mode = ESM_MODE_PLAY;

	runtime->private_data = es;
	runtime->hw = snd_es1968_playback;
	runtime->hw.buffer_bytes_max = runtime->hw.period_bytes_max =
		calc_available_memory_size(chip);

	spin_lock_irq(&chip->substream_lock);
	list_add(&es->list, &chip->substream_list);
	spin_unlock_irq(&chip->substream_lock);

	return 0;
}

static int snd_es1968_capture_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct esschan *es;
	int apu1, apu2;

	apu1 = snd_es1968_alloc_apu_pair(chip, ESM_APU_PCM_CAPTURE);
	if (apu1 < 0)
		return apu1;
	apu2 = snd_es1968_alloc_apu_pair(chip, ESM_APU_PCM_RATECONV);
	if (apu2 < 0) {
		snd_es1968_free_apu_pair(chip, apu1);
		return apu2;
	}
	
	es = kzalloc(sizeof(*es), GFP_KERNEL);
	if (!es) {
		snd_es1968_free_apu_pair(chip, apu1);
		snd_es1968_free_apu_pair(chip, apu2);
		return -ENOMEM;
	}

	es->apu[0] = apu1;
	es->apu[1] = apu1 + 1;
	es->apu[2] = apu2;
	es->apu[3] = apu2 + 1;
	es->apu_mode[0] = 0;
	es->apu_mode[1] = 0;
	es->apu_mode[2] = 0;
	es->apu_mode[3] = 0;
	es->running = 0;
	es->substream = substream;
	es->mode = ESM_MODE_CAPTURE;

	/*               */
	if ((es->mixbuf = snd_es1968_new_memory(chip, ESM_MIXBUF_SIZE)) == NULL) {
		snd_es1968_free_apu_pair(chip, apu1);
		snd_es1968_free_apu_pair(chip, apu2);
		kfree(es);
                return -ENOMEM;
        }
	memset(es->mixbuf->buf.area, 0, ESM_MIXBUF_SIZE);

	runtime->private_data = es;
	runtime->hw = snd_es1968_capture;
	runtime->hw.buffer_bytes_max = runtime->hw.period_bytes_max =
		calc_available_memory_size(chip) - 1024; /*                  */
	snd_pcm_hw_constraint_pow2(runtime, 0, SNDRV_PCM_HW_PARAM_BUFFER_BYTES);

	spin_lock_irq(&chip->substream_lock);
	list_add(&es->list, &chip->substream_list);
	spin_unlock_irq(&chip->substream_lock);

	return 0;
}

static int snd_es1968_playback_close(struct snd_pcm_substream *substream)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct esschan *es;

	if (substream->runtime->private_data == NULL)
		return 0;
	es = substream->runtime->private_data;
	spin_lock_irq(&chip->substream_lock);
	list_del(&es->list);
	spin_unlock_irq(&chip->substream_lock);
	snd_es1968_free_apu_pair(chip, es->apu[0]);
	kfree(es);

	return 0;
}

static int snd_es1968_capture_close(struct snd_pcm_substream *substream)
{
	struct es1968 *chip = snd_pcm_substream_chip(substream);
	struct esschan *es;

	if (substream->runtime->private_data == NULL)
		return 0;
	es = substream->runtime->private_data;
	spin_lock_irq(&chip->substream_lock);
	list_del(&es->list);
	spin_unlock_irq(&chip->substream_lock);
	snd_es1968_free_memory(chip, es->mixbuf);
	snd_es1968_free_apu_pair(chip, es->apu[0]);
	snd_es1968_free_apu_pair(chip, es->apu[2]);
	kfree(es);

	return 0;
}

static struct snd_pcm_ops snd_es1968_playback_ops = {
	.open =		snd_es1968_playback_open,
	.close =	snd_es1968_playback_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	snd_es1968_hw_params,
	.hw_free =	snd_es1968_hw_free,
	.prepare =	snd_es1968_pcm_prepare,
	.trigger =	snd_es1968_pcm_trigger,
	.pointer =	snd_es1968_pcm_pointer,
};

static struct snd_pcm_ops snd_es1968_capture_ops = {
	.open =		snd_es1968_capture_open,
	.close =	snd_es1968_capture_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	snd_es1968_hw_params,
	.hw_free =	snd_es1968_hw_free,
	.prepare =	snd_es1968_pcm_prepare,
	.trigger =	snd_es1968_pcm_trigger,
	.pointer =	snd_es1968_pcm_pointer,
};


/*
                
 */
#define CLOCK_MEASURE_BUFSIZE	16768	/*                                */

static void es1968_measure_clock(struct es1968 *chip)
{
	int i, apu;
	unsigned int pa, offset, t;
	struct esm_memory *memory;
	struct timeval start_time, stop_time;

	if (chip->clock == 0)
		chip->clock = 48000; /*                     */

	/*                                            */
	if ((apu = snd_es1968_alloc_apu_pair(chip, ESM_APU_PCM_PLAY)) < 0) {
		snd_printk(KERN_ERR "Hmm, cannot find empty APU pair!?\n");
		return;
	}
	if ((memory = snd_es1968_new_memory(chip, CLOCK_MEASURE_BUFSIZE)) == NULL) {
		snd_printk(KERN_ERR "cannot allocate dma buffer - using default clock %d\n", chip->clock);
		snd_es1968_free_apu_pair(chip, apu);
		return;
	}

	memset(memory->buf.area, 0, CLOCK_MEASURE_BUFSIZE);

	wave_set_register(chip, apu << 3, (memory->buf.addr - 0x10) & 0xfff8);

	pa = (unsigned int)((memory->buf.addr - chip->dma.addr) >> 1);
	pa |= 0x00400000;	/*                     */

	/*                */
	for (i = 0; i < 16; i++)
		apu_set_register(chip, apu, i, 0x0000);

	apu_set_register(chip, apu, 0, 0x400f);
	apu_set_register(chip, apu, 4, ((pa >> 16) & 0xff) << 8);
	apu_set_register(chip, apu, 5, pa & 0xffff);
	apu_set_register(chip, apu, 6, (pa + CLOCK_MEASURE_BUFSIZE/2) & 0xffff);
	apu_set_register(chip, apu, 7, CLOCK_MEASURE_BUFSIZE/2);
	apu_set_register(chip, apu, 8, 0x0000);
	apu_set_register(chip, apu, 9, 0xD000);
	apu_set_register(chip, apu, 10, 0x8F08);
	apu_set_register(chip, apu, 11, 0x0000);
	spin_lock_irq(&chip->reg_lock);
	outw(1, chip->io_port + 0x04); /*                     */
	outw(inw(chip->io_port + ESM_PORT_HOST_IRQ) | ESM_HIRQ_DSIE, chip->io_port + ESM_PORT_HOST_IRQ); /*                */
	spin_unlock_irq(&chip->reg_lock);

	snd_es1968_apu_set_freq(chip, apu, ((unsigned int)48000 << 16) / chip->clock); /*          */

	chip->in_measurement = 1;
	chip->measure_apu = apu;
	spin_lock_irq(&chip->reg_lock);
	snd_es1968_bob_inc(chip, ESM_BOB_FREQ);
	__apu_set_register(chip, apu, 5, pa & 0xffff);
	snd_es1968_trigger_apu(chip, apu, ESM_APU_16BITLINEAR);
	do_gettimeofday(&start_time);
	spin_unlock_irq(&chip->reg_lock);
	msleep(50);
	spin_lock_irq(&chip->reg_lock);
	offset = __apu_get_register(chip, apu, 5);
	do_gettimeofday(&stop_time);
	snd_es1968_trigger_apu(chip, apu, 0); /*      */
	snd_es1968_bob_dec(chip);
	chip->in_measurement = 0;
	spin_unlock_irq(&chip->reg_lock);

	/*                            */
	offset -= (pa & 0xffff);
	offset &= 0xfffe;
	offset += chip->measure_count * (CLOCK_MEASURE_BUFSIZE/2);

	t = stop_time.tv_sec - start_time.tv_sec;
	t *= 1000000;
	if (stop_time.tv_usec < start_time.tv_usec)
		t -= start_time.tv_usec - stop_time.tv_usec;
	else
		t += stop_time.tv_usec - start_time.tv_usec;
	if (t == 0) {
		snd_printk(KERN_ERR "?? calculation error..\n");
	} else {
		offset *= 1000;
		offset = (offset / t) * 1000 + ((offset % t) * 1000) / t;
		if (offset < 47500 || offset > 48500) {
			if (offset >= 40000 && offset <= 50000)
				chip->clock = (chip->clock * offset) / 48000;
		}
		printk(KERN_INFO "es1968: clocking to %d\n", chip->clock);
	}
	snd_es1968_free_memory(chip, memory);
	snd_es1968_free_apu_pair(chip, apu);
}


/*
 */

static void snd_es1968_pcm_free(struct snd_pcm *pcm)
{
	struct es1968 *esm = pcm->private_data;
	snd_es1968_free_dmabuf(esm);
	esm->pcm = NULL;
}

static int
snd_es1968_pcm(struct es1968 *chip, int device)
{
	struct snd_pcm *pcm;
	int err;

	/*                */
	if ((err = snd_es1968_init_dmabuf(chip)) < 0)
		return err;

	/*            */
	wave_set_register(chip, 0x01FC, chip->dma.addr >> 12);
	wave_set_register(chip, 0x01FD, chip->dma.addr >> 12);
	wave_set_register(chip, 0x01FE, chip->dma.addr >> 12);
	wave_set_register(chip, 0x01FF, chip->dma.addr >> 12);

	if ((err = snd_pcm_new(chip->card, "ESS Maestro", device,
			       chip->playback_streams,
			       chip->capture_streams, &pcm)) < 0)
		return err;

	pcm->private_data = chip;
	pcm->private_free = snd_es1968_pcm_free;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_es1968_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_es1968_capture_ops);

	pcm->info_flags = 0;

	strcpy(pcm->name, "ESS Maestro");

	chip->pcm = pcm;

	return 0;
}
/*
                                                       
 */
static void snd_es1968_suppress_jitter(struct es1968 *chip, struct esschan *es)
{
	unsigned int cp1;
	unsigned int cp2;
	unsigned int diff;

	cp1 = __apu_get_register(chip, 0, 5);
	cp2 = __apu_get_register(chip, 1, 5);
	diff = (cp1 > cp2 ? cp1 - cp2 : cp2 - cp1);

	if (diff > 1)
		__maestro_write(chip, IDR0_DATA_PORT, cp1);
}

/*
                 
 */
static void snd_es1968_update_pcm(struct es1968 *chip, struct esschan *es)
{
	unsigned int hwptr;
	unsigned int diff;
	struct snd_pcm_substream *subs = es->substream;
        
	if (subs == NULL || !es->running)
		return;

	hwptr = snd_es1968_get_dma_ptr(chip, es) << es->wav_shift;
	hwptr %= es->dma_size;

	diff = (es->dma_size + hwptr - es->hwptr) % es->dma_size;

	es->hwptr = hwptr;
	es->count += diff;

	if (es->count > es->frag_size) {
		spin_unlock(&chip->substream_lock);
		snd_pcm_period_elapsed(subs);
		spin_lock(&chip->substream_lock);
		es->count %= es->frag_size;
	}
}

/*                                                                    
                                                                      
                                                                              
                                                                    */
static void es1968_update_hw_volume(struct work_struct *work)
{
	struct es1968 *chip = container_of(work, struct es1968, hwvol_work);
	int x, val;

	/*                                                   
                                                  
            */
	x = inb(chip->io_port + 0x1c) & 0xee;
	/*                                     */
	outb(0x88, chip->io_port + 0x1c);
	outb(0x88, chip->io_port + 0x1d);
	outb(0x88, chip->io_port + 0x1e);
	outb(0x88, chip->io_port + 0x1f);

	if (chip->in_suspend)
		return;

#ifndef CONFIG_SND_ES1968_INPUT
	if (! chip->master_switch || ! chip->master_volume)
		return;

	val = snd_ac97_read(chip->ac97, AC97_MASTER);
	switch (x) {
	case 0x88:
		/*      */
		val ^= 0x8000;
		break;
	case 0xaa:
		/*           */
		if ((val & 0x7f) > 0)
			val--;
		if ((val & 0x7f00) > 0)
			val -= 0x0100;
		break;
	case 0x66:
		/*             */
		if ((val & 0x7f) < 0x1f)
			val++;
		if ((val & 0x7f00) < 0x1f00)
			val += 0x0100;
		break;
	}
	if (snd_ac97_update(chip->ac97, AC97_MASTER, val))
		snd_ctl_notify(chip->card, SNDRV_CTL_EVENT_MASK_VALUE,
			       &chip->master_volume->id);
#else
	if (!chip->input_dev)
		return;

	val = 0;
	switch (x) {
	case 0x88:
		/*                                                       
                                                             
                                            */
		val = KEY_MUTE;
		break;
	case 0xaa:
		/*                                      */
		val = KEY_VOLUMEUP;
		break;
	case 0x66:
		/*                                        */
		val = KEY_VOLUMEDOWN;
		break;
	}

	if (val) {
		input_report_key(chip->input_dev, val, 1);
		input_sync(chip->input_dev);
		input_report_key(chip->input_dev, val, 0);
		input_sync(chip->input_dev);
	}
#endif
}

/*
                    
 */
static irqreturn_t snd_es1968_interrupt(int irq, void *dev_id)
{
	struct es1968 *chip = dev_id;
	u32 event;

	if (!(event = inb(chip->io_port + 0x1A)))
		return IRQ_NONE;

	outw(inw(chip->io_port + 4) & 1, chip->io_port + 4);

	if (event & ESM_HWVOL_IRQ)
		schedule_work(&chip->hwvol_work);

	/*                             */
	outb(0xFF, chip->io_port + 0x1A);

	if ((event & ESM_MPU401_IRQ) && chip->rmidi) {
		snd_mpu401_uart_interrupt(irq, chip->rmidi->private_data);
	}

	if (event & ESM_SOUND_IRQ) {
		struct esschan *es;
		spin_lock(&chip->substream_lock);
		list_for_each_entry(es, &chip->substream_list, list) {
			if (es->running) {
				snd_es1968_update_pcm(chip, es);
				if (es->fmt & ESS_FMT_STEREO)
					snd_es1968_suppress_jitter(chip, es);
			}
		}
		spin_unlock(&chip->substream_lock);
		if (chip->in_measurement) {
			unsigned int curp = __apu_get_register(chip, chip->measure_apu, 5);
			if (curp < chip->measure_lastpos)
				chip->measure_count++;
			chip->measure_lastpos = curp;
		}
	}

	return IRQ_HANDLED;
}

/*
               
 */

static int
snd_es1968_mixer(struct es1968 *chip)
{
	struct snd_ac97_bus *pbus;
	struct snd_ac97_template ac97;
#ifndef CONFIG_SND_ES1968_INPUT
	struct snd_ctl_elem_id elem_id;
#endif
	int err;
	static struct snd_ac97_bus_ops ops = {
		.write = snd_es1968_ac97_write,
		.read = snd_es1968_ac97_read,
	};

	if ((err = snd_ac97_bus(chip->card, 0, &ops, NULL, &pbus)) < 0)
		return err;
	pbus->no_vra = 1; /*                         */

	memset(&ac97, 0, sizeof(ac97));
	ac97.private_data = chip;
	if ((err = snd_ac97_mixer(pbus, &ac97, &chip->ac97)) < 0)
		return err;

#ifndef CONFIG_SND_ES1968_INPUT
	/*                                                       */
	memset(&elem_id, 0, sizeof(elem_id));
	elem_id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strcpy(elem_id.name, "Master Playback Switch");
	chip->master_switch = snd_ctl_find_id(chip->card, &elem_id);
	memset(&elem_id, 0, sizeof(elem_id));
	elem_id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strcpy(elem_id.name, "Master Playback Volume");
	chip->master_volume = snd_ctl_find_id(chip->card, &elem_id);
#endif

	return 0;
}

/*
                   
 */

static void snd_es1968_ac97_reset(struct es1968 *chip)
{
	unsigned long ioaddr = chip->io_port;

	unsigned short save_ringbus_a;
	unsigned short save_68;
	unsigned short w;
	unsigned int vend;

	/*                    */
	save_ringbus_a = inw(ioaddr + 0x36);

	//                                                                              
	/*                                             */
	outw(inw(ioaddr + 0x3a) & 0xfffc, ioaddr + 0x3a);
	outw(inw(ioaddr + 0x3c) & 0xfffc, ioaddr + 0x3c);

	/*                 */
	outw(0x0000, ioaddr + 0x36);
	save_68 = inw(ioaddr + 0x68);
	pci_read_config_word(chip->pci, 0x58, &w);	/*                                          */
	pci_read_config_dword(chip->pci, PCI_SUBSYSTEM_VENDOR_ID, &vend);
	if (w & 1)
		save_68 |= 0x10;
	outw(0xfffe, ioaddr + 0x64);	/*               */
	outw(0x0001, ioaddr + 0x68);	/*            */
	outw(0x0000, ioaddr + 0x60);	/*                   */
	udelay(20);
	outw(0x0001, ioaddr + 0x60);	/*                   */
	msleep(20);

	outw(save_68 | 0x1, ioaddr + 0x68);	/*                */
	outw((inw(ioaddr + 0x38) & 0xfffc) | 0x1, ioaddr + 0x38);
	outw((inw(ioaddr + 0x3a) & 0xfffc) | 0x1, ioaddr + 0x3a);
	outw((inw(ioaddr + 0x3c) & 0xfffc) | 0x1, ioaddr + 0x3c);

	/*                      */
	/*                 */
	outw(0x0000, ioaddr + 0x36);
	outw(0xfff7, ioaddr + 0x64);	/*               */
	save_68 = inw(ioaddr + 0x68);
	outw(0x0009, ioaddr + 0x68);	/*                     */
	outw(0x0001, ioaddr + 0x60);	/*                 */
	udelay(20);
	outw(0x0009, ioaddr + 0x60);	/*                 */
	msleep(500);
	//                                                 
	outw(inw(ioaddr + 0x3a) & 0xfffc, ioaddr + 0x3a);
	outw(inw(ioaddr + 0x3c) & 0xfffc, ioaddr + 0x3c);

#if 0				/*                                                       */
	snd_printk(KERN_INFO "trying software reset\n");
	/*                             */
	outb(0x80 | 0x7c, ioaddr + 0x30);
	for (w = 0;; w++) {
		if ((inw(ioaddr + 0x30) & 1) == 0) {
			if (inb(ioaddr + 0x32) != 0)
				break;

			outb(0x80 | 0x7d, ioaddr + 0x30);
			if (((inw(ioaddr + 0x30) & 1) == 0)
			    && (inb(ioaddr + 0x32) != 0))
				break;
			outb(0x80 | 0x7f, ioaddr + 0x30);
			if (((inw(ioaddr + 0x30) & 1) == 0)
			    && (inb(ioaddr + 0x32) != 0))
				break;
		}

		if (w > 10000) {
			outb(inb(ioaddr + 0x37) | 0x08, ioaddr + 0x37);	/*                     */
			msleep(500);	/*         */
			outb(inb(ioaddr + 0x37) & ~0x08,
				ioaddr + 0x37);
			udelay(1);
			outw(0x80, ioaddr + 0x30);
			for (w = 0; w < 10000; w++) {
				if ((inw(ioaddr + 0x30) & 1) == 0)
					break;
			}
		}
	}
#endif
	if (vend == NEC_VERSA_SUBID1 || vend == NEC_VERSA_SUBID2) {
		/*                       */
		outw(0xf9ff, ioaddr + 0x64);
		outw(inw(ioaddr + 0x68) | 0x600, ioaddr + 0x68);
		outw(0x0209, ioaddr + 0x60);
	}

	/*           */
	outw(save_ringbus_a, ioaddr + 0x36);

	/*                              
                                              
                                                                   */
	outb(inb(ioaddr+0xc0)|(1<<5), ioaddr+0xc0);
	outb(0xff, ioaddr+0xc3);
	outb(0xff, ioaddr+0xc4);
	outb(0xff, ioaddr+0xc6);
	outb(0xff, ioaddr+0xc8);
	outb(0x3f, ioaddr+0xcf);
	outb(0x3f, ioaddr+0xd0);
}

static void snd_es1968_reset(struct es1968 *chip)
{
	/*       */
	outw(ESM_RESET_MAESTRO | ESM_RESET_DIRECTSOUND,
	     chip->io_port + ESM_PORT_HOST_IRQ);
	udelay(10);
	outw(0x0000, chip->io_port + ESM_PORT_HOST_IRQ);
	udelay(10);
}

/*
                          
 */
static void snd_es1968_chip_init(struct es1968 *chip)
{
	struct pci_dev *pci = chip->pci;
	int i;
	unsigned long iobase  = chip->io_port;
	u16 w;
	u32 n;

	/*                                                  
                                                          
                                                             
                                                            
                                                               
                                                            
                                                        
                                                          
         
  */
	
	/*              */
	pci_read_config_word(pci, ESM_CONFIG_A, &w);

	w &= ~DMA_CLEAR;	/*                */
	w &= ~(PIC_SNOOP1 | PIC_SNOOP2);	/*                           */
	w &= ~SAFEGUARD;	/*               */
	w |= POST_WRITE;	/*              */
	w |= PCI_TIMING;	/*               */
	/*                                   */
	w &= ~SWAP_LR;		/*                 
                                      
                 */
	w &= ~SUBTR_DECODE;	/*                        */

	pci_write_config_word(pci, ESM_CONFIG_A, w);

	/*              */

	pci_read_config_word(pci, ESM_CONFIG_B, &w);

	w &= ~(1 << 15);	/*                                    */
	/*                                  */
	w &= ~(1 << 14);	/*                */

	w &= ~SPDIF_CONFB;	/*                       */
	w |= HWV_CONFB;		/*        */
	w |= DEBOUNCE;		/*                                             */
	w &= ~GPIO_CONFB;	/*          */
	w |= CHI_CONFB;		/*                                                                */
	w &= ~IDMA_CONFB;	/*                         */
	w &= ~MIDI_FIX;		/*                      */
	w &= ~(1 << 1);		/*                          */
	w &= ~IRQ_TO_ISA;	/*                        */

	pci_write_config_word(pci, ESM_CONFIG_B, w);

	/*          */

	pci_read_config_word(pci, ESM_DDMA, &w);
	w &= ~(1 << 0);
	pci_write_config_word(pci, ESM_DDMA, w);

	/*
               
  */

	pci_read_config_word(pci, ESM_LEGACY_AUDIO_CONTROL, &w);

	w |= ESS_DISABLE_AUDIO;	/*                      */
	w &= ~ESS_ENABLE_SERIAL_IRQ;	/*              */
	w &= ~(0x1f);		/*                                       */

	pci_write_config_word(pci, ESM_LEGACY_AUDIO_CONTROL, w);

	/*                                  */
	pci_read_config_word(pci, 0x58, &w);
	w|=1<<2;	/*             */
	w|=1<<3;	/*                                      */
	w&=~(1<<11);	/*                                   */
	pci_write_config_word(pci, 0x58, w);
	
	/*             */

	snd_es1968_reset(chip);

	/*
                  
  */

	/*                                                    */
	outw(0xC090, iobase + ESM_RING_BUS_DEST); /*                      */
	udelay(20);
	outw(0x3000, iobase + ESM_RING_BUS_CONTR_A); /*                       */
	udelay(20);

	/*
                   
  */
	 
	snd_es1968_ac97_reset(chip);

	/*                    */

	n = inl(iobase + ESM_RING_BUS_CONTR_B);
	n &= ~RINGB_EN_SPDIF;	/*           */
	//                                            
	outl(n, iobase + ESM_RING_BUS_CONTR_B);

	/*                                                    
                                                                  */
	outb(0x88, iobase+0x1c);
	outb(0x88, iobase+0x1d);
	outb(0x88, iobase+0x1e);
	outb(0x88, iobase+0x1f);

	/*                                                                 
                                                  */

	outb(0, iobase + ASSP_CONTROL_B);
	outb(3, iobase + ASSP_CONTROL_A);	/*                     */
	outb(0, iobase + ASSP_CONTROL_C);	/*                                         */

	/*
                    
  */
	for (i = 0; i < 16; i++) {
		/*                                         */
		outw(0x01E0 + i, iobase + WC_INDEX);
		outw(0x0000, iobase + WC_DATA);

		/*                                                           
                     */
		outw(0x01D0 + i, iobase + WC_INDEX);
		outw(0x0000, iobase + WC_DATA);
	}
	wave_set_register(chip, IDR7_WAVE_ROMRAM,
			  (wave_get_register(chip, IDR7_WAVE_ROMRAM) & 0xFF00));
	wave_set_register(chip, IDR7_WAVE_ROMRAM,
			  wave_get_register(chip, IDR7_WAVE_ROMRAM) | 0x100);
	wave_set_register(chip, IDR7_WAVE_ROMRAM,
			  wave_get_register(chip, IDR7_WAVE_ROMRAM) & ~0x200);
	wave_set_register(chip, IDR7_WAVE_ROMRAM,
			  wave_get_register(chip, IDR7_WAVE_ROMRAM) | ~0x400);


	maestro_write(chip, IDR2_CRAM_DATA, 0x0000);
	/*                                   */
	/*                                */
	maestro_write(chip, 0x08, 0xB004);
	maestro_write(chip, 0x09, 0x001B);
	maestro_write(chip, 0x0A, 0x8000);
	maestro_write(chip, 0x0B, 0x3F37);
	maestro_write(chip, 0x0C, 0x0098);

	/*                                                    */
	maestro_write(chip, 0x0C,
		      (maestro_read(chip, 0x0C) & ~0xF000) | 0x8000);
	/*              */
	maestro_write(chip, 0x0C,
		      (maestro_read(chip, 0x0C) & ~0x0F00) | 0x0500);

	maestro_write(chip, 0x0D, 0x7632);

	/*                                           
                                   */

	w = inw(iobase + WC_CONTROL);

	w &= ~0xFA00;		/*                                    */
	w |= 0xA000;		/*                          */
	w &= ~0x0200;		/*                                                      
                                                 */
	w |= 0x0100;		/*                              */
	w |= 0x0080;		/*                                           */
	w &= ~0x0060;		/*                     */
	w |= 0x0020;		/*                      */
	/*                   */
	w &= ~0x000C;		/*                                                        */
	/*                   */
	w &= ~0x0001;		/*               */

	outw(w, iobase + WC_CONTROL);

	/*                               */
	for (i = 0; i < NR_APUS; i++) {
		for (w = 0; w < NR_APU_REGS; w++)
			apu_set_register(chip, i, w, 0);

	}
}

/*              */
static void snd_es1968_start_irq(struct es1968 *chip)
{
	unsigned short w;
	w = ESM_HIRQ_DSIE | ESM_HIRQ_HW_VOLUME;
	if (chip->rmidi)
		w |= ESM_HIRQ_MPU401;
	outb(w, chip->io_port + 0x1A);
	outw(w, chip->io_port + ESM_PORT_HOST_IRQ);
}

#ifdef CONFIG_PM_SLEEP
/*
             
 */
static int es1968_suspend(struct device *dev)
{
	struct pci_dev *pci = to_pci_dev(dev);
	struct snd_card *card = dev_get_drvdata(dev);
	struct es1968 *chip = card->private_data;

	if (! chip->do_pm)
		return 0;

	chip->in_suspend = 1;
	cancel_work_sync(&chip->hwvol_work);
	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);
	snd_pcm_suspend_all(chip->pcm);
	snd_ac97_suspend(chip->ac97);
	snd_es1968_bob_stop(chip);

	pci_disable_device(pci);
	pci_save_state(pci);
	pci_set_power_state(pci, PCI_D3hot);
	return 0;
}

static int es1968_resume(struct device *dev)
{
	struct pci_dev *pci = to_pci_dev(dev);
	struct snd_card *card = dev_get_drvdata(dev);
	struct es1968 *chip = card->private_data;
	struct esschan *es;

	if (! chip->do_pm)
		return 0;

	/*                        */
	pci_set_power_state(pci, PCI_D0);
	pci_restore_state(pci);
	if (pci_enable_device(pci) < 0) {
		printk(KERN_ERR "es1968: pci_enable_device failed, "
		       "disabling device\n");
		snd_card_disconnect(card);
		return -EIO;
	}
	pci_set_master(pci);

	snd_es1968_chip_init(chip);

	/*                                     */ 
	if (chip->dma.addr) {
		/*            */
		wave_set_register(chip, 0x01FC, chip->dma.addr >> 12);
	}

	snd_es1968_start_irq(chip);

	/*                    */
	snd_ac97_resume(chip->ac97);

	list_for_each_entry(es, &chip->substream_list, list) {
		switch (es->mode) {
		case ESM_MODE_PLAY:
			snd_es1968_playback_setup(chip, es, es->substream->runtime);
			break;
		case ESM_MODE_CAPTURE:
			snd_es1968_capture_setup(chip, es, es->substream->runtime);
			break;
		}
	}

	/*                   */
	if (chip->bobclient)
		snd_es1968_bob_start(chip);

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);
	chip->in_suspend = 0;
	return 0;
}

static SIMPLE_DEV_PM_OPS(es1968_pm, es1968_suspend, es1968_resume);
#define ES1968_PM_OPS	&es1968_pm
#else
#define ES1968_PM_OPS	NULL
#endif /*                 */

#ifdef SUPPORT_JOYSTICK
#define JOYSTICK_ADDR	0x200
static int snd_es1968_create_gameport(struct es1968 *chip, int dev)
{
	struct gameport *gp;
	struct resource *r;
	u16 val;

	if (!joystick[dev])
		return -ENODEV;

	r = request_region(JOYSTICK_ADDR, 8, "ES1968 gameport");
	if (!r)
		return -EBUSY;

	chip->gameport = gp = gameport_allocate_port();
	if (!gp) {
		printk(KERN_ERR "es1968: cannot allocate memory for gameport\n");
		release_and_free_resource(r);
		return -ENOMEM;
	}

	pci_read_config_word(chip->pci, ESM_LEGACY_AUDIO_CONTROL, &val);
	pci_write_config_word(chip->pci, ESM_LEGACY_AUDIO_CONTROL, val | 0x04);

	gameport_set_name(gp, "ES1968 Gameport");
	gameport_set_phys(gp, "pci%s/gameport0", pci_name(chip->pci));
	gameport_set_dev_parent(gp, &chip->pci->dev);
	gp->io = JOYSTICK_ADDR;
	gameport_set_port_data(gp, r);

	gameport_register_port(gp);

	return 0;
}

static void snd_es1968_free_gameport(struct es1968 *chip)
{
	if (chip->gameport) {
		struct resource *r = gameport_get_port_data(chip->gameport);

		gameport_unregister_port(chip->gameport);
		chip->gameport = NULL;

		release_and_free_resource(r);
	}
}
#else
static inline int snd_es1968_create_gameport(struct es1968 *chip, int dev) { return -ENOSYS; }
static inline void snd_es1968_free_gameport(struct es1968 *chip) { }
#endif

#ifdef CONFIG_SND_ES1968_INPUT
static int snd_es1968_input_register(struct es1968 *chip)
{
	struct input_dev *input_dev;
	int err;

	input_dev = input_allocate_device();
	if (!input_dev)
		return -ENOMEM;

	snprintf(chip->phys, sizeof(chip->phys), "pci-%s/input0",
		 pci_name(chip->pci));

	input_dev->name = chip->card->driver;
	input_dev->phys = chip->phys;
	input_dev->id.bustype = BUS_PCI;
	input_dev->id.vendor  = chip->pci->vendor;
	input_dev->id.product = chip->pci->device;
	input_dev->dev.parent = &chip->pci->dev;

	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(KEY_MUTE, input_dev->keybit);
	__set_bit(KEY_VOLUMEDOWN, input_dev->keybit);
	__set_bit(KEY_VOLUMEUP, input_dev->keybit);

	err = input_register_device(input_dev);
	if (err) {
		input_free_device(input_dev);
		return err;
	}

	chip->input_dev = input_dev;
	return 0;
}
#endif /*                         */

#ifdef CONFIG_SND_ES1968_RADIO
#define GPIO_DATA	0x60
#define IO_MASK		4      /*                                         
                                     */
#define IO_DIR		8      /*                                         
                                  */
/*                          */
#define STR_DATA	0x0040 /*       */
#define STR_CLK		0x0080 /*       */
#define STR_WREN	0x0100 /*       */
#define STR_MOST	0x0200 /*       */

static void snd_es1968_tea575x_set_pins(struct snd_tea575x *tea, u8 pins)
{
	struct es1968 *chip = tea->private_data;
	unsigned long io = chip->io_port + GPIO_DATA;
	u16 val = 0;

	val |= (pins & TEA575X_DATA) ? STR_DATA : 0;
	val |= (pins & TEA575X_CLK)  ? STR_CLK  : 0;
	val |= (pins & TEA575X_WREN) ? STR_WREN : 0;

	outw(val, io);
}

static u8 snd_es1968_tea575x_get_pins(struct snd_tea575x *tea)
{
	struct es1968 *chip = tea->private_data;
	unsigned long io = chip->io_port + GPIO_DATA;
	u16 val = inw(io);
	u8 ret;

	ret = 0;
	if (val & STR_DATA)
		ret |= TEA575X_DATA;
	if (val & STR_MOST)
		ret |= TEA575X_MOST;
	return ret;
}

static void snd_es1968_tea575x_set_direction(struct snd_tea575x *tea, bool output)
{
	struct es1968 *chip = tea->private_data;
	unsigned long io = chip->io_port + GPIO_DATA;
	u16 odir = inw(io + IO_DIR);

	if (output) {
		outw(~(STR_DATA | STR_CLK | STR_WREN), io + IO_MASK);
		outw(odir | STR_DATA | STR_CLK | STR_WREN, io + IO_DIR);
	} else {
		outw(~(STR_CLK | STR_WREN | STR_DATA | STR_MOST), io + IO_MASK);
		outw((odir & ~(STR_DATA | STR_MOST)) | STR_CLK | STR_WREN, io + IO_DIR);
	}
}

static struct snd_tea575x_ops snd_es1968_tea_ops = {
	.set_pins = snd_es1968_tea575x_set_pins,
	.get_pins = snd_es1968_tea575x_get_pins,
	.set_direction = snd_es1968_tea575x_set_direction,
};
#endif

static int snd_es1968_free(struct es1968 *chip)
{
	cancel_work_sync(&chip->hwvol_work);
#ifdef CONFIG_SND_ES1968_INPUT
	if (chip->input_dev)
		input_unregister_device(chip->input_dev);
#endif

	if (chip->io_port) {
		if (chip->irq >= 0)
			synchronize_irq(chip->irq);
		outw(1, chip->io_port + 0x04); /*                     */
		outw(0, chip->io_port + ESM_PORT_HOST_IRQ); /*             */
	}

#ifdef CONFIG_SND_ES1968_RADIO
	snd_tea575x_exit(&chip->tea);
	v4l2_device_unregister(&chip->v4l2_dev);
#endif

	if (chip->irq >= 0)
		free_irq(chip->irq, chip);
	snd_es1968_free_gameport(chip);
	pci_release_regions(chip->pci);
	pci_disable_device(chip->pci);
	kfree(chip);
	return 0;
}

static int snd_es1968_dev_free(struct snd_device *device)
{
	struct es1968 *chip = device->device_data;
	return snd_es1968_free(chip);
}

struct ess_device_list {
	unsigned short type;	/*           */
	unsigned short vendor;	/*                     */
};

static struct ess_device_list pm_whitelist[] = {
	{ TYPE_MAESTRO2E, 0x0e11 },	/*               */
	{ TYPE_MAESTRO2E, 0x1028 },
	{ TYPE_MAESTRO2E, 0x103c },
	{ TYPE_MAESTRO2E, 0x1179 },
	{ TYPE_MAESTRO2E, 0x14c0 },	/*                  */
	{ TYPE_MAESTRO2E, 0x1558 },
	{ TYPE_MAESTRO2E, 0x125d },	/*                               */
	{ TYPE_MAESTRO2, 0x125d },	/*                            */
};

static struct ess_device_list mpu_blacklist[] = {
	{ TYPE_MAESTRO2, 0x125d },
};

static int snd_es1968_create(struct snd_card *card,
			     struct pci_dev *pci,
			     int total_bufsize,
			     int play_streams,
			     int capt_streams,
			     int chip_type,
			     int do_pm,
			     int radio_nr,
			     struct es1968 **chip_ret)
{
	static struct snd_device_ops ops = {
		.dev_free =	snd_es1968_dev_free,
	};
	struct es1968 *chip;
	int i, err;

	*chip_ret = NULL;

	/*                   */
	if ((err = pci_enable_device(pci)) < 0)
		return err;
	/*                                                        */
	if (pci_set_dma_mask(pci, DMA_BIT_MASK(28)) < 0 ||
	    pci_set_consistent_dma_mask(pci, DMA_BIT_MASK(28)) < 0) {
		snd_printk(KERN_ERR "architecture does not support 28bit PCI busmaster DMA\n");
		pci_disable_device(pci);
		return -ENXIO;
	}

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (! chip) {
		pci_disable_device(pci);
		return -ENOMEM;
	}

	/*          */
	chip->type = chip_type;
	spin_lock_init(&chip->reg_lock);
	spin_lock_init(&chip->substream_lock);
	INIT_LIST_HEAD(&chip->buf_list);
	INIT_LIST_HEAD(&chip->substream_list);
	mutex_init(&chip->memory_mutex);
	INIT_WORK(&chip->hwvol_work, es1968_update_hw_volume);
	chip->card = card;
	chip->pci = pci;
	chip->irq = -1;
	chip->total_bufsize = total_bufsize;	/*          */
	chip->playback_streams = play_streams;
	chip->capture_streams = capt_streams;

	if ((err = pci_request_regions(pci, "ESS Maestro")) < 0) {
		kfree(chip);
		pci_disable_device(pci);
		return err;
	}
	chip->io_port = pci_resource_start(pci, 0);
	if (request_irq(pci->irq, snd_es1968_interrupt, IRQF_SHARED,
			KBUILD_MODNAME, chip)) {
		snd_printk(KERN_ERR "unable to grab IRQ %d\n", pci->irq);
		snd_es1968_free(chip);
		return -EBUSY;
	}
	chip->irq = pci->irq;
	        
	/*                   */
	for (i = 0; i < 32; i++)
		chip->maestro_map[i] = 0;

	/*               */
	for (i = 0; i < NR_APUS; i++)
		chip->apu[i] = ESM_APU_FREE;

	/*                 */
	pci_set_master(pci);

	if (do_pm > 1) {
		/*                                                  */
		unsigned short vend;
		pci_read_config_word(chip->pci, PCI_SUBSYSTEM_VENDOR_ID, &vend);
		for (i = 0; i < (int)ARRAY_SIZE(pm_whitelist); i++) {
			if (chip->type == pm_whitelist[i].type &&
			    vend == pm_whitelist[i].vendor) {
				do_pm = 1;
				break;
			}
		}
		if (do_pm > 1) {
			/*                           */
			printk(KERN_INFO "es1968: not attempting power management.\n");
			do_pm = 0;
		}
	}
	chip->do_pm = do_pm;

	snd_es1968_chip_init(chip);

	if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops)) < 0) {
		snd_es1968_free(chip);
		return err;
	}

	snd_card_set_dev(card, &pci->dev);

#ifdef CONFIG_SND_ES1968_RADIO
	err = v4l2_device_register(&pci->dev, &chip->v4l2_dev);
	if (err < 0) {
		snd_es1968_free(chip);
		return err;
	}
	chip->tea.v4l2_dev = &chip->v4l2_dev;
	chip->tea.private_data = chip;
	chip->tea.radio_nr = radio_nr;
	chip->tea.ops = &snd_es1968_tea_ops;
	strlcpy(chip->tea.card, "SF64-PCE2", sizeof(chip->tea.card));
	sprintf(chip->tea.bus_info, "PCI:%s", pci_name(pci));
	if (!snd_tea575x_init(&chip->tea, THIS_MODULE))
		printk(KERN_INFO "es1968: detected TEA575x radio\n");
#endif

	*chip_ret = chip;

	return 0;
}


/*
 */
static int snd_es1968_probe(struct pci_dev *pci,
			    const struct pci_device_id *pci_id)
{
	static int dev;
	struct snd_card *card;
	struct es1968 *chip;
	unsigned int i;
	int err;

	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable[dev]) {
		dev++;
		return -ENOENT;
	}

	err = snd_card_create(index[dev], id[dev], THIS_MODULE, 0, &card);
	if (err < 0)
		return err;
                
	if (total_bufsize[dev] < 128)
		total_bufsize[dev] = 128;
	if (total_bufsize[dev] > 4096)
		total_bufsize[dev] = 4096;
	if ((err = snd_es1968_create(card, pci,
				     total_bufsize[dev] * 1024, /*          */
				     pcm_substreams_p[dev], 
				     pcm_substreams_c[dev],
				     pci_id->driver_data,
				     use_pm[dev],
				     radio_nr[dev],
				     &chip)) < 0) {
		snd_card_free(card);
		return err;
	}
	card->private_data = chip;

	switch (chip->type) {
	case TYPE_MAESTRO2E:
		strcpy(card->driver, "ES1978");
		strcpy(card->shortname, "ESS ES1978 (Maestro 2E)");
		break;
	case TYPE_MAESTRO2:
		strcpy(card->driver, "ES1968");
		strcpy(card->shortname, "ESS ES1968 (Maestro 2)");
		break;
	case TYPE_MAESTRO:
		strcpy(card->driver, "ESM1");
		strcpy(card->shortname, "ESS Maestro 1");
		break;
	}

	if ((err = snd_es1968_pcm(chip, 0)) < 0) {
		snd_card_free(card);
		return err;
	}

	if ((err = snd_es1968_mixer(chip)) < 0) {
		snd_card_free(card);
		return err;
	}

	if (enable_mpu[dev] == 2) {
		/*                      */
		unsigned short vend;
		pci_read_config_word(chip->pci, PCI_SUBSYSTEM_VENDOR_ID, &vend);
		for (i = 0; i < ARRAY_SIZE(mpu_blacklist); i++) {
			if (chip->type == mpu_blacklist[i].type &&
			    vend == mpu_blacklist[i].vendor) {
				enable_mpu[dev] = 0;
				break;
			}
		}
	}
	if (enable_mpu[dev]) {
		if ((err = snd_mpu401_uart_new(card, 0, MPU401_HW_MPU401,
					       chip->io_port + ESM_MPU401_PORT,
					       MPU401_INFO_INTEGRATED |
					       MPU401_INFO_IRQ_HOOK,
					       -1, &chip->rmidi)) < 0) {
			printk(KERN_WARNING "es1968: skipping MPU-401 MIDI support..\n");
		}
	}

	snd_es1968_create_gameport(chip, dev);

#ifdef CONFIG_SND_ES1968_INPUT
	err = snd_es1968_input_register(chip);
	if (err)
		snd_printk(KERN_WARNING "Input device registration "
			"failed with error %i", err);
#endif

	snd_es1968_start_irq(chip);

	chip->clock = clock[dev];
	if (! chip->clock)
		es1968_measure_clock(chip);

	sprintf(card->longname, "%s at 0x%lx, irq %i",
		card->shortname, chip->io_port, chip->irq);

	if ((err = snd_card_register(card)) < 0) {
		snd_card_free(card);
		return err;
	}
	pci_set_drvdata(pci, card);
	dev++;
	return 0;
}

static void snd_es1968_remove(struct pci_dev *pci)
{
	snd_card_free(pci_get_drvdata(pci));
	pci_set_drvdata(pci, NULL);
}

static struct pci_driver es1968_driver = {
	.name = KBUILD_MODNAME,
	.id_table = snd_es1968_ids,
	.probe = snd_es1968_probe,
	.remove = snd_es1968_remove,
	.driver = {
		.pm = ES1968_PM_OPS,
	},
};

module_pci_driver(es1968_driver);
