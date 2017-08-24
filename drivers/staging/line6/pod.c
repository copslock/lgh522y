/*
 * Line6 Linux USB driver - 0.9.1beta
 *
 * Copyright (C) 2004-2010 Markus Grabner (grabner@icg.tugraz.at)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 */

#include <linux/slab.h>
#include <linux/wait.h>
#include <sound/control.h>

#include "audio.h"
#include "capture.h"
#include "driver.h"
#include "playback.h"
#include "pod.h"

#define POD_SYSEX_CODE 3
#define POD_BYTES_PER_FRAME 6	/*                      */

/*              */

enum {
	POD_SYSEX_SAVE      = 0x24,
	POD_SYSEX_SYSTEM    = 0x56,
	POD_SYSEX_SYSTEMREQ = 0x57,
	/*                             */  /*                  */
	POD_SYSEX_STORE     = 0x71,
	POD_SYSEX_FINISH    = 0x72,
	POD_SYSEX_DUMPMEM   = 0x73,
	POD_SYSEX_DUMP      = 0x74,
	POD_SYSEX_DUMPREQ   = 0x75

	/*                                           */
	/*                            */
};

enum {
	POD_MONITOR_LEVEL  = 0x04,
	POD_SYSTEM_INVALID = 0x10000
};

/*             */

enum {
	POD_DUMP_MEMORY = 2
};

enum {
	POD_BUSY_READ,
	POD_BUSY_WRITE,
	POD_CHANNEL_DIRTY,
	POD_SAVE_PRESSED,
	POD_BUSY_MIDISEND
};

static struct snd_ratden pod_ratden = {
	.num_min = 78125,
	.num_max = 78125,
	.num_step = 1,
	.den = 2
};

static struct line6_pcm_properties pod_pcm_properties = {
	.snd_line6_playback_hw = {
				  .info = (SNDRV_PCM_INFO_MMAP |
					   SNDRV_PCM_INFO_INTERLEAVED |
					   SNDRV_PCM_INFO_BLOCK_TRANSFER |
					   SNDRV_PCM_INFO_MMAP_VALID |
					   SNDRV_PCM_INFO_PAUSE |
#ifdef CONFIG_PM
					   SNDRV_PCM_INFO_RESUME |
#endif
					   SNDRV_PCM_INFO_SYNC_START),
				  .formats = SNDRV_PCM_FMTBIT_S24_3LE,
				  .rates = SNDRV_PCM_RATE_KNOT,
				  .rate_min = 39062,
				  .rate_max = 39063,
				  .channels_min = 2,
				  .channels_max = 2,
				  .buffer_bytes_max = 60000,
				  .period_bytes_min = 64,
				  .period_bytes_max = 8192,
				  .periods_min = 1,
				  .periods_max = 1024},
	.snd_line6_capture_hw = {
				 .info = (SNDRV_PCM_INFO_MMAP |
					  SNDRV_PCM_INFO_INTERLEAVED |
					  SNDRV_PCM_INFO_BLOCK_TRANSFER |
					  SNDRV_PCM_INFO_MMAP_VALID |
#ifdef CONFIG_PM
					  SNDRV_PCM_INFO_RESUME |
#endif
					  SNDRV_PCM_INFO_SYNC_START),
				 .formats = SNDRV_PCM_FMTBIT_S24_3LE,
				 .rates = SNDRV_PCM_RATE_KNOT,
				 .rate_min = 39062,
				 .rate_max = 39063,
				 .channels_min = 2,
				 .channels_max = 2,
				 .buffer_bytes_max = 60000,
				 .period_bytes_min = 64,
				 .period_bytes_max = 8192,
				 .periods_min = 1,
				 .periods_max = 1024},
	.snd_line6_rates = {
			    .nrats = 1,
			    .rats = &pod_ratden},
	.bytes_per_frame = POD_BYTES_PER_FRAME
};

static const char pod_version_header[] = {
	0xf2, 0x7e, 0x7f, 0x06, 0x02
};

/*                       */
static void pod_startup2(unsigned long data);
static void pod_startup3(struct usb_line6_pod *pod);

static char *pod_alloc_sysex_buffer(struct usb_line6_pod *pod, int code,
				    int size)
{
	return line6_alloc_sysex_buffer(&pod->line6, POD_SYSEX_CODE, code,
					size);
}

/*
                                       
*/
void line6_pod_process_message(struct usb_line6_pod *pod)
{
	const unsigned char *buf = pod->line6.buffer_message;

	if (memcmp(buf, pod_version_header, sizeof(pod_version_header)) == 0) {
		pod->firmware_version = buf[13] * 100 + buf[14] * 10 + buf[15];
		pod->device_id = ((int)buf[8] << 16) | ((int)buf[9] << 8) |
				 (int) buf[10];
		pod_startup3(pod);
		return;
	}

	/*                                               */
	if (buf[0] != (LINE6_SYSEX_BEGIN | LINE6_CHANNEL_DEVICE) &&
	    buf[0] != (LINE6_SYSEX_BEGIN | LINE6_CHANNEL_UNKNOWN)) {
		return;
	}
	if (memcmp(buf + 1, line6_midi_id, sizeof(line6_midi_id)) != 0)
		return;

	if (buf[5] == POD_SYSEX_SYSTEM && buf[6] == POD_MONITOR_LEVEL) {
		short value = ((int)buf[7] << 12) | ((int)buf[8] << 8) |
			      ((int)buf[9] << 4) | (int)buf[10];
		pod->monitor_level = value;
	}
}

/*
                                      
*/
void line6_pod_transmit_parameter(struct usb_line6_pod *pod, int param,
				  u8 value)
{
	line6_transmit_parameter(&pod->line6, param, value);
}

/*
                                      
*/
static int pod_set_system_param_int(struct usb_line6_pod *pod, int value,
				    int code)
{
	char *sysex;
	static const int size = 5;

	sysex = pod_alloc_sysex_buffer(pod, POD_SYSEX_SYSTEM, size);
	if (!sysex)
		return -ENOMEM;
	sysex[SYSEX_DATA_OFS] = code;
	sysex[SYSEX_DATA_OFS + 1] = (value >> 12) & 0x0f;
	sysex[SYSEX_DATA_OFS + 2] = (value >> 8) & 0x0f;
	sysex[SYSEX_DATA_OFS + 3] = (value >> 4) & 0x0f;
	sysex[SYSEX_DATA_OFS + 4] = (value) & 0x0f;
	line6_send_sysex_message(&pod->line6, sysex, size);
	kfree(sysex);
	return 0;
}

/*
                                                
*/
static ssize_t pod_get_serial_number(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct usb_interface *interface = to_usb_interface(dev);
	struct usb_line6_pod *pod = usb_get_intfdata(interface);
	return sprintf(buf, "%d\n", pod->serial_number);
}

/*
                                                   
*/
static ssize_t pod_get_firmware_version(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct usb_interface *interface = to_usb_interface(dev);
	struct usb_line6_pod *pod = usb_get_intfdata(interface);
	return sprintf(buf, "%d.%02d\n", pod->firmware_version / 100,
		       pod->firmware_version % 100);
}

/*
                                            
*/
static ssize_t pod_get_device_id(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct usb_interface *interface = to_usb_interface(dev);
	struct usb_line6_pod *pod = usb_get_intfdata(interface);
	return sprintf(buf, "%d\n", pod->device_id);
}

/*
                       
                                                                      
                                                                    
                                                                       
*/

static void pod_startup1(struct usb_line6_pod *pod)
{
	CHECK_STARTUP_PROGRESS(pod->startup_progress, POD_STARTUP_INIT);

	/*                          */
	line6_start_timer(&pod->startup_timer, POD_STARTUP_DELAY, pod_startup2,
			  (unsigned long)pod);
}

static void pod_startup2(unsigned long data)
{
	struct usb_line6_pod *pod = (struct usb_line6_pod *)data;
	struct usb_line6 *line6 = &pod->line6;
	CHECK_STARTUP_PROGRESS(pod->startup_progress, POD_STARTUP_VERSIONREQ);

	/*                           */
	line6_version_request_async(line6);
}

static void pod_startup3(struct usb_line6_pod *pod)
{
	CHECK_STARTUP_PROGRESS(pod->startup_progress, POD_STARTUP_WORKQUEUE);

	/*                                      */
	schedule_work(&pod->startup_work);
}

static void pod_startup4(struct work_struct *work)
{
	struct usb_line6_pod *pod =
	    container_of(work, struct usb_line6_pod, startup_work);
	struct usb_line6 *line6 = &pod->line6;

	CHECK_STARTUP_PROGRESS(pod->startup_progress, POD_STARTUP_SETUP);

	/*                */
	line6_read_serial_number(&pod->line6, &pod->serial_number);

	/*                       */
	line6_register_audio(line6);
}

/*                    */
static DEVICE_ATTR(device_id, S_IRUGO, pod_get_device_id, line6_nop_write);
static DEVICE_ATTR(firmware_version, S_IRUGO, pod_get_firmware_version,
		   line6_nop_write);
static DEVICE_ATTR(serial_number, S_IRUGO, pod_get_serial_number,
		   line6_nop_write);

/*                       */
static int snd_pod_control_monitor_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65535;
	return 0;
}

/*                      */
static int snd_pod_control_monitor_get(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_line6_pcm *line6pcm = snd_kcontrol_chip(kcontrol);
	struct usb_line6_pod *pod = (struct usb_line6_pod *)line6pcm->line6;
	ucontrol->value.integer.value[0] = pod->monitor_level;
	return 0;
}

/*                      */
static int snd_pod_control_monitor_put(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_line6_pcm *line6pcm = snd_kcontrol_chip(kcontrol);
	struct usb_line6_pod *pod = (struct usb_line6_pod *)line6pcm->line6;

	if (ucontrol->value.integer.value[0] == pod->monitor_level)
		return 0;

	pod->monitor_level = ucontrol->value.integer.value[0];
	pod_set_system_param_int(pod, ucontrol->value.integer.value[0],
				 POD_MONITOR_LEVEL);
	return 1;
}

/*                    */
static struct snd_kcontrol_new pod_control_monitor = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Monitor Playback Volume",
	.index = 0,
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.info = snd_pod_control_monitor_info,
	.get = snd_pod_control_monitor_get,
	.put = snd_pod_control_monitor_put
};

/*
                
*/
static void pod_destruct(struct usb_interface *interface)
{
	struct usb_line6_pod *pod = usb_get_intfdata(interface);

	if (pod == NULL)
		return;
	line6_cleanup_audio(&pod->line6);

	del_timer(&pod->startup_timer);
	cancel_work_sync(&pod->startup_work);
}

/*
                      
*/
static int pod_create_files2(struct device *dev)
{
	int err;

	CHECK_RETURN(device_create_file(dev, &dev_attr_device_id));
	CHECK_RETURN(device_create_file(dev, &dev_attr_firmware_version));
	CHECK_RETURN(device_create_file(dev, &dev_attr_serial_number));
	return 0;
}

/*
                         
*/
static int pod_try_init(struct usb_interface *interface,
			struct usb_line6_pod *pod)
{
	int err;
	struct usb_line6 *line6 = &pod->line6;

	init_timer(&pod->startup_timer);
	INIT_WORK(&pod->startup_work, pod_startup4);

	if ((interface == NULL) || (pod == NULL))
		return -ENODEV;

	/*                       */
	err = pod_create_files2(&interface->dev);
	if (err < 0)
		return err;

	/*                          */
	err = line6_init_audio(line6);
	if (err < 0)
		return err;

	/*                            */
	err = line6_init_midi(line6);
	if (err < 0)
		return err;

	/*                           */
	err = line6_init_pcm(line6, &pod_pcm_properties);
	if (err < 0)
		return err;

	/*                           */
	err = snd_ctl_add(line6->card,
			  snd_ctl_new1(&pod_control_monitor, line6->line6pcm));
	if (err < 0)
		return err;

	/*
                                                                   
                                                                    
            
  */

	if (pod->line6.properties->capabilities & LINE6_BIT_CONTROL) {
		pod->monitor_level = POD_SYSTEM_INVALID;

		/*                             */
		pod_startup1(pod);
	}

	return 0;
}

/*
                                                    
*/
int line6_pod_init(struct usb_interface *interface, struct usb_line6_pod *pod)
{
	int err = pod_try_init(interface, pod);

	if (err < 0)
		pod_destruct(interface);

	return err;
}

/*
                         
*/
void line6_pod_disconnect(struct usb_interface *interface)
{
	struct usb_line6_pod *pod;

	if (interface == NULL)
		return;
	pod = usb_get_intfdata(interface);

	if (pod != NULL) {
		struct snd_line6_pcm *line6pcm = pod->line6.line6pcm;
		struct device *dev = &interface->dev;

		if (line6pcm != NULL)
			line6_pcm_disconnect(line6pcm);

		if (dev != NULL) {
			/*                       */
			device_remove_file(dev, &dev_attr_device_id);
			device_remove_file(dev, &dev_attr_firmware_version);
			device_remove_file(dev, &dev_attr_serial_number);
		}
	}

	pod_destruct(interface);
}
