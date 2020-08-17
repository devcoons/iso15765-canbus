#include "examples.h"



static uint8_t send_frame1(canbus_md md, uint32_t id, uint8_t dlc, uint8_t* dt);
static uint8_t send_frame2(canbus_md md, uint32_t id, uint8_t dlc, uint8_t* dt);
static void indn1(n_indn_t* info);
static void indn2(n_indn_t* info);

static test_t test =
{
        .type = 0,
        .recv = 0,
        .handler1 =
        {
                .addr_md = N_ADM_MIXED11,
                .can_md = CANBUS_STANDARD,
                .clbs.send_frame = send_frame1,
                .clbs.on_error = on_error,
                .clbs.get_ms = getms,
                .clbs.indn = indn1,
                .clbs.ff_indn = NULL,
                .clbs.cfm = NULL,
                .clbs.cfg_cfm = NULL,
                .clbs.pdu_custom_pack = NULL,
                .clbs.pdu_custom_unpack = NULL,
                .config.stmin = 0x3,
                .config.bs = 0x0f,
                .config.n_bs = 0,
                .config.n_cr = 3
        },
        .handler2 =
        {
                .addr_md = N_ADM_MIXED11,
                .can_md = CANBUS_STANDARD,
                .clbs.send_frame = send_frame2,
                .clbs.on_error = on_error,
                .clbs.indn = indn2,
                .clbs.ff_indn = NULL,
                .clbs.cfm = NULL,
                .clbs.cfg_cfm = NULL,
                .clbs.pdu_custom_pack = NULL,
                .clbs.pdu_custom_unpack = NULL,
                .clbs.get_ms = getms,
                .config.stmin = 0x3,
                .config.bs = 0x0f,
                .config.n_bs = 0,
                .config.n_cr = 3,
        }
};

static n_req_t frame_sf =
{
        .n_ai.n_pr = 0x06,
        .n_ai.n_sa = 0x01,
        .n_ai.n_ta = 0x02,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {'H','e','y',':',')'},
        .msg_sz = 5,
};

static n_req_t frame_mf =
{
        .n_ai.n_pr = 0x06,
        .n_ai.n_sa = 0x01,
        .n_ai.n_ta = 0x02,
        .n_ai.n_ae = 0x00,
        .n_ai.n_tt = N_TA_T_PHY,
        .msg = {'T','h','i','s',' ','i','s',' ','a',' ','m','u','c','h',' ','l','o','n','g','e','r',' ','m','e','s','s','a','g','e',' ','t','h','a','t',' ','h','a','n','d','l','e','r',' ','1',' ','s','e','n','d','s',' ','t','o',' ','h','a','n','d','l','e','r',' ','2'},
        .msg_sz = 63,
};

static void indn1(n_indn_t* info)
{

}

static void indn2(n_indn_t* info)
{
        if (test.type == 1)
        {
                if (info->rslt == N_OK && info->msg_sz == 5 && memcmp(info->msg, "Hey:)", 5) == 0)
                {
                        test.recv = 0x01;
                }
                else
                {
                        test.recv = 0xFF;
                }
        }
        if (test.type == 2)
        {
                if (info->rslt == N_OK && info->msg_sz == 63 && memcmp(info->msg, "This is a much longer message that handler 1 sends to handler 2", 63) == 0)
                {
                        test.recv = 0x01;
                }
                else
                {
                        test.recv = 0xFF;
                }
        }
}
static uint8_t send_frame1(canbus_md mode, uint32_t id, uint8_t dlc, uint8_t* dt)
{
        canbus_frame_t frame = { .id = id, .dlc = 8, .mode = mode };
        memmove(frame.dt, dt, 8);
        iso15765_enqueue(&test.handler2, &frame);
        return 0;
}


static uint8_t send_frame2(canbus_md mode, uint32_t id, uint8_t dlc, uint8_t* dt)
{
        canbus_frame_t frame = { .id = id, .dlc = 8, .mode = mode };
        memmove(frame.dt, dt, 8);
        iso15765_enqueue(&test.handler1, &frame);
        return 0;
}

void init_ex_N_ADM_MIXED11()
{
        iso15765_init(&test.handler1);
        iso15765_init(&test.handler2);
}

void test_sf_ex_N_ADM_MIXED11()
{
        test.type = 1;
        test.recv = 0;
        iso15765_send(&test.handler1, &frame_sf);
        iso15765_process(&test.handler1);
        iso15765_process(&test.handler2);
        Sleep(1);
        if (test.recv == 1)
                printf("N_ADM_MIXED11:SF [OK]\n");
        else
                printf("N_ADM_MIXED11:SF [ER]\n");
}

void test_mf_ex_N_ADM_MIXED11()
{
        test.type = 2;
        test.recv = 0;

        iso15765_send(&test.handler1, &frame_mf);

        for (int i = 0; i < 500; i++)
        {
                iso15765_process(&test.handler1);
                iso15765_process(&test.handler2);
                Sleep(1);
                if (test.recv == 1)
                        break;
        }
        if (test.recv == 1)
                printf("N_ADM_MIXED11:MM [OK]\n");
        else
                printf("N_ADM_MIXED11:MM [ER]\n");
}

void test_rf_ex_N_ADM_MIXED11()
{
        for (int i = 0; i < 100; i++)
        {

                if ((rand() % 1000) > 500)
                {
                        test_sf_ex_N_ADM_MIXED11();
                }
                else
                {
                        test_mf_ex_N_ADM_MIXED11();
                }
        }
}