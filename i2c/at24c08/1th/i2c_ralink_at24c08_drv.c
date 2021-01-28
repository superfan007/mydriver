#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

static int major;
static struct class *class;
static struct i2c_client *at24c08_client;

static ssize_t at24c08_read(struct file * file, char __user *buf, size_t count, loff_t *off)
{
	unsigned char addr, data;
	
	copy_from_user(&addr, buf, 1);
	data = i2c_smbus_read_byte_data(at24c08_client, addr);
	copy_to_user(buf, &data, 1);
	return 1;
}

/* buf[0] : addr
 * buf[1] : data
 */
static ssize_t at24c08_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
	unsigned char ker_buf[2];
	unsigned char addr, data;

	copy_from_user(ker_buf, buf, 2);
	addr = ker_buf[0];
	data = ker_buf[1];

	printk("addr = 0x%02x, data = 0x%02x\n", addr, data);

	if (!i2c_smbus_write_byte_data(at24c08_client, addr, data))
		return 2;
	else
		return -EIO;	
}

static struct file_operations at24c08_fops = {
	.owner = THIS_MODULE,
	.read  = at24c08_read,
	.write = at24c08_write,
};

static int  at24c08_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	at24c08_client = client;
		
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "at24c08", &at24c08_fops);
	class = class_create(THIS_MODULE, "at24c08");
	device_create(class, NULL, MKDEV(major, 0), NULL, "at24c08"); /* /dev/at24c08 */
	
	return 0; 
}
static int at24c08_remove(struct i2c_client *client)
{
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "at24cxx");
		
	return 0;
}


static const struct i2c_device_id at24c08_id_table[] = {
	{ "at24c08", 0 },
	{}
};

/* 1. 分配/设置i2c_driver */
static struct i2c_driver at24c08_driver = {
	.driver	= {
		.name	= "at24c08_test",
		.owner	= THIS_MODULE,
	},
	.probe		= at24c08_probe,
	.remove		= at24c08_remove,
	.id_table	= at24c08_id_table,
};



static int at24c08_drv_init(void)
{
	i2c_add_driver(&at24c08_driver);
	return 0;
}

static void at24c08_drv_exit(void)
{
	i2c_del_driver(&at24c08_driver);
}

module_init(at24c08_drv_init);
module_exit(at24c08_drv_exit);
MODULE_LICENSE("GPL");