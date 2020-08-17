#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "iso15765_2.h"
#include <windows.h>
#include <sysinfoapi.h>


typedef struct
{
	uint8_t type;
	uint8_t recv;
	iso15765_t handler1;
	iso15765_t handler2;
}test_t;

void init_ex_N_ADM_NORMAL();
void init_ex_N_ADM_FIXED();
void init_ex_N_ADM_MIXED11();
void init_ex_N_ADM_EXTENDED();
void init_ex_N_ADM_MIXED29();

void test_sf_ex_N_ADM_NORMAL();
void test_sf_ex_N_ADM_FIXED();
void test_sf_ex_N_ADM_MIXED11();
void test_sf_ex_N_ADM_EXTENDED();
void test_sf_ex_N_ADM_MIXED29();

void test_mf_ex_N_ADM_NORMAL();
void test_mf_ex_N_ADM_FIXED();
void test_mf_ex_N_ADM_MIXED11();
void test_mf_ex_N_ADM_EXTENDED();
void test_mf_ex_N_ADM_MIXED29();

void test_rf_ex_N_ADM_NORMAL();
void test_rf_ex_N_ADM_FIXED();
void test_rf_ex_N_ADM_MIXED1();
void test_rf_ex_N_ADM_EXTENDED();
void test_rf_ex_N_ADM_MIXED29();

uint32_t getms();
void on_error(n_rslt err_type);