//#define ISO16765Example
#ifdef ISO16765Example

#include <stdio.h>
#include <stdlib.h>
#include "iso15765_2.h"
#include <windows.h>
#include <sysinfoapi.h>
#include <conio.h>

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

static uint8_t send_frame1(cbus_id_type id_type, uint32_t id, cbus_fr_format fr_fmt, uint8_t dlc, uint8_t* dt);
static uint8_t send_frame2(cbus_id_type id_type, uint32_t id, cbus_fr_format fr_fmt, uint8_t dlc, uint8_t* dt);
static void indn1(n_indn_t* info);
static void indn2(n_indn_t* info);
static void on_error(n_rslt err_type);
static uint32_t getms();

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

static iso15765_t handler1 =
{
        .addr_md = N_ADM_FIXED,
        .fr_id_type = CBUS_ID_T_EXTENDED,
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
        .addr_md = N_ADM_FIXED,
        .fr_id_type = CBUS_ID_T_EXTENDED,
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
        .fr_fmt = CBUS_FR_FRM_FD,
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
        .fr_fmt = CBUS_FR_FRM_STD,
        .msg = {0},
        .msg_sz = 0,
};

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/
uint16_t f_sz1 = 1;
uint16_t f_sz2 = 1;

static void rand_string(char* str, size_t size)
{
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";

        for (size_t n = 0; n < size; n++) 
        {
                int key = rand() % ((int)sizeof(charset) - 1);
                str[n] = charset[key];
        }

}
static uint32_t getms()
{
        return GetTickCount();
}

static void print_frame(uint8_t instance,uint8_t tp_mode, cbus_id_type mode, uint32_t id, uint8_t ctp_ft, uint8_t dlc, uint8_t* dt)
{
        printf(" (%d) - %d | TpMode: [%02x] | FrameType: [%2d]   IdType: [%d]   Id: [%8x]   DLC: [%02d]\t\t", instance, GetTickCount(), tp_mode, mode, ctp_ft, id, dlc);

        for (int s = 0; s < (int)dlc; s += 8)
        {
                int elements = (int)dlc - s > 8 ? 8 : (int)dlc - s;

                for (int i = s; i < (s+8); i++)
		{
                        printf((i< (s+elements))? "%02x " : "   ", dt[i]);
		}
                printf("\t");
                if ((s+elements) == (int)dlc)
		{
                        printf("\n");
		}
                else
		{
                        printf("\n     |\t\t\t\t\t\t\t\t\t\t\t\t\t");
		}
        }

}

static uint8_t send_frame1(cbus_id_type id_type, uint32_t id, cbus_fr_format fr_fmt, uint8_t dlc, uint8_t* dt)
{
        print_frame(1,handler2.addr_md, id_type,  id, fr_fmt,  dlc,  dt);

        canbus_frame_t frame = { .id = id, .dlc = dlc, .id_type = id_type, .fr_format= fr_fmt };
        memmove(frame.dt, dt, dlc);
        iso15765_enqueue(&handler2, &frame);
        return 0;
}


static uint8_t send_frame2(cbus_id_type id_type, uint32_t id, cbus_fr_format fr_fmt, uint8_t dlc, uint8_t* dt)
{     
        print_frame(2, handler1.addr_md, id_type, id, fr_fmt, dlc, dt);
        
        canbus_frame_t frame = { .id = id, .dlc = dlc, .id_type = id_type,  .fr_format = fr_fmt };
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
	{
                s = 'V';
	}
	
        if (memcmp(info->msg, frame2.msg, info->msg_sz) == 0)
	{
                v = 'V';
	}
	
        printf("- Reception of H1. Msg_sz:[%d] | SZ_CH[%c] MSG_CH[%c]\n",info->msg_sz,s,v);
   
        if ((v != 'V') || (s != 'V'))
        {
                printf("--------- ERROR -----------\n");
                Sleep(1000);
        }
        printf("- END OF TRANSMISSION/RECEPTION -\n\n\n");

        frame1.msg_sz = f_sz1;
        rand_string(frame1.msg, frame1.msg_sz);
        f_sz1 = f_sz1 == 512 ? 0 : f_sz1 + 1;
        iso15765_send(&handler1, &frame1);
}
static void indn2(n_indn_t* info)
{
        uint8_t v = 'X';
        uint8_t s = 'X';

        if (info->msg_sz == frame1.msg_sz)
	{
                s = 'V';
	}
	
        if (memcmp(info->msg, frame1.msg, info->msg_sz) == 0)
	{
                v = 'V';
	}
	
        printf("- Reception of H2. Msg_sz:[%d] | SZ_CH[%c] MSG_CH[%c]\n", info->msg_sz, s, v);
       
        if ((v != 'V') || (s != 'V'))
        {
                printf("--------- ERROR -----------\n");
                Sleep(1000);
        }
        printf("- END OF TRANSMISSION/RECEPTION -\n\n\n");
	
        frame2.msg_sz = f_sz2;
        rand_string(frame2.msg, frame2.msg_sz);
        f_sz2 = f_sz2 == 512 ? 0 : f_sz2 + 1;
        iso15765_send(&handler2, &frame2);
}
/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

int main()
{
        
        iso15765_init(&handler1);
        iso15765_init(&handler2);

        frame1.msg_sz = f_sz1;
        rand_string(frame1.msg, frame1.msg_sz);
	
        f_sz1++;

        iso15765_send(&handler1, &frame1);
        while (1)
        {
                iso15765_process(&handler1);
                iso15765_process(&handler2);
                Sleep(5);
        }
        return 0;
}
#endif
