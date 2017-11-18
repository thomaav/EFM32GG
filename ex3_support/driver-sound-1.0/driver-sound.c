#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#include <asm/io.h>
#include <asm/signal.h>
#include <asm/siginfo.h>

#define MAX_AMPLITUDE 0xF
#define NOTE_FREQUENCY 185
#define TOGGLES_PER_SEC (NOTE_FREQUENCY * 2)

#define DMA_UPPER_BASE         (0x400C2000)
#define DMA_STATUS       (0x0)
#define DMA_CONFIG       (0x1)
#define DMA_CTRLBASE     (0x2)
#define DMA_UPPER_REGISTERS 3

#define DMA_LOWER_BASE   (0x400C3000)
#define DMA_IF           (0x00)
#define DMA_IFS		 (0x01)
#define DMA_IFC		 (0x02)
#define DMA_IEN		 (0x03)
#define DMA_CTRL         (0x04)
#define DMA_CH0_CTRL     (0x64)
#define DMA_LOWER_REGISTERS 65
#define DMA_IRQ_LINE 20

#define PRS_BASE         (0x400CC000)
#define PRS_CH5_CTRL     (0x7)
#define PRS_REGISTERS 8

#define TIMER3_BASE      (0x40010C00)
#define TIMER3_CTRL      (0x00)
#define TIMER3_CMD	 (0x01)
#define TIMER3_STATUS	 (0x02)
#define TIMER3_IEN	 (0x03)
#define TIMER3_IF	 (0x04)
#define TIMER3_IFS	 (0x05)
#define TIMER3_IFC	 (0x06)
#define TIMER3_TOP	 (0x07)
#define TIMER3_TOPB	 (0x08)
#define TIMER3_CNT       (0x09)
#define TIMER3_REGISTERS 10
#define TIMER3_IRQ_LINE 19

#define DAC0_BASE        (0x40004000)
#define DAC0_CTRL        (0x00)
#define DAC0_STATUS   	 (0x01)
#define DAC0_CH0CTRL  	 (0x02)
#define DAC0_CH1CTRL  	 (0x03)
#define DAC0_IEN      	 (0x04)
#define DAC0_IF       	 (0x05)
#define DAC0_IFS      	 (0x06)
#define DAC0_IFC      	 (0x07)
#define DAC0_CH0DATA  	 (0x08)
#define DAC0_CH1DATA     (0x09)
#define DAC_REGISTERS 10

#define DRIVER_NAME "sound"
#define DEV_NR_COUNT 1
#define TIMER_INTERVAL_NS (20000000)
#define MS_TO_NS(x) (x * 1E6L)
#define US_TO_NS(x) (x * 1E3L)

struct sound_chrdev {
	dev_t devno;
	struct cdev cdev;
	struct class *cl;
	u32 *dma_upper_mem;
	u32 *dma_lower_mem;
	u32 *prs_mem;
	u32 *timer3_mem;
	u32 *dac_mem;
};

static struct sound_chrdev sound_dev;
static struct hrtimer hr_timer;
static ktime_t ktime;
static u8 sample_high = 0;
static u16 ticks = 0;

// DMA
static char *kbuf;
static dma_addr_t dma_addr_handle;
struct dma_pool *dma_pool;
static size_t pool_size = 512;
static size_t pool_align = 512;
static size_t pool_alloc = 512;

static void output(char *kbuf, dma_addr_t handle, size_t size, char *string)
{
	unsigned long diff;
	diff = (unsigned long)kbuf - handle;
	pr_info("kbuf=%12p, handle=%12p, size = %d\n", kbuf,
		(unsigned long *)handle, (int)size);
	pr_info("(kbuf-handle)= %12p, %12lu, PAGE_OFFSET=%12lu, compare=%lu\n",
		(void *)diff, diff, PAGE_OFFSET, diff - PAGE_OFFSET);
	strcpy(kbuf, string);
	pr_info("string written was, %s\n", kbuf);
}

static ssize_t sound_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

static int sound_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int sound_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations sound_fops = {
	.owner = THIS_MODULE,
	.read = sound_read,
	.open = sound_open,
	.release = sound_release
};

// PRS uses Timer3
/* static irqreturn_t timer3_irq_handler(int irq, void *dev_id) */
/* { */
/* 	iowrite32(0x1, sound_dev.timer3_mem + TIMER3_IFC); */
/* 	return IRQ_HANDLED; */
/* } */

static irqreturn_t dma_irq_handler(int irq, void *dev_id)
{
	;
	/* iowrite32(0x1, sound_dev.timer3_mem + TIMER3_IFC); */
	return IRQ_HANDLED;
}

enum hrtimer_restart sound_hrtimer_callback(struct hrtimer *timer)
{
	// could also use DAC0_COMBDATA
	if (sample_high) {
		iowrite32(MAX_AMPLITUDE, sound_dev.dac_mem + DAC0_CH0DATA);
		iowrite32(MAX_AMPLITUDE, sound_dev.dac_mem + DAC0_CH1DATA);
	} else {
		iowrite32(0x0, sound_dev.dac_mem + DAC0_CH1DATA);
		iowrite32(0x0, sound_dev.dac_mem + DAC0_CH1DATA);
	}

	if (++ticks >= 50) {
		printk(KERN_WARNING "[sound]: A second has passed.\n");
		sample_high = !sample_high;
		ticks = 0;
	}

	hrtimer_forward_now(timer, ktime);
	return HRTIMER_RESTART;
}

static int __init template_init(void)
{
	int err;
	struct resource *dma_upper_mem;
	struct resource *dma_lower_mem;
	struct resource *prs_mem;
	struct resource *timer3_mem;
	struct resource *dac_mem;
	struct device *chrdev;

	// initialize sound dev for ourselves to mimic scull
	sound_dev = (struct sound_chrdev) {
		.devno = 0,
		.cl = 0,
		.timer3_mem = 0,
		.dac_mem = 0,
	};

	// initialize a (_one_) chardev region with dynamic major part
	// dynamic and minor set to 0
	err = alloc_chrdev_region(&sound_dev.devno, 0, DEV_NR_COUNT, DRIVER_NAME);
	if (err) {
		printk(KERN_WARNING "[sound]: Could not allocate chrdev region for sound. (%i).\n", err);
		goto fail_alloc_chrdev;
	}

	// create the actual kernel character device
	cdev_init(&sound_dev.cdev, &sound_fops);
	sound_dev.cdev.owner = THIS_MODULE;
	sound_dev.cdev.ops = &sound_fops;

	// setup DMA before we tell the kernel about our new dev
	dma_upper_mem = request_mem_region(DMA_UPPER_BASE, DMA_UPPER_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!dma_upper_mem) {
		printk(KERN_WARNING "[sound]: Could not allocate memory for DMA registers.\n");
		err = -1;
		goto fail_request_dma_upper_mem_region;
	}
	dma_lower_mem = request_mem_region(DMA_LOWER_BASE, DMA_LOWER_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!dma_lower_mem) {
		printk(KERN_WARNING "[sound]: Could not allocate memory for DMA registers.\n");
		err = -1;
		goto fail_request_dma_lower_mem_region;
	}

	// setup PRS before we tell the kernel about our new dev
	prs_mem = request_mem_region(PRS_BASE, PRS_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!prs_mem) {
		printk(KERN_WARNING "[sound]: Could not allocate memory for PRS registers.\n");
		err = -1;
		goto fail_request_prs_mem_region;
	}

	// setup Timer3 before we tell the kernel about our new dev
	timer3_mem = request_mem_region(TIMER3_BASE, TIMER3_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!timer3_mem) {
		printk(KERN_WARNING "[sound]: Could not allocate memory for Timer3 registers.\n");
		err = -1;
		goto fail_request_timer3_mem_region;
	}

	// setup DAC before we tell the kernel about our new dev
	dac_mem = request_mem_region(DAC0_BASE, DAC_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!dac_mem) {
		printk(KERN_WARNING "[gamepad]: Could not allocate memory for DAC registers.\n");
		err = -1;
		goto fail_request_dac_mem_region;
	}

	sound_dev.dma_upper_mem = ioremap_nocache(DMA_UPPER_BASE, DMA_UPPER_REGISTERS * sizeof(u32));
	sound_dev.dma_lower_mem = ioremap_nocache(DMA_LOWER_BASE, DMA_LOWER_REGISTERS * sizeof(u32));
	sound_dev.prs_mem = ioremap_nocache(PRS_BASE, PRS_REGISTERS * sizeof(u32));
	sound_dev.timer3_mem = ioremap_nocache(TIMER3_BASE, DAC_REGISTERS * sizeof(u32));
	sound_dev.dac_mem = ioremap_nocache(DAC0_BASE, DAC_REGISTERS * sizeof(u32));

	// setup PRS system so Timer3 triggers PRS_CH5 by writing
	// 0b011111 to SOURCESEL (Timer3), 0b001 to SIGSEL (overflow)
	iowrite32(0x001F0001, sound_dev.prs_mem + PRS_CH5_CTRL);

	// initialize Timer3 with sample rate of 44100 for PRS
	iowrite32(0x13D, sound_dev.timer3_mem + TIMER3_TOP);
	iowrite32(0x1, sound_dev.timer3_mem + TIMER3_IEN);
	iowrite32(0x1, sound_dev.timer3_mem + TIMER3_CMD);

	// initialize DAC (prescale, both channels), also add PRS
	// control of DAC output enable (done by setting a single bit)
	iowrite32(0x50050, sound_dev.dac_mem + DAC0_CTRL);
	iowrite32(0x1, sound_dev.dac_mem + DAC0_CH0CTRL);
	iowrite32(0x1, sound_dev.dac_mem + DAC0_CH1CTRL);

	// we initially used Timer3, but there is no way we're going
	// to do it better than the people who made hrtimer, so let's
	// use high resolution kernel timers instead
	/* ktime = ktime_set(0, TIMER_INTERVAL_NS); */
	/* hrtimer_init(&hr_timer, CLOCK_REALTIME, HRTIMER_MODE_REL); */
	/* hr_timer.function = &sound_hrtimer_callback; */
	/* hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL); */

	// now that everything is set up, tell the kernel about the
	// new character device
	err = cdev_add(&sound_dev.cdev, sound_dev.devno, DEV_NR_COUNT);
	if (err) {
		printk(KERN_WARNING "[sound]: Error %d while adding sound.\n", err);
		goto fail_cdev_add;
	}

	// make the driver appear in /dev (i.e. user space) by
	// creating and registering its class
	sound_dev.cl = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(sound_dev.cl)) {
		printk(KERN_WARNING "[sound]: Failed to register device class for sound driver.\n");
		err = PTR_ERR(sound_dev.cl);
		goto fail_class_create;
	}

	chrdev = device_create(sound_dev.cl, NULL, sound_dev.devno, NULL, DRIVER_NAME);
	if (IS_ERR(chrdev)) {
		printk(KERN_WARNING "[sound]: Failed to create device from class.\n");
		err = PTR_ERR(chrdev);
		goto fail_device_create;
	}

	// seutp DMA control block now that we have a device (512-byte
	// alignmed needed). use dma_pool for coherent mapping (we
	// have no idea whether this is needed on an embedded system
	// like the dk3750- we get 160a0000 just about every time, which
	// is alligned accordingly)
	dma_pool = dma_pool_create("dma_sound_pool", NULL, pool_size, pool_align, pool_alloc);
	kbuf = dma_pool_alloc(dma_pool, GFP_KERNEL, &dma_addr_handle);
	output(kbuf, dma_addr_handle, 0, "This is the dma_pool_alloc() string");

	// also some setup values to DMA:
	// control block handle
	iowrite32(&dma_addr_handle, sound_dev.dma_upper_mem + DMA_CTRLBASE);
	// sourcesel (0b001010 - DAC0) and sigsel (0b0 - CH0) for DMA
	// to send sample to DACCH0 whenever the DAC is ready
	iowrite32(0x000A0000, sound_dev.dma_upper_mem + DMA_CTRLBASE);

	// we are not using the timer interrupt ourselves
	/* if (request_irq(TIMER3_IRQ_LINE , &timer3_irq_handler, 0, DRIVER_NAME, &sound_dev)) */
	/* 	printk(KERN_INFO "[sound]: Could not assign interrupt handler for Timer3.\n "); */
	// we _are_ using the DMA interrupt
	if (request_irq(DMA_IRQ_LINE , &dma_irq_handler, 0, DRIVER_NAME, &sound_dev))
		printk(KERN_INFO "[sound]: Could not assign interrupt handler for DMA.\n ");

	printk(KERN_INFO "[sound]: Module loaded successfully.\n");
	return 0;

 fail_device_create: ;
 fail_class_create: ;
 fail_cdev_add: ;
 fail_request_dac_mem_region: ;
 fail_request_timer3_mem_region: ;
 fail_request_prs_mem_region: ;
 fail_request_dma_lower_mem_region: ;
 fail_request_dma_upper_mem_region: ;
 fail_alloc_chrdev: return err;
}

static void __exit template_cleanup(void)
{
	dma_pool_free(dma_pool, kbuf, dma_addr_handle);
	dma_pool_destroy(dma_pool);
	hrtimer_cancel(&hr_timer);
	printk("I'm out...\n");
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Sound driver used for exercise 3 in TDT4258.");
MODULE_LICENSE("GPL");

