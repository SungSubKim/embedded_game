#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <asm/io.h> //for ioremap
MODULE_LICENSE("GPL");

#define SSD1306_ADDR 0x3c
#define AM2320_ADDR 0x5c

#define PERIPHERAL_BASE 0x3F000000UL
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)

void* gpio_ctr = NULL;
struct file* i2c_file = NULL;
uint8_t* ssd1306_val = NULL;

void ssd1306_command(uint8_t cmd){

}

void set_gpio_output(void *gpio_ctr, int gpio_nr) {
	int reg_id = gpio_nr / 10;
	int pos = gpio_nr % 10;
	uint32_t* fsel_reg = (uint32_t*) (gpio_ctr + 0x4 * reg_id);
	uint32_t fsel_val = *fsel_reg;
	uint32_t mask = 0x7 << (pos * 3);
	fsel_val = fsel_val & ~mask;
	uint32_t gpio_output_select = 0x1 << (pos * 3);
	fsel_val = fsel_val | gpio_output_select;
	*fsel_reg = fsel_val;
}


void set_gpio_input(void *gpio_ctr, int gpio_nr) {
	int reg_id = gpio_nr / 10;
	int pos = gpio_nr % 10;
	uint32_t* fsel_reg = (uint32_t*) (gpio_ctr + 0x4 * reg_id);
	uint32_t fsel_val = *fsel_reg;
	uint32_t mask = 0x7 << (pos * 3);
	fsel_val = fsel_val & ~mask;
	*fsel_reg = fsel_val;
}


void set_gpio_output_value(void *gpio_ctr, int gpio_nr, int value) {
	int reg_id = gpio_nr / 32;
	int pos = gpio_nr % 32;
	if(value) {
		#define GPIO_SET_OFFSET 0x1c
		uint32_t* output_set = (uint32_t*) (gpio_ctr + GPIO_SET_OFFSET + 0x4 * reg_id);
		*output_set = 0x1 << pos;
	}
	else {
		#define GPIO_CLR_OFFSET 0x28
		uint32_t* output_clr = (uint32_t*) (gpio_ctr + GPIO_CLR_OFFSET + 0x4 * reg_id);
		*output_clr = 0x1 << pos;
	}
}


void get_gpio_input_value(void *gpio_ctr, int gpio_nr, int *value) {
		int reg_id = gpio_nr / 32;
		int pos = gpio_nr % 32;
		#define GPIO_LEV_OFFSET 0x34
		uint32_t* level_reg = (uint32_t*) (gpio_ctr + GPIO_LEV_OFFSET + 0x4 * reg_id);
		uint32_t level = *level_reg & (0x1 << pos);
		*value = level? 1:0;
}


void set_gpio_pullup(void *gpio_ctr, int gpio_nr) {
	int reg_id = gpio_nr / 32;
	int pos = gpio_nr % 32;
#define GPIO_PUD_OFFSET 0x94
#define GPIO_PUDCLK_OFFSET 0x98
	uint32_t* pud_reg = (uint32_t*) (gpio_ctr + GPIO_PUD_OFFSET);
	uint32_t* pudclk_reg = (uint32_t*) (gpio_ctr + GPIO_PUDCLK_OFFSET + 0x4 * reg_id);
#define GPIO_PUD_PULLUP 0x2
	*pud_reg = GPIO_PUD_PULLUP;
	udelay(1);
	*pudclk_reg = (0x1 << pos);
	udelay(1);
	*pud_reg = 0;
	*pudclk_reg = 0;
}


#define MAJOR_NUM 0
#define DEVICE_NAME "rpikey"
#define CLASS_NAME "rpikey_class"


static int majorNumber;
static struct class* cRpiKeyClass = NULL;
static struct device* cRpiKeyDevice = NULL;
static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
long device_ioctl( struct file *file, unsigned int ioctl_num, unsigned long ioctl_param); 
struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};
static int device_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "rpi_key device_open(%p)\n", file);
	return 0;
}
static int device_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "rpi_key device_release(%p)\n", file);
	return 0;
}


long device_ioctl( struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
	if(ioctl_num == 100) {
		uint32_t param_value[4];
		get_gpio_input_value(gpio_ctr,22,&param_value[0]);
		get_gpio_input_value(gpio_ctr,23,&param_value[1]);
        get_gpio_input_value(gpio_ctr,24,&param_value[2]);
        get_gpio_input_value(gpio_ctr,25,&param_value[3]);
		copy_to_user((void*) ioctl_param, (void*) param_value, sizeof(uint32_t)*4);
	}
	if(ioctl_num == 101) {
		uint32_t param_value[3];
		int gpio13;
		int gpio19;
		int gpio26;
		copy_from_user((void*) param_value, (void*) ioctl_param, sizeof(uint32_t)*3);
		gpio26 = (int)param_value[0];
		gpio19 = (int)param_value[1];
		gpio13 = (int)param_value[2];
		set_gpio_output_value(gpio_ctr,13,gpio13);
		set_gpio_output_value(gpio_ctr,19,gpio19);
		set_gpio_output_value(gpio_ctr,26,gpio26);
	}
    if(ioctl_num == 200){

    }
    if(ioctl_num == 300){
        
    }
    if(ioctl_num == 301){

    }
	return 0;
}

static int init_gpio(void){
    gpio_ctr = ioremap(GPIO_BASE,0x1000);
	set_gpio_output(gpio_ctr,13);
	set_gpio_output(gpio_ctr,19);
	set_gpio_output(gpio_ctr,26);
	set_gpio_input(gpio_ctr,22);
	set_gpio_input(gpio_ctr,23);
    set_gpio_input(gpio_ctr,24);
    set_gpio_input(gpio_ctr,25);
	set_gpio_pullup(gpio_ctr,22);
	set_gpio_pullup(gpio_ctr,23);
    set_gpio_pullup(gpio_ctr,24);
    set_gpio_pullup(gpio_ctr,25);

    return 0x00;
}

static int init_i2c_ssd1306(void){
    ssd1306_val = vmalloc(sizeof(uint8_t) * 64 * 16);
    
    i2c_file = filp_open("/dev/i2c-1", O_RDWR, 0);

    return 0x00;
}

static int init_i2c_am2320(void){
    return 0x00;
}

static int __init rpi_key_init(void)
{
	majorNumber = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);
	cRpiKeyClass = class_create(THIS_MODULE, CLASS_NAME);
	cRpiKeyDevice=device_create(cRpiKeyClass, NULL,MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	
    int ret = 0;
    ret |= init_gpio();
    ret |= init_i2c_ssd1306();
    ret |= init_i2c_am2320();

	return ret;
}
static void __exit rpi_key_exit(void) {
	iounmap(gpio_ctr);
	device_destroy(cRpiKeyClass, MKDEV(majorNumber, 0));
	class_unregister(cRpiKeyClass);
	class_destroy(cRpiKeyClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
}

module_init(rpi_key_init);
module_exit(rpi_key_exit);
