/*!
@file   iso15765_2.h
@brief  Header file of the ISO15765-2 library
@t.odo	-
---------------------------------------------------------------------------

MIT License
Copyright (c) 2020 Io. D (Devcoons.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/

#ifndef DEVCOONS_ISO15765_2_H_
#define DEVCOONS_ISO15765_2_H_

#define I15765_MSG_SIZE		512	/* Max. size of the TP up to 4095 bytes */
#define I15765_QUEUE_ELMS	64	/* No. of max incoming frames that the
					 * reception buffer can hold */

/******************************************************************************
* Includes
******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include "iqueue.h"

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

/* --- CANTP Addressing Mode (ref: iso15765-2 p.28) ------------------------ */

typedef enum
{
	N_ADM_UNKN	= 0x00,		/* Not defined */
	N_ADM_NORMAL	= 0x14,		/* Normal Mode */
	N_ADM_FIXED     = 0x28,		/* Extended Mode */
	N_ADM_MIXED11	= 0x35,		/* Mixed 11bits ID Mode */
	N_ADM_EXTENDED  = 0x45,		/* Fixed Mode */
	N_ADM_MIXED29	= 0x59		/* Mixed 29bits ID Mode */
}addr_md;

/* --- Protocol Control Information Type (ref: iso15765-2 p.17) ------------ */

typedef enum
{
	N_PCI_T_SF	= 0x00U,	/* Single Frame */
	N_PCI_T_FF	= 0x01U,	/* First Frame */
	N_PCI_T_CF	= 0x02U,	/* Consecutive Frame */
	N_PCI_T_FC	= 0x03U,	/* Flow Control Frame */
	N_PCI_T_UN	= 0xFF		/* Unknown */
}pci_type;

/* --- Network Target Address Type (ref: iso15765-2 p.9,29,30) ------------- */

typedef enum
{
	N_TA_T_PHY	= 0x01,		/* Physical */
	N_TA_T_FUNC	= 0x02		/* Functional */
}ta_type;

/* --- Message Type (ref: iso15765-2 p.8) ---------------------------------- */

typedef enum
{
	N_MT_DIAG	= 0x00,		/* Diagnostics */
	N_MT_RDIAG	= 0x01		/* Remote Diagnostics */
}mtype;

/* --- FlowControl Parameters (ref: iso15765-2 p.--------------------------- */

typedef enum
{
	N_ST_MIN	= 0x00U,	/* SeparationTimeMin: the minimum time the sender should
					 * wait between the transmissions of two CF N_PDUs */
	N_BS		= 0x01U,	/* BlockSize: the max. number of N_PDUs the receiver allows 
					 * the sender to send, before waiting for an authorization 
					 * to continue transmission of the following N_PDUs */
}fl_param;

/* --- N rslts (ref: iso15765-2 p.10-11) ----------------------------------- */

typedef enum
{
	N_OK		= 0x0000,	/* service execution has completed successfully */
	N_TIMEOUT_A	= 0x0001,	/* when the timer N_Ar/N_As has passed its time-out
					 * value N_Asmax / N_Armax; */
	N_TIMEOUT_Bs	= 0x0002,	/* when the timer N_Bs has passed its time-out value
					 * N_Bsmax */
	N_TIMEOUT_Cr	= 0x0004,	/* when the timer N_Cr has passed its time-out value
					 * N_Crmax */
	N_WRG_SN	= 0x0008,	/* upon reception of an unexpected sequence number
					 * (PCI.SN) value */
	N_INV_FS	= 0x0010,	/* when an invalid or unknown FlowStatus value has
					 * been received in a flow control(FC) N_PDU */
	N_UNE_PDU	= 0x0020,	/* upon reception of an unexpected protocol data unit; */
	N_WFT_OVRN	= 0x0040,	/* upon reception of flow control WAIT frame that
					 * exceeds the maximum counter N_WFTmax */
	N_BUFFER_OVFLW	= 0x0080,	/* upon reception of a flow control (FC) N_PDU with
					 * FlowStatus = OVFLW */
	N_ERROR		= 0x0100,	/* when an error has been detected by the network layer
					 * and no other parameter value can be used to better 
					 * describe the error. */
	N_IDLE		= 0x0110,	/* service execution has completed successfully and not TXRX */
	N_TX_BUSY	= 0x0120,	/* service execution has completed successfully and TX pending */
	N_RX_BUSY	= 0x0140,	/* service execution has completed successfully and RX pending */
	N_WRG_PARAM	= 0x1003,	/* the service did not execute due to an undefined <Parameter> */
	N_WRG_VALUE	= 0x1004,	/* the service did not execute due to an out of range <Parameter_Value>; */
	N_INV		= 0x1005,	/* the service did not execute due to an invalid requested operation */
	N_MISSING_CLB	= 0x1006,	/* the service did not execute due to missing mandatory callback functions */
	N_NULL		= 0xFFFF,	/* the service has a null pointer (this should never happen) */
	N_FC_TIMEOUT	= 0x1001,	/* Flow Control Timeout. @n_timeouts.n_bs */
	N_CF_TIMEOUT	= 0x1002,	/* ConsecutiveFrame Timeout. @n_timeouts.n_cr */
	N_CAN_DT_INV	= 0x1003,	/* Invalid CAN Data Error */
	N_UNE_FC	= 0x1004,	/* FlowControl was received without being expected. */
	N_UNE_CF	= 0x1005,	/* ConsecutiveFrame was received without being expected. */
	N_RCV_INT_SF	= 0x1006,	/* Frames sequence was not correct. Service received a 
					 * single frame during a Multi-frame reception. */
	N_RCV_INT_FF	= 0x1007,	/* Frames sequence was not correct. Service received a 
					 * first frame during a Multi-frame reception. */
	N_RCV_INT_FC	= 0x1012,	/* ReceptionInterruptedWithFC */
	N_INV_SEQ_NUM	= 0x1008,	/* ConsecutiveFrame sequence number was invalid. */
	N_UNS_WF	= 0x1009,	/* Unsuported WaitFrame */
	N_MAX_WF_REAC	= 0x100A,	/* Maximum Wait Frames Reached */
	N_INV_PDU	= 0x1010,	/* PDU Conversion failed */
	N_INV_REQ_SZ	= 0x010E,	
	N_UNE_FC_STS	= 0x1011,	/* Flow control status was invalid and/or unexpected. */
	N_OVFLW		= 0x100F,	/* Buffer Overflow. */
}n_rslt;

/* --- N_PCI_T_FC Status (ref: iso15765-2 p.8) ---------------------------- */

typedef enum
{
	N_CONTINUE	= 0x00U,	/* continue to send, the authorization to continue */
	N_WAIT		= 0x01U,	/* the request to continue to wait */
	N_OVERFLOW	= 0x02U		/* buffer overflow, the indication that the number of bytes specified 
					 * in the FirstFrame of the segmented message exceeds the number of 
					 * bytes that can be stored in the buffer of the receiver entity */
}flow_sts;

/* --- CANBus Mode (ref: iso15765-2 p.8) ----------------------------------- */

typedef enum
{
	CANBUS_STANDARD	= 0x04U,	/* 11bits CAN Identifier */
	CANBUS_EXTENDED	= 0x08U		/* 29bits CAN Identifier */
}canbus_md;

/* --- dt I/O Stream Status (ref: iso15765-2 p.8) ------------------------ */

typedef enum
{
	N_S_IDLE	= 0x00,		/* Has to pending action */
	N_S_RX_BUSY	= 0x02,		/* Reception is in progress */
	N_S_TX_BUSY	= 0x04,		/* Transmission is in progress */
	N_S_TX_READY	= 0x05,		/* Transmission is ready to begin */
	N_S_TX_WAIT_FC	= 0x10,		/* Transmission is in progress and waits 
					 * for a Flow control frame */
}stream_sts;

/* --- Network Layer Timing PARAMs (ref: iso15765-2 p.) -------------------- */

typedef struct
{
	uint32_t n_bs;		/* FlowControl N_PDU not received (lost, overwritten) on the
				 * sender side or preceding FirstFrame N_PDU or	ConsecutiveFrame 
				 * N_PDU not received(lost, overwritten) on the receiver side. 
				 * Abort message transmissionand issue N_USData.confirm with 
				 * <N_Result> = N_TIMEOUT_Bs. */
	uint32_t n_cr;		/* ConsecutiveFrame N_PDU not received (lost, overwritten) on
				 * the receiver side or preceding FC N_PDU not received(lost,
				 * overwritten) on the sender side. Abort message reception and issue
				 * N_USData.indication with <N_Result> = N_TIMEOUT_Cr */
	uint32_t n_cs;
}n_timeouts;

/* --- CANBus Frame (ref: iso15765-2 p.) ----------------------------------- */

typedef struct
{
	uint32_t id;		/* CAN Frame ID */
	uint32_t mode;		/* CAN Frame mode `canbus_md` */
	uint16_t dlc;		/* Size of data */
	uint8_t dt[8];		/* Actual data of the frame */
}canbus_frame_t;

/* --- Address information (ref: iso15765-2 p.) ---------------------------- */

typedef struct
{
	uint8_t n_pr;		/* Network Address Priority */
	uint8_t n_sa;		/* Network Source Address */
	uint8_t n_ta;		/* Network Target Address */
	uint8_t n_ae;		/* Network Address Extension */
	ta_type n_tt;		/* Network Target Address type */
}n_ai_t;

/* --- Protocol control information (ref: iso15765-2 p.) ------------------- */

typedef struct
{
	uint8_t fs;		/* FlowStatus: whether the sending network entity can 
				 * proceed with the message transmission 'flow_sts' */
	uint8_t bs;		/* BlockSize */
	uint8_t sn;		/* SequenceNumber: specify the order of the consecutive frames */
	uint8_t st;		/* SeparationTime: Requested separation time */
	pci_type pt;		/* Type of the received pdu 'pci_type' */
	uint16_t dl;		/* Frame data length (if 0 frame must be ignored) */
}n_pci_t;

/* --- Protocol dt unit (ref: iso15765-2 p.) ------------------------------- */

typedef struct
{
	mtype n_mt;		/* Message Type */
	n_ai_t n_ai;		/* Address information */
	n_pci_t n_pci;		/* Protocol control information */
	uint8_t dt[8];		/* Data Field */
	uint8_t sz;		/* Actual data size */
} n_pdu_t;



/* --- N_USdt.cfm (ref: iso15765-2 p.6) ------------------------------------ */

typedef struct
{
	n_ai_t n_ai;		/* Address information */
	n_pci_t n_pci;		/* Protocol control information */
	n_rslt rslt;		/* Result of the request */
}n_cfm_t;

/* --- N_FF.indn (ref: iso15765-2 p.6) ------------------------------------ */

typedef struct
{
	n_ai_t n_ai;		/* Address information */
	n_pci_t n_pci;		/* Protocol control information */
	uint16_t msg_sz;	/* Size of the message that will be received */
}n_ff_indn_t;

/* --- N_USData.request (ref: iso15765-2 p6.) ------------------------------ */

typedef struct
{
	n_ai_t n_ai;			/* Address information */
	n_pci_t n_pci;			/* Protocol control information */
	uint16_t msg_sz;		/* Message actual size */
	uint8_t msg[I15765_MSG_SIZE];	/* Message to be transmitted */
}n_req_t;

/* --- N_USdt.indn (ref: iso15765-2 p.7) ---------------------------------- */

typedef struct
{
	n_ai_t n_ai;			/* Address information */
	n_pci_t n_pci;			/* Protocol control information */
	n_rslt rslt;			/* Result of the reception */
	uint16_t msg_sz;		/* Received message actual size */
	uint8_t msg[I15765_MSG_SIZE];	/* Received message data */
}n_indn_t;

typedef struct
{
	n_ai_t n_ai;			/* Address information. Not supported: 
					 * the lib handles only 1 stream */
	n_pci_t n_pci;			/* Protocol control information. Not supported:
					 * the lib handles only 1 stream) */
	fl_param param;			/* Parameter to change */
	uint8_t pval;			/* Value to set */
}n_chg_param_req_t;

typedef struct
{
	n_ai_t n_ai;			/* Address information. Not supported: 
					 * the lib handles only 1 stream */
	n_pci_t n_pci;			/* Protocol control information. Not 
					 * supported: because the lib handles only 1 stream */
	fl_param param;			/* Requested Parameter to change */
	uint8_t pval;			/* Requested Value to set */
	n_rslt rslt;			/* Result of the request */
}n_chg_param_cfm_t;


typedef enum
{
	N_INDN		= 0x01,		/* N_USData.indication */
	N_FF_INDN	= 0x02,		/* N_USData_FF.indication */
	N_CONF		= 0x03,		/* N_USData.confirm */
	N_CHG_P_CONF	= 0x04		/* N_ChangeParameter.confirm */
}signal_tp;

/* --- Callbacks  ---------------------------------------------------------- */

typedef struct
{
	void (*indn)			(n_indn_t*);		/* Indication Callback: Will be fired when a reception
								 * is available or an error occured during the reception. */
	void (*ff_indn)			(n_ff_indn_t*);		/* First Frame Indication Callback: Will be fired when a 
								 * FF is received, giving back some useful information */
	void (*cfm)			(n_cfm_t*);		/* This callback confirms to the higher layers that the requested 
								 * service has been carried out */
	void (*cfg_cfm)			(n_chg_param_cfm_t*);	/* This service confirms to the upper layer that the request to 
								 * change a specific protocol has been carried out */
	void (*pdu_custom_pack)		(n_pdu_t*, uint32_t*);	/* Custom CAN ID packing for 11bits ID. If assinged the default 
								 * packing will be skipped */
	void (*pdu_custom_unpack)	(n_pdu_t*, uint32_t*);	/* Custom CAN ID uppacking for 11bits ID. If assinged the default 
								 * uppacking will be skipped */
	void (*on_error)		(n_rslt);		/* Will be fired in any occured error. */
	uint32_t(*get_ms)();					/* Time-source for the library in ms(required) */
	uint8_t(*send_frame)		(addr_md, uint32_t,	/* Callback to assing the Network Layer. This callback */
					 uint8_t, uint8_t*);	/* will be fired when a transmission of a canbus frame is ready. */
}n_callbacks_t;

/* --- PDU Stream  --------------------------------------------------------- */

typedef struct
{
	n_pdu_t pdu;			/* Keep information about the in/out frame in a PDU format */
	uint8_t cf_cnt;			/* Current block sequence number (ConsecutiveFrame) */
	uint8_t wf_cnt;			/* Current received wait flow control frames */
	uint8_t cfg_wf;			/* Max supported Wait Flow Control frames */
	uint8_t stmin;			/* Frames transmission rate */
	uint8_t cfg_bs;			/* Max. supported block sequence (ConsecutiveFrame) */
	stream_sts sts;			/* Stream status */
	uint16_t msg_sz;		/* Actual message buffer size */
	uint16_t msg_pos;		/* Transmit message buffer position */
	n_timeouts last_upd;		/* Time keeper for timouts */
	uint8_t msg[I15765_MSG_SIZE];	/* Received/Transmit message buffer */
}n_iostream_t;

/* --- iso15765 timing configuration (ref: iso15765-2 p.25)----------------- */

typedef struct
{
	uint8_t stmin;			/* Default min. frame transmission separation */
	uint8_t bs;			/* Max. Block size during transmission */
	uint8_t wf;			/* Max. accepted Wait Requests from the FlowControl */
	uint16_t n_bs;			/* Time until reception of the next FlowControl N_PDU */
	uint16_t n_cr;			/* Time until reception of the next ConsecutiveFrame N_PDU */
}n_config_t;

/* --- iso15765 Handler  --------------------------------------------------- */

typedef struct
{
	addr_md addr_md;		/* Selected address mode of the TP */
	canbus_md can_md;		/* CANBus frame type */
	n_iostream_t in;		/* Incoming data stream (reception) */
	n_iostream_t out;		/* Outcoming data stream (transmission) */
	n_pdu_t fl_pdu;			/* Flow control pdu */
	n_callbacks_t clbs;		/* Callbacks */
	n_config_t config;		/* Default configuration to be used. (timing etc) */	
	n_timeouts cfg_timeout;		/* Timeouts configuration */
	iqueue_t inqueue;		/* Queue handler for the incoming canbus frames */
	uint8_t inq_buf[I15765_QUEUE_ELMS * sizeof(canbus_frame_t)]; /* Queue buffer */
}iso15765_t;

/******************************************************************************
* Declaration | Public Functions
******************************************************************************/

n_rslt iso15765_init(iso15765_t* instance);

n_rslt iso15765_send(iso15765_t* instance,n_req_t* frame);

n_rslt iso15765_enqueue(iso15765_t* instance, canbus_frame_t* frame);

n_rslt iso15765_process(iso15765_t* instance);

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif