

#define TEMP_ALERT_MASK  0x00000002
#define MICROC_TO_C      1000000U
#define NANOSECOND_TO_SECONDS 1000000000UL
#define NANOSECOND_TO_MILISECOND 1000000UL
#define NANOSECOND_TO_MICROSECOND 1000UL


struct SimTempSample {
    unsigned long timestamp_ns;
    int temp_mC;
    unsigned int flags;
};
