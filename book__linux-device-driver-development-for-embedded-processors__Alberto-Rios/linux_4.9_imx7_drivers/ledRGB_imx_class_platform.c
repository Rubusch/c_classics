
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/leds.h>

#define GPDAT1_offset 	   0x00
#define GPDIR1_offset 	   0x04
#define GPDAT6_offset 	   0x50000
#define GPDIR6_offset 	   0x50004

#define GPIO1_DIR_MASK 1 << 2
#define GPIO1_DATA_MASK 1 << 2

#define GPIO6_DIR_MASK (1 << 20 | 1 << 21)
#define GPIO6_DATA_MASK (1 << 20 | 1 << 21)

#define LED_RED_MASK 1 << 2
#define LED_GREEN_MASK 1 << 20
#define LED_BLUE_MASK 1 << 21


struct led_dev
{
	u32 led_mask; /* different mask if led is R,G or B */
	void __iomem *base;
	struct led_classdev cdev;
};

static void led_control(struct led_classdev *led_cdev, enum led_brightness b)
{
	u32 read, write;
	struct led_dev *led = container_of(led_cdev, struct led_dev, cdev);

	if (b != LED_OFF) { /* LED ON */

		if (led->led_mask == LED_RED_MASK) {
			read = ioread32(led->base + GPDAT1_offset);
			write = read | led->led_mask;
			iowrite32(write, led->base + GPDAT1_offset);
		}
		if ((led->led_mask == LED_GREEN_MASK) || (led->led_mask == LED_BLUE_MASK)) {
			read = ioread32(led->base + GPDAT6_offset);
			write = read | led->led_mask;
			iowrite32(write, led->base + GPDAT6_offset);
		}
	}
		
	else {
			
		if (led->led_mask == LED_RED_MASK) {
			read = ioread32(led->base + GPDAT1_offset);
			write = read & ~(led->led_mask);
			iowrite32(write, led->base + GPDAT1_offset);
		}
		if ((led->led_mask == LED_GREEN_MASK) || (led->led_mask == LED_BLUE_MASK)) {
			read = ioread32(led->base + GPDAT6_offset);
			write = read & ~(led->led_mask);
			iowrite32(write, led->base + GPDAT6_offset);
		}
	}
}

static int __init ledclass_probe(struct platform_device *pdev)
{

	u32 GPDIR1_read, GPDIR1_write;
	u32 GPDIR6_read, GPDIR6_write;
	u32 GPDAT1_read, GPDAT1_write;
	u32 GPDAT6_read, GPDAT6_write;
	void __iomem *g_ioremap_addr;
	struct device_node *child;
	struct resource *r;
	struct device *dev = &pdev->dev;
	int count, ret;

	dev_info(dev, "platform_probe enter\n");

	/* get our first memory resource from device tree */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	dev_info(dev, "r->start = 0x%08lx\n", (long unsigned int)r->start);
	dev_info(dev, "r->end = 0x%08lx\n", (long unsigned int)r->end);

	/* ioremap our memory region */
	g_ioremap_addr = devm_ioremap(dev, r->start, resource_size(r));
	if (!g_ioremap_addr) {
		dev_err(dev, "ioremap failed \n");
		return -ENOMEM;
	}

	count = of_get_child_count(dev->of_node);
	if (!count)
		return -EINVAL;

	dev_info(dev, "there are %d nodes\n", count);


	/* Set GPIO1_IO_2 direction bit to output */
	GPDIR1_read = ioread32(g_ioremap_addr + GPDIR1_offset);
	GPDIR1_write = GPDIR1_read | (GPIO1_DIR_MASK);
	iowrite32(GPDIR1_write, g_ioremap_addr + GPDIR1_offset);
	
	GPDIR6_read = ioread32(g_ioremap_addr + GPDIR6_offset);
	GPDIR6_write = GPDIR6_read | (GPIO6_DIR_MASK);
	iowrite32(GPDIR6_write, g_ioremap_addr + GPDIR6_offset);

	/* set all leds to 0 output */
	GPDAT1_read = ioread32(g_ioremap_addr + GPDAT1_offset);
	GPDAT1_write = GPDAT1_read & ~(GPIO1_DATA_MASK);
	iowrite32(GPDAT1_write, g_ioremap_addr + GPDAT1_offset);
	GPDAT6_read = ioread32(g_ioremap_addr + GPDAT6_offset);
	GPDAT6_write = GPDAT6_read & ~(GPIO6_DATA_MASK);
	iowrite32(GPDAT6_write, g_ioremap_addr + GPDAT6_offset);

	for_each_child_of_node(dev->of_node, child){

		struct led_dev *led_device;
		struct led_classdev *cdev;
		led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if (!led_device)
			return -ENOMEM;

		cdev = &led_device->cdev;

		led_device->base = g_ioremap_addr;

		of_property_read_string(child, "label", &cdev->name);

		if (strcmp(cdev->name,"red") == 0) {
			led_device->led_mask = LED_RED_MASK;
			led_device->cdev.default_trigger = "heartbeat";
		}
		else if (strcmp(cdev->name,"green") == 0) {
			led_device->led_mask = LED_GREEN_MASK;
		}
		else if (strcmp(cdev->name,"blue") == 0) {
			led_device->led_mask = LED_BLUE_MASK;
		}
		else {
			dev_info(dev, "Bad device tree value\n");
			return -EINVAL;
		}

		/* Disable timer trigger until led is on */
		led_device->cdev.brightness = LED_OFF;
		led_device->cdev.brightness_set = led_control;

		ret = devm_led_classdev_register(dev, &led_device->cdev);
		if (ret) {
			dev_err(dev, "failed to register the led %s\n", cdev->name);
			of_node_put(child);
			return ret;
		}
	}
	
	dev_info(dev, "leds_probe exit\n");

	return 0;
}

static int __exit ledclass_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "leds_remove enter\n");

	dev_info(&pdev->dev, "leds_remove exit\n");

	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,RGBclassleds"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = ledclass_probe,
	.remove = ledclass_remove,
	.driver = {
		.name = "RGBclassleds",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int ledRGBclass_init(void)
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

static void ledRGBclass_exit(void)
{
	pr_info("led driver enter\n");

	platform_driver_unregister(&led_platform_driver);

	pr_info("led driver exit\n");
}

module_init(ledRGBclass_init);
module_exit(ledRGBclass_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a driver that turns on/off RGB leds \
		   using the LED subsystem");

