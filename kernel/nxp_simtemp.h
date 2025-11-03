#define TEMP_STEP 1111111
#define ALERT_TEMP 50000000
#define RESET_TEMP 80000000
#define CERO_DEGREE 0
#define NEW_SAMPLE_MASK  0x00000001
#define TEMP_ALERT_MASK  0x00000002
#define TEMP_ALERT_CLEAR 0x00000001

static enum hrtimer_restart temp_timer_callback(struct hrtimer *timer);