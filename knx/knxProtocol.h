#ifndef __KNX_PROTOCOL_H__
#define __KNX_PROTOCOL_H__

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

enum {
    ROLE_TRANSMITTER = 0,
    ROLE_RECEIVER,
    ROLE_TRANSMITTER_RECEIVER,
} network_role;


#endif /* __KNX_PROTOCOL_H__ */
