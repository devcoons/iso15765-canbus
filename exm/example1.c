#include <stdio.h>
#include <stdlib.h>
#include "iso15765_2.h"
#include <windows.h>
#include <sysinfoapi.h>

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

static uint8_t send_frame1(canbus_md md, uint32_t id, uint8_t dlc, uint8_t* dt);
static uint8_t send_frame2(canbus_md md, uint32_t id, uint8_t dlc, uint8_t* dt);
static void indn1(n_indn_t* info);
static void indn2(n_indn_t* info);
static void on_error(n_rslt err_type);
static uint32_t getms();

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

static iso15765_t handler1 =
{
        .addr_md = N_ADM_NORMAL,
        .can_md = CANBUS_STANDARD,
        .clbs.send_frame = send_frame1,
        .clbs.on_error = on_error,
        .clbs.get_ms = getms,
        .clbs.indn = indn1,
        .config.stmin = 0x3,
        .config.bs = 0x0f,
        .config.n_bs = 100,
        .config.n_cr = 3
};

static iso15765_t handler2 =
{
        .addr_md = N_ADM_NORMAL,
        .can_md = CANBUS_STANDARD,
        .clbs.send_frame = send_frame2,
        .clbs.on_error = on_error,
        .clbs.indn = indn2,
        .clbs.get_ms = getms,
        .config.stmin = 0x3,
        .config.bs = 0x0f,
        .config.n_bs = 100,
        .config.n_cr = 3
};


n_req_t frame1_1 =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x01,
        .n_ai.n_ta = 0x02,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {'T','h','i','s',' ','i','s',' ','a',' ','l','o','n','g',' ','m','e','s','s','a','g','e',' ','f','r','o','m',' ','h','a','n','d','l','e','r',' ','1'},
        .msg_sz = 37,
};
n_req_t frame1_2 =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x01,
        .n_ai.n_ta = 0x02,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {'H','e','y',':','('},
        .msg_sz = 5,
};


n_req_t frame2_1 =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x02,
        .n_ai.n_ta = 0x01,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {'T','h','i','s',' ','i','s',' ','a',' ','m','u','c','h',' ','l','o','n','g','e','r',' ','m','e','s','s','a','g','e',' ','t','h','a','t',' ','h','a','n','d','l','e','r',' ','2',' ','s','e','n','d','s',' ','t','o',' ','h','a','n','d','l','e','r',' ','1'},
        .msg_sz = 63,
};
n_req_t frame2_2 =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x02,
        .n_ai.n_ta = 0x01,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {'H','e','y',':',')'},
        .msg_sz = 5,
};
/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

static uint32_t getms()
{
        return GetTickCount();
}

static uint8_t send_frame1(canbus_md mode, uint32_t id, uint8_t dlc, uint8_t* dt)
{
        printf("R:2 - %d  #1# - id:%x    dlc:%02x    ",GetTickCount(), id, dlc);
        for (int i = 0; i < 8; i++) 
                printf("%02x ", dt[i]);
        printf("\t\t");
        for (int i = 0; i < 8; i++) 
                printf("%c ", dt[i]);
        printf("\n");

        canbus_frame_t frame = { .id = id, .dlc = 8, .mode = mode };
        memmove(frame.dt, dt, 8);
        iso15765_enqueue(&handler2, &frame);
        return 0;
}


static uint8_t send_frame2(canbus_md mode, uint32_t id, uint8_t dlc, uint8_t* dt)
{
        printf("R:1 - %d  #1# - id:%x    dlc:%02x    ", GetTickCount(), id, dlc);
        for (int i = 0; i < 8; i++) 
                printf("%02x ", dt[i]);
        printf("\t\t");
        for (int i = 0; i < 8; i++) 
                printf("%c ", dt[i]);
        printf("\n");

        canbus_frame_t frame = { .id = id, .dlc = 8, .mode = mode };
        memmove(frame.dt, dt, 8);
        iso15765_enqueue(&handler1, &frame);
        return 0;
}

static void on_error(n_rslt err_type)
{
        printf("ERROR OCCURED!:%04x", err_type);
}

static void indn1(n_indn_t* info)
{
        printf("\n-- RCV Callback of 1st: ");
        for (int i = 0; i < info->msg_sz; i++) 
                printf("%c", info->msg[i]);
        printf("\nSending MSG to 1st.\n\n");

        if ((rand() % 1000) > 500)
                iso15765_send(&handler1, &frame1_1);
        else
                iso15765_send(&handler1, &frame1_2);
}
static void indn2(n_indn_t* info)
{
        printf("\n-- RCV Callback of 2nd: ");
        for (int i = 0; i < info->msg_sz; i++) 
                printf("%c", info->msg[i]);
        printf("\n-- Sending MSG to 1st.\n\n");

        if((rand()%1000) > 500)
                iso15765_send(&handler2, &frame2_1);
        else
                iso15765_send(&handler2, &frame2_2);
}
/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

int main()
{
        iso15765_init(&handler1);
        iso15765_init(&handler2);

        iso15765_send(&handler1, &frame1_1);
        while(1)
        {
                iso15765_process(&handler1);
                iso15765_process(&handler2);
                Sleep(5);
        }
        return 0;
}

