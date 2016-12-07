#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikita Atroshenko <nikitatroshenko@github.com>");

static int timeout = 2;
static int timecount;

static struct timer_list timer;

static ssize_t timecount_show(struct kobject *kobj, struct kobj_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d\n", timecount);
}

static void timer_callback(unsigned long arg)
{
	printk("TIMER TICKS! HELLO, WORLD! %d\n", timecount);
	mod_timer(&timer, jiffies + timeout * HZ);
	timecount--;

	if(timecount == 0)
		del_timer(&timer);
}

static ssize_t timer_collect_counts(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%du", &timecount);

	if (timecount > 0) {
		timer.expires = jiffies + timeout * HZ;
		timer.function = timer_callback;
		add_timer(&timer);
	} else {
		del_timer_sync(&timer);
	}

	return count;
}

static struct kobj_attribute timecount_attribute =
	__ATTR(timecount, 0664, timecount_show, timer_collect_counts);

static struct attribute *attrs[] = {
	&timecount_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *timer_kobj;

static int __init timer_module_init(void)
{
	int retval;

	timer_kobj = kobject_create_and_add("output_count", kernel_kobj);
	if (!timer_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(timer_kobj, &attr_group);

	if (retval)
		kobject_put(timer_kobj);
	else
		init_timer(&timer);

	return retval;
}

static void __exit timer_module_exit(void)
{
	kobject_put(timer_kobj);
}

module_init(timer_module_init);
module_exit(timer_module_exit);

