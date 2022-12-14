
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct ltc2422_state {
	struct spi_device *spi;
	u8 buffer[4];
};

static const struct iio_chan_spec ltc2422_channel[] = {

{
	.type		= IIO_VOLTAGE,
	.indexed	= 1,
	.output		= 1,
	.channel	= 0,
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
}

};

static int ltc2422_read_raw(struct iio_dev *indio_dev,
	struct iio_chan_spec const *chan, int *val, int *val2, long m)
{
	int ret;
	struct ltc2422_state *st = iio_priv(indio_dev);

	switch (m) {
	case IIO_CHAN_INFO_RAW:

		ret = spi_read(st->spi, &st->buffer, 3);
		if (ret < 0)
			return ret;

		*val  = st->buffer[0] << 16;
		*val |= st->buffer[1] << 8;
		*val |= st->buffer[2];

		dev_info(&st->spi->dev, "the value is %x\n", *val);

		return IIO_VAL_INT;

	default:
		return -EINVAL;
	}
}

static const struct iio_info ltc2422_info = {
	.read_raw = &ltc2422_read_raw,
	.driver_module = THIS_MODULE,
};

static int ltc2422_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ltc2422_state *st;
	int err;
	dev_info(&spi->dev, "my_probe() function is called.\n");

	const struct spi_device_id *id = spi_get_device_id(spi);

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (indio_dev == NULL)
		return -ENOMEM;

	st = iio_priv(indio_dev);

	st->spi = spi;

	indio_dev->dev.parent = &spi->dev;
	indio_dev->channels = ltc2422_channel;
	indio_dev->info = &ltc2422_info;
	indio_dev->name = id->name;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	err = devm_iio_device_register(&spi->dev, indio_dev);
	if (err < 0)
		return err;

	return 0;
}


static const struct of_device_id ltc2422_dt_ids[] = {
	{ .compatible = "arrow,ltc2422", },
	{ }
};

MODULE_DEVICE_TABLE(of, ltc2422_dt_ids);

static const struct spi_device_id ltc2422_id[] = {
	{ .name = "ltc2422", },
	{ }
};
MODULE_DEVICE_TABLE(spi, ltc2422_id);


static struct spi_driver ltc2422_driver = {
	.driver = {
		.name	= "ltc2422",
		.owner	= THIS_MODULE,
		.of_match_table = ltc2422_dt_ids,
	},
	.probe		= ltc2422_probe,
	.id_table	= ltc2422_id,
};

module_spi_driver(ltc2422_driver);

MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("LTC2422 DUAL ADC");
MODULE_LICENSE("GPL");


