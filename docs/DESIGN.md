Following file describes the desing of the driver according to following topics:
	- Architecture
	- API description
	- Variables


ARCHITECTURE
Driver module will initialize all the necessary structures to work, module is waiting for user space action to set the timer and start the process
of reading data. Module can work without any problem with a 10 ms time, less of that module stock in the poll.



|------------- User Aplication -------------|
|											|
|----- File Descriptor 						|
|---------- WRITE (Configuration timer)		|		
|---------- POLL  (Wait for new sample)     |
|---------- READ  (Read new sample)	        |
| 											|
|----- Display								|
|------------- Year, Month, Day				|
|------------- Hour, Minute, Sec, Milisec	|
|------------- Alert						|
|											|
|-------------------------------------------|



|------- Module Driver -------- API --------|
|											|
|----- Platform Driver	|					|
|----- Platform Device 	|					|
|----- Char Device		|	-- INIT			|		
|----- Class device    	|	   (Driver)		|
|----- Device create	|					|
|						|					|
|----- Create work_queue|					|
|----- Init hr timer    |   -- WRITE 		|
|----- Set callback		|	  (Call by user)|				
|						|					|
|----- Poll wait 		|   -- POLL 		|
|						|	  (Call by user)|
|						|					|
|----- Task to the queue|   -- CALLBACK		|
|----- Timer restart	|	   (Driver)		|
|						|					|
|----- Update data	    | 				    |
|----- Wake up			|	-- TASK IN QUEUE|
|----- Check threshold  |	   (Driver)		|
|						|					|
|----- Validate wake up |	-- READ			|
|----- Copy data		|	  (Call by user)|
|						|					|
|						|					|
|------ Cancel timer    |					|
|------ Finish queue    |					|
|------ Destroy queue   |					|
|------ Destroy device  |   -- EXIT			|
|------ Destroy class	|	   (Driver)		|
|------ Chardevice unreg|					|
|------ Platform device |					|
|------ Platform driver |					|
|-------------------------------------------|


|--------------  Headers Module --------------|
|     										  |
|------------- platform_device.h -------------|
|------------- hrtimer.h         -------------|
|------------- workqueue.h       -------------|
|------------- ktime.h           -------------|
|------------- poll.h 			 -------------|
|------------- module.h			 -------------|
|------------- kernel.h			 -------------|
|------------- init.h			 -------------|
|------------- types.h			 -------------|
|------------- of.h 			 -------------|
|---------------------------------------------|



API Description
This part is used to describe the functionality for each function in the driver.

FUNCTION    : static void temperature_update(struct work_struct *work)
DESCRIPTION : This function will be added in a queue work and executed in kernel process context, this function is used to simulate a read data for temperature sensor
			  and notify to poll that there is a new sample using wake_up_interruptible.
			  

FUNCTION	: static enum hrtimer_restart temp_timer_callback(struct hrtimer *timer)
DESCRIPTION : Callback function after timer is completed, this function is used to restart the timer and include 'temperature_update' function in the queue work 


FUNCTION	: static unsigned int simtemp_poll(struct file *file, poll_table *wait)
DESCRIPTION : User space uses this function to wait until new sample is available, if new sample is available user space can read them


FUNCTION	: static ssize_t simtemp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
DESCRIPTION : User space uses this function to get the data from kernel module (timestamp, temp, flags)


FUNCTION	: static ssize_t simtemp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
DESCRIPTION : User space uses this function to set timer time in seconds or milliseconds

FUNCTION	: static int simtemp_probe(struct platform_device *client) 
DESCRIPTION : This function is called only if the platform device is compatible with device ID, dynamic memory is assigned to handle device and data

FUNCTION	: static void simtemp_remove(struct platform_device *client) 
DESCRIPTION : This function is called when the module is removed, only display a message to know that the module was removed

FUNCTION	: static int __init simtemp_init(void)
DESCRIPTION : This functions is used to called : driver register, device register, char device register, class create, device create   

FUNCTION	: static void __exit simtemp_exit(void)
DESCRIPTION : This function is used to cancel the timer, run pending queue task, delete queue, delete device, delete class, unregister char device, unregister platform device and driver


Variables
List of variables using in the driver

__u64 time_sample
Variable use to set the value of the timer  (Configure by write function in space user)

__s32 temp_alert 						
Variable to set the threshold, this is value can not be configured, change in code to use a different value

static int chrdev_ret
Variable use only with the purpose to validate if the char device register was sucess 
 
static struct class *simtemp_class
Pointer to the class of the device (/sys/class/my_driver)

static struct device *simtemp_device
Pointr to the device associated to the previous class (/dev/my_driver)

static struct simtemp_drv *global_simtemp
Global pointer to a structure for the driver (data and platform device)

static struct platform_device *tmp_simtemp
Blobal pointer to platform device, only use because virtual box does not have a DTS to modify

static struct hrtimer temp_timer
High resolution timer to simulate temperature

static struct workqueue_struct *simtemp_queue
Pointer for the queue with all the task

static struct work_struct temperaure_work;
Struct to be queue in the above queue

bool temp_done
Variable to validate if temperatures was read
