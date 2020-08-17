#include <stdio.h>
#include <stdlib.h>
#include "iso15765_2.h"
#include <windows.h>
#include <sysinfoapi.h>
#include "examples.h"
/******************************************************************************
* Declaration | Static Functions
******************************************************************************/
/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

uint32_t getms()
{
        return GetTickCount();
}

void on_error(n_rslt err_type)
{
        printf("ERROR OCCURED!:%04x", err_type);
}

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

int main()
{
        init_ex_N_ADM_NORMAL();
        test_sf_ex_N_ADM_NORMAL();
        test_mf_ex_N_ADM_NORMAL();
        test_rf_ex_N_ADM_NORMAL();

        init_ex_N_ADM_FIXED();
        test_sf_ex_N_ADM_FIXED();
        test_mf_ex_N_ADM_FIXED();
        test_rf_ex_N_ADM_FIXED();

        init_ex_N_ADM_EXTENDED();
        test_sf_ex_N_ADM_EXTENDED();
        test_mf_ex_N_ADM_EXTENDED();
        test_rf_ex_N_ADM_EXTENDED();

        init_ex_N_ADM_MIXED29();
        test_sf_ex_N_ADM_MIXED29();
        test_mf_ex_N_ADM_MIXED29();
        test_rf_ex_N_ADM_MIXED29();

        init_ex_N_ADM_MIXED11();
        test_sf_ex_N_ADM_MIXED11();
        test_mf_ex_N_ADM_MIXED11();
        test_rf_ex_N_ADM_MIXED11();
        return 0;
}

