

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/trusty/smcall.h>
#include <linux/trusty/sm_err.h>
#include <linux/trusty/trusty.h>
#include <asm/compiler.h>

static struct platform_device *trusty_test_pdev;


#ifdef CONFIG_ARM64
#define SMC_ARG0		"x0"
#define SMC_ARG1		"x1"
#define SMC_ARG2		"x2"
#define SMC_ARG3		"x3"
#define SMC_ARCH_EXTENSION	""
#define SMC_REGISTERS_TRASHED	"x4","x5","x6","x7","x8","x9","x10","x11", \
				"x12","x13","x14","x15","x16","x17"
#else
#define SMC_ARG0		"r0"
#define SMC_ARG1		"r1"
#define SMC_ARG2		"r2"
#define SMC_ARG3		"r3"
#define SMC_ARCH_EXTENSION	".arch_extension sec\n"
#define SMC_REGISTERS_TRASHED	"ip"
#endif
static inline ulong smc(ulong r0, ulong r1, ulong r2, ulong r3)
{
	register ulong _r0 asm(SMC_ARG0) = r0;
	register ulong _r1 asm(SMC_ARG1) = r1;
	register ulong _r2 asm(SMC_ARG2) = r2;
	register ulong _r3 asm(SMC_ARG3) = r3;

	asm volatile(
		__asmeq("%0", SMC_ARG0)
		__asmeq("%1", SMC_ARG1)
		__asmeq("%2", SMC_ARG2)
		__asmeq("%3", SMC_ARG3)
		__asmeq("%4", SMC_ARG0)
		__asmeq("%5", SMC_ARG1)
		__asmeq("%6", SMC_ARG2)
		__asmeq("%7", SMC_ARG3)
		SMC_ARCH_EXTENSION
		"smc	#0"	/* switch to secure world */
		: "=r" (_r0), "=r" (_r1), "=r" (_r2), "=r" (_r3)
		: "r" (_r0), "r" (_r1), "r" (_r2), "r" (_r3)
		: SMC_REGISTERS_TRASHED);
	return _r0;
}

static ssize_t trusty_test_show(struct device *d, struct device_attribute *attr, char *buf)
{
    int ret;
    int version_str_len;
    char buf[64];
    int i;
    
    printk(KERN_EMERG "trusty_test: trusty_test_show!\n");

    ret = smc(SMC_FC_GET_VERSION_STR, -1, 0, 0);
    version_str_len = ret;
    
    for(i = 0; i < version_str_len; i++)
    {
        ret = smc(SMC_FC_GET_VERSION_STR, i, 0, 0);
        buf[i] = ret;
    }
    
    printf("get API Version: %s\n", buf);

	return 0;
}

static ssize_t trusty_test_store(struct device *d, struct device_attribute *attr,const char *buf, size_t count)
{
    printk(KERN_EMERG "trusty_test: trusty_test_store!\n");   
	return 1;
}

static DEVICE_ATTR_RW(trusty_test);

static int trsty_test_driver_probe(struct platform_device *pdev)
{
    int ret;
    printk(KERN_EMERG "trusty_test: trsty_test_driver_probe!\n");  
    
	ret = device_create_file(&pdev->dev, &dev_attr_trusty_test);
    
    return 0;
}

static struct platform_driver trsty_test_driver = {  
    .probe      = trsty_test_driver_probe,  
    .driver     = {  
        .name       = "trusty_test",  
        .owner      = THIS_MODULE,  
    },  
};  

static int __init trusty_test_init(void)
{

    printk(KERN_EMERG "trusty_test: trusty_test_init!\n"); 
    trusty_test_pdev = platform_device_alloc("trusty_test", -1);
    platform_device_add(trusty_test_pdev);
    
    
    platform_driver_register(&trsty_test_driver);

    
    return 0;
}


static void __exit trusty_test_exit(void)
{
    printk(KERN_EMERG "trusty_test: trusty_test_exit!\n"); 
    platform_driver_unregister(&trsty_test_driver);
    platform_device_unregister(trusty_test_pdev);
}


module_init(trusty_test_init);
module_exit(trusty_test_exit);
MODULE_LICENSE("GPL");

