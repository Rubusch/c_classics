
#include <linux/module.h>
#include <linux/fs.h> /* struct file_operations */
/* platform_driver_register(), platform_set_drvdata() */
#include <linux/platform_device.h>
#include <linux/io.h> /* devm_ioremap(), iowrite32() */
#include <linux/of.h> /* of_property_read_string() */
#include <linux/uaccess.h> /* copy_from_user(), copy_to_user() */
#include <linux/miscdevice.h> /* misc_register() */

/* declare a private structure */
struct led_dev
{
	struct miscdevice led_misc_device; /* assign device for each led */
	u32 led_mask; /* different mask if led is R,G or B */
	const char *led_name; /* assigned value cannot be modified */
	char led_value[8];
};

/* Declare physical addresses */
static int PIO_SODR1 = 0xFC038050;
static int PIO_CODR1 = 0xFC038054;
static int PIO_MSKR1 = 0xFC038040;
static int PIO_CFGR1 = 0xFC038044;

/* Declare __iomem pointers that will keep virtual addresses */
static void __iomem *PIO_SODR1_W;
static void __iomem *PIO_CODR1_W;
static void __iomem *PIO_MSKR1_V;
static void __iomem *PIO_CFGR1_V;

/* Declare masks to configure the different registers */
#define PIO_PB0_MASK (1 << 0) /* blue */
#define PIO_PB5_MASK (1 << 5) /* green */
#define PIO_PB6_MASK (1 << 6) /* red */
#define PIO_CFGR1_MASK (1 << 8) /* masked bits direction (output), no PUEN, no PDEN */
#define PIO_MASK_ALL_LEDS (PIO_PB0_MASK | PIO_PB5_MASK | PIO_PB6_MASK)

/* send on/off value from our terminal to control each led */
static ssize_t led_write(struct file *file, const char __user *buff,
						 size_t count, loff_t *ppos)
{
	const char *led_on = "on";
	const char *led_off = "off";
	struct led_dev *led_device;

	pr_info("led_write() is called.\n");

	led_device = container_of(file->private_data,
				  struct led_dev, led_misc_device);

	/*
	 * terminal echo add \n character.
	 * led_device->led_value = "on\n" or "off\n after copy_from_user"
	 * count = 3 for "on\n" and 4 for "off\n"
	 */
	if(copy_from_user(led_device->led_value, buff, count)) {
		pr_info("Bad copied value\n");
		return -EFAULT;
	}

	/*
	 * Replace \n for \0 in led_device->led_value
	 * char array to create a char string
	 */
	led_device->led_value[count-1] = '\0';

	pr_info("This message is received from User Space: %s\n",
			led_device->led_value);

	/* compare strings to switch on/off the LED */
	if(!strcmp(led_device->led_value, led_on)) {
		iowrite32(led_device->led_mask, PIO_CODR1_W);
	}
	else if (!strcmp(led_device->led_value, led_off)) {
		iowrite32(led_device->led_mask, PIO_SODR1_W);
	}
	else {
		pr_info("Bad value\n");
		return -EINVAL;
	}

	pr_info("led_write() is exit.\n");
	return count;
}

/*
 * read each LED status on/off
 * use cat from terminal to read
 * led_read is entered until *ppos > 0
 * twice in this function
 */
static ssize_t led_read(struct file *file, char __user *buff,
						size_t count, loff_t *ppos)
{
	int len;
	struct led_dev *led_device;

	led_device = container_of(file->private_data, struct led_dev,
							  led_misc_device);

	if(*ppos == 0){
		len = strlen(led_device->led_value);
		pr_info("the size of the message is %d\n", len); /* 2 for on */
		led_device->led_value[len] = '\n'; /* add \n after on/off */
		if(copy_to_user(buff, &led_device->led_value, len+1)){
			pr_info("Failed to return led_value to user space\n");
			return -EFAULT;
		}
		*ppos+=1; /* increment *ppos to exit the function in next call */
		return sizeof(led_device->led_value); /* exit first func call */
	}

	return 0; /* exit and do not recall func again */
}

static const struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.read = led_read,
	.write = led_write,
};

static int __init led_probe(struct platform_device *pdev)
{
	/* create a private structure */
	struct led_dev *led_device;
	int ret_val;

	/* initialize all the leds to off */
	char led_val[8] = "off\n";

	pr_info("led_probe enter\n");
	
	/* Get virtual addresses */
	PIO_MSKR1_V = devm_ioremap(&pdev->dev, PIO_MSKR1, sizeof(u32));
	PIO_SODR1_W = devm_ioremap(&pdev->dev, PIO_SODR1, sizeof(u32));
	PIO_CODR1_W = devm_ioremap(&pdev->dev, PIO_CODR1, sizeof(u32));
	PIO_CFGR1_V = devm_ioremap(&pdev->dev, PIO_CFGR1, sizeof(u32));
	
	/* Initialize all the virtual registers */
	iowrite32(PIO_MASK_ALL_LEDS, PIO_MSKR1_V); /* Enable all leds */
	iowrite32(PIO_CFGR1_MASK, PIO_CFGR1_V); /* set enabled leds to output */
	iowrite32(PIO_MASK_ALL_LEDS, PIO_SODR1_W); /* Clear all the leds */

	/* Allocate a private structure */
	led_device = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);

	/*
	 * read each node label property in each probe() call
	 * probe() is called 3 times, once per compatible = "arrow,RGBleds"
	 * found below each ledred, ledgreen and ledblue node
	 */
	of_property_read_string(pdev->dev.of_node, "label", &led_device->led_name);

	/* create a device for each led found */
	led_device->led_misc_device.minor = MISC_DYNAMIC_MINOR;
	led_device->led_misc_device.name = led_device->led_name;
	led_device->led_misc_device.fops = &led_fops;

	/* Assigns a different mask for each led */
	if (strcmp(led_device->led_name,"ledred") == 0) {
		led_device->led_mask = PIO_PB6_MASK;
	}
	else if (strcmp(led_device->led_name,"ledgreen") == 0) {
		led_device->led_mask = PIO_PB5_MASK;
	}
	else if (strcmp(led_device->led_name,"ledblue") == 0) {
		led_device->led_mask = PIO_PB0_MASK;
	}
	else {
		pr_info("Bad device tree value\n");
		return -EINVAL;
	}
	
	/* Initialize each led status to off */
	memcpy(led_device->led_value, led_val, sizeof(led_val));

	/* register each led device */
	ret_val = misc_register(&led_device->led_misc_device);
	if (ret_val) return ret_val; /* misc_register returns 0 if success */

	/*
	 * Attach the private structure to the pdev structure
	 * to recover it in each remove() function call
	 */
	platform_set_drvdata(pdev, led_device);

	pr_info("leds_probe exit\n");

	return 0;
}

/* The remove() function is called 3 times once per led */
static int __exit led_remove(struct platform_device *pdev)
{

	struct led_dev *led_device = platform_get_drvdata(pdev);

	pr_info("leds_remove enter\n");

	misc_deregister(&led_device->led_misc_device);

	pr_info("leds_remove exit\n");

	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,RGBleds"},
	{},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = led_probe,
	.remove = led_remove,
	.driver = {
		.name = "RGBleds",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int led_init(void)
{
	int ret_val;
	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&led_platform_driver);
	if (ret_val !=0)
	{
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;
	}

	pr_info("demo_init exit\n");
	return 0;
}

static void led_exit(void)
{
	pr_info("led driver enter\n");

	/* Clear all the leds before exiting */
	iowrite32(PIO_MASK_ALL_LEDS, PIO_SODR1_W);

	platform_driver_unregister(&led_platform_driver);

	pr_info("led driver exit\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off \
					three led devices");

