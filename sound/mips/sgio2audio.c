/*
 *   Sound driver for Silicon Graphics O2 Workstations A/V board audio.
 *
 *   Copyright 2003 Vivien Chappelier <vivien.chappelier@linux-mips.org>
 *   Copyright 2008 Thomas Bogendoerfer <tsbogend@alpha.franken.de>
 *   Mxier part taken from mace_audio.c:
 *   Copyright 2007 Thorben Jändling <tj.trevelyan@gmail.com>
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
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <asm/ip32/ip32_ints.h>
#include <asm/ip32/mace.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#define SNDRV_GET_ID
#include <sound/initval.h>
#include <sound/ad1843.h>


MODULE_AUTHOR("Vivien Chappelier <vivien.chappelier@linux-mips.org>");
MODULE_DESCRIPTION("SGI O2 Audio");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{Silicon Graphics, O2 Audio}}");

static int index = SNDRV_DEFAULT_IDX1;  /*             */
static char *id = SNDRV_DEFAULT_STR1;   /*                  */

module_param(index, int, 0444);
MODULE_PARM_DESC(index, "Index value for SGI O2 soundcard.");
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for SGI O2 soundcard.");


#define AUDIO_CONTROL_RESET              BIT(0) /*                          */
#define AUDIO_CONTROL_CODEC_PRESENT      BIT(1) /*                   */

#define CODEC_CONTROL_WORD_SHIFT        0
#define CODEC_CONTROL_READ              BIT(16)
#define CODEC_CONTROL_ADDRESS_SHIFT     17

#define CHANNEL_CONTROL_RESET           BIT(10) /*                  */
#define CHANNEL_DMA_ENABLE              BIT(9)  /*                        */
#define CHANNEL_INT_THRESHOLD_DISABLED  (0 << 5) /*                    */
#define CHANNEL_INT_THRESHOLD_25        (1 << 5) /*                         */
#define CHANNEL_INT_THRESHOLD_50        (2 << 5) /*                         */
#define CHANNEL_INT_THRESHOLD_75        (3 << 5) /*                         */
#define CHANNEL_INT_THRESHOLD_EMPTY     (4 << 5) /*                     */
#define CHANNEL_INT_THRESHOLD_NOT_EMPTY (5 << 5) /*                      */
#define CHANNEL_INT_THRESHOLD_FULL      (6 << 5) /*                     */
#define CHANNEL_INT_THRESHOLD_NOT_FULL  (7 << 5) /*                      */

#define CHANNEL_RING_SHIFT              12
#define CHANNEL_RING_SIZE               (1 << CHANNEL_RING_SHIFT)
#define CHANNEL_RING_MASK               (CHANNEL_RING_SIZE - 1)

#define CHANNEL_LEFT_SHIFT 40
#define CHANNEL_RIGHT_SHIFT 8

struct snd_sgio2audio_chan {
	int idx;
	struct snd_pcm_substream *substream;
	int pos;
	snd_pcm_uframes_t size;
	spinlock_t lock;
};

/*                                        */
struct snd_sgio2audio {
	struct snd_card *card;

	/*       */
	struct snd_ad1843 ad1843;
	spinlock_t ad1843_lock;

	/*          */
	struct snd_sgio2audio_chan channel[3];

	/*           */
	void *ring_base;
	dma_addr_t ring_base_dma;
};

/*               */

/*
                                                                            
  
                                                                 
 */
static int read_ad1843_reg(void *priv, int reg)
{
	struct snd_sgio2audio *chip = priv;
	int val;
	unsigned long flags;

	spin_lock_irqsave(&chip->ad1843_lock, flags);

	writeq((reg << CODEC_CONTROL_ADDRESS_SHIFT) |
	       CODEC_CONTROL_READ, &mace->perif.audio.codec_control);
	wmb();
	val = readq(&mace->perif.audio.codec_control); /*           */
	udelay(200);

	val = readq(&mace->perif.audio.codec_read);

	spin_unlock_irqrestore(&chip->ad1843_lock, flags);
	return val;
}

/*
                                                                           
 */
static int write_ad1843_reg(void *priv, int reg, int word)
{
	struct snd_sgio2audio *chip = priv;
	int val;
	unsigned long flags;

	spin_lock_irqsave(&chip->ad1843_lock, flags);

	writeq((reg << CODEC_CONTROL_ADDRESS_SHIFT) |
	       (word << CODEC_CONTROL_WORD_SHIFT),
	       &mace->perif.audio.codec_control);
	wmb();
	val = readq(&mace->perif.audio.codec_control); /*           */
	udelay(200);

	spin_unlock_irqrestore(&chip->ad1843_lock, flags);
	return 0;
}

static int sgio2audio_gain_info(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_info *uinfo)
{
	struct snd_sgio2audio *chip = snd_kcontrol_chip(kcontrol);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = ad1843_get_gain_max(&chip->ad1843,
					     (int)kcontrol->private_value);
	return 0;
}

static int sgio2audio_gain_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_sgio2audio *chip = snd_kcontrol_chip(kcontrol);
	int vol;

	vol = ad1843_get_gain(&chip->ad1843, (int)kcontrol->private_value);

	ucontrol->value.integer.value[0] = (vol >> 8) & 0xFF;
	ucontrol->value.integer.value[1] = vol & 0xFF;

	return 0;
}

static int sgio2audio_gain_put(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_sgio2audio *chip = snd_kcontrol_chip(kcontrol);
	int newvol, oldvol;

	oldvol = ad1843_get_gain(&chip->ad1843, kcontrol->private_value);
	newvol = (ucontrol->value.integer.value[0] << 8) |
		ucontrol->value.integer.value[1];

	newvol = ad1843_set_gain(&chip->ad1843, kcontrol->private_value,
		newvol);

	return newvol != oldvol;
}

static int sgio2audio_source_info(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_info *uinfo)
{
	static const char *texts[3] = {
		"Cam Mic", "Mic", "Line"
	};
	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = 3;
	if (uinfo->value.enumerated.item >= 3)
		uinfo->value.enumerated.item = 1;
	strcpy(uinfo->value.enumerated.name,
	       texts[uinfo->value.enumerated.item]);
	return 0;
}

static int sgio2audio_source_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_sgio2audio *chip = snd_kcontrol_chip(kcontrol);

	ucontrol->value.enumerated.item[0] = ad1843_get_recsrc(&chip->ad1843);
	return 0;
}

static int sgio2audio_source_put(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_sgio2audio *chip = snd_kcontrol_chip(kcontrol);
	int newsrc, oldsrc;

	oldsrc = ad1843_get_recsrc(&chip->ad1843);
	newsrc = ad1843_set_recsrc(&chip->ad1843,
				   ucontrol->value.enumerated.item[0]);

	return newsrc != oldsrc;
}

/*                         */
static struct snd_kcontrol_new sgio2audio_ctrl_pcm0 = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "PCM Playback Volume",
	.index          = 0,
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.private_value  = AD1843_GAIN_PCM_0,
	.info           = sgio2audio_gain_info,
	.get            = sgio2audio_gain_get,
	.put            = sgio2audio_gain_put,
};

/*                         */
static struct snd_kcontrol_new sgio2audio_ctrl_pcm1 = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "PCM Playback Volume",
	.index          = 1,
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.private_value  = AD1843_GAIN_PCM_1,
	.info           = sgio2audio_gain_info,
	.get            = sgio2audio_gain_get,
	.put            = sgio2audio_gain_put,
};

/*                            */
static struct snd_kcontrol_new sgio2audio_ctrl_reclevel = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "Capture Volume",
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.private_value  = AD1843_GAIN_RECLEV,
	.info           = sgio2audio_gain_info,
	.get            = sgio2audio_gain_get,
	.put            = sgio2audio_gain_put,
};

/*                             */
static struct snd_kcontrol_new sgio2audio_ctrl_recsource = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "Capture Source",
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.info           = sgio2audio_source_info,
	.get            = sgio2audio_source_get,
	.put            = sgio2audio_source_put,
};

/*                    */
static struct snd_kcontrol_new sgio2audio_ctrl_line = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "Line Playback Volume",
	.index          = 0,
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.private_value  = AD1843_GAIN_LINE,
	.info           = sgio2audio_gain_info,
	.get            = sgio2audio_gain_get,
	.put            = sgio2audio_gain_put,
};

/*                  */
static struct snd_kcontrol_new sgio2audio_ctrl_cd = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "Line Playback Volume",
	.index          = 1,
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.private_value  = AD1843_GAIN_LINE_2,
	.info           = sgio2audio_gain_info,
	.get            = sgio2audio_gain_get,
	.put            = sgio2audio_gain_put,
};

/*                   */
static struct snd_kcontrol_new sgio2audio_ctrl_mic = {
	.iface          = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name           = "Mic Playback Volume",
	.access         = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.private_value  = AD1843_GAIN_MIC,
	.info           = sgio2audio_gain_info,
	.get            = sgio2audio_gain_get,
	.put            = sgio2audio_gain_put,
};


static int snd_sgio2audio_new_mixer(struct snd_sgio2audio *chip)
{
	int err;

	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_pcm0, chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_pcm1, chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_reclevel, chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_recsource, chip));
	if (err < 0)
		return err;
	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_line, chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_cd, chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&sgio2audio_ctrl_mic, chip));
	if (err < 0)
		return err;

	return 0;
}

/*                               */

/*                                                               */
/*                                   */
static int snd_sgio2audio_dma_pull_frag(struct snd_sgio2audio *chip,
					unsigned int ch, unsigned int count)
{
	int ret;
	unsigned long src_base, src_pos, dst_mask;
	unsigned char *dst_base;
	int dst_pos;
	u64 *src;
	s16 *dst;
	u64 x;
	unsigned long flags;
	struct snd_pcm_runtime *runtime = chip->channel[ch].substream->runtime;

	spin_lock_irqsave(&chip->channel[ch].lock, flags);

	src_base = (unsigned long) chip->ring_base | (ch << CHANNEL_RING_SHIFT);
	src_pos = readq(&mace->perif.audio.chan[ch].read_ptr);
	dst_base = runtime->dma_area;
	dst_pos = chip->channel[ch].pos;
	dst_mask = frames_to_bytes(runtime, runtime->buffer_size) - 1;

	/*                               */
	chip->channel[ch].size += (count >> 3); /*           */
	ret = chip->channel[ch].size >= runtime->period_size;
	chip->channel[ch].size %= runtime->period_size;

	while (count) {
		src = (u64 *)(src_base + src_pos);
		dst = (s16 *)(dst_base + dst_pos);

		x = *src;
		dst[0] = (x >> CHANNEL_LEFT_SHIFT) & 0xffff;
		dst[1] = (x >> CHANNEL_RIGHT_SHIFT) & 0xffff;

		src_pos = (src_pos + sizeof(u64)) & CHANNEL_RING_MASK;
		dst_pos = (dst_pos + 2 * sizeof(s16)) & dst_mask;
		count -= sizeof(u64);
	}

	writeq(src_pos, &mace->perif.audio.chan[ch].read_ptr); /*          */
	chip->channel[ch].pos = dst_pos;

	spin_unlock_irqrestore(&chip->channel[ch].lock, flags);
	return ret;
}

/*                                                                    */
/*                                   */
static int snd_sgio2audio_dma_push_frag(struct snd_sgio2audio *chip,
					unsigned int ch, unsigned int count)
{
	int ret;
	s64 l, r;
	unsigned long dst_base, dst_pos, src_mask;
	unsigned char *src_base;
	int src_pos;
	u64 *dst;
	s16 *src;
	unsigned long flags;
	struct snd_pcm_runtime *runtime = chip->channel[ch].substream->runtime;

	spin_lock_irqsave(&chip->channel[ch].lock, flags);

	dst_base = (unsigned long)chip->ring_base | (ch << CHANNEL_RING_SHIFT);
	dst_pos = readq(&mace->perif.audio.chan[ch].write_ptr);
	src_base = runtime->dma_area;
	src_pos = chip->channel[ch].pos;
	src_mask = frames_to_bytes(runtime, runtime->buffer_size) - 1;

	/*                               */
	chip->channel[ch].size += (count >> 3); /*           */
	ret = chip->channel[ch].size >= runtime->period_size;
	chip->channel[ch].size %= runtime->period_size;

	while (count) {
		src = (s16 *)(src_base + src_pos);
		dst = (u64 *)(dst_base + dst_pos);

		l = src[0]; /*             */
		r = src[1]; /*             */

		*dst = ((l & 0x00ffffff) << CHANNEL_LEFT_SHIFT) |
			((r & 0x00ffffff) << CHANNEL_RIGHT_SHIFT);

		dst_pos = (dst_pos + sizeof(u64)) & CHANNEL_RING_MASK;
		src_pos = (src_pos + 2 * sizeof(s16)) & src_mask;
		count -= sizeof(u64);
	}

	writeq(dst_pos, &mace->perif.audio.chan[ch].write_ptr); /*          */
	chip->channel[ch].pos = src_pos;

	spin_unlock_irqrestore(&chip->channel[ch].lock, flags);
	return ret;
}

static int snd_sgio2audio_dma_start(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio *chip = snd_pcm_substream_chip(substream);
	struct snd_sgio2audio_chan *chan = substream->runtime->private_data;
	int ch = chan->idx;

	/*                   */
	writeq(CHANNEL_CONTROL_RESET, &mace->perif.audio.chan[ch].control);
	udelay(10);
	writeq(0, &mace->perif.audio.chan[ch].control);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/*                    */
		snd_sgio2audio_dma_push_frag(chip, ch, CHANNEL_RING_SIZE - 32);
	}
	/*                                                   */
	writeq(CHANNEL_DMA_ENABLE | CHANNEL_INT_THRESHOLD_50,
	       &mace->perif.audio.chan[ch].control);
	return 0;
}

static int snd_sgio2audio_dma_stop(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio_chan *chan = substream->runtime->private_data;

	writeq(0, &mace->perif.audio.chan[chan->idx].control);
	return 0;
}

static irqreturn_t snd_sgio2audio_dma_in_isr(int irq, void *dev_id)
{
	struct snd_sgio2audio_chan *chan = dev_id;
	struct snd_pcm_substream *substream;
	struct snd_sgio2audio *chip;
	int count, ch;

	substream = chan->substream;
	chip = snd_pcm_substream_chip(substream);
	ch = chan->idx;

	/*                */
	count = CHANNEL_RING_SIZE -
		readq(&mace->perif.audio.chan[ch].depth) - 32;
	if (snd_sgio2audio_dma_pull_frag(chip, ch, count))
		snd_pcm_period_elapsed(substream);

	return IRQ_HANDLED;
}

static irqreturn_t snd_sgio2audio_dma_out_isr(int irq, void *dev_id)
{
	struct snd_sgio2audio_chan *chan = dev_id;
	struct snd_pcm_substream *substream;
	struct snd_sgio2audio *chip;
	int count, ch;

	substream = chan->substream;
	chip = snd_pcm_substream_chip(substream);
	ch = chan->idx;
	/*               */
	count = CHANNEL_RING_SIZE -
		readq(&mace->perif.audio.chan[ch].depth) - 32;
	if (snd_sgio2audio_dma_push_frag(chip, ch, count))
		snd_pcm_period_elapsed(substream);

	return IRQ_HANDLED;
}

static irqreturn_t snd_sgio2audio_error_isr(int irq, void *dev_id)
{
	struct snd_sgio2audio_chan *chan = dev_id;
	struct snd_pcm_substream *substream;

	substream = chan->substream;
	snd_sgio2audio_dma_stop(substream);
	snd_sgio2audio_dma_start(substream);
	return IRQ_HANDLED;
}

/*          */
/*                         */
static struct snd_pcm_hardware snd_sgio2audio_pcm_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
		 SNDRV_PCM_INFO_MMAP_VALID |
		 SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER),
	.formats =          SNDRV_PCM_FMTBIT_S16_BE,
	.rates =            SNDRV_PCM_RATE_8000_48000,
	.rate_min =         8000,
	.rate_max =         48000,
	.channels_min =     2,
	.channels_max =     2,
	.buffer_bytes_max = 65536,
	.period_bytes_min = 32768,
	.period_bytes_max = 65536,
	.periods_min =      1,
	.periods_max =      1024,
};

/*                            */
static int snd_sgio2audio_playback1_open(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->hw = snd_sgio2audio_pcm_hw;
	runtime->private_data = &chip->channel[1];
	return 0;
}

static int snd_sgio2audio_playback2_open(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->hw = snd_sgio2audio_pcm_hw;
	runtime->private_data = &chip->channel[2];
	return 0;
}

/*                           */
static int snd_sgio2audio_capture_open(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->hw = snd_sgio2audio_pcm_hw;
	runtime->private_data = &chip->channel[0];
	return 0;
}

/*                    */
static int snd_sgio2audio_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->private_data = NULL;
	return 0;
}


/*                    */
static int snd_sgio2audio_pcm_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *hw_params)
{
	return snd_pcm_lib_alloc_vmalloc_buffer(substream,
						params_buffer_bytes(hw_params));
}

/*                  */
static int snd_sgio2audio_pcm_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_vmalloc_buffer(substream);
}

/*                  */
static int snd_sgio2audio_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_sgio2audio_chan *chan = substream->runtime->private_data;
	int ch = chan->idx;
	unsigned long flags;

	spin_lock_irqsave(&chip->channel[ch].lock, flags);

	/*                                          */
	chip->channel[ch].pos = 0;
	chip->channel[ch].size = 0;
	chip->channel[ch].substream = substream;

	/*                   */
	/*                                  */
	switch (substream->stream) {
	case SNDRV_PCM_STREAM_PLAYBACK:
		ad1843_setup_dac(&chip->ad1843,
				 ch - 1,
				 runtime->rate,
				 SNDRV_PCM_FORMAT_S16_LE,
				 runtime->channels);
		break;
	case SNDRV_PCM_STREAM_CAPTURE:
		ad1843_setup_adc(&chip->ad1843,
				 runtime->rate,
				 SNDRV_PCM_FORMAT_S16_LE,
				 runtime->channels);
		break;
	}
	spin_unlock_irqrestore(&chip->channel[ch].lock, flags);
	return 0;
}

/*                  */
static int snd_sgio2audio_pcm_trigger(struct snd_pcm_substream *substream,
				      int cmd)
{
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		/*                      */
		snd_sgio2audio_dma_start(substream);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		/*                     */
		snd_sgio2audio_dma_stop(substream);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/*                  */
static snd_pcm_uframes_t
snd_sgio2audio_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_sgio2audio *chip = snd_pcm_substream_chip(substream);
	struct snd_sgio2audio_chan *chan = substream->runtime->private_data;

	/*                                  */
	return bytes_to_frames(substream->runtime,
			       chip->channel[chan->idx].pos);
}

/*           */
static struct snd_pcm_ops snd_sgio2audio_playback1_ops = {
	.open =        snd_sgio2audio_playback1_open,
	.close =       snd_sgio2audio_pcm_close,
	.ioctl =       snd_pcm_lib_ioctl,
	.hw_params =   snd_sgio2audio_pcm_hw_params,
	.hw_free =     snd_sgio2audio_pcm_hw_free,
	.prepare =     snd_sgio2audio_pcm_prepare,
	.trigger =     snd_sgio2audio_pcm_trigger,
	.pointer =     snd_sgio2audio_pcm_pointer,
	.page =        snd_pcm_lib_get_vmalloc_page,
	.mmap =        snd_pcm_lib_mmap_vmalloc,
};

static struct snd_pcm_ops snd_sgio2audio_playback2_ops = {
	.open =        snd_sgio2audio_playback2_open,
	.close =       snd_sgio2audio_pcm_close,
	.ioctl =       snd_pcm_lib_ioctl,
	.hw_params =   snd_sgio2audio_pcm_hw_params,
	.hw_free =     snd_sgio2audio_pcm_hw_free,
	.prepare =     snd_sgio2audio_pcm_prepare,
	.trigger =     snd_sgio2audio_pcm_trigger,
	.pointer =     snd_sgio2audio_pcm_pointer,
	.page =        snd_pcm_lib_get_vmalloc_page,
	.mmap =        snd_pcm_lib_mmap_vmalloc,
};

static struct snd_pcm_ops snd_sgio2audio_capture_ops = {
	.open =        snd_sgio2audio_capture_open,
	.close =       snd_sgio2audio_pcm_close,
	.ioctl =       snd_pcm_lib_ioctl,
	.hw_params =   snd_sgio2audio_pcm_hw_params,
	.hw_free =     snd_sgio2audio_pcm_hw_free,
	.prepare =     snd_sgio2audio_pcm_prepare,
	.trigger =     snd_sgio2audio_pcm_trigger,
	.pointer =     snd_sgio2audio_pcm_pointer,
	.page =        snd_pcm_lib_get_vmalloc_page,
	.mmap =        snd_pcm_lib_mmap_vmalloc,
};

/*
                                              
 */

/*                     */
static int snd_sgio2audio_new_pcm(struct snd_sgio2audio *chip)
{
	struct snd_pcm *pcm;
	int err;

	/*                                                        */
	err = snd_pcm_new(chip->card, "SGI O2 Audio", 0, 1, 1, &pcm);
	if (err < 0)
		return err;

	pcm->private_data = chip;
	strcpy(pcm->name, "SGI O2 DAC1");

	/*               */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_sgio2audio_playback1_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
			&snd_sgio2audio_capture_ops);

	/*                                                         */
	err = snd_pcm_new(chip->card, "SGI O2 Audio", 1, 1, 0, &pcm);
	if (err < 0)
		return err;

	pcm->private_data = chip;
	strcpy(pcm->name, "SGI O2 DAC2");

	/*               */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_sgio2audio_playback2_ops);

	return 0;
}

static struct {
	int idx;
	int irq;
	irqreturn_t (*isr)(int, void *);
	const char *desc;
} snd_sgio2_isr_table[] = {
	{
		.idx = 0,
		.irq = MACEISA_AUDIO1_DMAT_IRQ,
		.isr = snd_sgio2audio_dma_in_isr,
		.desc = "Capture DMA Channel 0"
	}, {
		.idx = 0,
		.irq = MACEISA_AUDIO1_OF_IRQ,
		.isr = snd_sgio2audio_error_isr,
		.desc = "Capture Overflow"
	}, {
		.idx = 1,
		.irq = MACEISA_AUDIO2_DMAT_IRQ,
		.isr = snd_sgio2audio_dma_out_isr,
		.desc = "Playback DMA Channel 1"
	}, {
		.idx = 1,
		.irq = MACEISA_AUDIO2_MERR_IRQ,
		.isr = snd_sgio2audio_error_isr,
		.desc = "Memory Error Channel 1"
	}, {
		.idx = 2,
		.irq = MACEISA_AUDIO3_DMAT_IRQ,
		.isr = snd_sgio2audio_dma_out_isr,
		.desc = "Playback DMA Channel 2"
	}, {
		.idx = 2,
		.irq = MACEISA_AUDIO3_MERR_IRQ,
		.isr = snd_sgio2audio_error_isr,
		.desc = "Memory Error Channel 2"
	}
};

/*             */

static int snd_sgio2audio_free(struct snd_sgio2audio *chip)
{
	int i;

	/*                 */
	writeq(AUDIO_CONTROL_RESET, &mace->perif.audio.control);
	udelay(1);
	writeq(0, &mace->perif.audio.control);

	/*               */
	for (i = 0; i < ARRAY_SIZE(snd_sgio2_isr_table); i++)
		free_irq(snd_sgio2_isr_table[i].irq,
			 &chip->channel[snd_sgio2_isr_table[i].idx]);

	dma_free_coherent(NULL, MACEISA_RINGBUFFERS_SIZE,
			  chip->ring_base, chip->ring_base_dma);

	/*                   */
	kfree(chip);
	return 0;
}

static int snd_sgio2audio_dev_free(struct snd_device *device)
{
	struct snd_sgio2audio *chip = device->device_data;

	return snd_sgio2audio_free(chip);
}

static struct snd_device_ops ops = {
	.dev_free = snd_sgio2audio_dev_free,
};

static int snd_sgio2audio_create(struct snd_card *card,
				 struct snd_sgio2audio **rchip)
{
	struct snd_sgio2audio *chip;
	int i, err;

	*rchip = NULL;

	/*                                               */
	/*                                      */
	if (!(readq(&mace->perif.audio.control) & AUDIO_CONTROL_CODEC_PRESENT))
		return -ENOENT;

	chip = kzalloc(sizeof(struct snd_sgio2audio), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;

	chip->card = card;

	chip->ring_base = dma_alloc_coherent(NULL, MACEISA_RINGBUFFERS_SIZE,
					     &chip->ring_base_dma, GFP_USER);
	if (chip->ring_base == NULL) {
		printk(KERN_ERR
		       "sgio2audio: could not allocate ring buffers\n");
		kfree(chip);
		return -ENOMEM;
	}

	spin_lock_init(&chip->ad1843_lock);

	/*                     */
	for (i = 0; i < 3; i++) {
		spin_lock_init(&chip->channel[i].lock);
		chip->channel[i].idx = i;
	}

	/*               */
	for (i = 0; i < ARRAY_SIZE(snd_sgio2_isr_table); i++) {
		if (request_irq(snd_sgio2_isr_table[i].irq,
				snd_sgio2_isr_table[i].isr,
				0,
				snd_sgio2_isr_table[i].desc,
				&chip->channel[snd_sgio2_isr_table[i].idx])) {
			snd_sgio2audio_free(chip);
			printk(KERN_ERR "sgio2audio: cannot allocate irq %d\n",
			       snd_sgio2_isr_table[i].irq);
			return -EBUSY;
		}
	}

	/*                     */
	writeq(AUDIO_CONTROL_RESET, &mace->perif.audio.control);
	udelay(1);
	writeq(0, &mace->perif.audio.control);
	msleep_interruptible(1); /*                      */

	/*               */
	writeq(chip->ring_base_dma, &mace->perif.ctrl.ringbase);

	/*                         */
	chip->ad1843.read = read_ad1843_reg;
	chip->ad1843.write = write_ad1843_reg;
	chip->ad1843.chip = chip;

	/*                             */
	err = ad1843_init(&chip->ad1843);
	if (err < 0) {
		snd_sgio2audio_free(chip);
		return err;
	}

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);
	if (err < 0) {
		snd_sgio2audio_free(chip);
		return err;
	}
	*rchip = chip;
	return 0;
}

static int snd_sgio2audio_probe(struct platform_device *pdev)
{
	struct snd_card *card;
	struct snd_sgio2audio *chip;
	int err;

	err = snd_card_create(index, id, THIS_MODULE, 0, &card);
	if (err < 0)
		return err;

	err = snd_sgio2audio_create(card, &chip);
	if (err < 0) {
		snd_card_free(card);
		return err;
	}
	snd_card_set_dev(card, &pdev->dev);

	err = snd_sgio2audio_new_pcm(chip);
	if (err < 0) {
		snd_card_free(card);
		return err;
	}
	err = snd_sgio2audio_new_mixer(chip);
	if (err < 0) {
		snd_card_free(card);
		return err;
	}

	strcpy(card->driver, "SGI O2 Audio");
	strcpy(card->shortname, "SGI O2 Audio");
	sprintf(card->longname, "%s irq %i-%i",
		card->shortname,
		MACEISA_AUDIO1_DMAT_IRQ,
		MACEISA_AUDIO3_MERR_IRQ);

	err = snd_card_register(card);
	if (err < 0) {
		snd_card_free(card);
		return err;
	}
	platform_set_drvdata(pdev, card);
	return 0;
}

static int snd_sgio2audio_remove(struct platform_device *pdev)
{
	struct snd_card *card = platform_get_drvdata(pdev);

	snd_card_free(card);
	return 0;
}

static struct platform_driver sgio2audio_driver = {
	.probe	= snd_sgio2audio_probe,
	.remove	= snd_sgio2audio_remove,
	.driver = {
		.name	= "sgio2audio",
		.owner	= THIS_MODULE,
	}
};

module_platform_driver(sgio2audio_driver);
