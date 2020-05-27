#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define AUTO_DEV_NODE	1
 
#define GLOBALMEM_SIZE 0X1000 /*ȫ���ڴ����4KB*/
#define MEM_CLEAR 0x1 /*����ȫ���ڴ�*/
#define GLOBALMEM_MAJOR 220
 
static int globalmem_major = GLOBALMEM_MAJOR;/*Ԥ���globalmem�����豸��*/

#if AUTO_DEV_NODE == 1
static struct class *pclass;
#endif

static dev_t devno;



/*globalmem���豸�ṹ�壺�����˶�Ӧ��globalmem�ַ��豸��cdev �� ʹ���ڴ�mem[GLOBALMEM_SIZE]*/
struct globalmem_dev
{
 struct cdev cdev;  //cdev�ṹ��
 unsigned char mem[GLOBALMEM_SIZE];  //ȫ���ڴ�
};
 
struct globalmem_dev *globalmem_devp;  //�豸�ṹ��ָ��
 
/*�ļ��򿪺���*/
int globalmem_open(struct inode *inode,struct file *filp)
{
 filp->private_data = globalmem_devp; //���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ��
 return 0;
}
 
/*�ļ��ͷź���*/
int globalmem_release(struct inode *inode,struct file *filp)
{
 return 0;
}
 
/*�豸���ƺ�����ioctl()�������ܵ�MEM_CLEAR���������ȫ���ڴ����Ч���ݳ������㣬�����豸��֧�ֵ����ioctl()����Ӧ�÷���-EINVAL*/
static long globalmen_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
 struct globalmem_dev *dev = filp->private_data; //����豸�ṹ��ָ��
 switch(cmd)
 {
  case  MEM_CLEAR:
   memset(dev->mem,0,GLOBALMEM_SIZE);
   printk(KERN_INFO"globalmem is set to zero\n");
   break;
  default:
   return -EINVAL;
 }
 return 0;
}
/*����������д������Ҫ�����豸�ṹ���mem[]�������û��ռ佻�����ݣ������ŷ����ֽ�����������û����ļ���дƫ��λ��*/
static ssize_t globalmem_read(struct file *filp,char __user *buf,size_t size,loff_t *opps)
{
 unsigned long p = *opps;
 unsigned int count = size;
 int ret = 0;
 struct globalmem_dev *dev = filp->private_data; //����豸�ṹ��ָ��
 
 if(p >= GLOBALMEM_SIZE)  //�����ͻ�ȡ��Ч��д����
 {
  return count ? -ENXIO:0;
 }
 if(count > GLOBALMEM_SIZE - p)
 {
  count = GLOBALMEM_SIZE - p;
 }
 
 if(copy_to_user(buf,(void *)(dev->mem+p),count))  //�ں˿ռ�->�û��ռ�
 {
  ret = -EFAULT;
 }
 else
 {
  *opps += count;
  ret = count;
  printk(KERN_INFO"read %d bytes(s) from %ld\n",count,p);
 }
 return ret;
}
/*д����*/
static ssize_t globalmem_write(struct file *filp,const char __user *buf,size_t size,loff_t *ppos)
{
 unsigned long p = *ppos;
 unsigned int count = size;
 int ret=0;
 
 struct globalmem_dev *dev = filp->private_data;
 
 if(p >= GLOBALMEM_SIZE)  //�����ͻ�ȡ��Ч��д����
 {
  return count? -ENXIO:0;
 }
 
 if(count > GLOBALMEM_SIZE - p)
 {
  count = GLOBALMEM_SIZE - p;
 }
 
 if(copy_from_user(dev->mem + p,buf,count)) // �û��ռ�->�ں˿ռ�
 {
  ret = -EFAULT;
 }
 else
 {
  *ppos =+ count;
  ret = count;
  printk(KERN_INFO"written %d bytes(s) from %ld\n",count,p);
 }
 return ret;
}
/*seek�ļ���λ������seek()�������ļ���λ����ʼ��ַ�������ļ���ͷ(SEEK_SET,0)����ǰλ��(SEEK_CUR,1)���ļ�β(SEEK_END,2)*/
static loff_t globalmem_llseek(struct file *filp,loff_t offset,int orig)
{
 loff_t ret = 0;
 printk(KERN_NOTICE"seek in offset: %d, orig: %d\n",offset,orig);
 switch(orig)
 {
  case 0:   //����ļ���ʼλ��ƫ��
   if(offset <0 )
   {
    ret = -EINVAL;
    break;
   }
   
   if((unsigned int )offset > GLOBALMEM_SIZE)
   {
    ret = - EINVAL;
    break;
   }
   filp->f_pos = (unsigned int)offset;
   ret = filp->f_pos;
   break;
   
  case 1:   //����ļ���ǰλ��ƫ��
   if((filp->f_pos + offset) > GLOBALMEM_SIZE)
   {
    ret = -EINVAL;
    break;
   }
   if((filp->f_pos + offset)<0)
   {
    ret = -EINVAL;
    break;
   }
   filp->f_pos +=offset;
   ret = filp->f_pos;
   break;
  default:
   ret = -EINVAL;
   break; 
 }
 return ret;
}
/*�ļ������ṹ��*/
static const struct file_operations globalmem_fops=
{
 .owner = THIS_MODULE,
 .llseek = globalmem_llseek,
 .read = globalmem_read,
 .write = globalmem_write,
 .unlocked_ioctl = globalmen_ioctl,
 .open = globalmem_open,
 .release = globalmem_release,
};
 
/*��ʼ����ע��cdev*/
static void globalmem_setup_cdev(struct globalmem_dev *dev,int index)
{
 int err;
 devno = MKDEV(globalmem_major,index);
 cdev_init(&dev->cdev,&globalmem_fops);
 dev->cdev.owner = THIS_MODULE;
 //dev->cdev.ops = &globalmem_fops;
 err = cdev_add(&dev->cdev,devno,1);
 if(err)
 {
  printk(KERN_NOTICE"Error %d adding LED%d",err,index);
 }
 #if AUTO_DEV_NODE == 1
 pclass = class_create(THIS_MODULE,"globalmem");
 if(!pclass)
 {
	printk(KERN_NOTICE"Error create class failed\n");
 }
 device_create(pclass,NULL,devno,NULL,"globalmem");
 #endif
}
/*�豸����ģ����غ���*/
static int __init globalmem_init(void)
{
 int result;
 devno = MKDEV(globalmem_major,0);
 
 if(globalmem_major) //�����豸��
 	result = register_chrdev_region(devno,1,"globalmem");
 else  //��̬�����豸��
 {
  result = alloc_chrdev_region(&devno,0,1,"globalmem");
  globalmem_major = MAJOR(devno);
 }
 
 if(result < 0)
 {
  return result;
 }
 
 globalmem_devp = kmalloc(sizeof(struct globalmem_dev),GFP_KERNEL); //�����豸�ṹ����ڴ�
 if(!globalmem_devp)
 {
  result = -ENOMEM;
  goto fail_malloc;
 }
 
 memset(globalmem_devp,0,sizeof(struct globalmem_dev));
 globalmem_setup_cdev(globalmem_devp,0);
 return 0;
 
 fail_malloc:unregister_chrdev_region(devno,1);
 return result;
}
module_init(globalmem_init);


/*ģ��ж�غ���*/
static void __exit globalmem_exit(void)
{
	unregister_chrdev_region(devno,1); //�ͷ��豸��

	cdev_del(&globalmem_devp->cdev); //ע��cdev.

	kfree(globalmem_devp);          //�ͷ��豸�ṹ���ڴ�
#if AUTO_DEV_NODE == 1
	device_destroy(pclass,devno);

	class_destroy(pclass);
#endif
}
module_exit(globalmem_exit); 

MODULE_AUTHOR("NOODLE");
MODULE_LICENSE("GPL");

