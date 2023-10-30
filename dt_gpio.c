#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/proc_fs.h>

/* Meta Information */
MODULE_LICENSE("GPL");

/* Declate the probe and remove functions */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "brightlight,mydev"
	},

	{/* sentinel */}
};

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
	.probe = dt_probe,
	.remove = dt_remove,
	.driver = {
		.name = "my_device_driver",
		.of_match_table = my_driver_ids
	},
};

//gpio variable
static struct gpio_desc *my_led = NULL;

static struct proc_dir_entry *proc_file;

/**
 * @brief Write data to buffer
 */
static ssize_t my_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	switch (user_buffer[0]) {
		case '0':
			gpiod_set_value(my_led, user_buffer[0] - '0');
			break;
		case '1':
			gpiod_set_value(my_led, user_buffer[0] - '0');
			break;
		default:
			break;
	}
	return count;
}

static struct proc_ops fops = {
	.proc_write = my_write,
};

static int dt_probe(struct platform_device*pdev) {
	struct device *dev = &pdev->dev;

	printk("dt-gpio - now im in the probe function!\n");

	if(!device_property_present(dev, "y-led-gpio")) {
		printk("dt-gpio - device property y-led-gpio not found!\n");
		return -1;
	}

	my_led = gpiod_get(dev, "y-led", GPIOD_OUT_LOW);
	if(IS_ERR(my_led)) {
		printk("dt-gpio - could not set up gpio!\n");
		return -1;
	}

	proc_file = proc_create("my-led", 0666, NULL, &fops);
	if(proc_file == NULL) {
		printk("dt-gpio - Error creating /proc/my-led\n");
		gpiod_put(my_led);
		return -ENOMEM;
	}

	return 0;
}

static int dt_remove(struct platform_device *pdev) {
	printk("dt-gpio - now im in the remove function!\n");
	gpiod_put(my_led);
	proc_remove(proc_file);
	return 0;
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("dt-gpio - Hello, Kernel!\n");

	if(platform_driver_register(&my_driver)) {
		printk("dt-gpio - could not load driver!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("dt-gpio - Goodbye, Kernel\n");

	platform_driver_unregister(&my_driver);
}

module_init(my_init);
module_exit(my_exit);