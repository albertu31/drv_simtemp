#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/platform_device.h>

/*Driver struct*/
struct simtemp_sample {
    __u64 timestamp_ns;   // monotonic timestamp
    __s32 temp_mC;        // milli-degree Celsius (e.g., 44123 = 44.123 °C)
    __u32 flags;          // bit0=NEW_SAMPLE, bit1=THRESHOLD_CROSSED (extend as needed)
} __attribute__((packed));


/*Driver struct definition*/
struct simtemp_drv{
    struct platform_device *client;
    struct simtemp_sample *data;
};

/*Global pointer for acess driver*/
static struct simtemp_drv *global_simtemp = NULL;

/* Temp variable to simulate a device, if DTS is avaibale is not necessary kernel will create it*/
static struct platform_device *tmp_simtemp;


/*Probe function*/
static int simtemp_probe(struct platform_device *client) 
{
    global_simtemp = devm_kzalloc(&client->dev, sizeof(*global_simtemp), GFP_KERNEL);

    /*Handle error with Memory assigment*/
    if(!global_simtemp) 
    {
        dev_err(&client->dev, "Error Memory Assigment\n");
        return -ENOMEM;  // o el código de error apropiado
    }
    else
    {
        pr_info("Platform device added: %s\n", client->name);
        return 0; 
    }
}

/*Remove function*/
static void simtemp_remove(struct platform_device *client) 
{
    /*Not necessary free memory because devm_kzalloc is used*/
    pr_info("Platform device removed: %s\n", client->name);
}

/*Compatibility table name, necessary only if DTS is used*/
static const struct of_device_id simtemp_of_match[] = {
    { .compatible = "nxp,simtemp" },
    { }
};
MODULE_DEVICE_TABLE(of, simtemp_of_match);

static struct platform_driver simtemp_driver = {
    .probe = simtemp_probe,
    .remove = simtemp_remove,
    .driver = {
        .name = "simtemp",
        .of_match_table = simtemp_of_match, /*Necessary only if DTS is used*/
        .owner = THIS_MODULE,
    },
};


/* VirtualBox/QEMU can not modify DTS, register driver manually*/
static int __init simtemp_init(void)
{
    int ret;

    pr_info("Driver Module Initialitation\n");

    /* Register platform driver */
    ret = platform_driver_register(&simtemp_driver);
    if(ret) 
    {
        pr_err("Driver could not be registered\n");
        return ret;
    }
    else
    {
        /* Register a platform device to use probe,remove because  VirtualBox/QEMU can not use dts*/
        tmp_simtemp = platform_device_register_simple("simtemp", -1, NULL, 0);
        if(IS_ERR(tmp_simtemp)) 
        {
            pr_err("Device could not be registered\n");
            platform_driver_unregister(&simtemp_driver);
            return PTR_ERR(tmp_simtemp);
        }
    }

    return 0;
}

/* Exit function: unregister both */
static void __exit simtemp_exit(void)
{
    pr_info("Module removed\n");
    platform_device_unregister(tmp_simtemp);
    platform_driver_unregister(&simtemp_driver);
}

module_init(simtemp_init);
module_exit(simtemp_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Rodriguez");
MODULE_DESCRIPTION("Platform driver to simulate temperature values");

