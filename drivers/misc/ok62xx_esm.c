#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>

#define		OF_OFFSET(base,pin)		((base) + ((pin) * 0x20))

#define		ESM_SFT_RST			0x0c
#define		ESM_EN				0x08
#define		ESM_ERR_EN_SET			0x18	//Config Error Interrupt Enable Set Register

#define		ESM_INT_PRIO(pin)		OF_OFFSET(0x410, pin)
#define		ESM_STS(pin)			OF_OFFSET(0x404, pin)
#define		ESM_INTR_EN_SET(pin)		OF_OFFSET(0x408, pin)
#define		ESM_PIN_EN_SET(pin)		OF_OFFSET(0x414, pin)


struct esm_data
{
	void __iomem	*esm_addr;
	uint32_t	esm_pin;
};

static struct esm_data esm;

static void iomap_write_esm(struct esm_data *data, uint32_t offset, uint32_t val)
{
	writel(val, data->esm_addr + offset);
}

static int iomap_read_esm(struct esm_data *data, uint32_t offset)
{
	return readl(data->esm_addr + offset);
}

static int esm_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev_of_node(dev);
	ret = of_property_read_u32(np, "forlinx,esm-pin", &esm.esm_pin);
	if (ret < 0) {
		dev_err(dev, "invalid format for forlinx,esm-pin, ret = %d\n", ret);
		esm.esm_pin = 5;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "esm");
	esm.esm_addr = devm_ioremap_resource(dev, res);
	if (IS_ERR(esm.esm_addr)) {
		dev_err(dev, "found no memory resource for esm\n");
		return -EINVAL;
	}

	iomap_write_esm(&esm, ESM_SFT_RST, 0x0f);
	iomap_write_esm(&esm, ESM_ERR_EN_SET, 0x1 << esm.esm_pin);

	iomap_write_esm(&esm, ESM_INT_PRIO(esm.esm_pin), 0x1);
	iomap_write_esm(&esm, ESM_STS(esm.esm_pin), 0x1);
	iomap_write_esm(&esm, ESM_INTR_EN_SET(esm.esm_pin), 0x1);
	iomap_write_esm(&esm, ESM_PIN_EN_SET(esm.esm_pin), 0x1);

	iomap_write_esm(&esm, ESM_EN, 0x0f);

	return 0;
}

static const struct of_device_id esm_match_table[] = {
    { .compatible = "forlinx,ok62xx-esm", },
    {}
};

MODULE_DEVICE_TABLE(of, esm_match_table);

static struct platform_driver esm_platform_driver = {
	.probe = esm_probe,
	.driver = {
		.name = "esm",
		.of_match_table = esm_match_table,
	},
};

module_platform_driver(esm_platform_driver);

MODULE_AUTHOR("zyj@forlinx.com");
MODULE_DESCRIPTION("ok62xx esm driver");
MODULE_LICENSE("GPL");
