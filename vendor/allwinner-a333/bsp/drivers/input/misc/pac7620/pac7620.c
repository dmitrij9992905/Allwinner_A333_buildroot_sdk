
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
//#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
//#include <mach/gpio.h>
#include <linux/gpio.h>
#include "pac7620.h"

#include <linux/delay.h>
#include <linux/version.h>

#include "../../init-input.h"

#define KEY_GESTURE_UP                          KEY_F15
#define KEY_GESTURE_DOWN                        KEY_F16
#define KEY_GESTURE_LEFT                        KEY_F17
#define KEY_GESTURE_RIGHT                       KEY_F18
#define KEY_GESTURE_FORWARD                     KEY_F19
#define KEY_GESTURE_BACKWARD                    KEY_F20
#define KEY_GESTURE_CLOCKWISE                   KEY_F21
#define KEY_GESTURE_COUNT_CLOCKWISE             KEY_F22
#define KEY_GESTURE_WAVE                        KEY_F23

static const unsigned short normal_i2c[] = {0x73, I2C_CLIENT_END};

typedef struct {
	struct i2c_client	*client;
	struct input_dev *keyboard_input_dev;
	int irq;
	bank_e bank;
	bool gestrue_enable;
} pac7620_data_t;

static  pac7620_data_t pac7620data;

static struct ctp_config_info ctp_info = {
	.input_type = CTP_TYPE,
	.np_name = "pac7620",
};



static int pac7620_i2c_write(u8 reg, u8 *data, int len)
{
	u8  buf[20];
	int rc;
	int ret = 0;
	int i;

	buf[0] = reg;
	if (len >= 20) {
		printk("%s (%d) : FAILED: buffer size is limitted(20) %d\n", __func__, __LINE__, len);
		dev_err(&pac7620data.client->dev, "pac7620_i2c_write FAILED: buffer size is limitted(20)\n");
		return -1;
	}

	for( i=0 ; i<len; i++ ) {
		buf[i+1] = data[i];
	}
 
	rc = i2c_master_send(pac7620data.client, buf, len+1);

	if (rc != len+1) {
		printk("%s (%d) : FAILED: writing to reg 0x%x\n", __func__, __LINE__, reg);

		ret = -1;
	}

	return ret;
}


static int pac7620_i2c_read(u8 reg, u8 *data)
{
	u8  buf[20];
	int rc;		
	
	buf[0] = reg;
	
	rc = i2c_master_send(pac7620data.client, buf, 1);
	if (rc != 1) {
		printk("%s (%d) : FAILED: writing to address 0x%x\n", __func__, __LINE__, reg);
		return -1;
	}	
	
	rc = i2c_master_recv(pac7620data.client, buf, 1);
	if (rc != 1) {
		printk("%s (%d) : FAILED: reading data\n", __func__, __LINE__);
		return -1;
	}	
		
	*data = buf[0] ;		
	return 0;
}

static int pac7620_set_reg(u8 reg, u8 data)
{
	int ret = pac7620_i2c_write(reg, &data, 1);
	
	printk("%s (%d) : set register , addr = 0x%x, data = 0x%x \n", __func__, __LINE__, reg, data);


	return  ret;
}

static int pac7620_bank_select(bank_e bank)
{
	switch(bank){
		case BANK0:
			pac7620_set_reg(PAC7620_REGITER_BANK_SEL, PAC7620_BANK0);
			break;
		case BANK1:
			pac7620_set_reg(PAC7620_REGITER_BANK_SEL, PAC7620_BANK1);
			break;
		default:
			break;
	}
	
	pac7620data.bank = bank;
	
	return 0;
}


/*
static int pac7620_interrupt_mask(mode_e mode)
{	
	printk("%s (%d) : pac7620 interrupt mask : 0x%x.\n", __func__, __LINE__, mode);

	pac7620_bank_select(BANK0);
	
	if(mode == IRMOTION_ENABLED){
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_0,0xFF);
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_1,0x01);		
	}else if(mode == PROXIMITY_ENABLED){
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_0,0x00);
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_1,0x02);
	}else if(mode == ALL_ENABLED){
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_0,0xFF);
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_1,0x03);
	}else if(mode == ALL_DISABLE){
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_0,0x00);
		pac7620_set_reg(PAC7620_ADDR_GES_PS_DET_MASK_1,0x00);
	}

	pac7620_register_debug();

	return 0;
}
*/

static long _read_addr = 0x01 ;
static ssize_t write_reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	char s[256];  
	char *p = s ;
	printk("%s (%d) : write register\n", __func__, __LINE__);

	memcpy(s, buf, size);
	
	*(s+1)='\0';
	*(s+4)='\0';
	*(s+7)='\0';
	
	if(*p == 'w')
	{
		long write_addr, write_data ;

		p += 2;
		if(!kstrtol(p, 16, &write_addr))
		{
			p += 3 ;
			if(!kstrtol(p, 16, &write_data))
			{		
				printk("w 0x%x 0x%x\n", (unsigned int)write_addr, (unsigned int)write_data);
				pac7620_set_reg( (u8)write_addr, (u8)write_data);
			}
		}
	}
	else if(*p == 'r')
	{
		p+=2;
		
		if(!kstrtol(p, 16, &_read_addr))
		{
			u8 data = 0;
			if(!pac7620_i2c_read((u8)_read_addr, &data))
			{
				printk("r 0x%x 0x%x\n", (unsigned int)_read_addr, data);
			}
		}
	}
	return size;
}

static ssize_t read_reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret ;
 	char *s= buf;  
 	u8 data = 0;

	printk("%s (%d) : read register\n", __func__, __LINE__);
	ret = pac7620_i2c_read(_read_addr, &data);
	
	if(ret)
		s += sprintf(s,"Error\n");  
	else
  	s += sprintf(s,"Addr 0x%x, Data 0x%x\n",(unsigned int)_read_addr, data);  
	
	return (s - buf);
}

static DEVICE_ATTR(rw_reg, S_IRUGO | S_IWUSR | S_IWGRP, read_reg_show, write_reg_store);

static struct attribute *rw_reg_sysfs_attrs[] = {
	&dev_attr_rw_reg.attr,
	NULL
};

static struct attribute_group rw_reg_attribute_group = {
	.attrs = rw_reg_sysfs_attrs,
};

static int pac7620_init_reg(void)
{
	//Near_normal_mode_V5_6.15mm_121017 for 940nm
	int i=0;
	u8 data0 = 0, data1 = 0;
	int ret ;
	
	pac7620_bank_select(BANK0);	//wakeup
	pac7620_bank_select(BANK0);
	
	ret = pac7620_i2c_read(0, &data0);
	if( ret )
	{
		return 0 ;
	}
	
	ret = pac7620_i2c_read(1, &data1);
	if( ret )
	{
		return 0 ;
	}
	
	printk("%s (%d) : ADDR0 = 0x%x, ADDR1 = 0x%x.\n", __func__, __LINE__, data0, data1);
	if( (data0 != 0x20 ) || (data1 != 0x76) )
	{
		return 0 ;
	}

	for(i = 0; i < INIT_REG_ARRAY_SIZE;i++){
		pac7620_set_reg(init_register_array[i][0],init_register_array[i][1]);
	}
	
	printk("%s (%d) : pac7620 initialize register.\n", __func__, __LINE__);
	
	return 0;
}

static void pac7620_runtime_suspend(bool enable) {

	if (enable) {
	// Now we go into suspend mode
	pac7620_set_reg(0xEF,0x01); // Switch to bank 1
	//pac7620_set_reg(0x7E,0x00); // disable SPI, Modify By Angelo, 20130228
	pac7620_set_reg(0x72,0x00); // disable 7620
	
	pac7620_set_reg(0xEF,0x00); // Switch to bank 0
	pac7620_set_reg(0x03,0x01); // boy modify @ 2014_1225	

	disable_irq(pac7620data.irq);
	input_set_power_enable(&(ctp_info.input_type), 0);

	} else {
		u8 data;
	int err = 0;

	input_set_power_enable(&(ctp_info.input_type), 1);
	err = pac7620_init_reg();
	if (err < 0) {
		printk("pac7620_init_reg err=%d %s", err, __func__);
		return;
	}

	// Now we go into resume mode
	// Read ID 3 times to make sure I2C is active
	pac7620_i2c_read(0x00, &data);
	pac7620_i2c_read(0x00, &data);
	pac7620_i2c_read(0x00, &data);
	
	pac7620_set_reg(0xEF,0x01); // Switch to bank 1
	pac7620_set_reg(0x72,0x01); // Enable 7620
	pac7620_set_reg(0xEF,0x00); //
	pac7620_set_reg(0x5E,0x12); // Enable DMSP_CLK_manual, Modify By Neil, 20180524
	pac7620_set_reg(0xEE,0x03); // Reset DSP
	udelay(500);//delay 500us
	pac7620_set_reg(0xEE,0x07); // 
	pac7620_set_reg(0x5E,0x10); // Disable DMSP_CLK_manual, Modify By Neil, 20180524
	//pac7620_set_reg(0x7E,0x01); // Enable SPI, Modify By Angelo, 20130228	

    enable_irq(pac7620data.irq);	

	}

	printk ("pac7620_runtime_suspend %d", enable);
	return;

}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0))
static ssize_t runtime_suspend_show(const struct class *cls,
	       const struct class_attribute *attr, char *buf)
#else
static ssize_t runtime_suspend_show(struct class *cls,
	    struct class_attribute *attr, char *buf)
#endif
{
	return sprintf(buf, "%d\n", (int)pac7620data.gestrue_enable);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0))
static ssize_t runtime_suspend_store(const struct class *cls,
	       const struct class_attribute *attr,
	       const char *buf, size_t count)
#else
static ssize_t runtime_suspend_store(struct class *cls,
	       struct class_attribute *attr,
	       const char *buf, size_t count)
#endif
{
	unsigned long data;

	data = simple_strtoul(buf, NULL, 10);

	if (data == 0) {
		pac7620_runtime_suspend(false);
	} else if (data == 1) {
		pac7620_runtime_suspend(true);
	}

	printk("pac7620 gestrue_enable %d", (int)pac7620data.gestrue_enable);

	return count;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0))
static ssize_t gesture_enable_show(const struct class *cls,
	       const struct class_attribute *attr, char *buf)
#else
static ssize_t gesture_enable_show(struct class *cls,
	       struct class_attribute *attr, char *buf)
#endif
{
	return sprintf(buf, "%d\n", (int)pac7620data.gestrue_enable);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0))
static ssize_t gesture_enable_store(const struct class *cls,
	       const struct class_attribute *attr,
	       const char *buf, size_t count)
#else
static ssize_t gesture_enable_store(struct class *cls,
	       struct class_attribute *attr,
	       const char *buf, size_t count)
#endif
{
	unsigned long data;

	data = simple_strtoul(buf, NULL, 10);

	if (data == 0) {
		pac7620data.gestrue_enable = false;
	} else {
		pac7620data.gestrue_enable = true;
	}

	printk("pac7620 gestrue_enable %d", (int)pac7620data.gestrue_enable);
	return count;
}

static CLASS_ATTR_RW(runtime_suspend);
static CLASS_ATTR_RW(gesture_enable);

static struct attribute *gesture_control_class_attrs[] = {
	&class_attr_runtime_suspend.attr,
	&class_attr_gesture_enable.attr,
	NULL
};

ATTRIBUTE_GROUPS(gesture_control_class);

static struct class gesture_control_class = {
	.name = "gesture_control",
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0))
	.owner = THIS_MODULE,
#endif
	.class_groups = gesture_control_class_groups,
};

static int pac7620_init_keyboard_data(void)
{
	int ret = 0;

	printk("%s (%d) : initialize data\n", __func__, __LINE__);
	
	pac7620data.keyboard_input_dev = input_allocate_device();
	
	if (!pac7620data.keyboard_input_dev) {
		printk("%s (%d) : could not allocate keyboard input device\n", __func__, __LINE__);
		return -ENOMEM;
	}

	input_set_drvdata(pac7620data.keyboard_input_dev, &pac7620data);
	pac7620data.keyboard_input_dev->name = "gesture_keyboard_sensor";	

	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_UP);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_DOWN);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_LEFT);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_RIGHT);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_FORWARD);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_BACKWARD);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_CLOCKWISE);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_COUNT_CLOCKWISE);
	input_set_capability(pac7620data.keyboard_input_dev, EV_KEY, KEY_GESTURE_WAVE);

	input_set_capability(pac7620data.keyboard_input_dev, EV_REL, REL_X);
	input_set_capability(pac7620data.keyboard_input_dev, EV_REL, REL_Y);
	input_set_capability(pac7620data.keyboard_input_dev, EV_REL, REL_Z);
	ret = input_register_device(pac7620data.keyboard_input_dev);
	if (ret < 0) {
		input_free_device(pac7620data.keyboard_input_dev);
		printk("%s (%d) : could not register input device\n", __func__, __LINE__);	
		return ret;
	}
	
	ret = sysfs_create_group(&pac7620data.keyboard_input_dev->dev.kobj, &rw_reg_attribute_group);	
	if (ret) {
		printk("%s (%d) : could not create sysfs group\n", __func__, __LINE__);
		return 0;
	}	
	return 0;	
}

#define PAC7620_INT_GIO 38	//P8 pin3 GPIO1_6


irqreturn_t pac7620_irq_thread_fn(int irq, void *data)
{
	int ret;
	u8 int_flag1 = 0, int_flag2 = 0;
	unsigned int keyCode = KEY_RESERVED;


	ret = gpio_get_value(ctp_info.int_number);
	printk("%s (%d) : gpio value = %d\n", __func__, __LINE__, ret);
	
	pac7620_bank_select(BANK0);
	ret = pac7620_i2c_read(PAC7620_ADDR_GES_PS_DET_FLAG_0, &int_flag1);
	if(!ret)
		printk("%s (%d) : interrupt flag1 = 0x%x\n", __func__, __LINE__, int_flag1);
	else
		goto rtn;				
		
	ret = pac7620_i2c_read(PAC7620_ADDR_GES_PS_DET_FLAG_1, &int_flag2);
	if(!ret)
		printk("%s (%d) : interrupt flag2 = 0x%x\n", __func__, __LINE__, int_flag2);
	else
		goto rtn;				

	if (pac7620data.gestrue_enable == false) {
		goto rtn;
	}

	switch(int_flag1)
	{
		case 	GES_RIGHT_FLAG:
			keyCode = KEY_GESTURE_RIGHT;
			break;
		case 	GES_LEFT_FLAG:
			keyCode = KEY_GESTURE_LEFT;
			break;
		case 	GES_UP_FLAG:
			keyCode = KEY_GESTURE_UP;
			break;
		case 	GES_DOWN_FLAG:
			keyCode = KEY_GESTURE_DOWN;
			break;
		case 	GES_FORWARD_FLAG:
			keyCode = KEY_GESTURE_FORWARD;
			break;
		case 	GES_BACKWARD_FLAG:
			keyCode = KEY_GESTURE_BACKWARD;
			break;
		case 	GES_CLOCKWISE_FLAG:
			keyCode = KEY_GESTURE_CLOCKWISE ;
			break;
		case 	GES_COUNT_CLOCKWISE_FLAG:
			keyCode = KEY_GESTURE_COUNT_CLOCKWISE ;
			break;
		default:
			break;
	}
	switch (int_flag2) {
		case GES_WAVE_FLAG:
			keyCode = KEY_GESTURE_WAVE;
			break;
		default:
			break;
	}
	if (keyCode != KEY_RESERVED) {
		input_report_key(pac7620data.keyboard_input_dev, keyCode, 1);
		input_sync(pac7620data.keyboard_input_dev);
		input_report_key(pac7620data.keyboard_input_dev, keyCode, 0);
		input_sync(pac7620data.keyboard_input_dev);
	}

rtn:
	printk("%s (%d) \n", __func__, __LINE__);
	return IRQ_HANDLED;
}

static int pac7620_init_interrupt(void)
{
	int result;

	result = gpio_request(ctp_info.int_number, "pac7620_int_gpio");
    if (result != 0)
	{
		printk("pac7620 interrupt request failed!\n");
		goto rtn;
	}
		
	result = gpio_direction_input(ctp_info.int_number);
	if (result != 0)
	{
		printk("pac7620 direction failed!\n");
		goto rtn;
	}

	pac7620data.irq = gpio_to_irq(ctp_info.int_number);
	
	result = request_threaded_irq(pac7620data.irq, NULL,
				  pac7620_irq_thread_fn,
				  IRQF_TRIGGER_FALLING | IRQF_ONESHOT, // trugger -> level change
				  "pac7620_irq", &pac7620data);
	
	if (result != 0)
	{
		printk("pac7620 request_irq failed \n");
		goto rtn;
	}	

//	enable_irq(pac7620data.irq);	

rtn:
	printk("%s (%d) : gpio %d, irq %d, ret (%d)\n", __func__, __LINE__, ctp_info.irq_gpio.gpio, pac7620data.irq , result);
	
	return result;
	
}

//static struct kobject *example_kobj;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
static int pac7620_i2c_probe(struct i2c_client *client)
#else
static int pac7620_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
#endif
{
	int err = 0;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

	printk("%s (%d) : probe module\n", __func__, __LINE__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		return err;
	}	

	pac7620data.client = client;

	pac7620data.gestrue_enable = false;

	err = pac7620_init_reg();
	if (err < 0) {
		return err;
	}
	
	err = pac7620_init_keyboard_data();
	if (err < 0) {
		return err;
	}

	err = pac7620_init_interrupt();
	if (err < 0) {
		return err;
	}
	
//  example_kobj = kobject_create_and_add("pac7620", kernel_kobj);
//  if (!example_kobj)
//       return -ENOMEM;

  /* Create the files associated with this kobject */
//  err = sysfs_create_group(example_kobj, &rw_reg_attribute_group);
//  if (err)
//       kobject_put(example_kobj);

	err = class_register(&gesture_control_class);
	if (err < 0) {
		return err;
	}

	return err;	
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
static void pac7620_i2c_remove(struct i2c_client *client)
#else
static int pac7620_i2c_remove(struct i2c_client *client)
#endif
{
	pac7620_set_reg(0x03,0x00);

	free_irq(pac7620data.irq, &pac7620data);
	
	gpio_free(ctp_info.irq_gpio.gpio);

	class_unregister(&gesture_control_class);
	sysfs_remove_group(&pac7620data.keyboard_input_dev->dev.kobj,
			   &rw_reg_attribute_group);
	
	input_unregister_device(pac7620data.keyboard_input_dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
	return;
#else
	return 0;
#endif
}


static int pac7620_suspend(struct device *dev)
{
	// Now we go into suspend mode
	pac7620_set_reg(0xEF,0x01); // Switch to bank 1
	//pac7620_set_reg(0x7E,0x00); // disable SPI, Modify By Angelo, 20130228
	pac7620_set_reg(0x72,0x00); // disable 7620
	
	pac7620_set_reg(0xEF,0x00); // Switch to bank 0
	pac7620_set_reg(0x03,0x01); // boy modify @ 2014_1225	

	disable_irq(pac7620data.irq);
	input_set_power_enable(&(ctp_info.input_type), 0);

	return 0;
}

static int pac7620_resume(struct device *dev)
{
	u8 data;
	int err = 0;

	input_set_power_enable(&(ctp_info.input_type), 1);
	err = pac7620_init_reg();
	if (err < 0) {
		printk("pac7620_init_reg err=%d %s", err, __func__);
		return 0;
	}

	// Now we go into resume mode
	// Read ID 3 times to make sure I2C is active
	pac7620_i2c_read(0x00, &data);
	pac7620_i2c_read(0x00, &data);
	pac7620_i2c_read(0x00, &data);
	
	pac7620_set_reg(0xEF,0x01); // Switch to bank 1
	pac7620_set_reg(0x72,0x01); // Enable 7620
	pac7620_set_reg(0xEF,0x00); //
	pac7620_set_reg(0x5E,0x12); // Enable DMSP_CLK_manual, Modify By Neil, 20180524
	pac7620_set_reg(0xEE,0x03); // Reset DSP
	udelay(500);//delay 500us
	pac7620_set_reg(0xEE,0x07); // 
	pac7620_set_reg(0x5E,0x10); // Disable DMSP_CLK_manual, Modify By Neil, 20180524
	//pac7620_set_reg(0x7E,0x01); // Enable SPI, Modify By Angelo, 20130228	

    enable_irq(pac7620data.irq);	

	return 0;
}

static const struct dev_pm_ops pac7620_pm_ops = {
	.suspend = pac7620_suspend,
	.resume = pac7620_resume
};

static const struct i2c_device_id pac7620_device_id[] = {
	{"pac7620", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, pac7620_device_id);

static const struct of_device_id pac7620_of_match[] = {
	{.compatible = "allwinner,pac7620"},
	{},
};

static struct i2c_driver pac7620_i2c_driver = {
	.driver = {
		.of_match_table = pac7620_of_match,
		.name = "pac7620",
		.owner = THIS_MODULE,
		.pm = &pac7620_pm_ops
	},
	.probe = pac7620_i2c_probe,
	.remove = pac7620_i2c_remove,
	.id_table = pac7620_device_id,
	.address_list	= normal_i2c,
};

static int startup(void)
{
	int ret = -1;

	printk("function=%s=========LINE=%d. \n", __func__, __LINE__);

	if (input_sensor_startup(&(ctp_info.input_type))) {
		printk("%s: err.\n", __func__);
		return -1;
	} else
		ret = input_sensor_init(&(ctp_info.input_type));

	if (0 != ret) {
	    printk("%s:gsensor.init_platform_resource err. \n", __func__);
	}

	//twi_id = gsensor_info.twi_id;
	input_set_power_enable(&(ctp_info.input_type), 1);
	return 0;
}

static int __init pac7620_init(void)
{
	printk("%s (%d) : init module\n", __func__, __LINE__);
    if (startup() != 0)
		return -1;

	return i2c_add_driver(&pac7620_i2c_driver);
}

static void __exit pac7620_exit(void)
{
	printk("%s (%d) : exit module\n", __func__, __LINE__);	

	i2c_del_driver(&pac7620_i2c_driver);
}

module_init(pac7620_init);
module_exit(pac7620_exit);
MODULE_AUTHOR("pantech");
MODULE_DESCRIPTION("pac7620 motion sensor driver");
MODULE_LICENSE("GPL");

