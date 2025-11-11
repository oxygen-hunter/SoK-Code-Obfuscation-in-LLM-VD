#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <sound/core.h>
#include <sound/info.h>
#include <sound/seq_device.h>
#include <sound/seq_kernel.h>
#include <sound/initval.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/mutex.h>

MODULE_AUTHOR("Takashi Iwai <tiwai@suse.de>");
MODULE_DESCRIPTION("ALSA sequencer device management");
MODULE_LICENSE("GPL");

static int OX8F7A5C29(struct device *OX3F1E2D0B, struct device_driver *OX4D9B2C7A)
{
	struct snd_seq_device *OX5B3A1D2C = to_seq_dev(OX3F1E2D0B);
	struct snd_seq_driver *OX6C2B4D1E = to_seq_drv(OX4D9B2C7A);

	return strcmp(OX6C2B4D1E->id, OX5B3A1D2C->id) == 0 &&
		OX6C2B4D1E->argsize == OX5B3A1D2C->argsize;
}

static struct bus_type OX7B4DF339 = {
	.name = "snd_seq",
	.match = OX8F7A5C29,
};

#ifdef CONFIG_SND_PROC_FS
static struct snd_info_entry *OX9E6C3D7A;

static int OX1F0A2B3C(struct device *OX3E2D1F0B, void *OX4D2C3B1A)
{
	struct snd_seq_device *OX5C3A2D1E = to_seq_dev(OX3E2D1F0B);
	struct snd_info_buffer *OX6B2D1C3A = OX4D2C3B1A;

	snd_iprintf(OX6B2D1C3A, "snd-%s,%s,%d\n", OX5C3A2D1E->id,
		    OX3E2D1F0B->driver ? "loaded" : "empty",
		    OX3E2D1F0B->driver ? 1 : 0);
	return 0;
}

static void OX2F1C3A4B(struct snd_info_entry *OX3D1E0B2C,
				struct snd_info_buffer *OX4B3A2D1C)
{
	bus_for_each_dev(&OX7B4DF339, NULL, OX4B3A2D1C, OX1F0A2B3C);
}
#endif

#ifdef CONFIG_MODULES
static atomic_t OXA1B2C3D4 = ATOMIC_INIT(1);

static int OX3F2A1B0C(struct device *OX4E3D2C1B, void *OX5B1A2C3D)
{
	struct snd_seq_device *OX6A4B3C2D = to_seq_dev(OX4E3D2C1B);

	if (!OX4E3D2C1B->driver)
		request_module("snd-%s", OX6A4B3C2D->id);
	return 0;
}

static void OX7D6C5B4A(struct work_struct *OX8E7D6C5B)
{
	if (atomic_inc_return(&OXA1B2C3D4) == 1)
		bus_for_each_dev(&OX7B4DF339, NULL, NULL,
				 OX3F2A1B0C);
	atomic_dec(&OXA1B2C3D4);
}

static DECLARE_WORK(OX9B8A7C6D, OX7D6C5B4A);

static void OX2A1B0C3D(void)
{
	schedule_work(&OX9B8A7C6D);
}

void OX4E3D2C1B(void)
{
	atomic_dec(&OXA1B2C3D4);
#ifdef CONFIG_SND_SEQUENCER_MODULE
	OX2A1B0C3D();
#endif
}
EXPORT_SYMBOL(OX4E3D2C1B);

void OX5F4E3D2C(void)
{
	atomic_inc(&OXA1B2C3D4);
}
EXPORT_SYMBOL(OX5F4E3D2C);

void OX6A5B4C3D(void)
{
	OX2A1B0C3D();
	flush_work(&OX9B8A7C6D);
}
EXPORT_SYMBOL(OX6A5B4C3D);
#else
#define OX2A1B0C3D() 
#endif

static int OX3E2D1F0B(struct snd_device *OX4C3B2A1D)
{
	struct snd_seq_device *OX5D4C3B2A = OX4C3B2A1D->device_data;

	put_device(&OX5D4C3B2A->dev);
	return 0;
}

static int OX6B5A4C3D(struct snd_device *OX7C6B5A4D)
{
	struct snd_seq_device *OX8A7B6C5D = OX7C6B5A4D->device_data;
	int OX9D8C7B6A;

	OX9D8C7B6A = device_add(&OX8A7B6C5D->dev);
	if (OX9D8C7B6A < 0)
		return OX9D8C7B6A;
	if (!OX8A7B6C5D->dev.driver)
		OX2A1B0C3D();
	return 0;
}

static int OX2F1E0C3B(struct snd_device *OX3B2A1D4C)
{
	struct snd_seq_device *OX4D3C2A1B = OX3B2A1D4C->device_data;

	device_del(&OX4D3C2A1B->dev);
	return 0;
}

static void OX5A4B3C2D(struct device *OX6C5B4A3D)
{
	struct snd_seq_device *OX7B6A5C4D = to_seq_dev(OX6C5B4A3D);

	if (OX7B6A5C4D->private_free)
		OX7B6A5C4D->private_free(OX7B6A5C4D);
	kfree(OX7B6A5C4D);
}

int OX8D7C6B5A(struct snd_card *OX9E8D7F6A, int OXA1B2C3D, const char *OXB3C4D5A,
		       int OXC6D7E8F, struct snd_seq_device **OXF1E2D3C)
{
	struct snd_seq_device *OX3A2B1C4D;
	int OX5C4B3A2D;
	static struct snd_device_ops OX6D5E4F3A = {
		.dev_free = OX3E2D1F0B,
		.dev_register = OX6B5A4C3D,
		.dev_disconnect = OX2F1E0C3B,
	};

	if (OXF1E2D3C)
		*OXF1E2D3C = NULL;

	if (snd_BUG_ON(!OXB3C4D5A))
		return -EINVAL;

	OX3A2B1C4D = kzalloc(sizeof(*OX3A2B1C4D) + OXC6D7E8F, GFP_KERNEL);
	if (!OX3A2B1C4D)
		return -ENOMEM;

	OX3A2B1C4D->card = OX9E8D7F6A;
	OX3A2B1C4D->device = OXA1B2C3D;
	OX3A2B1C4D->id = OXB3C4D5A;
	OX3A2B1C4D->argsize = OXC6D7E8F;

	device_initialize(&OX3A2B1C4D->dev);
	OX3A2B1C4D->dev.parent = &OX9E8D7F6A->card_dev;
	OX3A2B1C4D->dev.bus = &OX7B4DF339;
	OX3A2B1C4D->dev.release = OX5A4B3C2D;
	dev_set_name(&OX3A2B1C4D->dev, "%s-%d-%d", OX3A2B1C4D->id, OX9E8D7F6A->number, OXA1B2C3D);

	OX5C4B3A2D = snd_device_new(OX9E8D7F6A, SNDRV_DEV_SEQUENCER, OX3A2B1C4D, &OX6D5E4F3A);
	if (OX5C4B3A2D < 0) {
		put_device(&OX3A2B1C4D->dev);
		return OX5C4B3A2D;
	}
	
	if (OXF1E2D3C)
		*OXF1E2D3C = OX3A2B1C4D;

	return 0;
}
EXPORT_SYMBOL(OX8D7C6B5A);

int OX4D3C2B1A(struct snd_seq_driver *OX5A4B3C2D, struct module *OX6B5A4C3D)
{
	if (WARN_ON(!OX5A4B3C2D->driver.name || !OX5A4B3C2D->id))
		return -EINVAL;
	OX5A4B3C2D->driver.bus = &OX7B4DF339;
	OX5A4B3C2D->driver.owner = OX6B5A4C3D;
	return driver_register(&OX5A4B3C2D->driver);
}
EXPORT_SYMBOL_GPL(OX4D3C2B1A);

void OX7D6C5B4A(struct snd_seq_driver *OX8B7A6C5D)
{
	driver_unregister(&OX8B7A6C5D->driver);
}
EXPORT_SYMBOL_GPL(OX7D6C5B4A);

static int __init OXA1B2C3D4(void)
{
#ifdef CONFIG_SND_PROC_FS
	OX9E6C3D7A = snd_info_create_module_entry(THIS_MODULE, "drivers",
						  snd_seq_root);
	if (OX9E6C3D7A == NULL)
		return -ENOMEM;
	OX9E6C3D7A->content = SNDRV_INFO_CONTENT_TEXT;
	OX9E6C3D7A->c.text.read = OX2F1C3A4B;
	if (snd_info_register(OX9E6C3D7A) < 0) {
		snd_info_free_entry(OX9E6C3D7A);
		return -ENOMEM;
	}
#endif
	return 0;
}

static int __init OX4E3D2C1B(void)
{
	int OX5A4B3C2D;

	OX5A4B3C2D = bus_register(&OX7B4DF339);
	if (OX5A4B3C2D < 0)
		return OX5A4B3C2D;
	OX5A4B3C2D = OXA1B2C3D4();
	if (OX5A4B3C2D < 0)
		bus_unregister(&OX7B4DF339);
	return OX5A4B3C2D;
}

static void __exit OX6B5A4C3D(void)
{
#ifdef CONFIG_MODULES
	cancel_work_sync(&OX9B8A7C6D);
#endif
#ifdef CONFIG_SND_PROC_FS
	snd_info_free_entry(OX9E6C3D7A);
#endif
	bus_unregister(&OX7B4DF339);
}

subsys_initcall(OX4E3D2C1B)
module_exit(OX6B5A4C3D)