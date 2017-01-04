/* -----------------------------------------------------------
 * File name   : char_device_driver.c
 * Description : 
 * Author      : Vishwas Satish Patel
 * -----------------------------------------------------------
*/

/*
 * -----------------------------------------------------------
 * Include section
 * -----------------------------------------------------------
 */ 

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

/*
 * -----------------------------------------------------------
 * MACRO (define) section
 * -----------------------------------------------------------
 */
 
#define ramdisk_size (size_t)       100
#define CHGACCDIR                   1
#define DEVICE_NAME                 "mycdrv"

#define CDRV_IOC_MAGIC              'Z'
#define ASP_CHGACCDIR               _IOW(CDRV_IOC_MAGIC, 1, int)
#define CHGACCDIR                   1
/*
 * -----------------------------------------------------------
 * Type definition section
 * -----------------------------------------------------------
 */
 
/*
 * -----------------------------------------------------------
 * Global prototypes section
 * -----------------------------------------------------------
 */


/*
 * -----------------------------------------------------------
 * Local prototypes section
 * -----------------------------------------------------------
 */


/*
 * -----------------------------------------------------------
 * Global data section
 * -----------------------------------------------------------
 */
int     CURR_MODE = 0;
int     ret;
dev_t   dev_num, device_number;
int     major_no;
int     minor_no;
int     NO_OF_DEVICES = 3;
module_param(NO_OF_DEVICES, int, 0);

 /*
 * -----------------------------------------------------------
 * Local (static) data section
 * -----------------------------------------------------------
 */

struct asp_mycdrv
{
    struct  list_head list;
    struct  cdev dev;
    char    *ramdisk;
    struct  semaphore sem;
    int     devNo;
};

static struct class *cdevice_class = NULL;

static LIST_HEAD(mycdrv_list);

/*
 * -----------------------------------------------------------
 * Local (static) and inline functions section
 * -----------------------------------------------------------
 */
 
/*
 * -----------------------------------------------------------
 * Function     : device_open
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
int device_open(struct inode *inode, struct file *filp)
{
    struct asp_mycdrv *cdrv = container_of(inode->i_cdev, struct asp_mycdrv, dev);
    
	pr_info("Opening mycdrv_%d device\n", cdrv->devNo);
	filp->private_data = cdrv; 
	
    return 0;
}

/*
 * -----------------------------------------------------------
 * Function     : device_close
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
static int device_close(struct inode *inode, struct file *filp) 
{
	struct asp_mycdrv *cdrv = filp->private_data;
    
	pr_info("Closing mycdrv_%d device\n", cdrv->devNo);
    
	return 0;
}

/*
 * -----------------------------------------------------------
 * Function     : device_read
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
ssize_t device_read(struct file *filp, char *buffer, size_t buf_count, loff_t *curr_offset)
{
    struct  asp_mycdrv *cdrv = filp->private_data;
    int     no_of_bytes = 0;
    int     i = 0;

    if(down_interruptible(&cdrv->sem)!=0)
	{
		pr_info("Error: Could not lock the device during read\n");
		return -1;
	}
    //pr_info("READing function, buf_count=%d, *curr_offset=%d\n", buf_count, (int)*curr_offset);	
	if ((buf_count + *curr_offset) > ramdisk_size && CURR_MODE == 0) 
    {
		pr_info("Trying to read past end of device in regular direction, hence aborting\n");
        up(&cdrv->sem);
		return 0;
	}
	if ( ((long)(*curr_offset) - (long)buf_count) < 0 && CURR_MODE == 1)
    {
		pr_info("Trying to read past end of device in reverse direction, hence aborting\n");
        up(&cdrv->sem);
		return 0;
    }

    if (CURR_MODE == 0)
    {
	    pr_info("READing in regular direction\n");
	    no_of_bytes = buf_count - copy_to_user(buffer, cdrv->ramdisk + *curr_offset, buf_count);
	    *curr_offset += no_of_bytes;
    }
    else
    {
	    pr_info("READing in reverse direction\n");
        for(i = 0; i < buf_count; i++)
        {
	        copy_to_user(buffer + i, cdrv->ramdisk + *curr_offset - i, 1);
            no_of_bytes++;
        }
        *curr_offset -= no_of_bytes;        
    }
    
	//pr_info("READING function, no_of_bytes=%d, pos=%d\n", no_of_bytes, (int)*curr_offset);
    up(&cdrv->sem);
	return no_of_bytes;
}

/*
 * -----------------------------------------------------------
 * Function     : device_write
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
ssize_t device_write(struct file *filp, const char *buffer, size_t buf_count, loff_t *curr_offset)
{
    struct  asp_mycdrv *cdrv = filp->private_data;
    int     no_of_bytes = 0;
    int     i = 0;
    
    if(down_interruptible(&cdrv->sem)!=0)
	{
		pr_info("Error: Could not lock the device during write\n");
		return -1;
	}
	//pr_info("WRITING function, buf_count=%d, *curr_offset=%d\n", buf_count, (int)*curr_offset);
    
	if ((buf_count + *curr_offset) > ramdisk_size && CURR_MODE == 0) 
    {
		pr_info("Trying to write past end of device in regular direction, hence aborting\n");
	    up(&cdrv->sem);
		return 0;
	}
	
	if ( ((long)(*curr_offset) - (long)buf_count) < 0 && CURR_MODE == 1)
    {
        pr_info("Trying to write past end of device in reverse direction, hence aborting\n");
	    up(&cdrv->sem);
		return 0;
	}

    if (CURR_MODE == 0)
    {
        pr_info("WRITing in regular direction\n");
	    no_of_bytes = buf_count - copy_from_user(cdrv->ramdisk + *curr_offset, buffer, buf_count);
	    *curr_offset += no_of_bytes;
    }
    else
    {
        pr_info("WRITing in reverse direction\n");
        for(i = 0; i < buf_count; i++)
        {
	        copy_from_user(cdrv->ramdisk + *curr_offset - i, buffer + i, 1);
            no_of_bytes++;
        }
        *curr_offset -= no_of_bytes;
    }
        
	//pr_info("\n WRITING function, no_of_bytes=%d, pos=%d\n", no_of_bytes, (int)*curr_offset);
	up(&cdrv->sem);
	return no_of_bytes;
}

/*
 * -----------------------------------------------------------
 * Function     : device_lseek
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
loff_t device_lseek(struct file *file, loff_t offset, int orig)
{
    struct asp_mycdrv *cdrv = file->private_data;
    off_t testpos;

    if(down_interruptible(&cdrv->sem)!=0)
	{
		pr_info("Error: Could not lock the device during lseek\n");
		return -1;
	}
		
    switch (orig) 
    {
        case 0:     // SEEK_SET
            testpos = offset;
            pr_info( "testpos in SEEK_SET is: %d", (int)testpos);
            break;
        case 1:     // SEEK_CUR
            testpos = file->f_pos + offset;
            pr_info( "testpos in SEEK_CUR is: %d", (int)testpos);
            break;
        case 2:     // SEEK_END
            testpos = ramdisk_size + offset;
            pr_info( "testpos in SEEK_END is: %d", (int)testpos);
            break;
        default:
            return -EINVAL;
	}

	if (testpos < 0)
        testpos = 0;
    
    if (testpos > ramdisk_size)
        testpos = ramdisk_size;
	
    file->f_pos = testpos;
	
    pr_info("Seeking to position = %ld\n", (long)testpos);
	up(&cdrv->sem);
	return testpos;
}

/*
 * -----------------------------------------------------------
 * Function     : device_ioctl
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
    struct  asp_mycdrv *cdrv = file->private_data;
    int     dir, old_dir;

    if(down_interruptible(&cdrv->sem)!=0)
	{
		pr_info("Error: Could not lock the device during ioctl\n");
		return -1;
	}
   
    if (_IOC_TYPE(cmd) == CDRV_IOC_MAGIC  && _IOC_NR(cmd) == CHGACCDIR) 
    {
        dir = (int)arg;
       
        if(dir != 0 && dir != 1)
        {
            pr_info("Error: Invalid direction bit %d for ioctl of mycdrv_%d", dir, cdrv->devNo);
	        up(&cdrv->sem);
            return -1;
        }
        
        pr_info("The direction = %d",dir);
        
        old_dir = CURR_MODE;
        CURR_MODE = dir;
        up(&cdrv->sem);
        return old_dir;
   }
   return -1;
}

struct file_operations file_operations = 
{
    .owner          = THIS_MODULE,
    .open           = device_open,
    .release        = device_close,
    .read           = device_read,
    .write          = device_write,
    .llseek         = device_lseek,
    .unlocked_ioctl = device_ioctl,
};

/*
 * -----------------------------------------------------------
 * Function     : driver_entry
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
static int driver_entry(void)
{
    struct asp_mycdrv *temp;
    int i = 0;
    char device_name[20];

    ret = alloc_chrdev_region(&device_number, 0, NO_OF_DEVICES, DEVICE_NAME);
    if (ret < 0) 
    {
        pr_info( "Error : Failed to allocate major number for mycdrv\n");
        return ret;
    }

    major_no = MAJOR(device_number);
    minor_no = MINOR(device_number);

    if ((cdevice_class = class_create(THIS_MODULE, "class_char_drv")) == NULL)
    {
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    for (i = 0; i < NO_OF_DEVICES; i++)
    {
        sprintf(device_name, "%s_%d", DEVICE_NAME, i);
         
	    dev_num = MKDEV(major_no, minor_no + i);
        pr_info("mycdrv_%d: major : %d, minor = %d\n", i, major_no, minor_no + i);

        temp= (struct asp_mycdrv *)kmalloc(sizeof(struct asp_mycdrv), GFP_KERNEL);
        temp->ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
        
        cdev_init(&temp->dev, &file_operations);
        temp->dev.ops = &file_operations;
        temp->devNo = i;
    	temp->dev.owner = THIS_MODULE;

        ret = cdev_add(&temp->dev, dev_num, 1);
    	if(ret < 0)
        {
    		pr_info("Error: Failed to add cdev to kernel to mycdrv_%d", i);
        	return ret;
    	}
        INIT_LIST_HEAD(&temp->list);

        list_add(&(temp->list), &(mycdrv_list));
        sema_init(&temp->sem,1);
        
        if (device_create(cdevice_class, NULL, dev_num, NULL, device_name) == NULL)
        {
            class_destroy(cdevice_class);
            unregister_chrdev_region(dev_num, 1);
            return -1;
        }
    }
    
	return 0;
}

/*
 * -----------------------------------------------------------
 * Function     : driver_exit
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */ 
static void driver_exit(void)
{
    struct list_head *pos, *q;
    struct asp_mycdrv *temp;

	list_for_each_safe(pos, q, &mycdrv_list)
	{
        temp= list_entry(pos, struct asp_mycdrv, list);
        list_del(pos);
        cdev_del(&temp->dev);
		dev_num = temp->dev.dev;                
        kfree(temp->ramdisk);
		device_destroy(cdevice_class, dev_num);
        kfree(temp);
        pr_info( "Unloading the device mycdrv_%d\n", MINOR(dev_num));
	}
    unregister_chrdev_region(device_number, NO_OF_DEVICES);
    class_destroy(cdevice_class);
}


module_init(driver_entry);
module_exit(driver_exit);

MODULE_LICENSE("GPL v2");

/*
 * -----------------------------------------------------------
 * End of File
 * -----------------------------------------------------------
 */
