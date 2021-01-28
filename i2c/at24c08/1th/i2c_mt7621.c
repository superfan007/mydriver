#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_i2c.h>
#include <linux/of_gpio.h>


#include <asm/irq.h>
#define CLK_DIV_MASK		0xfff

#define REG_SM0CFG0		0x08
#define REG_SM0DOUT		0x10
#define REG_SM0DIN		0x14
#define REG_SM0ST		0x18
#define REG_SM0AUTO		0x1C
#define REG_SM0CFG1		0x20
#define REG_SM0CFG2		0x28
#define REG_SM0CTL0		0x40
#define REG_SM0CTL1		0x44
#define REG_SM0D0		0x50
#define REG_SM0D1		0x54
#define REG_PINTEN		0x5C
#define REG_PINTST		0x60
#define REG_PINTCL		0x64

/* REG_SM0CFG0 */
#define I2C_DEVADDR_MASK	0x7f

/* REG_SM0ST */
#define I2C_DATARDY		BIT(2)
#define I2C_SDOEMPTY		BIT(1)
#define I2C_BUSY		BIT(0)

/* REG_SM0AUTO */
#define READ_CMD		BIT(0)

/* REG_SM0CFG1 */
#define BYTECNT_MAX		64
#define SET_BYTECNT(x)		(x - 1)

/* REG_SM0CFG2 */
#define AUTOMODE_EN		BIT(0)

/* REG_SM0CTL0 */
#define ODRAIN_HIGH_SM0		BIT(31)
#define VSYNC_SHIFT		28
#define VSYNC_MASK		0x3
#define VSYNC_PULSE		(0x1 << VSYNC_SHIFT)
#define VSYNC_RISING		(0x2 << VSYNC_SHIFT)
#define CLK_DIV_SHIFT		16
#define CLK_DIV_MASK		0xfff
#define DEG_CNT_SHIFT		8
#define DEG_CNT_MASK		0xff
#define WAIT_HIGH		BIT(6)    // 1<<6
#define DEG_EN			BIT(5)
#define CS_STATUA		BIT(4)
#define SCL_STATUS		BIT(3)
#define SDA_STATUS		BIT(2)
#define SM0_EN			BIT(1)
#define SCL_STRECH		BIT(0)

/* REG_SM0CTL1 */
#define ACK_SHIFT		16
#define ACK_MASK		0xff
#define PGLEN_SHIFT		8
#define PGLEN_MASK		0x7
#define SM0_MODE_SHIFT		4
#define SM0_MODE_MASK		0x7
#define SM0_MODE_START		0x1
#define SM0_MODE_WRITE		0x2
#define SM0_MODE_STOP		0x3
#define SM0_MODE_READ_NACK	0x4
#define SM0_MODE_READ_ACK	0x5
#define SM0_TRI_BUSY		BIT(0)

//#define PRINTK printk
#define PRINTK(...) 

enum s3c24xx_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};
typedef unsigned char U8;
struct mt7621_i2c_regs {
	/*
	unsigned int iiccon;
	unsigned int iicstat;
	unsigned int iicadd;
	unsigned int iicds;
	unsigned int iiclc;
	*/
	 U8 sm0cfg0[8];
	 U8 sm0dout0[8];
	 U8 sm0din[8];
	 U8 sm0st[8];
	 U8 sm0auto[8];
	 U8 sm0cfg1[8];
	 U8 sm0cfg2[8];
	 U8 sm0ctl0[8];
	 U8 sm0ctl1[8];
	 U8 sm0d0[8];
	 U8 sm0d1[8];
	 U8 pinten[8];	//irq enable
	 U8 pintet[8];	//irq status
	 U8 pintcl[8];	//irq clear
};

struct mt7621_i2c_xfer_data {
	struct i2c_msg *msgs;
	int msn_num;
	int cur_msg;
	int cur_ptr;
	int state;
	int err;
	wait_queue_head_t wait;
};

static struct mt7621_i2c_xfer_data mt7621_i2c_xfer_data;
void *base=0x1E000900;

static struct mt7621_i2c_regs *mt7621_i2c_regs;

static void mtk_i2c_w32(void *base, u32 val, unsigned reg)
{
	iowrite32(val, base + reg);
}


static int mt7621_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	printk("enter mt7621_i2c_xfer");
	return 0;
}

static u32 mt7621_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}


static const struct i2c_algorithm mt7621_i2c_algo = {
//	.smbus_xfer     = ,
	.master_xfer	= mt7621_i2c_xfer,
	.functionality	= mt7621_i2c_func,
};

/* 1. ����/����i2c_adapter
 */
static struct i2c_adapter mt7621_i2c_adapter = {
 .name			 = "mt7621_gf",
 .algo			 = &mt7621_i2c_algo,
 .owner 		 = THIS_MODULE,
};

static int isLastMsg(void)
{
	return (mt7621_i2c_xfer_data.cur_msg == mt7621_i2c_xfer_data.msn_num - 1);
}

static int isEndData(void)
{
	return (mt7621_i2c_xfer_data.cur_ptr >= mt7621_i2c_xfer_data.msgs->len);
}

static int isLastData(void)
{
	return (mt7621_i2c_xfer_data.cur_ptr == mt7621_i2c_xfer_data.msgs->len - 1);
}

/*
 * I2C��ʼ��
 */
static void mt7621_i2c_init(void)
{
	struct clk *clk;
	u32 clk_div,cur_clk;
	u32 reg;
	
	clk = clk_get(NULL, "i2c");
	clk_enable(clk);
	
	cur_clk = 1000000;
	clk_div = clk_get_rate(clk) / cur_clk;
	printk("cur_clk:%d,clk_div:%d\n", cur_clk, clk_div);
	
    // ѡ�����Ź��ܣ�GPE15:IICSDA, GPE14:IICSCL
    //s3c_gpio_cfgpin(S3C2410_GPE(14), S3C2410_GPE14_IICSCL);
	//s3c_gpio_cfgpin(S3C2410_GPE(15), S3C2410_GPE15_IICSDA);

    /* bit[7] = 1, ʹ��ACK
     * bit[6] = 0, IICCLK = PCLK/16
     * bit[5] = 1, ʹ���ж�
     * bit[3:0] = 0xf, Tx clock = IICCLK/16
     * PCLK = 50MHz, IICCLK = 3.125MHz, Tx Clock = 0.195MHz
     */

	/*
    mt7621_i2c_regs->iiccon = (1<<7) | (0<<6) | (1<<5) | (0xf);  // 0xaf    10101111

    mt7621_i2c_regs->iicadd  = 0x10;     // S3C24xx slave address = [7:1]	00010000
    mt7621_i2c_regs->iicstat = 0x10;     // I2C�������ʹ��(Rx/Tx)			00010000
    */	
    
    printk("init mt7621 i2c....\n");
    /* ctrl0 */
	reg = ODRAIN_HIGH_SM0 | VSYNC_PULSE | (clk_div << CLK_DIV_SHIFT) |
		WAIT_HIGH | SM0_EN;  
	mtk_i2c_w32(base, reg, REG_SM0CTL0); 
	
	/* auto mode */
	mtk_i2c_w32(base, AUTOMODE_EN, REG_SM0CFG2);
	printk("exit mt7621 i2c init\n");
}

static int i2c_bus_mt7621_init(void)
{
	/* 2. Ӳ����ص����� */
	//mt7621_i2c_regs = ioremap(0x1E000908, sizeof(struct mt7621_i2c_regs));
	
	mt7621_i2c_init();

	init_waitqueue_head(&mt7621_i2c_xfer_data.wait);
	
	/* 3. ע��i2c_adapter */
	i2c_add_adapter(&mt7621_i2c_adapter);
	
	return 0;
}

static void i2c_bus_mt7621_exit(void)
{
	i2c_del_adapter(&mt7621_i2c_adapter);	
	//iounmap(mt7621_i2c_regs);
}

module_init(i2c_bus_mt7621_init);
module_exit(i2c_bus_mt7621_exit);
MODULE_LICENSE("GPL");