#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/ktime.h>
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

__u64 time_sample;    // Time for each temperature sample

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

/*Function to be run in the queue*/
static void temperature_read(struct work_struct *work)
{
    ktime_t time_read = ktime_get();

    /* Update data data for sensor */
    global_simtemp->data->temp_mC += (__s32)TEMP_STEP;
    global_simtemp->data->timestamp_ns = ktime_to_ns(time_read);
    global_simtemp->data->flags |= NEW_SAMPLE_MASK; 

    /* Set flag if temperature is above 50° */
    if(ALERT_TEMP < global_simtemp->data->temp_mC)
    {
        global_simtemp->data->flags |= TEMP_ALERT_MASK; 

        /* Reset temperature to avoid burning*/
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
    INIT_WORK(&temperaure_work, temperature_read);

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
    /*Delete timer*/
    int ret = hrtimer_cancel(&temp_timer);
    pr_info("hrtimer cancelado (%d)\n", ret);
    /*Clear works in the queue*/
    flush_workqueue(simtemp_queue); 
    /*Delete queue*/
    destroy_workqueue(simtemp_queue);

    pr_info("Module removed\n");
    platform_device_unregister(tmp_simtemp);
    platform_driver_unregister(&simtemp_driver);
}

module_init(simtemp_init);
module_exit(simtemp_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Rodriguez");
MODULE_DESCRIPTION("Platform driver to simulate temperature values");