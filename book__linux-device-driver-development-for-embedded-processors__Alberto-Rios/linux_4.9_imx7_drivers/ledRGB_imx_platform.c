
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
#define GPDAT1 0x30200000
#define GPDIR1 0x30200004
#define GPDAT6 0x30250000
#define GPDIR6 0x30250004

/* Declare masks to configure the different registers */
#define GPIO1_DIR_MASK 1 << 2
#define GPIO1_DATA_MASK 1 << 2

#define GPIO6_DIR_MASK (1 << 20 | 1 << 21)
#define GPIO6_DATA_MASK (1 << 20 | 1 << 21)

#define LED_RED_MASK 1 << 2
#define LED_GREEN_MASK 1 << 20
#define LED_BLUE_MASK 1 << 21

/* Declare __iomem pointers that will keep virtual addresses */
static void __iomem *GPDAT1_V;
static void __iomem *GPDIR1_V;
static void __iomem *GPDAT6_V;
static void __iomem *GPDIR6_V;

/* send on/off value from our terminal to control each led */
static ssize_t led_write(struct file *file, const char __user *buff,
						 size_t count, loff_t *ppos)
{
	const char *led_on = "on";
	const char *led_off = "off";
	struct led_dev *led_device;
	u32 read, write;

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

	pr_info("This message received from User Space: %s\n",
			led_device->led_value);

	/* compare strings to switch on/off the LED */
	if (strcmp(led_device->led_name,"ledred") == 0) {

		if(!strcmp(led_device->led_value, led_on)) {
			read = ioread32(GPDAT1_V);
			write = read | led_device->led_mask;
			iowrite32(write, GPDAT1_V);
		}
		else if (!strcmp(led_device->led_value, led_off)) {
			read = ioread32(GPDAT1_V);
			write = read & ~(led_device->led_mask);
			iowrite32(write, GPDAT1_V);
		}
		else {
			pr_info("Bad value\n");
			return -EINVAL;
		}
	}
	else if (strcmp(led_device->led_name,"ledgreen") == 0) {

		if(!strcmp(led_device->led_value, led_on)) {
			read = ioread32(GPDAT6_V);
			write = read | led_device->led_mask;
			iowrite32(write, GPDAT6_V);
		}
		else if (!strcmp(led_device->led_value, led_off)) {
			read = ioread32(GPDAT6_V);
			write = read & ~(led_device->led_mask);
			iowrite32(write, GPDAT6_V);
		}
		else {
			pr_info("Bad value\n");
			return -EINVAL;
		}
	}
	else if (strcmp(led_device->led_name,"ledblue") == 0) {

		if(!strcmp(led_device->led_value, led_on)) {
			read = ioread32(GPDAT6_V);
			write = read | led_device->led_mask;
			iowrite32(write, GPDAT6_V);
		}
		else if (!strcmp(led_device->led_value, led_off)) {
			read = ioread32(GPDAT6_V);
			write = read & ~(led_device->led_mask);
			iowrite32(write, GPDAT6_V);
		}
		else {
			pr_info("Bad value\n");
			return -EINVAL;
		}
	}
	else {
		pr_info("Device not found\n");
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
static ssize_t led_read(struct file *file, char __user *buff, size_t count, loff_t *ppos)
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
	int ret_val;
	u32 GPDIR1_read, GPDIR1_write;
	u32 GPDIR6_read, GPDIR6_write;
	u32 GPDAT1_read, GPDAT1_write;
	u32 GPDAT6_read, GPDAT6_write;

	/* create a private structure */
	struct led_dev *led_device;
	
	/* initialize all the leds to off */
	char led_val[8] = "off\n";

	pr_info("leds_probe enter\n");

	/* Get virtual addresses */
	GPDAT1_V = devm_ioremap(&pdev->dev, GPDAT1, sizeof(u32));
	GPDIR1_V = devm_ioremap(&pdev->dev, GPDIR1, sizeof(u32));
	GPDAT6_V = devm_ioremap(&pdev->dev, GPDAT6, sizeof(u32));
	GPDIR6_V = devm_ioremap(&pdev->dev, GPDIR6, sizeof(u32));

	/* Set GPIO direction bits to output */
	GPDIR1_read = ioread32(GPDIR1_V);
	GPDIR1_write = GPDIR1_read | (GPIO1_DIR_MASK);
	iowrite32(GPDIR1_write, GPDIR1_V);
	
	GPDIR6_read = ioread32(GPDIR6_V);
	GPDIR6_write = GPDIR6_read | (GPIO6_DIR_MASK);
	iowrite32(GPDIR6_write, GPDIR6_V);

	/* set all leds to 0 output */
	GPDAT1_read = ioread32(GPDAT1_V);
	GPDAT1_write = GPDAT1_read & ~(GPIO1_DATA_MASK);
	iowrite32(GPDAT1_write, GPDAT1_V);
	GPDAT6_read = ioread32(GPDAT6_V);
	GPDAT6_write = GPDAT6_read & ~(GPIO6_DATA_MASK);
	iowrite32(GPDAT6_write, GPDAT6_V);
	
	/* Allocate private structure */
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
		led_device->led_mask = LED_RED_MASK;
	}
	else if (strcmp(led_device->led_name,"ledgreen") == 0) {
		led_device->led_mask = LED_GREEN_MASK;
	}
	else if (strcmp(led_device->led_name,"ledblue") == 0) {
		led_device->led_mask = LED_BLUE_MASK;
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

	pr_info("led_init enter\n");

	ret_val = platform_driver_register(&led_platform_driver);
	if (ret_val !=0)
	{
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;

	}	

	pr_info("led_init exit\n");
	return 0;
}

static void led_exit(void)
{
	u32 GPDAT1_read, GPDAT1_write;
	u32 GPDAT6_read, GPDAT6_write;

	pr_info("led_exit enter\n");

	/* set all leds to 0 output */
	GPDAT1_read = ioread32(GPDAT1_V);
	GPDAT1_write = GPDAT1_read & ~(GPIO1_DATA_MASK);
	iowrite32(GPDAT1_write, GPDAT1_V);
	GPDAT6_read = ioread32(GPDAT6_V);
	GPDAT6_write = GPDAT6_read & ~(GPIO6_DATA_MASK);
	iowrite32(GPDAT6_write, GPDAT6_V);

	platform_driver_unregister(&led_platform_driver);

	pr_info("led_exit exit\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off \
					three led devices");

