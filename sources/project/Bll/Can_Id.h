/**
 * @file Can_Id.h
 * @brief contains CAN bus Ids for different devices
 * @author Mykhaylo Shcherbak
 * @em mikl74@yahoo.com
 * @date 19-03-2017
 * @version 1.00
 */

#ifndef SOURCE_BLL_CAN_ID_H_
#define SOURCE_BLL_CAN_ID_H_
/**
 * @brief Number of bits to shift device group. The last device number (0xF) is broadcast to all group.
 * For example display data on the second device is (1<<4) | 2
 */
#define CAN_ID_SHIFT 4
/**
 * @brief Mask of device in class. Also ID of broadcast
 */
#define DEVICE_IN_CLASS_ID_MASK ((1<<CAN_ID_SHIFT) - 1)
/**
 * @brief Can ID's for device functions
 */
typedef enum
{
	DGI_BROADCAST = 0,//!< Unused now
	DGI_DISPLAY   = 1, //!< Display group (all 7 seg displays)
	DGI_VOLTAGE   = 64 //!< Sends voltage. Byte 0 is DGI_, byte 1 is device number, byte 2 and 3 are voltage in mV
} CAN_Device_Group_Id_t;

/**
 * @brief Commands (byte 0)
 */
typedef enum
{
	CMD_DISPLAY_SEGMENTS 	= 0,   //!< Bytes 1 and 2 contain bitmasks
	CMD_SET_BRIGHTNESS  	= 1,   //!< Set display brightness
	CMD_DISPLAY_2DIGIT 		= 2,   //!< Displays 2 digits from the first data byte and 2 DPs from the second one
	CMD_BLINKING_MASK 		= 3,   //!< Displays 2 masks blinking. [0..1] Mask1, [2..3] Mask2, [4] On time in 100ms tics for mask 1, [5] The same for mask 2
	CMD_SET_TIMEOUT   		= 4	   //!< Sets timeout for turning off [0..1] Timeout in seconds
} Commands_t;

#endif /* SOURCE_BLL_CAN_ID_H_ */
