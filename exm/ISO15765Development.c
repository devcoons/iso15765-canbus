#include <stdio.h>
#include <stdlib.h>
#include "iso15765_2.h"
#include <windows.h>
#include <sysinfoapi.h>

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

static uint8_t send_frame1(canbus_md mode, uint32_t id, uint8_t ctp_ft, uint8_t dlc, uint8_t* dt);
static uint8_t send_frame2(canbus_md mode, uint32_t id, uint8_t ctp_ft, uint8_t dlc, uint8_t* dt);
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



n_req_t frame1 =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x01,
        .n_ai.n_ta = 0x02,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .ctp_ft = CTP_T_FD,
      .msg = {0},
        .msg_sz = 0,
};

n_req_t frame2 =
{
        .n_ai.n_pr = 0x07,
        .n_ai.n_sa = 0x02,
        .n_ai.n_ta = 0x01,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .ctp_ft = CTP_T_STD,
        .msg = {0},
        .msg_sz = 0,
};

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

static void rand_string(char* str, size_t size)
{
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";

                for (size_t n = 0; n < size; n++) {
                        int key = rand() % (int)(sizeof charset - 1);
                        str[n] = charset[key];
                }

}

static uint32_t getms()
{
        return GetTickCount();
}

static uint8_t send_frame1(canbus_md mode, uint32_t id, uint8_t ctp_ft, uint8_t dlc, uint8_t* dt)
{
        printf("R:2 - %d  #1# - id:%x    dlc:0x%02x", GetTickCount(), id, dlc);
        if (dlc <= 2)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t\t\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else  if (dlc <= 5)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else if (dlc <= 7)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else if (dlc <= 8)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else
        {
                for (int s = 0; s < dlc; s+=8)
                {
                        printf("\n\t\t\t\t\t\t\t");
                        for (int i = s; i < s+8; i++)
                                printf("%02x ", dt[i]);
                        printf("\t\t");
                        for (int i = s; i < s + 8; i++)
                                printf("%c ", dt[i]);

                }
              
        }
        printf("\n");
        canbus_frame_t frame = { .id = id, .dlc = dlc, .mode = mode };
        memmove(frame.dt, dt, dlc);
        iso15765_enqueue(&handler2, &frame);
        return 0;
}


static uint8_t send_frame2(canbus_md mode, uint32_t id, uint8_t ctp_ft, uint8_t dlc, uint8_t* dt)
{
        
        printf("R:2 - %d  #1# - id:%x    dlc:0x%02x", GetTickCount(), id, dlc);
        if (dlc <= 2)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t\t\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        if (dlc <= 5)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else if (dlc <= 7)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else if (dlc <= 8)
        {
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%02x ", dt[i]);
                printf("\t\t");
                for (int i = 0; i < dlc; i++)
                        printf("%c ", dt[i]);
        }
        else
        {
                for (int s = 0; s < dlc; s += 8)
                {
                        printf("\n\t\t\t\t\t\t\t");
                        for (int i = s; i < s + 8; i++)
                                printf("%02x ", dt[i]);
                        printf("\t\t");
                        for (int i = s; i < s + 8; i++)
                                printf("%c ", dt[i]);

                }

        }
        printf("\n");
        
        canbus_frame_t frame = { .id = id, .dlc = dlc, .mode = mode };
        memmove(frame.dt, dt, dlc);
        iso15765_enqueue(&handler1, &frame);
        return 0;
}

static void on_error(n_rslt err_type)
{
        printf("ERROR OCCURED!:%04x", err_type);
}

static void indn1(n_indn_t* info)
{
        uint8_t v = 'X';
        uint8_t s = 'X';

        if (info->msg_sz == frame2.msg_sz)
                s = 'V';

        if (memcmp(info->msg, frame2.msg, info->msg_sz) == 0)
                v = 'V';

        printf("\n\n\n- Reception of H1. Msg_sz:[%d] | SZ_CH[%c] MSG_CH[%c]\n\n\n",info->msg_sz,s,v);
   
        if (v != 'V' || s != 'V')
        {
                printf("--------- ERROR -----------\n");
                Sleep(1000);
        }

        frame1.ctp_ft = (rand() % 1000) > 500 ? CTP_T_STD : CTP_T_FD;
        frame1.msg_sz = (rand() % 500);
        rand_string(frame1.msg, frame1.msg_sz);

        iso15765_send(&handler1, &frame1);
}
static void indn2(n_indn_t* info)
{
        uint8_t v = 'X';
        uint8_t s = 'X';

        if (info->msg_sz == frame1.msg_sz)
                s = 'V';

        if (memcmp(info->msg, frame1.msg, info->msg_sz) == 0)
                v = 'V';

        printf("\n\n\n- Reception of H2. Msg_sz:[%d] | SZ_CH[%c] MSG_CH[%c]\n\n\n", info->msg_sz, s, v);

        if (v != 'V' || s != 'V')
        {
                printf("--------- ERROR -----------\n");
                Sleep(1000);
        }

        frame2.ctp_ft = (rand() % 1000) > 500 ? CTP_T_STD : CTP_T_FD;
        frame2.msg_sz = (rand() % 500);
        rand_string(frame2.msg, frame2.msg_sz);

        iso15765_send(&handler2, &frame2);
}
/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

int main()
{
        iso15765_init(&handler1);
        iso15765_init(&handler2);

        frame1.ctp_ft = (rand() % 1000) > 500 ? CTP_T_STD : CTP_T_FD;
        frame1.msg_sz = (rand() % 500);
        rand_string(frame1.msg, frame1.msg_sz);

        iso15765_send(&handler1, &frame1);
        while (1)
        {
                iso15765_process(&handler1);
                iso15765_process(&handler2);
                Sleep(2);
        }
        return 0;
}

