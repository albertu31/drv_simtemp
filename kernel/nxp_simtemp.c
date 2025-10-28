#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/ktime.h>
#include <linux/poll.h>
#include "nxp_simtemp.h"


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

/* Time for each temperature sample */
__u64 time_sample;    
__s32 temp_alert = ALERT_TEMP;

/* Variables to create char device and file to manipulate it*/
static int chrdev_ret;
static struct class *simtemp_class;
static struct device *simtemp_device;

/*Global pointer for acess driver*/
static struct simtemp_drv *global_simtemp = NULL;

/* Temp variable to simulate a device, if DTS is avaibale is not necessary kernel will create it*/
static struct platform_device *tmp_simtemp = NULL;

/*High resolution timer to simulate temperature */
static struct hrtimer temp_timer;

/*Pointer for the queue with all the task*/
static struct workqueue_struct *simtemp_queue = NULL;

/*Struct to be queue */
static struct work_struct temperaure_work;

/*Variable to validate if temperatures was read*/
bool temp_done = true;

/* Init variables for poll*/
static DECLARE_WAIT_QUEUE_HEAD(simtemp_qwait);
static DEFINE_MUTEX(simtemp_lock);

/*Function to be run in the queue*/
static void temperature_update(struct work_struct *work)
{
    ktime_t time_read = ktime_get_real();

    /* Update data data for sensor */
    global_simtemp->data->temp_mC += (__s32)TEMP_STEP;
    global_simtemp->data->timestamp_ns = ktime_to_ns(time_read);
    global_simtemp->data->flags |= NEW_SAMPLE_MASK; 

    /* Wake poll function, new sample is ready*/
    wake_up_interruptible(&simtemp_qwait);

    /* Set flag if temperature is above 50° */
    if(temp_alert < global_simtemp->data->temp_mC)
    {
        global_simtemp->data->flags |= TEMP_ALERT_MASK; 

        /* Reset temperature to avoid burning something */
        if(RESET_TEMP < global_simtemp->data->temp_mC)
        {
            global_simtemp->data->temp_mC = (__s32)CERO_DEGREE;
            global_simtemp->data->flags &= TEMP_ALERT_CLEAR; 
        }
    }

    pr_info("Time: %llu \n", global_simtemp->data->timestamp_ns);
    pr_info("Temperature: %d\n", global_simtemp->data->temp_mC);
    pr_info("Flags: %d\n", global_simtemp->data->flags);

    temp_done = true;
}

/*Function call during interruption period, restart timer*/
static enum hrtimer_restart temp_timer_callback(struct hrtimer *timer)
{
    if(temp_done) 
    {
        /*Add temperature_work to the queue*/
        queue_work(simtemp_queue, &temperaure_work);
        temp_done = false;
    } 
    else 
    {
        pr_info("Temperature work pending to run");
    }

    /*Restart timer*/
    hrtimer_forward_now(timer, time_sample);
    return HRTIMER_RESTART;
}

static unsigned int simtemp_poll(struct file *file, poll_table *wait)
{
    unsigned int simtemp_mask = 0;

    /* Set qwait to the queue*/
    poll_wait(file, &simtemp_qwait, wait);

    /* Validate that data is available */
    if(global_simtemp->data->flags & NEW_SAMPLE_MASK)
    {
        simtemp_mask = POLLIN; 
    }

    return simtemp_mask;
}


static ssize_t simtemp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    /* Copy of data to be used on user space*/
    struct simtemp_sample sample_userspace;
    ssize_t count_data;

    /* Sleep function until new sample */
    if(wait_event_interruptible(simtemp_qwait, global_simtemp->data->flags & NEW_SAMPLE_MASK))
    {
        return -ERESTARTSYS;
    }
        
    /* Lock code for other users until read function detects a new sample*/    
    mutex_lock(&simtemp_lock);

    sample_userspace = *global_simtemp->data;

    /* Clear new sample flag */
    global_simtemp->data->flags &= ~NEW_SAMPLE_MASK;

    /* Unlock code for other user until read function detects a new sample*/    
    mutex_unlock(&simtemp_lock);

    /* Validate size of buf using count*/
    if(count < sizeof(sample_userspace))
    {
        return -EINVAL;
    }

    /* Copy data to user space */
    if(copy_to_user(buf, &sample_userspace, sizeof(sample_userspace)))
    {
        return -EFAULT;
    }

    count_data = sizeof(sample_userspace);

    return count_data;
}


static const struct file_operations simtemp_fops = {
    .owner = THIS_MODULE,
    .read = simtemp_read,
    .poll = simtemp_poll,
};

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

    global_simtemp->data = devm_kzalloc(&client->dev, sizeof(*global_simtemp->data), GFP_KERNEL);

    /*Handle error with Memory assigment*/
    if(!global_simtemp->data) 
    {
        dev_err(&client->dev, "Error Memory Assigment\n");
        return -ENOMEM;  // o el código de error apropiado
    }

    pr_info("Platform device added: %s\n", client->name);
        
    /*Create the queue list*/
    simtemp_queue = create_singlethread_workqueue("simtemp_queue");
    /*Assignt function to struct*/
    INIT_WORK(&temperaure_work, temperature_update);

    /*Assignt value to counter variable*/
    time_sample = ktime_set(0, INIT_TIME);
    /*Assignt kind of timer*/
    hrtimer_init(&temp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    /*Assignt callback function to timer*/
    temp_timer.function = temp_timer_callback;
    /*Start timer to count*/
    hrtimer_start(&temp_timer, time_sample, HRTIMER_MODE_REL);

    return 0; 

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

    /* Register a platform device to use probe,remove because  VirtualBox/QEMU can not use dts*/
    tmp_simtemp = platform_device_register_simple("simtemp", -1, NULL, 0);
    if(IS_ERR(tmp_simtemp)) 
    {
        pr_err("Device could not be registered\n");
        platform_driver_unregister(&simtemp_driver);
        return PTR_ERR(tmp_simtemp);
    }

    /* Register a char device to be used in user space*/
    chrdev_ret = register_chrdev(0, "simtemp", &simtemp_fops);
    if(chrdev_ret < 0) 
    {
        pr_err("Error during device char register\n");
        platform_device_unregister(tmp_simtemp);
        platform_driver_unregister(&simtemp_driver);
        return chrdev_ret;
    }

    /* Create class for device */
    simtemp_class = class_create("simtemp_class");
    if(IS_ERR(simtemp_class)) 
    {
        platform_device_unregister(tmp_simtemp);
        platform_driver_unregister(&simtemp_driver);
        unregister_chrdev(chrdev_ret, "simtemp");
        pr_err("Error to create the device \n");
        return PTR_ERR(simtemp_class);
    }

    /* Create node for /dev/simtemp */
    simtemp_device = device_create(simtemp_class, NULL, MKDEV(chrdev_ret, 0), NULL, "simtemp");
    if(IS_ERR(simtemp_device)) 
    {
        platform_device_unregister(tmp_simtemp);
        platform_driver_unregister(&simtemp_driver);
        unregister_chrdev(chrdev_ret, "simtemp");
        class_destroy(simtemp_class);
        pr_err("Error to create the device\n");
        return PTR_ERR(simtemp_device);
    }

    return 0;
}

/* Exit function: unregister both */
static void __exit simtemp_exit(void)
{
    /*Delete timer*/
    hrtimer_cancel(&temp_timer);
    /*Clear works in the queue*/
    flush_workqueue(simtemp_queue); 
    /*Delete queue*/
    destroy_workqueue(simtemp_queue);
    /*Delete device file*/
    device_destroy(simtemp_class, MKDEV(chrdev_ret, 0));
    /*Delete class of device*/
    class_destroy(simtemp_class);
    /*Delete char device*/
    unregister_chrdev(chrdev_ret, "simtemp");
    /*Delete device*/
    platform_device_unregister(tmp_simtemp);
    /*Delete driver*/
    platform_driver_unregister(&simtemp_driver);
}

module_init(simtemp_init);
module_exit(simtemp_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Rodriguez");
MODULE_DESCRIPTION("Platform driver to simulate temperature values");