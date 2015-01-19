#ifndef __KNX_PROTOCOL_H__
#define __KNX_PROTOCOL_H__

#include "knxCommon.h"

/* global variables */
extern uint16_t gTxInterval;

/* Macro */
#define VERSION_STR "0.0.2"

#define GET_OFFSET_RECV_C(b) (b + 0)
#define GET_OFFSET_RECV_HEADER(b) (b + 1)
#define GET_OFFSET_RECV_RF_INFO(b) (b + 2)
#define GET_OFFSET_RECV_KNX_CTRL(b) (b + 9)
//get the offset of TX serial number of Domain Address
#define GET_OFFSET_RECV_TXSN_OR_DA(b) (b + 3)
#define GET_OFFSET_RECV_APPL_HEADER(b) (b + 10)
#define GET_OFFSET_RECV_SRC(b) GET_OFFSET_RECV_APPL_HEADER(b)
#define GET_OFFSET_RECV_APPL_DATA (b + 17)

#define LENGTH_BEFORE_APPL_DATA 17
#define LENGTH_RSSI_INFO 1

#define RC1180_MAX_BUFFER_SIZE 64

/* KNX1 Timing Table 
 * Note:(KNX2 has all of the features of KNX1 inaddition to 
 * KNX RF Multi functions, and multi repeater)
 * The measure unit is micro-second (us)
 * */
#define TIME_RX_TXD 180
#define TIME_TXD	590
#define TIME_TXD_IDLE	900



/* network role enumeration */
enum {
    ROLE_TRANSMITTER = 0,
    ROLE_RECEIVER,
    ROLE_TRANSMITTER_RECEIVER,
} network_role;

/* functions */

/* transfer data type for baud rate */
static inline uint16_t transfer_wait_time(speed_t spd, int char_len)
{
	switch(spd)
	{
		case B9600:
			return (TIME_RX_TXD + (TIME_TXD * 3 * char_len) + TIME_TXD_IDLE);
			break;
		case B19200:
			return (TIME_RX_TXD + ((TIME_TXD + 70) * char_len) + TIME_TXD_IDLE);
			break;
		default:
			return (TIME_RX_TXD + (TIME_TXD * char_len) + TIME_TXD_IDLE);
			break;
	}
}

/* return actual user data length */
static inline uint16_t get_user_data_length(uint16_t total_len)
{
	if (total_len < 1)
		return 0;
	if (gEnableRssi)
		return total_len - LENGTH_BEFORE_APPL_DATA - LENGTH_RSSI_INFO;;
	return total_len - LENGTH_BEFORE_APPL_DATA;
}

#endif /* __KNX_PROTOCOL_H__ */
