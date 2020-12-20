# ISO15765-2 CANBus TP 
![C/C++ CI](https://github.com/devcoons/iso15765-canbus/workflows/C/C++%20CI/badge.svg)  [![Codacy Badge](https://app.codacy.com/project/badge/Grade/5a80fc004df744e888729e512eec1fda)](https://www.codacy.com/manual/devcoons/iso15765-canbus?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=devcoons/iso15765-canbus&amp;utm_campaign=Badge_Grade)

*Compiler flags: **-O3 -Wfatal-errors -Wall -std=c11***

An implementation of the **ISO15765-2 (ISO-TP)** protocol in a platform agnostic C library. The library interacts in a transparent way with the lower ISO layers which means that the user must define the connection for the reception and the transmission of the CANBus frames. In this way you have a complete control and reusability of this library in different platforms. This library can support **UDS**, **OBDII** and any other application layer protocol that requires ISO15765-2 TP.

>This ISO defines common requirements for vehicle diagnostic systems implemented on a Controller Area Network (CAN) communication link, as specified in ISO 11898.
>Although primarily intended for diagnostic systems, it also meets requirements from other CAN-based systems needing a network layer protocol.

## How to use

-  Include the header file `iso15765_2.h`
-  Define an `iso15765_t` handler, create and attach the required shim/callbacks (`send_frame` `on_error` `get_ms`) of the handler
```C
/* Required shim/callback functions */
static uint8_t send_frame(canbus_md md, uint32_t id, uint8_t dlc, uint8_t* data);
static void usdata_indication(n_indn_t* info);
static void on_error(n_rslt err_type);
static uint32_t getms();

/* ISOTP handler */
static iso15765_t handler =
{
	.addr_md = N_ADM_FIXED,		// Selected address mode of the TP
	.can_md = CANBUS_EXTENDED,	// CANBus frame type
	.config.stmin = 0x05,		// Default min. frame transmission separation
	.config.bs = 0x0F,		// Maximun size of the block sequence
	.config.n_bs = 800,		// Time until reception of the next FlowControl N_PDU
 	.config.n_cr = 250,		// Time until reception of the next ConsecutiveFrame N_PDU
	.clbs.get_ms = getms,		// Time-source for the library in ms(required)
	.clbs.on_error = on_error,	// Callback which will be executed in any occured error.
	.clbs.send_frame = send_frame,	// This callback will be fired when a transmission of a canbus frame is ready.
	.clbs.indn = usdata_indication	// Indication Callback: Will be fired when a reception
					// is available or an error occured during the reception.
};

static uint8_t send_frame(canbus_md mode, uint32_t id, uint8_t dlc, uint8_t* data)
{
    // Here transmit the frame through CANBus
    return 0;
}

static void usdata_indication(n_indn_t* info)
{
    // Use the incoming message. This function should be fired by the library when a new complete message arrives.
}

static void on_error(n_rslt err_type)
{
    // Check the errors etc..
}

static uint32_t getms()
{
    // return time in ms; 
    // ex. return GetTickCount();
}
```

-  In your main, first initialize the iso15765_t handler `iso15765_init(&handler);`

-  To send a message, create a `n_req_t` and use the function `iso15765_send`. For example:

```C
n_req_t frame =
{
    .n_ai.n_pr = 0x06,		// Network Address Priority
    .n_ai.n_sa = 0x01,		// Network Source Address
    .n_ai.n_ta = 0x02,		// Network Target Address
    .n_ai.n_ae = 0x00,		// Network Address Extension
    .n_ai.n_tt = N_TA_T_PHY,	// Network Target Address type
    .msg = {1,2,3,4,5,6,7,8,	// Message
    	    9,10,11,12,13,14,
	    15,16,17,18,19,20},
    .msg_sz = 20,		// Message size
};
...
iso15765_send(&handler, &frame);
```
-  To push an incoming frame to the library use the function `iso15765_enqueue(&handler, &frame);`. It is suggested to put this function inside the frame reception callback of your interface
-  Use the `iso15765_process(&handler);` to allow the library to process the in/out streams of data. Normally you could put this function in a thread to run continuously.
-  As described before, any new/completed incoming message should be handled in the callback `static void usdata_indication(indn_t* info)`

Below is a **complete loopback example**. The service send a message to itself by enqueing the transmitted frame in the inbound stream.

```C
#include <stdio.h>
#include <stdlib.h>
#include "iso15765_2.h"
#include <windows.h>
#include <sysinfoapi.h>

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

static uint8_t send_frame(canbus_md md, uint32_t id, uint8_t dlc, uint8_t* dt);
static void on_error(n_rslt err_type);
static uint32_t getms();

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

static iso15765_t handler =
{
        .addr_md = N_ADM_NORMAL,
        .can_md = CANBUS_STANDARD,
        .clbs.send_frame = send_frame,
        .clbs.on_error = on_error,
        .clbs.get_ms = getms,
        .config.stmin = 0x3,
        .config.bs = 0x0f,
        .config.n_bs = 100,
        .config.n_cr = 3
};

n_req_t frame =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x01,
        .n_ai.n_ta = 0x02,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23},
        .msg_sz = 23,
};

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

static uint32_t getms()
{
        return GetTickCount();
}

static uint8_t send_frame(canbus_md mode, uint32_t id, uint8_t dlc, uint8_t* dt)
{
        printf("%d  #1# - id:%x    dlc:%02x    ",GetTickCount(), id, dlc);
        for (int i = 0; i < 8; i++) printf("%02x ", dt[i]);
        printf("\n");
        canbus_frame_t frame = { .id = id, .dlc = 8, .mode = mode };
        memmove(frame.dt, dt, 8);
        iso15765_enqueue(&handler, &frame);
        return 0;
}

static void on_error(n_rslt err_type)
{
        printf("ERROR OCCURED!:%d", err_type);
}

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

int main()
{
        iso15765_init(&handler);

        iso15765_send(&handler, &frame);
        while(1)
        {
                iso15765_process(&handler);
                Sleep(5);
        }
        return 0;
}
```

Please check the folder **`exm`** for more examples

## Development

This library is experimental and is still under development. The purpose is to create a complete ISO15765 library with all the described features. Feel free to suggest anything. If you use this library please ref.

## Contributing
We would love you to contribute to `iso15765-canbus`, pull requests are welcome!

## Support

Support me maintain this project https://paypal.me/iikem

## License
This project is released under the MIT License
