#include <linux/phy.h>
#include <linux/init.h>

static int phy_write_ext(struct phy_device *dev, int reg, int val)
{
	phy_write(dev, 0x1e, reg);
	phy_write(dev, 0x1f, val);
	return 0;
}
static int phy_fixup(struct phy_device *dev)
{
	phy_write_ext(dev, 0xa00d, 0x70);	
	phy_write_ext(dev, 0xa00e, 0x2600);	
	return 0;
}

static int __init fixup_init(void)
{
	if(IS_BUILTIN(CONFIG_PHYLIB)){
		phy_register_fixup_for_uid(0x11a, 0xfff, phy_fixup);
	}
	return 0;
}

module_init(fixup_init);
MODULE_LICENSE("GPL");
