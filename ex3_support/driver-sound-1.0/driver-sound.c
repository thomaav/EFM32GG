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

#define MIN_AMPLITUDE 0x0
#define MAX_AMPLITUDE 0xF
#define NOTE_FREQUENCY 185
#define TOGGLES_PER_SEC (NOTE_FREQUENCY * 2)

// we have to request quite a lot of memory regions here, as we are
// not using the platform device method specified in the compendium
#define DMA_UPPER_BASE   (0x400C2000)
#define DMA_STATUS       (0x00)
#define DMA_CONFIG       (0x01)
#define DMA_CTRLBASE     (0x02)
#define DMA_ALTCTRLBASE  (0x03)
#define DMA_CHWAITSTATUS (0x04)
#define DMA_CHSWREQ	 (0x05)
#define DMA_CHUSEBURSTS	 (0x06)
#define DMA_CHUSEBURSTC	 (0x07)
#define DMA_CHREQMASKS	 (0x08)
#define DMA_CHREQMASKC	 (0x09)
#define DMA_CHENS	 (0x0A)
#define DMA_CHENC	 (0x0B)
#define DMA_CHALTS	 (0x0C)
#define DMA_CHALTC       (0x0D)
#define DMA_ERRORC       (0x13)
#define DMA_UPPER_REGISTERS 20

#define DMA_LOWER_BASE   (0x400C3000)
#define DMA_IF           (0x00)
#define DMA_IFS		 (0x01)
#define DMA_IFC		 (0x02)
#define DMA_IEN		 (0x03)
#define DMA_CTRL         (0x04)
#define DMA_RDS          (0x05)
#define DMA_LOOP0        (0x08)
#define DMA_CH0_CTRL     (0x40)
#define DMA_LOWER_REGISTERS 65
#define DMA_IRQ_LINE 20

// 0x100 / 0x40 = 4, since we are working with u32-registers
#define DMA_PRIMARY_CH0  (0x000)
#define DMA_ALT_CH0      (0x40)

#define PRS_BASE         (0x400CC000)
#define PRS_CH0_CTRL     (0x4)
#define PRS_CH5_CTRL     (0x9)
#define PRS_REGISTERS 10

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
#define DAC_IRQ_LINE 21

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

static u32 *min_amplitude;
static u32 *max_amplitude;

// DMA
// we need 512*3 allocations from the pool:
// 1: control block
// 2: source for 0 samples
// 3: source for 1 samples
static char *kbuf_control;
static char *kbuf_0;
static char *kbuf_1;
static dma_addr_t dma_addr_handle_control;
static dma_addr_t dma_addr_handle_0;
static dma_addr_t dma_addr_handle_1;
struct dma_pool *dma_pool;
static size_t pool_alloc_size = 512;
static size_t pool_align = 512;

static u8 first_dma_interrupt = 1;

/*
  Print the absolute address value of a register, as well as its
  contents.
 */
static void read_register_contents(u32 * reg)
{
	u32 register_state;
	register_state = ioread32(reg);
	pr_info("[sound]: %p=%x. \n", reg, register_state);
}

/*
  Print some output after allocation from a DMA pool to know the
  region and alignment of the allocation, as well as the memory was
  writable.
 */
/* static void output(char *kbuf, dma_addr_t handle, size_t size, char *string) */
/* { */
/* 	unsigned long diff; */
/* 	diff = (unsigned long) kbuf - handle; */
/* 	pr_info("kbuf=%12p, handle=%12p, size = %d\n", kbuf, */
/* 		(unsigned long *) handle, (int) size); */
/* 	pr_info("(kbuf-handle)= %12p, %12lu, PAGE_OFFSET=%12lu, compare=%lu\n", */
/* 		(void *) diff, diff, PAGE_OFFSET, diff - PAGE_OFFSET); */
/* 	strcpy(kbuf, string); */
/* 	pr_info("string written was, %s\n", kbuf); */
/* } */

static ssize_t sound_read(struct file *filp, char __user * buf, size_t count,
			  loff_t * f_pos)
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

/*
  Print out status and IF value of the DMA to check if something is
  wrong - which in this case it always is with AHB-bus errors.
 */
static irqreturn_t dma_irq_handler(int irq, void *dev_id)
{
	if (first_dma_interrupt) {
		first_dma_interrupt = 0;
		pr_info("[sound]: DMA interrupt.\n");
		read_register_contents(sound_dev.dma_upper_mem + DMA_STATUS);
		read_register_contents(sound_dev.dma_lower_mem + DMA_IF);
		iowrite32(0x1, sound_dev.dma_upper_mem + DMA_ERRORC);
		iowrite32(0x00000001, sound_dev.dma_lower_mem + DMA_IFC);
		iowrite32(0x1, sound_dev.dma_upper_mem + DMA_CHENS);
		iowrite32(0x1, sound_dev.dma_upper_mem + DMA_CHALTS);
	}
	return IRQ_HANDLED;
}

/* static irqreturn_t dac_irq_handler(int irq, void *dev_id) */
/* { */
/* 	pr_info("[sound]: DAC interrupt.\n"); */
/* 	/\* iowrite32(0x1, sound_dev.dma_upper_mem + DMA_ERRORC); *\/ */
/* 	/\* iowrite32(0x00000001, sound_dev.dma_lower_mem + DMA_IFC); *\/ */
/* 	return IRQ_HANDLED; */
/* } */

/*
  Callback used in earlier implementations of the sound driver to be
  called at regular intervals with hrtimer.
 */
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
	.devno = 0,.cl = 0,.timer3_mem = 0,.dac_mem = 0,};

	// initialize a (_one_) chardev region with dynamic major part
	// dynamic and minor set to 0
	err =
	    alloc_chrdev_region(&sound_dev.devno, 0, DEV_NR_COUNT, DRIVER_NAME);
	if (err) {
		printk(KERN_WARNING
		       "[sound]: Could not allocate chrdev region for sound. (%i).\n",
		       err);
		goto fail_alloc_chrdev;
	}
	// create the actual kernel character device
	cdev_init(&sound_dev.cdev, &sound_fops);
	sound_dev.cdev.owner = THIS_MODULE;
	sound_dev.cdev.ops = &sound_fops;

	//
	// memory management like this should absolutely be factored
	// out and prettified - we just didn't have time.
	//

	// setup DMA before we tell the kernel about our new dev
	dma_upper_mem =
	    request_mem_region(DMA_UPPER_BASE,
			       DMA_UPPER_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!dma_upper_mem) {
		printk(KERN_WARNING
		       "[sound]: Could not allocate memory for DMA registers.\n");
		err = -1;
		goto fail_request_dma_upper_mem_region;
	}
	dma_lower_mem =
	    request_mem_region(DMA_LOWER_BASE,
			       DMA_LOWER_REGISTERS * sizeof(u32), DRIVER_NAME);
	if (!dma_lower_mem) {
		printk(KERN_WARNING
		       "[sound]: Could not allocate memory for DMA registers.\n");
		err = -1;
		goto fail_request_dma_lower_mem_region;
	}
	// setup PRS before we tell the kernel about our new dev
	prs_mem =
	    request_mem_region(PRS_BASE, PRS_REGISTERS * sizeof(u32),
			       DRIVER_NAME);
	if (!prs_mem) {
		printk(KERN_WARNING
		       "[sound]: Could not allocate memory for PRS registers.\n");
		err = -1;
		goto fail_request_prs_mem_region;
	}
	// setup Timer3 before we tell the kernel about our new dev
	timer3_mem =
	    request_mem_region(TIMER3_BASE, TIMER3_REGISTERS * sizeof(u32),
			       DRIVER_NAME);
	if (!timer3_mem) {
		printk(KERN_WARNING
		       "[sound]: Could not allocate memory for Timer3 registers.\n");
		err = -1;
		goto fail_request_timer3_mem_region;
	}
	// setup DAC before we tell the kernel about our new dev
	dac_mem =
	    request_mem_region(DAC0_BASE, DAC_REGISTERS * sizeof(u32),
			       DRIVER_NAME);
	if (!dac_mem) {
		printk(KERN_WARNING
		       "[gamepad]: Could not allocate memory for DAC registers.\n");
		err = -1;
		goto fail_request_dac_mem_region;
	}

	sound_dev.dma_upper_mem =
	    ioremap_nocache(DMA_UPPER_BASE, DMA_UPPER_REGISTERS * sizeof(u32));
	sound_dev.dma_lower_mem =
	    ioremap_nocache(DMA_LOWER_BASE, DMA_LOWER_REGISTERS * sizeof(u32));
	sound_dev.prs_mem =
	    ioremap_nocache(PRS_BASE, PRS_REGISTERS * sizeof(u32));
	sound_dev.timer3_mem =
	    ioremap_nocache(TIMER3_BASE, DAC_REGISTERS * sizeof(u32));
	sound_dev.dac_mem =
	    ioremap_nocache(DAC0_BASE, DAC_REGISTERS * sizeof(u32));

	// old, from using hrtimer that wouldn't work well without
	// actual high resolution timers
	/* ktime = ktime_set(0, TIMER_INTERVAL_NS); */
	/* hrtimer_init(&hr_timer, CLOCK_REALTIME, HRTIMER_MODE_REL); */
	/* hr_timer.function = &sound_hrtimer_callback; */
	/* hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL); */

	// now that everything is set up, tell the kernel about the
	// new character device
	err = cdev_add(&sound_dev.cdev, sound_dev.devno, DEV_NR_COUNT);
	if (err) {
		printk(KERN_WARNING "[sound]: Error %d while adding sound.\n",
		       err);
		goto fail_cdev_add;
	}
	// make the driver appear in /dev (i.e. user space) by
	// creating and registering its class
	sound_dev.cl = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(sound_dev.cl)) {
		printk(KERN_WARNING
		       "[sound]: Failed to register device class for sound driver.\n");
		err = PTR_ERR(sound_dev.cl);
		goto fail_class_create;
	}

	chrdev =
	    device_create(sound_dev.cl, NULL, sound_dev.devno, NULL,
			  DRIVER_NAME);
	if (IS_ERR(chrdev)) {
		printk(KERN_WARNING
		       "[sound]: Failed to create device from class.\n");
		err = PTR_ERR(chrdev);
		goto fail_device_create;
	}
	// setup PRS system so Timer3 triggers PRS_CH5 by writing
	// 0b011111 to SOURCESEL (Timer3), 0b001 to SIGSEL (overflow)
	iowrite32(0x001F0001, sound_dev.prs_mem + PRS_CH0_CTRL);

	// initialize Timer3 with sample rate of 44100 for PRS
	iowrite32(0x13D, sound_dev.timer3_mem + TIMER3_TOP);
	iowrite32(0x1, sound_dev.timer3_mem + TIMER3_IEN);
	iowrite32(0x1, sound_dev.timer3_mem + TIMER3_CMD);

	// initialize DAC (prescale, both channels), also add PRS
	// control of DAC output enable (done by setting a single
	// bit), and then setting PRSSEL to 0x5 in CHnCTRL
	iowrite32(0x50050, sound_dev.dac_mem + DAC0_CTRL);
	iowrite32(0x05, sound_dev.dac_mem + DAC0_CH0CTRL);
	iowrite32(0x05, sound_dev.dac_mem + DAC0_CH1CTRL);

	// seutp DMA control block now that we have a device (512-byte
	// alignmed needed). use dma_pool for coherent mapping (we
	// have no idea whether this is needed on an embedded system
	// like the dk3750- we get 160a0000 just about every time
	// (disregarding the page offset), which is alligned
	// accordingly)
	dma_pool =
	    dma_pool_create("dma_sound_pool", NULL, pool_alloc_size, pool_align,
			    0);

	// control block
	kbuf_control =
	    dma_pool_alloc(dma_pool, GFP_KERNEL, &dma_addr_handle_control);
	/* output(kbuf_control, dma_addr_handle_control, 0, "This is the dma_pool_alloc() string"); */

	// 0s
	kbuf_0 = dma_pool_alloc(dma_pool, GFP_KERNEL, &dma_addr_handle_0);
	/* output(kbuf_0, dma_addr_handle_0, 0, "This is the dma_pool_alloc() string"); */
	*((u32 *) kbuf_0) = MIN_AMPLITUDE;

	// 1s
	kbuf_1 = dma_pool_alloc(dma_pool, GFP_KERNEL, &dma_addr_handle_1);
	/* output(kbuf_1, dma_addr_handle_1, 0, "This is the dma_pool_alloc() string"); */
	*((u32 *) kbuf_1) = MAX_AMPLITUDE;

	// control block handle so the DMA knows where its control block is
	iowrite32(((u32 *) dma_addr_handle_control),
		  sound_dev.dma_upper_mem + DMA_CTRLBASE);

	// config for control block:
	// write to DMA_CH0, both primary and alternate, to point to
	// the end of the memory allocated by the pool for now (0x200
	// = 512), and to the DAC. also write R_power to arbitrate
	// after _every_ byte, so that every transfer is one sample
	// (this is simply 0b0000, as well as 0b0 for byte transfers,
	// so we don't actually have to write anything but ping pong
	// mode in practice), and set n_minus_1 to period length

	// src end pointer
	iowrite32((u32 *) dma_addr_handle_0,
		  ((u32 *) dma_addr_handle_control) + DMA_PRIMARY_CH0);
	iowrite32((u32 *) dma_addr_handle_1,
		  ((u32 *) dma_addr_handle_control) + DMA_ALT_CH0);

	// dst end pointer
	iowrite32(((u32 *) DAC0_BASE) + DAC0_CH0DATA,
		  ((u32 *) dma_addr_handle_control) + DMA_PRIMARY_CH0 + 0x1);
	iowrite32(((u32 *) DAC0_BASE) + DAC0_CH0DATA,
		  ((u32 *) dma_addr_handle_control) + DMA_ALT_CH0 + 0x1);

	// [2:0] == 0b011 for ping pong mode and 0x32 for 50 transfers
	// per cycle, C upper bits to set increment off and byte
	// width, so that the address remains set
	iowrite32(0xCC000323,
		  ((u32 *) dma_addr_handle_control) + DMA_PRIMARY_CH0 + 0x2);
	iowrite32(0xCC000323,
		  ((u32 *) dma_addr_handle_control) + DMA_ALT_CH0 + 0x2);

	// sourcesel (0b001010 - DAC0) and sigsel (0b0 - CH0) for DMA
	// to send sample to DAC_CH0 whenever the DAC is ready
	iowrite32(0x000A0000, sound_dev.dma_lower_mem + DMA_CH0_CTRL);

	// enable DMA interrupt generation
	iowrite32(0x1, sound_dev.dma_lower_mem + DMA_IEN);

	// we are using DMA_CH0, so enable it
	iowrite32(0x1, sound_dev.dma_upper_mem + DMA_CHENS);
	iowrite32(0x1, sound_dev.dma_upper_mem + DMA_CHALTS);

	// enable looping on DMA_CH0
	iowrite32(0x103FF, sound_dev.dma_lower_mem + DMA_LOOP0);

	// enable the DMA
	iowrite32(0x1, sound_dev.dma_upper_mem + DMA_CONFIG);

	if (request_irq
	    (DMA_IRQ_LINE, &dma_irq_handler, 0, DRIVER_NAME, &sound_dev))
		printk(KERN_INFO
		       "[sound]: Could not assign interrupt handler for DMA.\n ");

	iowrite32(0xF, sound_dev.dac_mem + DAC0_CH0DATA);

	printk(KERN_INFO "[sound]: Module loaded successfully.\n");
	return 0;

	// if the driver had worked, we would do some actual clean up
	// here and in template_cleanup
 fail_device_create:;
 fail_class_create:;
 fail_cdev_add:;
 fail_request_dac_mem_region:;
 fail_request_timer3_mem_region:;
 fail_request_prs_mem_region:;
 fail_request_dma_lower_mem_region:;
 fail_request_dma_upper_mem_region:;
 fail_alloc_chrdev:return err;
}

static void __exit template_cleanup(void)
{
	dma_pool_free(dma_pool, kbuf_control, dma_addr_handle_control);
	dma_pool_free(dma_pool, kbuf_0, dma_addr_handle_0);
	dma_pool_free(dma_pool, kbuf_1, dma_addr_handle_1);
	kfree(min_amplitude);
	kfree(max_amplitude);
	dma_pool_destroy(dma_pool);
	hrtimer_cancel(&hr_timer);
	printk("I'm out...\n");
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Sound driver used for exercise 3 in TDT4258.");
MODULE_LICENSE("GPL");
