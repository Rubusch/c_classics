
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>

/* Include header files for ioremap() and copy_to_user() */
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

/* LED_RED_PB6, LED_GREEN_PB5, LED_BLUE_PB0 */

/* Declare physical GPDAT, GPDIR and ENET_PAUR addresses */

static int PIO_SODR1 = 0xFC038050;
static int PIO_CODR1 = 0xFC038054;
static int PIO_MSKR1 = 0xFC038040;
static int PIO_CFGR1 = 0xFC038044;
static int GMAC_SAB1 = 0xF8008088;

/* Declare __iomem pointers that will keep virtual addresses */
static void __iomem *PIO_SODR1_W;
static void __iomem *PIO_CODR1_W;
static void __iomem *PIO_MSKR1_V;
static void __iomem *PIO_CFGR1_V;
static void __iomem *GMAC_SAB1_V;


#define ENET_ADDRESS_LOWER_LEN 8
#define PIO_PB0_MASK (1 << 0)
#define PIO_PB5_MASK (1 << 5)
#define PIO_PB6_MASK (1 << 6)
#define PIO_CFGR1_MASK (1 << 8) // direction masked bits (1 output), no PUEN, no PDEN
#define PIO_MASK_ALL_LEDS (PIO_PB0_MASK | PIO_PB5_MASK | PIO_PB6_MASK)

static ssize_t my_dev_write(struct file *file, const char __user *buff,
							size_t count, loff_t *ppos)
{
	pr_info("my_dev_write() is called.\n");
	return 0;
}

static ssize_t my_dev_read(struct file *file, char __user *buff, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t size;
	pr_info("my_dev_read() is called.\n");
	
	/* we convert the 4 bytes int address to a 8 bytes char value
	 * this is a char array now, and not a real int hex value. Do not
	 * get confused when pr_info %C and see characters that math with
	 * the hex values.
	 * there are other functions to convert the char value to its real
	 * int hex value.
	 */
	if((*ppos == 0) && count > ENET_ADDRESS_LOWER_LEN){
		size = sprintf(buf, "0x%08x", ioread32(GMAC_SAB1_V));
		buf[size] = '\n'; // we are replacing the NULL character
		if(copy_to_user(buff, buf, size+1)){
			return -EFAULT;
		}
		*ppos = ENET_ADDRESS_LOWER_LEN;
		return size+1;
	}
	return 0;
}


static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.read = my_dev_read,
	.write = my_dev_write,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static struct miscdevice helloworld_miscdevice = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "mydev",
		.fops = &my_dev_fops,
};

static int __init my_probe(struct platform_device *pdev)
{
	int retval;
	int i=0;
	unsigned int temp;

	pr_info("platform_probe enter\n");

	/* Get virtual addresses*/
	PIO_MSKR1_V = ioremap(PIO_MSKR1, sizeof(u32));
	PIO_SODR1_W = ioremap(PIO_SODR1, sizeof(u32));
	PIO_CODR1_W = ioremap(PIO_CODR1, sizeof(u32));
	PIO_CFGR1_V = ioremap(PIO_CFGR1, sizeof(u32));
	GMAC_SAB1_V = ioremap(GMAC_SAB1, sizeof(u32));
	

	iowrite32(PIO_MASK_ALL_LEDS, PIO_MSKR1_V); // Enable all leds
	iowrite32(PIO_CFGR1_MASK, PIO_CFGR1_V); // set enabled leds to output

	iowrite32(PIO_MASK_ALL_LEDS, PIO_SODR1_W); // Clear all the leds

	 /* Toggle one of the leds */
	for (i=0; i<5; i++)
	{
		pr_info("LED ON\n");
		iowrite32(PIO_PB0_MASK, PIO_CODR1_W);
		msleep(250);
		pr_info("LED OFF\n");
		iowrite32(PIO_PB0_MASK, PIO_SODR1_W);
		msleep(250);
	}
	
	retval = misc_register(&helloworld_miscdevice);
	if (retval) return retval; /* misc_register returns 0 if success */
	pr_info("mydev: got minor %i\n",helloworld_miscdevice.minor);
	return 0;
}
static int __exit my_remove(struct platform_device *pdev)
{
	misc_deregister(&helloworld_miscdevice);
	iounmap(PIO_MSKR1_V);
	iounmap(PIO_SODR1_W);
	iounmap(PIO_CODR1_W);
	iounmap(PIO_CFGR1_V);
	iounmap(GMAC_SAB1_V);

	pr_info("platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,hellokeys"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "hellokeys",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int demo_init(void)
{
	int ret_val;
	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&my_platform_driver);
	if (ret_val !=0)
	{
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;

	}
	pr_info("demo_init exit\n");
	return 0;
}

static void demo_exit(void)
{
	pr_info("demo_exit enter\n");

	platform_driver_unregister(&my_platform_driver);

	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off a led when probing and read MAC address");

