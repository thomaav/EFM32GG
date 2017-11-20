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

#include <asm/io.h>
#include <asm/signal.h>
#include <asm/siginfo.h>

// gamepad input
#define GPIO_PC_BASE     (0x40006048)
#define GPIO_CTRL        (0x00)
#define GPIO_MODEL       (0x01)
#define GPIO_MODEH       (0x02)
#define GPIO_DOUT        (0x03)
#define GPIO_DOUTSET     (0x04)
#define GPIO_DOUTCLR     (0x05)
#define GPIO_DOUTTGL     (0x06)
#define GPIO_DIN         (0x07)
#define GPIO_PINLOCKN    (0x08)
#define GPIO_PC_SIZE 9

// GPIO interrupts
#define GPIO_IRQ_BASE    (0x40006100)
#define GPIO_EXTIPSELL   (0x0)
#define GPIO_EXTIPSELH   (0x1)
#define GPIO_EXTIRISE    (0x2)
#define GPIO_EXTIFALL    (0x3)
#define GPIO_IEN         (0x4)
#define GPIO_IF          (0x5)
#define GPIO_IFS         (0x6)
#define GPIO_IFC         (0x7)
#define GPIO_IRQ_SIZE 8
#define GPIO_EVEN_IRQ_LINE 17
#define GPIO_ODD_IRQ_LINE 18

#define DRIVER_NAME "gamepad"
#define DEV_NR_COUNT 1

// own device-specific structure to mimic scull
//
// it is also possible to define whether to use alloc_chrdev or
// register_chrdev by specifying a major number explicitly, but that
// is a waste of memory here (and we don't need to use
// MKDEV/{MAJOR,MINOR}), as we are not concerned with loading any
// other modules at all
struct gp_chrdev {
	dev_t devno;
	struct cdev cdev;
	struct class *cl;
	struct fasync_struct *asqueue;
	u32 *gpio_pc_mem;
	u32 *gpio_irq_mem;
};

static struct gp_chrdev gp_dev;
u8 first_interrupt;

/*
  Read the state of the gamepad buttons and convert into a byte-sized
  format that is active-high. Pass the value to the user space buffer.
 */
static ssize_t gamepad_read(struct file *filp, char __user * buf, size_t count,
			    loff_t * f_pos)
{
	u32 GPIO_DIN_state;
	u8 gp_state;
	size_t bytes_to_copy = 1;

	GPIO_DIN_state = ioread32(gp_dev.gpio_pc_mem + GPIO_DIN);
	gp_state = ~(GPIO_DIN_state & 0xFF);

	// we only read 1 byte in the gamepad driver, no matter if we
	// are passed a longer count, seeing as doing a read and copy
	// of more data makes no sense in this context
	if (copy_to_user(buf, &gp_state, bytes_to_copy))
		return -EFAULT;

	// cat uses count = 4096 (page size) for read, thus we need to
	// return 0 (end-of-file) if we read less than what is given
	// by count
	return (count == bytes_to_copy ? bytes_to_copy : 0);
}

/*
  Register an interested process to the SIGIO signal.
 */
static int gamepad_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &gp_dev.asqueue);
}

static int gamepad_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
  Unregister processes from the SIGIO signal.
 */
static int gamepad_release(struct inode *inode, struct file *filp)
{
	gamepad_fasync(-1, filp, 0);
	return 0;
}

static struct file_operations gp_fops = {
	.owner = THIS_MODULE,
	.read = gamepad_read,
	.fasync = gamepad_fasync,
	.open = gamepad_open,
	.release = gamepad_release
};

/*
  Add a message to the async. messaging queue used to signal user
  space that a SIGIO signal has occurred.
 */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	u32 GPIO_IF_state;

	// if we push this first interrupt to the asqueue, we will be
	// one msg behind at all times, making it seem like we lag
	if (first_interrupt) {
		first_interrupt = 0;
		return IRQ_HANDLED;
	}

	GPIO_IF_state = ioread32(gp_dev.gpio_irq_mem + GPIO_IF);
	iowrite32(GPIO_IF_state, gp_dev.gpio_irq_mem + GPIO_IFC);
	kill_fasync(&gp_dev.asqueue, SIGIO, POLL_IN);
	return IRQ_HANDLED;
}

static int __init gamepad_init(void)
{
	int err;
	struct resource *gpio_pc_mem;
	struct resource *gpio_irq_mem;
	struct device *chrdev;

	// we do not want to catch the first interrupt, as it is a
	// false positive
	first_interrupt = 1;

	// initialize our device. we could also use memset with 0 and
	// length sizeof(struct gp_chrdev) to empty the initial struct
	gp_dev = (struct gp_chrdev) {
	.devno = 0,.cl = 0,.gpio_pc_mem = 0,.gpio_irq_mem = 0};

	// initialize a (_one_) chardev region with dynamic major part
	// dynamic and minor set to 0
	err = alloc_chrdev_region(&gp_dev.devno, 0, DEV_NR_COUNT, DRIVER_NAME);
	if (err) {
		printk(KERN_WARNING
		       "[gamepad]: Could not allocate chrdev region for gamepad. (%i).\n",
		       err);
		goto fail_alloc_chrdev;
	}
	// create the actual kernel character device
	cdev_init(&gp_dev.cdev, &gp_fops);
	gp_dev.cdev.owner = THIS_MODULE;
	gp_dev.cdev.ops = &gp_fops;

	// setup GPIO before we tell the kernel about our new dev
	gpio_pc_mem =
	    request_mem_region(GPIO_PC_BASE, GPIO_PC_SIZE * sizeof(u32),
			       DRIVER_NAME);
	if (!gpio_pc_mem) {
		printk(KERN_WARNING
		       "[gamepad]: Could not allocate memory for GPIO registers.\n");
		err = -1;
		goto fail_request_gpio_mem_region;
	}
	// setup IRQ as well the same way we did GPIO
	gpio_irq_mem =
	    request_mem_region(GPIO_IRQ_BASE, GPIO_IRQ_SIZE * sizeof(u32),
			       DRIVER_NAME);
	if (!gpio_irq_mem) {
		printk(KERN_WARNING
		       "[gamepad]: Could not allocate memory for IRQ registers.\n");
		err = -1;
		goto fail_request_irq_mem_region;
	}
	// ensure I/O memory is accessible to the kernel with mapping
	gp_dev.gpio_pc_mem =
	    ioremap_nocache(GPIO_PC_BASE, GPIO_PC_SIZE * sizeof(u32));
	gp_dev.gpio_irq_mem =
	    ioremap_nocache(GPIO_IRQ_BASE, GPIO_IRQ_SIZE * sizeof(u32));

	// drive strength, input mode, internal pull-up (we don't need
	// to enable clocks here - they are already enabled)
	iowrite32(0x2, gp_dev.gpio_pc_mem + GPIO_CTRL);
	iowrite32(0x33333333, gp_dev.gpio_pc_mem + GPIO_MODEL);
	iowrite32(0xFF, gp_dev.gpio_pc_mem + GPIO_DOUT);

	// IRQ enable
	iowrite32(0x22222222, gp_dev.gpio_irq_mem + GPIO_EXTIPSELL);
	iowrite32(0xFF, gp_dev.gpio_irq_mem + GPIO_EXTIFALL);

	// now that everything is set up, tell the kernel about the
	// new character device
	err = cdev_add(&gp_dev.cdev, gp_dev.devno, DEV_NR_COUNT);
	if (err) {
		printk(KERN_WARNING
		       "[gamepad]: Error %d while adding gamepad.\n", err);
		goto fail_cdev_add;
	}
	// make the driver appear in /dev (i.e. user space) by
	// creating and registering its class
	gp_dev.cl = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(gp_dev.cl)) {
		printk(KERN_WARNING
		       "[gamepad]: Failed to register device class.\n");
		err = PTR_ERR(gp_dev.cl);
		goto fail_class_create;
	}

	chrdev =
	    device_create(gp_dev.cl, NULL, gp_dev.devno, NULL, DRIVER_NAME);
	if (IS_ERR(chrdev)) {
		printk(KERN_WARNING
		       "[gamepad]: Failed to create device from class.\n");
		err = PTR_ERR(chrdev);
		goto fail_device_create;
	}
	// do not return if interrupts cannot be registered, as they
	// are not specifically required to make the driver work
	if (request_irq
	    (GPIO_EVEN_IRQ_LINE, &gpio_irq_handler, 0, DRIVER_NAME, &gp_dev))
		printk(KERN_INFO
		       "[gamepad]: Could not assign interrupt handler for GPIO Even.\n ");
	if (request_irq
	    (GPIO_ODD_IRQ_LINE, &gpio_irq_handler, 0, DRIVER_NAME, &gp_dev))
		printk(KERN_INFO
		       "[gamepad]: Could not assign interrupt handler for GPIO Odd.\n ");

	// now enable interrupt generation, as we have a functioning
	// char device
	iowrite32(0xFF, gp_dev.gpio_irq_mem + GPIO_IEN);

	printk(KERN_INFO "[gamepad]: Module loaded successfully.\n");
	return 0;

	// fall through goto to release all resources allocated should
	// there be a failure
 fail_device_create:class_destroy(gp_dev.cl);
 fail_class_create:cdev_del(&gp_dev.cdev);
 fail_cdev_add:
	iounmap(gp_dev.gpio_irq_mem);
	release_mem_region(GPIO_IRQ_BASE, GPIO_IRQ_SIZE * sizeof(u32));
 fail_request_irq_mem_region:
	iounmap(gp_dev.gpio_pc_mem);
	release_mem_region(GPIO_PC_BASE, GPIO_PC_SIZE * sizeof(u32));
 fail_request_gpio_mem_region:unregister_chrdev_region(gp_dev.devno,
				 DEV_NR_COUNT);
 fail_alloc_chrdev:return err;
}

static void __exit gamepad_cleanup(void)
{
	// we did not necessarily actually get irq17, irq18, but any
	// further errors trying to free them will not matter
	free_irq(GPIO_EVEN_IRQ_LINE, &gp_dev);
	free_irq(GPIO_ODD_IRQ_LINE, &gp_dev);
	device_destroy(gp_dev.cl, gp_dev.devno);
	class_destroy(gp_dev.cl);
	cdev_del(&gp_dev.cdev);
	iounmap(gp_dev.gpio_irq_mem);
	release_mem_region(GPIO_IRQ_BASE, GPIO_IRQ_SIZE * sizeof(u32));
	iounmap(gp_dev.gpio_pc_mem);
	release_mem_region(GPIO_PC_BASE, GPIO_PC_SIZE * sizeof(u32));
	unregister_chrdev_region(gp_dev.devno, DEV_NR_COUNT);

	printk("[gamepad]: Module removed successfully.\n");
}

module_init(gamepad_init);
module_exit(gamepad_cleanup);

MODULE_DESCRIPTION("Gamepad driver used for exercise 3 in tdt4258");
MODULE_LICENSE("GPL");
