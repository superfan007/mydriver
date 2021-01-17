#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>

static struct resource beep_resource[] =
{
	[0] ={
		.start = 0x1E000900,
		.end =  0x1E000900 + 0x64,
		.flags = IORESOURCE_MEM,
	},
};

static void hello_release(struct device *dev)
{
	printk("hello_release\n");
	return ;
}



static struct platform_device hello_device=
{
    .name = "i2c-mt7621",
    .id = -1,
    .dev.release = hello_release,
    .num_resources = ARRAY_SIZE(beep_resource),
    .resource = beep_resource,
};

static int hello_init(void)
{
	printk("hello_init");
	return platform_device_register(&hello_device);
}

static void hello_exit(void)
{
	printk("hello_exit");
	platform_device_unregister(&hello_device);
	return;
}

MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);

/*
 #include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/platform_device.h> 
#include <linux/fs.h>  
#include <asm/uaccess.h>    
#include <linux/pci.h>    
#include <mach/map.h>    
#include <linux/sched.h>  
#include <linux/gpio.h>     

 

  
struct platform_device *my_led_dev;  
  
static int __init platform_dev_init(void)  
{  
    int ret;  
      
 //分配一个 platform_device结构体
    my_led_dev = platform_device_alloc("platform_led", -1);    
      
    ret = platform_device_add(my_led_dev);//将自定义的设备添加到内核设备架构中
      
    if(ret)  
        platform_device_put(my_led_dev);//销毁platform设备结构  
      
    return ret;  
}  
  
static void __exit platform_dev_exit(void)  
{  
    platform_device_unregister(my_led_dev);//注销platform_device
}  
  
module_init(platform_dev_init);  
module_exit(platform_dev_exit);  
  
MODULE_AUTHOR("Sola");  
MODULE_LICENSE("GPL");  
*/