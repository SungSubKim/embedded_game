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
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/buffer_head.h>
#include <asm/io.h> //for ioremap
#include <asm/uaccess.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
MODULE_LICENSE("GPL");

#define I2C_RETRIES	0x0701	/* number of times a device address should
				   be polled when not acknowledging */
#define I2C_TIMEOUT	0x0702	/* set timeout in units of 10 ms */

/* NOTE: Slave address is 7 or 10 bits, but 10-bit addresses
 * are NOT supported! (due to code brokenness)
 */
#define I2C_SLAVE	0x0703	/* Use this slave address */
#define I2C_SLAVE_FORCE	0x0706	/* Use this slave address, even if it
				   is already in use by a driver! */
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit */

#define I2C_FUNCS	0x0705	/* Get the adapter functionality mask */

#define I2C_RDWR	0x0707	/* Combined R/W transfer (one STOP only) */

#define I2C_PEC		0x0708	/* != 0 to use PEC with SMBus */
#define I2C_SMBUS	0x0720	/* SMBus transfer */

#define SSD1306_NAME ("ssd1306")
#define SSD1306_ADDR (0x3c)
#define AM2320_NAME  ("am2320")
#define AM2320_ADDR  (0x5c)

#define PERIPHERAL_BASE 0x3F000000UL
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)


void* gpio_ctr = NULL;
struct file* i2c_file = NULL;
uint8_t* ssd1306_val = NULL;

static struct i2c_adapter *embedpj_i2c_adapter = NULL;

struct i2c_client *ssd1306_client, *am2320_client;
static struct i2c_device_id embedpj_idtable[] = {
    {SSD1306_NAME, 1},
    {AM2320_NAME, 2},
    {}
};

MODULE_DEVICE_TABLE(i2c, embedpj_idtable);

static struct i2c_board_info ssd1306_board_info = {
    I2C_BOARD_INFO(SSD1306_NAME, SSD1306_ADDR)
};

static struct i2c_board_info am2320_board_info = {
    I2C_BOARD_INFO(AM2320_NAME, AM2320_ADDR)
};

static int i2c_write(struct i2c_client *client, uint8_t *buf, int len){
    //printk(KERN_INFO "i2c_write %d bytes\n", len);
    /*int tt;
    for(tt=0; tt<len; tt++){
        printk(KERN_INFO "%x ", buf[tt]);
    }
    printk(KERN_INFO "\n");
    */
    int ret = i2c_master_send(client, buf, len);
    return ret;
}

static int i2c_read(struct i2c_client *client, uint8_t *buf, uint8_t len){
    int ret = i2c_master_recv(client, buf, len);
    return ret;
}

static void ssd1306_command(uint8_t data){
    uint8_t buf[2];
    buf[0] = (0<<7) | (0<<6);
    buf[1] = data;
    i2c_write(ssd1306_client, buf, 2);
}

static void ssd1306_data(uint8_t *data, uint32_t len){
    uint8_t* buf;
    buf = kmalloc(sizeof(uint8_t) * (len+1), GFP_KERNEL);
    if(buf == NULL){
        printk(KERN_INFO "ssd1306_data kernel memory failed\n");
    }
    buf[0] = (0<<7) | (1<<6);
    memcpy(buf+1, data, len);
    i2c_write(ssd1306_client, buf, len+1);
    kfree(buf);
}

static void ssd1306_cursor(uint8_t x_bit, uint8_t y_page){
    ssd1306_command(0x20);
    ssd1306_command(0);

    ssd1306_command(0x21);
    ssd1306_command(x_bit);
    ssd1306_command(127); // column end (showing is height)

    ssd1306_command(0x22);
    ssd1306_command(y_page);
    ssd1306_command(7); // row end (showing is row)
}

static void ssd1306_fill(uint8_t with_page){
    ssd1306_cursor(0,0);
    uint8_t *data;
    data = (uint8_t*) kmalloc(8*128 * sizeof(uint8_t), GFP_KERNEL);
    if(data == NULL){
        printk(KERN_INFO "ssd1306_fill kernel memory failed\n");
    }
    int page, col;
    for(col=0; col<128; col++){
        for(page=0; page<8; page++){
            //printk(KERN_INFO "page setup (%d, %d) => %d\n", page, col, 128*page+col);
            data[(128*page)+col] = with_page;            
        }
    }
    printk(KERN_INFO "ssd1306 fill page setup done %x\n", with_page);
    ssd1306_data(data, 8*128);
    kfree(data);
}

static int ssd1306_remove(struct i2c_client *client){
    ssd1306_command(0xAE);
    return 0;
}

static int am2320_remove(struct i2c_client *client){
    // do nothing
    return 0;
}

static int ssd1306_display_init(void){
    ssd1306_command(0xA8);
    ssd1306_command(0x3F);

    ssd1306_command(0xD3);
    ssd1306_command(0x00);
    
    ssd1306_command(0x40);

    ssd1306_command(0xA0);

    ssd1306_command(0xC0);

    ssd1306_command(0xDA);
    ssd1306_command(0x12);

    ssd1306_command(0x81);
    ssd1306_command(0x7F);

    ssd1306_command(0xA4);

    ssd1306_command(0xA6);

    ssd1306_command(0xD5);
    ssd1306_command(0x80);

    ssd1306_command(0x8D);
    ssd1306_command(0x14);

    ssd1306_command(0xAF);

    return 0;
}

static int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id){
    //embedpj_idtable[id->driver_data] = *client;

    ssd1306_display_init();

    return 0;
}

static int am2320_sensor_wakeup(void){
    uint8_t buf[1];
    buf[0] = 0;
    i2c_write(am2320_client, buf, 1); // wake up
    printk(KERN_INFO "AM2320 sensor wake up signal sent\n");

    return 0;
}

static int am2320_probe(struct i2c_client *client, const struct i2c_device_id *id){
    //embedpj_idtable[id->driver_data] = *client;
    // do nothing

    return 0;
}

static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = SSD1306_NAME
    },
    .id_table = embedpj_idtable,
    .probe = ssd1306_probe,
    .remove = ssd1306_remove
};

static struct i2c_driver am2320_driver = {
    .driver = {
        .name = AM2320_NAME
    },
    .id_table = embedpj_idtable,
    .probe = am2320_probe,
    .remove = am2320_remove
};

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
        uint64_t param_value[128];
        copy_from_user((void*) param_value, (void*) ioctl_param, sizeof(uint64_t)*128);
        int showing_row_idx, showing_col_idx, page, col;
        uint8_t *ssd1306_tmp = kmalloc(sizeof(uint8_t) * 128 * 8, GFP_KERNEL);
        if(ssd1306_tmp == NULL){
            printk(KERN_INFO "ssd1306 ioctl kernel memory failed\n");
        }
        for(col=0; col<128; col++){
            for(page=0; page<8; page++){
                ssd1306_tmp[(128*page)+col] = 0x00; // tmp clear            
            }
        }
        for(showing_col_idx = 0; showing_col_idx < 64; showing_col_idx++){
            for(showing_row_idx=0; showing_row_idx < 128; showing_row_idx++){
                uint8_t actual_col = showing_row_idx;
                uint8_t actual_row_page = showing_col_idx/8;
                uint8_t actual_row_offset = showing_col_idx%8;
                uint64_t showing_extract = 1;
                showing_extract <<= showing_col_idx;
                ssd1306_tmp[(128*(actual_row_page))+actual_col] |= 
                    showing_extract & param_value[showing_row_idx]?1<<actual_row_offset:0<<actual_row_offset;
            }
        }
        ssd1306_cursor(0,0);
        ssd1306_data(ssd1306_tmp, 128*8);

        kfree(ssd1306_tmp);
    }
    if(ioctl_num == 300){
        uint16_t param_value[1];
        copy_from_user((void*) param_value, (void*) ioctl_param, sizeof(uint16_t)*1);
        am2320_sensor_wakeup();
        uint8_t buf[3];
        buf[0] = 0x3;
        buf[1] = 0x0;
        buf[2] = 0x4; // read request
        i2c_write(am2320_client, buf, 3);
        printk(KERN_INFO "AM2320 sensor read request\n");  
    }
    if(ioctl_num == 301){
        uint16_t param_value[2];
		uint8_t buf[8];
        i2c_read(am2320_client, buf, 8);
        uint16_t humidity_m10 = buf[2] << 8 | buf[3];
        uint16_t temperature_m10 = buf[4] << 8 | buf[5];
        printk(KERN_INFO "AM2320 sensor load req, hum %d, temp %d\n", humidity_m10, temperature_m10);
        param_value[0] = humidity_m10;
        param_value[1] = temperature_m10;
		copy_to_user((void*) ioctl_param, (void*) param_value, sizeof(uint16_t)*2);
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
    ssd1306_val = kmalloc(sizeof(uint8_t) * 128 * 8, GFP_KERNEL);
    if(ssd1306_val == NULL){
        printk(KERN_INFO "ssd1306_init kernel memory failed\n");
    }

    ssd1306_client = i2c_new_client_device(embedpj_i2c_adapter, &ssd1306_board_info);
    if(ssd1306_client == NULL){
        printk(KERN_INFO "SSD1306 client null\n");
    }
    i2c_add_driver(&ssd1306_driver);
    
    ssd1306_fill(0x75);
    
    printk(KERN_INFO "SSD1306 Load done?\n");

    return 0x00;
}

static int init_i2c_am2320(void){
    am2320_client = i2c_new_client_device(embedpj_i2c_adapter, &am2320_board_info);
    if (am2320_client == NULL){
        printk(KERN_INFO "AM2320 client null\n");
    }
    i2c_add_driver(&am2320_driver);
    
    printk(KERN_INFO "AM2320 Load done?\n");

    return 0x00;
}

static int __init rpi_key_init(void)
{
	majorNumber = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);
	cRpiKeyClass = class_create(THIS_MODULE, CLASS_NAME);
	cRpiKeyDevice=device_create(cRpiKeyClass, NULL,MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	
    int ret = 0;
    ret |= init_gpio();
    embedpj_i2c_adapter = i2c_get_adapter(1); // 1: rpi i2c bus is 1
    ret |= init_i2c_ssd1306();
    ret |= init_i2c_am2320();
    i2c_put_adapter(embedpj_i2c_adapter);

	return ret;
}
static void __exit rpi_key_exit(void) {
	iounmap(gpio_ctr);
    i2c_unregister_device(ssd1306_client);
    i2c_unregister_device(am2320_client);
    i2c_del_driver(&ssd1306_driver);
    i2c_del_driver(&am2320_driver);
    kfree(ssd1306_val);
	device_destroy(cRpiKeyClass, MKDEV(majorNumber, 0));
	class_unregister(cRpiKeyClass);
	class_destroy(cRpiKeyClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
}

module_init(rpi_key_init);
module_exit(rpi_key_exit);
