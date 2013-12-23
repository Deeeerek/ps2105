#ifndef __TG_PS2000
#define __TG_PS2000

#define BUFFER_SIZE 	      1024
#define BUFFER_SIZE_STREAMING 100000
#define MAX_CHANNELS          4

typedef enum {
    MODEL_NONE = 0,
    MODEL_PS2104 = 2104,
    MODEL_PS2105 = 2105,
    MODEL_PS2202 = 2202,
    MODEL_PS2203 = 2203,
    MODEL_PS2204 = 2204,
    MODEL_PS2205 = 2205
} MODEL_TYPE;

typedef struct
{
    PS2000_THRESHOLD_DIRECTION  channelA;
    PS2000_THRESHOLD_DIRECTION  channelB;
    PS2000_THRESHOLD_DIRECTION  channelC;
    PS2000_THRESHOLD_DIRECTION  channelD;
    PS2000_THRESHOLD_DIRECTION  ext;
} DIRECTIONS;

typedef struct
{
    PS2000_PWQ_CONDITIONS       *conditions;
    short                       nConditions;
    PS2000_THRESHOLD_DIRECTION  direction;
    unsigned long               lower;
    unsigned long               upper;
    PS2000_PULSE_WIDTH_TYPE     type;
} PULSE_WIDTH_QUALIFIER;


typedef struct
{
    PS2000_CHANNEL channel;
    float threshold;
    short direction;
    float delay;
} SIMPLE;

typedef struct
{
    short hysterisis;
    DIRECTIONS directions;
    short nProperties;
    PS2000_TRIGGER_CONDITIONS * conditions;
    PS2000_TRIGGER_CHANNEL_PROPERTIES * channelProperties;
    PULSE_WIDTH_QUALIFIER pwq;
    unsigned long totalSamples;
    short autoStop;
    short triggered;
} ADVANCED;


typedef struct 
{
    SIMPLE simple;
    ADVANCED advanced;
} TRIGGER_CHANNEL;

typedef struct {
    short DCcoupled;
    short range;
    short enabled;
    short values [BUFFER_SIZE];
} CHANNEL_SETTINGS;

typedef struct  {
    short handle;
    MODEL_TYPE model;
    PS2000_RANGE firstRange;
    PS2000_RANGE lastRange; 
    TRIGGER_CHANNEL trigger;
    short maxTimebase;
    short timebases;
    short noOfChannels;
    CHANNEL_SETTINGS channelSettings[PS2000_MAX_CHANNELS];
    short               hasAdvancedTriggering;
    short               hasFastStreaming;
    short               hasEts;
    short               hasSignalGenerator;
} UNIT_MODEL; 

#endif

