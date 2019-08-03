#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/delay.h>
#include <linux/of.h>
#include <linux/i2c.h>
/*
**this is a test driver for sf16ax8-i2c
**
*/


#define I2C_BASE	(0xB8102000)
/*
 * Registers offset
 */

#define SF_IC_CON		0x0
#define SF_IC_SAR		0x8
#define SF_IC_DATA_CMD		0x10
#define SF_IC_SS_SCL_HCNT	0x14
#define SF_IC_SS_SCL_LCNT	0x18
#define SF_IC_FS_SCL_HCNT	0x1c
#define SF_IC_FS_SCL_LCNT	0x20
#define SF_IC_INTR_STAT		0x2c
#define SF_IC_INTR_MASK		0x30
#define SF_IC_RAW_INTR_STAT	0x34
#define SF_IC_RX_TL		0x38
#define SF_IC_TX_TL		0x3c
#define SF_IC_CLR_INTR		0x40
#define SF_IC_CLR_RX_UNDER	0x44
#define SF_IC_CLR_RX_OVER	0x48
#define SF_IC_CLR_TX_OVER	0x4c
#define SF_IC_CLR_RD_REQ	0x50
#define SF_IC_CLR_TX_ABRT	0x54
#define SF_IC_CLR_RX_DONE	0x58
#define SF_IC_CLR_ACTIVITY	0x5c
#define SF_IC_CLR_STOP_DET	0x60
#define SF_IC_CLR_START_DET	0x64
#define SF_IC_CLR_GEN_CALL	0x68
#define SF_IC_ENABLE		0x6c
#define SF_IC_STATUS		0x70
#define SF_IC_TXFLR		0x74
#define SF_IC_RXFLR		0x78
#define SF_IC_SDA_HOLD		0x7c
#define SF_IC_TX_ABRT_SOURCE	0x80
#define SF_IC_ENABLE_STATUS	0x9c
#define SF_IC_IGNORE_ACK    0xa0
/*
#define SF_IC_COMP_PARAM_1	0xf4
#define SF_IC_COMP_VERSION	0xf8
#define SF_IC_SDA_HOLD_MIN_VERS	0x3131312A
#define SF_IC_COMP_TYPE		0xfc
#define SF_IC_COMP_TYPE_VALUE	0x44570140
*/
#define SF_IC_INTR_RX_UNDER	0x001
#define SF_IC_INTR_RX_OVER	0x002
#define SF_IC_INTR_RX_FULL	0x004
#define SF_IC_INTR_TX_OVER	0x008
#define SF_IC_INTR_TX_EMPTY	0x010
#define SF_IC_INTR_RD_REQ	0x020
#define SF_IC_INTR_TX_ABRT	0x040
#define SF_IC_INTR_RX_DONE	0x080
#define SF_IC_INTR_ACTIVITY	0x100
#define SF_IC_INTR_STOP_DET	0x200
#define SF_IC_INTR_START_DET	0x400
#define SF_IC_INTR_GEN_CALL	0x800

#define SF_IC_INTR_DEFAULT_MASK		(SF_IC_INTR_RX_FULL | \
					 SF_IC_INTR_TX_EMPTY | \
					 SF_IC_INTR_TX_ABRT | \
					 SF_IC_INTR_STOP_DET)

#define SF_IC_STATUS_ACTIVITY	0x1

#define SF_IC_ERR_TX_ABRT	0x1

#define SF_IC_TAR_10BITADDR_MASTER BIT(12)

/*
 * status codes
 */
#define STATUS_IDLE			0x0
#define STATUS_WRITE_IN_PROGRESS	0x1
#define STATUS_READ_IN_PROGRESS		0x2
/*
* define i2c mask, only rx full and rd req irq occured.
*/


#define I2C_MASK  		(SF_IC_INTR_RX_FULL | \
						 SF_IC_INTR_RD_REQ | \
						 SF_IC_INTR_RX_DONE)	

#define TIMEOUT			20 /* ms */

/*
 * hardware abort codes from the SF_IC_TX_ABRT_SOURCE register
 *
 * only expected abort codes are listed here
 * refer to the datasheet for the full list
 */
#define ABRT_7B_ADDR_NOACK	0
#define ABRT_10ADDR1_NOACK	1
#define ABRT_10ADDR2_NOACK	2
#define ABRT_TXDATA_NOACK	3
#define ABRT_GCALL_NOACK	4
#define ABRT_GCALL_READ		5
#define ABRT_SBYTE_ACKDET	7
#define ABRT_SBYTE_NORSTRT	9
#define ABRT_10B_RD_NORSTRT	10
#define ABRT_MASTER_DIS		11
#define ARB_LOST		12

#define SF_IC_TX_ABRT_7B_ADDR_NOACK	(1UL << ABRT_7B_ADDR_NOACK)
#define SF_IC_TX_ABRT_10ADDR1_NOACK	(1UL << ABRT_10ADDR1_NOACK)
#define SF_IC_TX_ABRT_10ADDR2_NOACK	(1UL << ABRT_10ADDR2_NOACK)
#define SF_IC_TX_ABRT_TXDATA_NOACK	(1UL << ABRT_TXDATA_NOACK)
#define SF_IC_TX_ABRT_GCALL_NOACK	(1UL << ABRT_GCALL_NOACK)
#define SF_IC_TX_ABRT_GCALL_READ	(1UL << ABRT_GCALL_READ)
#define SF_IC_TX_ABRT_SBYTE_ACKDET	(1UL << ABRT_SBYTE_ACKDET)
#define SF_IC_TX_ABRT_SBYTE_NORSTRT	(1UL << ABRT_SBYTE_NORSTRT)
#define SF_IC_TX_ABRT_10B_RD_NORSTRT	(1UL << ABRT_10B_RD_NORSTRT)
#define SF_IC_TX_ABRT_MASTER_DIS	(1UL << ABRT_MASTER_DIS)
#define SF_IC_TX_ARB_LOST		(1UL << ARB_LOST)

#define SF_IC_TX_ABRT_NOACK		(SF_IC_TX_ABRT_7B_ADDR_NOACK | \
					 SF_IC_TX_ABRT_10ADDR1_NOACK | \
					 SF_IC_TX_ABRT_10ADDR2_NOACK | \
					 SF_IC_TX_ABRT_TXDATA_NOACK | \
					 SF_IC_TX_ABRT_GCALL_NOACK)
#define BYTES_ONE_INTR	(1)
#define SHM_SIZE 	 (10) //1 << 10 PAGE 

int dev_major = 256;
int dev_minor = 0;


u8 *shmem, *memp;
int msg_length = 0;
struct page *shm_page;

int test_open(struct inode*, struct file*);

int test_release(struct inode*,struct file*);

ssize_t test_read(struct file*,char *,size_t, loff_t *);

ssize_t test_write(struct file*,const char*,size_t, loff_t *);

int test_mmap (struct file*, struct vm_area_struct *);

long test_ioctl(struct file*,unsigned int, unsigned long);

struct file_operations test_fops ={
	owner: THIS_MODULE,
	open: test_open,
    release:test_release,
    read:  test_read,
	write:test_write,
	unlocked_ioctl:test_ioctl,
	mmap:  test_mmap,
};

struct test_dev{
	int     sym_var;
	struct   cdev    cdev;
};

struct test_dev  *test_dev;

//void i2c_print_reg(struct sf_i2c_dev *dev);


int test_open(struct inode*inode, struct file*filp)
{
	//printk("%s()is called.\n", __func__);

	shmem = memp;
	writeb(0x0, (u32 *)(I2C_BASE + SF_IC_ENABLE));
	writeb(0x1, (u32 *)(I2C_BASE + SF_IC_ENABLE));
	return 0;
}

int test_release(struct inode*inode, struct file*filp)
{
	//printk("%s()is called.\n", __func__);
	
	shmem = memp;
	return 0;
}

irqreturn_t i2c_sf_irq(int this_irq, void *dev_id)
{
	u16 tmp = 0;
	if(readw((u32 *)(I2C_BASE + SF_IC_RAW_INTR_STAT)) & SF_IC_INTR_RX_FULL){
		while(readw((u32 *)(I2C_BASE + SF_IC_STATUS)) & (1 << 3)){
			tmp = readw((u32 *)(I2C_BASE + SF_IC_DATA_CMD));		

			if(memp + (1 << SHM_SIZE)   * 1024 * 4 >  shmem){
				writeb((u8)tmp, shmem++);
				//	printk("write value is %x, waite address is %x\n", tmp, shmem - 1);
			}else{
				printk("over data area, reset data point!\n");
				shmem = memp;
			}
		}
	} else {
		/*stop interrupt, master has received enough data from device,transmit shouled be stopped*/
		//	printk("status is %x; abrt is %x\n",readw((u32 *)(I2C_BASE + SF_IC_RAW_INTR_STAT)),readw((u32 *)(I2C_BASE + SF_IC_TX_ABRT_SOURCE)));	

		/*rx  done interrupt*/
		if(readw((u32 *)(I2C_BASE + SF_IC_RAW_INTR_STAT)) & SF_IC_INTR_RX_DONE){
				/*clear read interrupt*/
				tmp = readw((u32 *)(I2C_BASE + SF_IC_CLR_RX_DONE));

		}

		// read interrupt
		if(readw((u32 *)(I2C_BASE + SF_IC_INTR_STAT)) & SF_IC_INTR_RD_REQ){
				//printk("read interrupt!\n");
				//printk("status is %x; abrt is %x\n",readw((u32 *)(I2C_BASE + SF_IC_RAW_INTR_STAT)),readw((u32 *)(I2C_BASE + SF_IC_TX_ABRT_SOURCE)));
				readw((u32 *)(I2C_BASE + SF_IC_CLR_RD_REQ));
			//	for(i = 0; i < BYTES_ONE_INTR; i++)
				while(readw((u32 *)(I2C_BASE + SF_IC_STATUS))& 1 << 1)
					writew(*(shmem++), (u32 *)(I2C_BASE + SF_IC_DATA_CMD));
		}


	}
	return IRQ_HANDLED;
}

ssize_t test_read(struct file*filp, char *buf, size_t len, loff_t *off)
{
	int ret = 0;
	ret = copy_to_user(buf, memp, len);
	if(ret)
		printk("copy data to user failed.\n");
	shmem = memp;
	return 0;
}

ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	int ret;
	/*
	ret =  copy_from_user(memp, buf, strlen(buf));
	printk("ret = %d : buf length = %d\n", ret, strlen(buf));
	msg_length += strlen(buf);
	return strlen(buf);
	*/
	extern int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
	extern int __i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
	typedef enum cmd_type_t{
		no_exception = 0x0,
		adapter_point_to_null=0x1,
		msgs_point_to_null = 0x2,
		msgs_num_negative = 0x4,
		msgs_num_unmatch_msgs_size=0x8
	 }cmd_type;
	char *strcmd, tmp;
	cmd_type cmd = no_exception;
	struct i2c_adapter *test_adapter, *test_adapter_normal;
	struct device_node *i2c_node;
	struct i2c_msg *test_msgs, *test_msgs_normal;

	i2c_node = of_find_node_by_name(NULL, "i2c");
	/*get  i2c-1 device node*/
	i2c_node = i2c_node->sibling;
	test_adapter_normal = of_find_i2c_adapter_by_node(i2c_node);
	put_device(&test_adapter_normal->dev);
		
	test_msgs_normal = (struct i2c_msg*)kmalloc(sizeof(struct i2c_msg), GFP_KERNEL);
	if(!test_msgs_normal)
		printk("i2c_msg : alloc memory failed\n");
	else{
		test_msgs_normal->flags = 0;
		test_msgs_normal->addr = 0x55;
		test_msgs_normal->len = 10;
		test_msgs_normal->buf = (unsigned char *)kmalloc(10 * sizeof(char), GFP_KERNEL);	
	}

	strcmd = (char *)kmalloc(len * sizeof(char), GFP_KERNEL);

	if(strcmd == NULL){
		printk("alloc buf failed\n");
		return -ENOMEM;
	}
	ret = copy_from_user(strcmd, buf, len);	


	if(strstr(strcmd, "adapter_point_to_null"))
		cmd = adapter_point_to_null;
	if(strstr(strcmd, "msgs_point_to_null"))
		cmd = msgs_point_to_null;
	if(strstr(strcmd, "msgs_num_negative"))
		cmd = msgs_num_negative;
	if(strstr(strcmd, "msgs_num_unmatch_msgs_size"))
		cmd = msgs_num_unmatch_msgs_size;

	switch(cmd){
		case adapter_point_to_null:
			printk("adapter_point_to_null test!\n");
			i2c_transfer(test_adapter, test_msgs_normal, 1);
			__i2c_transfer(test_adapter,test_msgs_normal, 1);
			break;
		case msgs_point_to_null:
			printk("msgs_point_to_null test!\n");
			i2c_transfer(test_adapter_normal, test_msgs, 1);
			__i2c_transfer(test_adapter_normal, test_msgs, 1);	
			break;
		case msgs_num_negative:
			printk("msgs_num_negative test!\n");
			i2c_transfer(test_adapter_normal, test_msgs_normal, -1);
			__i2c_transfer(test_adapter_normal, test_msgs_normal, -1);
			break;
		case msgs_num_unmatch_msgs_size:
			printk("msgs_num_unmatch_msgs_size test!\n");
			i2c_transfer(test_adapter_normal, test_msgs_normal, 2);
			__i2c_transfer(test_adapter_normal, test_msgs_normal, 2);
			break;
		case no_exception:
			printk("no_exception test!\n");
			i2c_transfer(test_adapter_normal, test_msgs_normal, 1);
			__i2c_transfer(test_adapter_normal, test_msgs_normal, 1);
			break;
	}
	return len;
}

long test_ioctl(struct file *filp, unsigned int op, unsigned long flag)
{
	printk("%s()is called.\n", __func__);

	return 0;
}

void test_vma_open(struct vm_area_struct *vma)
{
	printk("%s()is called.\n", __func__);
}


void test_vma_close(struct vm_area_struct *vma)
{
	printk("%s()is called.\n", __func__);
}


static struct vm_operations_struct test_remap_vm_ops = {
	.open =test_vma_open,
	.close =test_vma_close,
};

int test_mmap (struct file*filp, struct vm_area_struct *vma)
{
	printk("%s()is called.\n", __func__);
	if(remap_pfn_range(vma, vma->vm_start, page_to_pfn(shm_page),vma->vm_end -vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	vma->vm_ops =&test_remap_vm_ops;
	test_vma_open(vma);

	return 0;
}

int test_init(void)
{
	int ret,err;
	u32 addr;
	u32 addr_len;
	dev_t devno;
 	struct device_node *i2c_test_node;
	i2c_test_node = (struct device_node *)kmalloc(sizeof(struct device_node), GFP_KERNEL);
	i2c_test_node = of_find_compatible_node(NULL, NULL, "siflower,sfax8-i2c");
	printk("node name is %s and full name  is %s\n",i2c_test_node->name, i2c_test_node->full_name);
	/*
 	struct device_node *i2c_test_node;
	i2c_test_node = (struct device_node *)kmalloc(sizeof(struct device_node), GFP_KERNEL);
	if(i2c_test_node == NULL){
		printk("%s: alloc device node fail!\n", __func__);
		return -ENOMEM;
	}
	else{
		i2c_test_node = of_find_compatible_node(NULL, NULL, "siflower,i2c-test");
		printk(KERN_DEBUG "node name is %s and full name  is %s\n",i2c_test_node->name, i2c_test_node->full_name);
		if(of_property_read_u32(i2c_test_node, "addr-len", &addr_len)){
			printk("%s: can't get i2c test addr len!\n", __func__ ); 	
			return -ENODATA;
		}
		if(of_property_read_u32(i2c_test_node, "addr", &addr)){
			printk("%s: can't get i2c test addr!\n", __func__ ); 	
			return -ENODATA;
		} 
		printk("addr_len is %d , addr is %x\n", addr_len, addr);
	}
	*/
	devno =MKDEV(dev_major,dev_minor);
	addr_len = 10;
	addr = 0x155;
	ret= register_chrdev_region(devno, 1, "i2ctest");

	if(ret < 0)
	{
		printk("i2ctest register failure.\n");
		return ret;
	}
	else
		printk("i2ctest register successfully.\n");


	test_dev = kmalloc(sizeof(struct test_dev), GFP_KERNEL);

	if(!test_dev)
	{
		ret = -ENOMEM;
		printk("create device failed.\n");
	}
	else
	{
		test_dev->sym_var = 0;
		cdev_init(&test_dev->cdev, &test_fops);
		test_dev->cdev.owner = THIS_MODULE;
		err = cdev_add(&test_dev->cdev, devno, 1);

		if(err < 0)
			printk("Add device failure\n");
	}

	shm_page = alloc_pages(GFP_KERNEL, SHM_SIZE);
	if(shm_page == NULL)
		printk("alloc page failed!\n");
	shmem= page_address(shm_page);
	printk("alloc page ok! address is 0x%x\n", shmem);

	memp = shmem;

	//config i2c-2 as 10bit slave mode
	if(addr_len == 10)
		writew(0x28, (u32 *)(I2C_BASE + SF_IC_CON));	
	else
		writew(0x20, (u32 *)(I2C_BASE + SF_IC_CON));	
		

	//config slave addr
	writew((u16)addr, (u32 *)(I2C_BASE + SF_IC_SAR));

	writew(BYTES_ONE_INTR - 1, (u32 *)(I2C_BASE + SF_IC_RX_TL));	
	writew(BYTES_ONE_INTR - 1, (u16*)(I2C_BASE + SF_IC_TX_TL));	
	writew(I2C_MASK, (u32 *)(I2C_BASE + SF_IC_INTR_MASK));
	ret = request_irq(227, i2c_sf_irq, IRQF_SHARED, "i2c-test", &test_dev);		
	if(ret)
		printk("request i2c test irq failed! ret = %d\n", ret);

	writeb(0x1, (u32 *)(I2C_BASE + SF_IC_ENABLE));
	return ret;
}

int test_exit(void)
{
	return 0;
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
