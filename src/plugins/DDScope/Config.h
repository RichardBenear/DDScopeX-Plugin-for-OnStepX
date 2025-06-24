// Config.h


#define ODRIVE_COMM_MODE           OD_CAN  // Use OD_UART or OD_CAN...I2C may be added later
#define ODRIVE_SWAP_AXES               ON  // ODrive axis 0 = OnStep axis 2 = ALT
                                           // ODrive axis 1 = OnStep axis 1 = AZM
#define ODRIVE_SERIAL             Serial3  // Teensy HW serial
#define ODRIVE_SERIAL_BAUD         115200  // 19200 default
#define ODRIVE_SLEW_DIRECT            OFF  // ON=using ODrive trapezoidal move profile. OFF=using OnStep move profile
#define ODRIVE_ABSOLUTE                ON  // using absolute encoder
#define ODRIVE_SYNC_LIMIT              80  // in arc seconds..one encoder tick
                                           // encoder resolution=2^14=16380; 16380/360=45.5 ticks/deg 
                                           // 45.5/60=0.7583 ticks/min; 0.7583/60 = .00126 ticks/sec
                                           // or 1/0.7583 = 1.32 arc-min/tick;  1.32*60 sec = 79.2 arc sec per encoder tick
#define ODRIVE_UPDATE_MS              200  // 5 HZ position update rate