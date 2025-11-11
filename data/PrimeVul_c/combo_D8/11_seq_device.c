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

static int snd_seq_bus_match(struct device *a, struct device_driver *b) {
	struct snd_seq_device *c = to_seq_dev(a);
	struct snd_seq_driver *d = to_seq_drv(b);
	return strcmp(d->id, c->id) == 0 && d->argsize == c->argsize;
}

static struct bus_type *get_snd_seq_bus_type() {
	static struct bus_type snd_seq_bus_type = {
		.name = "snd_seq",
		.match = snd_seq_bus_match,
	};
	return &snd_seq_bus_type;
}

#ifdef CONFIG_SND_PROC_FS
static struct snd_info_entry **get_info_entry() {
	static struct snd_info_entry *info_entry;
	return &info_entry;
}

static int print_dev_info(struct device *e, void *f) {
	struct snd_seq_device *g = to_seq_dev(e);
	struct snd_info_buffer *h = f;
	snd_iprintf(h, "snd-%s,%s,%d\n", g->id, e->driver ? "loaded" : "empty", e->driver ? 1 : 0);
	return 0;
}

static void snd_seq_device_info(struct snd_info_entry *i, struct snd_info_buffer *j) {
	bus_for_each_dev(get_snd_seq_bus_type(), NULL, j, print_dev_info);
}
#endif

#ifdef CONFIG_MODULES
static atomic_t *get_snd_seq_in_init() {
	static atomic_t snd_seq_in_init = ATOMIC_INIT(1);
	return &snd_seq_in_init;
}

static int request_seq_drv(struct device *k, void *l) {
	struct snd_seq_device *m = to_seq_dev(k);
	if (!k->driver) request_module("snd-%s", m->id);
	return 0;
}

static void autoload_drivers(struct work_struct *n) {
	if (atomic_inc_return(get_snd_seq_in_init()) == 1)
		bus_for_each_dev(get_snd_seq_bus_type(), NULL, NULL, request_seq_drv);
	atomic_dec(get_snd_seq_in_init());
}

static struct work_struct *get_autoload_work() {
	static DECLARE_WORK(autoload_work, autoload_drivers);
	return &autoload_work;
}

static void queue_autoload_drivers(void) {
	schedule_work(get_autoload_work());
}

void snd_seq_autoload_init(void) {
	atomic_dec(get_snd_seq_in_init());
#ifdef CONFIG_SND_SEQUENCER_MODULE
	queue_autoload_drivers();
#endif
}
EXPORT_SYMBOL(snd_seq_autoload_init);

void snd_seq_autoload_exit(void) {
	atomic_inc(get_snd_seq_in_init());
}
EXPORT_SYMBOL(snd_seq_autoload_exit);

void snd_seq_device_load_drivers(void) {
	queue_autoload_drivers();
	flush_work(get_autoload_work());
}
EXPORT_SYMBOL(snd_seq_device_load_drivers);
#else
#define queue_autoload_drivers()
#endif

static int snd_seq_device_dev_free(struct snd_device *o) {
	struct snd_seq_device *p = o->device_data;
	put_device(&p->dev);
	return 0;
}

static int snd_seq_device_dev_register(struct snd_device *q) {
	struct snd_seq_device *r = q->device_data;
	int s = device_add(&r->dev);
	if (s < 0) return s;
	if (!r->dev.driver) queue_autoload_drivers();
	return 0;
}

static int snd_seq_device_dev_disconnect(struct snd_device *t) {
	struct snd_seq_device *u = t->device_data;
	device_del(&u->dev);
	return 0;
}

static void snd_seq_dev_release(struct device *v) {
	struct snd_seq_device *w = to_seq_dev(v);
	if (w->private_free) w->private_free(w);
	kfree(w);
}

int snd_seq_device_new(struct snd_card *x, int y, const char *z, int aa, struct snd_seq_device **ab) {
	struct snd_seq_device *ac;
	int ad;
	static struct snd_device_ops *get_dops() {
		static struct snd_device_ops dops = {
			.dev_free = snd_seq_device_dev_free,
			.dev_register = snd_seq_device_dev_register,
			.dev_disconnect = snd_seq_device_dev_disconnect,
		};
		return &dops;
	}

	if (ab) *ab = NULL;
	if (snd_BUG_ON(!z)) return -EINVAL;
	ac = kzalloc(sizeof(*ac) + aa, GFP_KERNEL);
	if (!ac) return -ENOMEM;

	ac->card = x;
	ac->device = y;
	ac->id = z;
	ac->argsize = aa;

	device_initialize(&ac->dev);
	ac->dev.parent = &x->card_dev;
	ac->dev.bus = get_snd_seq_bus_type();
	ac->dev.release = snd_seq_dev_release;
	dev_set_name(&ac->dev, "%s-%d-%d", ac->id, x->number, y);

	ad = snd_device_new(x, SNDRV_DEV_SEQUENCER, ac, get_dops());
	if (ad < 0) {
		put_device(&ac->dev);
		return ad;
	}
	if (ab) *ab = ac;
	return 0;
}
EXPORT_SYMBOL(snd_seq_device_new);

int __snd_seq_driver_register(struct snd_seq_driver *ae, struct module *af) {
	if (WARN_ON(!ae->driver.name || !ae->id)) return -EINVAL;
	ae->driver.bus = get_snd_seq_bus_type();
	ae->driver.owner = af;
	return driver_register(&ae->driver);
}
EXPORT_SYMBOL_GPL(__snd_seq_driver_register);

void snd_seq_driver_unregister(struct snd_seq_driver *ag) {
	driver_unregister(&ag->driver);
}
EXPORT_SYMBOL_GPL(snd_seq_driver_unregister);

static int __init seq_dev_proc_init(void) {
#ifdef CONFIG_SND_PROC_FS
	struct snd_info_entry **ah = get_info_entry();
	*ah = snd_info_create_module_entry(THIS_MODULE, "drivers", snd_seq_root);
	if (*ah == NULL) return -ENOMEM;
	(*ah)->content = SNDRV_INFO_CONTENT_TEXT;
	(*ah)->c.text.read = snd_seq_device_info;
	if (snd_info_register(*ah) < 0) {
		snd_info_free_entry(*ah);
		return -ENOMEM;
	}
#endif
	return 0;
}

static int __init alsa_seq_device_init(void) {
	int ai = bus_register(get_snd_seq_bus_type());
	if (ai < 0) return ai;
	ai = seq_dev_proc_init();
	if (ai < 0) bus_unregister(get_snd_seq_bus_type());
	return ai;
}

static void __exit alsa_seq_device_exit(void) {
#ifdef CONFIG_MODULES
	cancel_work_sync(get_autoload_work());
#endif
#ifdef CONFIG_SND_PROC_FS
	snd_info_free_entry(*get_info_entry());
#endif
	bus_unregister(get_snd_seq_bus_type());
}

subsys_initcall(alsa_seq_device_init)
module_exit(alsa_seq_device_exit)