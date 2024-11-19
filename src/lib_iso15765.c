/*!
@file   iso15765_2.c
@brief  Source file of the ISO15765-2 library
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

/******************************************************************************
* Includes
******************************************************************************/

#include "lib_iso15765.h"

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

/* Structs used by the service to inform the user(upper layer) of an event */
static n_indn_t sgn_indn;
static n_ff_indn_t sgn_ff_indn;
static n_cfm_t sgn_conf;
static n_chg_param_cfm_t sgn_chg_cfm;

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

/*
 * Default empty callback functions. The user can assing his own during the init
 * of the service.
 */
static void indn(n_indn_t* info)
{
	ISO_15675_UNUSED(info);
}

static void ff_indn(n_ff_indn_t* info)
{
	ISO_15675_UNUSED(info);
}

static void cfm(n_cfm_t* info)
{
	ISO_15675_UNUSED(info);
}

static void cfg_cfm(n_chg_param_cfm_t* info)
{
	ISO_15675_UNUSED(info);
}

/*
 * Helper function to find the closest can_dl
 */
inline static uint8_t n_get_closest_can_dl(uint8_t size, cbus_fr_format tmt)
{
	uint8_t rval = 0;

	if (tmt == CBUS_FR_FRM_STD)
	{
		rval = (size <= 0x08U) ? size : 0x08U;
	}
	else
	{
		if (size <= 8)
		{
			rval = size;
		}
		else if (size <= 12)
		{
			rval = 12;
		}
		else if (size <= 16)
		{
			rval = 16;
		}
		else if (size <= 20)
		{
			rval = 20;
		}
		else if (size <= 24)
		{
			rval = 24;
		}
		else if (size <= 32)
		{
			rval = 32;
		}
		else if (size <= 48)
		{
			rval = 48;
		}
		else
		{
			rval = 64;
		}
	}

	return rval;
}

/*
 * Helper function to find the closest can_dl
 */
inline static uint8_t n_get_dt_offset(addr_md address, pci_type pci, uint16_t data_size)
{
	uint8_t offset = (address & 0x01);

	switch (pci)
	{
	case N_PCI_T_SF:
		offset += 1;
		offset += data_size <= (8 - offset) ? 0 : 1;
		break;
	case N_PCI_T_FF:
		offset += 2;
		offset += data_size > 4095 ? 4 : 0;
		break;
	case N_PCI_T_CF:
		offset += 1;
		break;
	case N_PCI_T_FC:
		offset += 3;
		break;
	default:
		offset = 0;
		break;
	}
	return offset;
}

/*
 * Helper function to find which PCI_Type the outbound stream has to use
 */
inline static pci_type n_out_frame_type(iso15765_t* instance)
{
	pci_type result = N_PCI_T_CF;

	if (instance->out.cf_cnt == 0)
	{
		if ((instance->addr_md & 0x01) == 1)
		{
			if (instance->out.fr_fmt == CBUS_FR_FRM_STD)
			{
				result = instance->out.msg_sz <= 6 ? N_PCI_T_SF : N_PCI_T_FF;
			}
			else
			{
				result = instance->out.msg_sz <= 61 ? N_PCI_T_SF : N_PCI_T_FF;
			}
		}
		else
		{
			if (instance->out.fr_fmt == CBUS_FR_FRM_STD)
			{
				result = instance->out.msg_sz <= 7 ? N_PCI_T_SF : N_PCI_T_FF;
			}
			else
			{
				result = instance->out.msg_sz <= 62 ? N_PCI_T_SF : N_PCI_T_FF;
			}
		}
	}
	return result;
}

/*
 * Convert partially the CANBus Frame from the PCI
 */
inline static n_rslt n_pci_pack(addr_md mode, n_pdu_t* n_pdu, const uint8_t* dt)
{
	n_rslt result = N_ERROR;

	if (n_pdu != NULL && dt != NULL)
	{
		uint8_t offs = (mode & 0x01);

		switch (n_pdu->n_pci.pt)
		{
		case N_PCI_T_SF:
			if (n_pdu->n_pci.dl <= (uint16_t)(7 - offs))
			{
				n_pdu->dt[0 + offs] = (n_pdu->n_pci.pt) << 4
					| (uint8_t)(n_pdu->n_pci.dl & 0x0F);
			}
			else
			{
				n_pdu->dt[0 + offs] = 0 + ((n_pdu->n_pci.pt) << 4);
				n_pdu->dt[1 + offs] = (uint8_t)n_pdu->n_pci.dl;
			}
			result = N_OK;
			break;
		case N_PCI_T_CF:
			n_pdu->dt[0 + offs] = (n_pdu->n_pci.pt) << 4
				| n_pdu->n_pci.sn;
			result = N_OK;
			break;
		case N_PCI_T_FF:
			n_pdu->dt[0 + offs] = (n_pdu->n_pci.pt) << 4
				| (n_pdu->n_pci.dl & 0x0F00) >> 8;
			n_pdu->dt[1 + offs] = n_pdu->n_pci.dl & 0x00FF;
			result = N_OK;
			break;
		case N_PCI_T_FC:
			n_pdu->dt[0 + offs] = (n_pdu->n_pci.pt) << 4
				| n_pdu->n_pci.fs;
			n_pdu->dt[1 + offs] = n_pdu->n_pci.bs;
			n_pdu->dt[2 + offs] = n_pdu->n_pci.st;
			result = N_OK;
			break;
		default:
			result = N_ERROR;
			break;
		}
	}
	return result;
}

/*
 * Convert the PCI from the CANBus Frame
 */
inline static n_rslt n_pci_unpack(addr_md mode, n_pdu_t* n_pdu, uint8_t dlc, uint8_t* dt)
{
    n_rslt result = N_ERROR;

    if ((n_pdu != NULL) && (dt != NULL))
    {
        uint8_t offs = (uint8_t)(mode & 0x01U); // Make use and purpose of 'offs' explicit

        // Document the operation's intent clearly to avoid being seen as 'dead code'
        n_pdu->n_pci.pt = (uint8_t)((dt[offs] & 0xF0U) >> 4U);

        switch (n_pdu->n_pci.pt)
        {
            case N_PCI_T_SF:
                // Conditional operation based on 'dlc', not dead code
                n_pdu->n_pci.dl = (dlc <= 8U) ? (uint8_t)(dt[offs] & 0x0FU) : dt[1U + offs];
                result = N_OK;
                break;

            case N_PCI_T_CF:
                // Direct assignment, the operation depends on 'dt' content
                n_pdu->n_pci.sn = (uint8_t)(dt[offs] & 0x0FU);
                n_pdu->sz = dlc - (1U + offs);
                result = N_OK;
                break;

            case N_PCI_T_FF:
                // Combine two bytes into a larger value, clearly intentional
                n_pdu->n_pci.dl = ((uint16_t)(dt[offs] & 0x0FU) << 8U) | dt[1U + offs];
                n_pdu->sz = dlc - (2U + offs);
                result = N_OK;
                break;

            case N_PCI_T_FC:
                // Sequential assignments based on protocol, clearly used
                n_pdu->n_pci.fs = (uint8_t)(dt[offs] & 0x0FU);
                n_pdu->n_pci.bs = dt[1U + offs];
                n_pdu->n_pci.st = dt[2U + offs];
                n_pdu->sz = dlc - (3U + offs); // Adjust for correct data length calculation
                result = N_OK;
                break;

            default:
                result = N_ERROR;
                break;
        }
    }

    return result;
}

/*
 * Helper function of 'n_pdu_pack' to use the frame payload.
 */
inline static n_rslt n_pdu_pack_dt(addr_md mode, n_pdu_t* n_pdu, uint8_t* dt)
{
	n_rslt result = N_ERROR;

	if (dt != NULL)
	{
		switch (n_pdu->n_pci.pt)
		{
		case N_PCI_T_SF:
			memmove(&n_pdu->dt[n_get_dt_offset(mode, N_PCI_T_SF, n_pdu->sz)], dt, n_pdu->sz);
			break;
		case N_PCI_T_FF:
			memmove(&n_pdu->dt[n_get_dt_offset(mode, N_PCI_T_FF, n_pdu->sz)], dt, n_pdu->sz);
			break;
		case N_PCI_T_CF:
			memmove(&n_pdu->dt[n_get_dt_offset(mode, N_PCI_T_CF, n_pdu->sz)], dt, n_pdu->sz);
			break;
		case N_PCI_T_FC:
			memmove(&n_pdu->dt[n_get_dt_offset(mode, N_PCI_T_FC, n_pdu->sz)], dt, n_pdu->sz);

			break;

		default:
			break;
		}
		result = N_OK;
	}
	return result;
}

/*
 * Helper function of 'n_pdu_unpack' to use the frame payload.
 */
inline static n_rslt n_pdu_unpack_dt(addr_md mode, n_pdu_t* n_pdu, uint8_t* dt)
{
	n_rslt result = N_ERROR;

	if ((n_pdu != NULL) && (dt != NULL))
	{
		switch (n_pdu->n_pci.pt)
		{
		case N_PCI_T_SF:
			memmove(n_pdu->dt, &dt[n_get_dt_offset(mode, N_PCI_T_SF, n_pdu->n_pci.dl)], n_pdu->n_pci.dl);
			result = N_OK;
			break;
		case N_PCI_T_FF:
			memmove(n_pdu->dt, &dt[n_get_dt_offset(mode, N_PCI_T_FF, n_pdu->sz)], n_pdu->sz);
			result = N_OK;
			break;
		case N_PCI_T_CF:
			memmove(n_pdu->dt, &dt[n_get_dt_offset(mode, N_PCI_T_CF, n_pdu->sz)], n_pdu->sz);
			result = N_OK;
			break;
		case N_PCI_T_FC:
			memmove(n_pdu->dt, &dt[n_get_dt_offset(mode, N_PCI_T_FC, n_pdu->sz)], n_pdu->sz);
			result = N_OK;
			break;
		default:
			result = N_ERROR;
			break;
		}
	}
	return result;
}

/*
 * Convert CANBus frame from PDU
 */
inline static n_rslt n_pdu_pack(addr_md mode, n_pdu_t* n_pdu, uint32_t* id, uint8_t* dt)
{
    if ((dt == NULL) || (id == NULL))
    {
        return N_ERROR;
    }

    switch (mode)
    {
        case N_ADM_EXTENDED:
        	*id = 0x80U
                | (n_pdu->n_ai.n_pr << 8)
                | (n_pdu->n_ai.n_ae << 3)
                | (n_pdu->n_ai.n_sa) 
                | ((n_pdu->n_ai.n_tt == N_TA_T_PHY) ? 0x40U : 0x00U); 
				n_pdu->dt[0] = n_pdu->n_ai.n_ta;
			break;
        case N_ADM_NORMAL:
            *id = 0x80U
                | (n_pdu->n_ai.n_pr << 8)
                | (n_pdu->n_ai.n_ta << 3)
                | n_pdu->n_ai.n_sa
                | ((n_pdu->n_ai.n_tt == N_TA_T_PHY) ? 0x40U : 0x00U);
            break;
        case N_ADM_MIXED29:
            *id = (n_pdu->n_ai.n_pr << 26) 
                | ((n_pdu->n_ai.n_tt == N_TA_T_PHY) ? 0xCEU : 0xCDU) << 16
                | (n_pdu->n_ai.n_ta << 8)
                | n_pdu->n_ai.n_sa;
            n_pdu->dt[0] = n_pdu->n_ai.n_ae;
            break;
        case N_ADM_FIXED:
            *id = (n_pdu->n_ai.n_pr << 26)
                | ((n_pdu->n_ai.n_tt == N_TA_T_PHY) ? 0xDAU : 0xDBU) << 16
                | (n_pdu->n_ai.n_ta << 8)
                | n_pdu->n_ai.n_sa;
            break;
        case N_ADM_MIXED11:
            *id = 0x80U
                | (n_pdu->n_ai.n_pr << 8)
                | (n_pdu->n_ai.n_ta << 3)
                | n_pdu->n_ai.n_sa
                | ((n_pdu->n_ai.n_tt == N_TA_T_PHY) ? 0x40U : 0x00U);
            n_pdu->dt[0] = n_pdu->n_ai.n_ae;
            break;
        default:
            return N_ERROR;
    }

    n_pci_pack(mode, n_pdu, dt);
    return n_pdu_pack_dt(mode, n_pdu, dt);
}

/*
 * Convert PDU from CANBus frame
 */
inline static n_rslt n_pdu_unpack(addr_md mode, n_pdu_t* n_pdu, uint32_t id, uint8_t dlc, uint8_t* dt)
{
	if (n_pdu == NULL || dt == NULL)
	{
		return N_ERROR;
	}

	switch (mode)
	{
	case N_ADM_MIXED11:
		n_pdu->n_ai.n_ae = dt[0];
		__attribute__((fallthrough));
	case N_ADM_NORMAL:
		n_pdu->n_ai.n_pr = (uint8_t)((id & 0x700U) >> 8);
		n_pdu->n_ai.n_ta = (uint8_t)((id & 0x38U) >> 3);
		n_pdu->n_ai.n_sa = (uint8_t)(id & 0x07U);
		n_pdu->n_ai.n_tt = (uint8_t)(((id & 0x40U) >> 6) == 1 ? N_TA_T_PHY : N_TA_T_FUNC);
		break;
	case N_ADM_MIXED29:
		n_pdu->n_ai.n_ae = dt[0];
		n_pdu->n_ai.n_pr = (uint8_t)((id & 0x1C000000) >> 26);
		n_pdu->n_ai.n_tt = (uint8_t)(((id & 0x00FF0000) >> 16) == 0xCE ? N_TA_T_PHY : N_TA_T_FUNC);
		n_pdu->n_ai.n_ta = (uint8_t)((id & 0x0000FF00) >> 8);
		n_pdu->n_ai.n_sa = (uint8_t)((id & 0x000000FF) >> 0);
		break;
	case N_ADM_FIXED:
		n_pdu->n_ai.n_pr = (uint8_t)((id & 0x1C000000) >> 26);
		n_pdu->n_ai.n_tt = (uint8_t)(((id & 0x00FF0000) >> 16) == 0xDA ? N_TA_T_PHY : N_TA_T_FUNC);
		n_pdu->n_ai.n_ta = (uint8_t)((id & 0x0000FF00) >> 8);
		n_pdu->n_ai.n_sa = (uint8_t)((id & 0x000000FF) >> 0);
		break;
	case N_ADM_EXTENDED:
		n_pdu->n_ai.n_pr = (uint8_t)((id & 0x700U) >> 8);
		n_pdu->n_ai.n_ae = (uint8_t)((id & 0xF8U) >> 3);
		n_pdu->n_ai.n_sa = (uint8_t)(id & 0x07U);
		n_pdu->n_ai.n_tt = (uint8_t)((id & 0x40U) >> 6 == 1 ? N_TA_T_PHY : N_TA_T_FUNC);
		n_pdu->n_ai.n_ta = dt[0];
		break;
	default:
		return N_UNE_PDU;
	}

	n_pci_unpack(mode, n_pdu, dlc, dt);
	n_pdu_unpack_dt(mode, n_pdu, dt);

	return N_OK;
}

/*
 * Given the correct parameters, the service informs the upper-layer/user about
 * an event by using the appropriate callbacks. The function does not support
 * the N_CHG_P_CONF signal type.
 */
inline static void signaling(signal_tp tp, n_iostream_t* strm, void(*cb)(void*), uint16_t msg_sz, n_rslt sgn_rslt)
{
	if (cb != NULL)
	{
		switch (tp)
		{
		case N_INDN:
			sgn_indn.rslt = sgn_rslt;
			sgn_indn.msg_sz = msg_sz;
			sgn_indn.fr_fmt = strm->fr_fmt;
			memmove(&sgn_indn.n_ai, &strm->pdu.n_ai, sizeof(n_ai_t));
			memmove(&sgn_indn.n_pci, &strm->pdu.n_pci, sizeof(n_pci_t));
			memmove(&sgn_indn.msg, strm->msg, msg_sz);
			strm->sts = N_S_IDLE;
			cb(&sgn_indn);
			break;
		case N_FF_INDN:
			sgn_ff_indn.fr_fmt = strm->fr_fmt;
			sgn_ff_indn.msg_sz = msg_sz;
			memmove(&sgn_ff_indn.n_ai, &strm->pdu.n_ai, sizeof(n_ai_t));
			memmove(&sgn_ff_indn.n_pci, &strm->pdu.n_pci, sizeof(n_pci_t));
			strm->sts = (uint8_t)((uint32_t)strm->sts | (uint32_t)N_S_RX_BUSY);
			cb(&sgn_ff_indn);
			break;
		case N_CONF:
			sgn_conf.rslt = sgn_rslt;
			memmove(&sgn_conf.n_ai, &strm->pdu.n_ai, sizeof(n_ai_t));
			memmove(&sgn_conf.n_pci, &strm->pdu.n_pci, sizeof(n_pci_t));
			cb(&sgn_conf);
			break;
		default:
			return;
		}
	}
	return;
}

/*
 * Check if any timeout should be occured.
 */
inline static n_rslt process_timeouts(iso15765_t* ih)
{
	if (ih->out.sts != N_S_TX_WAIT_FC || ih->out.last_upd.n_bs == 0 || ih->config.n_bs == 0
		|| (ih->clbs.get_ms() - ih->config.n_bs) < ih->out.last_upd.n_bs)
	{
		return N_OK;
	}

	/* if timeout occures then reset the counters and report to the upper layer */
	ih->out.cf_cnt = 0x0;
	signaling(N_INDN, &ih->out, (void*)ih->clbs.indn, ih->out.msg_sz, N_TIMEOUT_Bs);
	ih->clbs.on_error(N_TIMEOUT_Bs);
	return N_TIMEOUT_Bs;
}

/*
 * Sends a Flow Control Frame upon request from the reception procedure
 */
static n_rslt send_N_PCI_T_FC(iso15765_t* ih)
{
	uint32_t id;

	ih->out.sts |= N_S_TX_BUSY;
	ih->fl_pdu.n_pci.pt = N_PCI_T_FC;
	ih->fl_pdu.n_pci.bs = ih->config.bs;
	ih->fl_pdu.n_pci.st = ih->config.stmin;
	ih->fl_pdu.n_ai.n_ae = ih->in.pdu.n_ai.n_ae;
	ih->fl_pdu.n_ai.n_sa = ih->in.pdu.n_ai.n_ta;
	ih->fl_pdu.n_ai.n_ta = ih->in.pdu.n_ai.n_sa;
	ih->fl_pdu.n_ai.n_pr = ih->in.pdu.n_ai.n_pr;
	ih->fl_pdu.n_ai.n_tt = ih->in.pdu.n_ai.n_tt;
	ih->in.cfg_bs = ih->config.bs;

	if (n_pdu_pack(ih->addr_md, &ih->fl_pdu, &id, ih->out.msg) != N_OK)
	{
		ih->out.sts = (ih->out.sts & (~N_S_TX_BUSY));
		return N_ERROR;
	}

	ih->clbs.send_frame(ih->fr_id_type, id, ih->in.fr_fmt, n_get_dt_offset(ih->addr_md, N_PCI_T_FC, ih->fl_pdu.sz), ih->fl_pdu.dt);
	ih->out.sts = (ih->out.sts & (~N_S_TX_BUSY));
	return N_OK;
}

/*
 * Helper function to set some basic stream parameters value
 */
inline static void set_stream_data(n_iostream_t* ist, uint8_t cf, uint8_t wf, stream_sts sts)
{
	ist->cf_cnt = cf;
	ist->wf_cnt = wf;
	ist->sts = sts;
}

/*
 * Check if current Wait Flow status counter reached the max WFS
 */
inline static n_rslt check_max_wf_capacity(iso15765_t* ih)
{
	if (ih->out.wf_cnt < ih->config.wf)
	{
		return N_OK;
	}

	ih->clbs.on_error(N_WFT_OVRN);
	set_stream_data(&ih->out, 0, 0, N_S_IDLE);
	return N_WFT_OVRN;
}

/*
 * Process inbound First Frame reception and report to the upper layer using the
 * indication callback function.
 */
static n_rslt process_in_ff(iso15765_t* ih)
{
	if (ih->in.msg_sz > I15765_MSG_SIZE)
	{
		ih->clbs.on_error(N_INV_REQ_SZ);
		return N_INV_REQ_SZ;
	}
	/* If reception is in progress: Terminate the current reception, report an
	* N_USData.indication, with <N_Result> set to N_UNEXP_PDU, to the upper layer, and
	* process the FF N_PDU as the start of a new reception.*/
	if ((ih->in.sts & N_S_RX_BUSY) != 0)
	{
		ih->clbs.on_error(N_UNE_PDU);
		signaling(N_INDN, &ih->in, (void*)ih->clbs.indn, ih->in.msg_sz, N_UNE_PDU);
	}

	/* Copy all data, init the CFrames reception parameters and send a FC */
	memmove(ih->in.msg, ih->in.pdu.dt, ih->in.pdu.sz);
	ih->in.msg_sz = ih->in.pdu.n_pci.dl;
	ih->in.msg_pos = ih->in.pdu.sz;
	ih->in.cf_cnt = 0;
	ih->in.wf_cnt = 0;
	signaling(N_FF_INDN, &ih->in, (void*)ih->clbs.ff_indn, ih->in.msg_sz, N_OK);
	send_N_PCI_T_FC(ih);
	return N_OK;
}

/*
 * Process inbound Single Frame reception and report to the upper layer using the
 * indication callback function.
 */
static n_rslt process_in_sf(iso15765_t* ih)
{
	/* If reception is in progress: Terminate the current reception, report an
	* N_USData.indication, with <N_Result> set to N_UNEXP_PDU, to the upper layer, and
	* process the SF N_PDU as the start of a new reception.*/
	if ((ih->in.sts & N_S_RX_BUSY) != 0)
	{
		ih->clbs.on_error(N_UNE_PDU);
		signaling(N_INDN, &ih->in, (void*)ih->clbs.indn, ih->in.msg_sz, N_UNE_PDU);
	}
	memmove(&ih->in.msg[0], ih->in.pdu.dt, ih->in.pdu.n_pci.dl);
	ih->in.sts = N_S_IDLE;
	signaling(N_INDN, &ih->in, (void*)ih->clbs.indn, ih->in.pdu.n_pci.dl, N_OK);
	return N_OK;
}

/*
 * Process inbound Consecutive Frames. Perform all the required checks according
 * to (ref: iso15765-2 p.26) and if everything is ok copy all the data to the
 * inbound stream buffer and update the reception parameters (CF_cnt,timeouts etc)
 */
static n_rslt process_in_cf(iso15765_t* ih)
{
	n_rslt rslt = N_OK;

	/* According to (ref: iso15765-2 p.26) if we are not in progress of
	* reception we should ignore it */
	if ((ih->in.sts & N_S_RX_BUSY) == 0)
	{
		rslt = N_UNE_CF;
		goto in_cf_error;
	}

	/* Increase the CF counter and check if the reception sequence is ok */
	ih->in.cf_cnt = ih->in.cf_cnt + 1 > 0xFF ? 0 : ih->in.cf_cnt + 1;
	if ((ih->in.cf_cnt & 0x0f) != ih->in.pdu.n_pci.sn)
	{
		rslt = N_INV_SEQ_NUM;
		goto in_cf_error;
	}

	/* As long as everything is ok the we copy the frame data to the inbound
	* stream buffer. Afterwards check if the message size is completed and
	* signal the user and afterwards reset the inboud stream */
	memmove(&ih->in.msg[ih->in.msg_pos], ih->in.pdu.dt, ih->in.pdu.sz);
	ih->in.msg_pos += ih->in.pdu.sz;

	if (ih->in.msg_pos >= ih->in.msg_sz)
	{
		signaling(N_INDN, &ih->in, (void*)ih->clbs.indn, ih->in.msg_sz, N_OK);
		memset(&ih->in, 0, sizeof(n_iostream_t));
		ih->in.last_upd.n_cr = 0;
		//	ih->in.sts = N_S_IDLE;
		return N_OK;
	}
	/* if we reach the max CF counter, then we send a FC frame */
	if(ih->config.bs != 0)
	{
		if (ih->in.cf_cnt == ih->config.bs)
		{
			ih->in.cf_cnt = 0;
			send_N_PCI_T_FC(ih);
		}
	}
	/* Update the Cr timer */
	ih->in.last_upd.n_cr = ih->clbs.get_ms();
	return rslt;

in_cf_error:
	ih->clbs.on_error(rslt);
	ih->in.sts = N_S_IDLE;
	return rslt;
}

/*
 * Process inbound Flow Control Frames. Outcome depends on the stream status
 * (if it is busy etc) as well as the Flow Control Status.
 */
static n_rslt process_in_fc(iso15765_t* ih)
{
	n_rslt rslt = N_UNE_PDU;

	/* According to (ref: iso15765-2 p.26) if we are not expecting FC frame
	* we should ignore it */
	if (ih->out.sts != N_S_TX_WAIT_FC)
	{
		return rslt;
	}

	switch (ih->in.pdu.n_pci.fs)
	{
	case N_WAIT:
		/* Increase the WF counter, check if we reached the WF Limit to abort
		* the reception and (if not WF overflow) update the Bs time */
		ih->out.wf_cnt += 1;
		if (check_max_wf_capacity(ih) != N_WFT_OVRN)
		{
			return N_OK;			
		}
		ih->out.last_upd.n_bs = ih->clbs.get_ms();
		rslt = N_WFT_OVRN;
		break;
	case N_OVERFLOW:
		rslt = N_BUFFER_OVFLW;
		break;
	case N_CONTINUE:
		/* Store the requested transmission parameters (from receiver)
		* to the outbound stream, reset the counters of CFs(1) and WFs(0)
		* and change the outbound stream status to Ready */
		ih->out.cfg_bs = ih->in.pdu.n_pci.bs;
		ih->out.stmin = ih->in.pdu.n_pci.st;
		set_stream_data(&ih->out, 1, 0, N_S_TX_READY);
		return N_OK;
	default:
		rslt = N_UNE_FC_STS;
		break;
	}

	/* If there is an error (only way to be here) then reset the
	* outbound stream, set the counters of CFs(0) and WFs(0)
	* and change the outbound stream status to Idle. Use on_error
	* callback to inform the upper layer */
	set_stream_data(&ih->out, 0, 0, N_S_IDLE);
	ih->clbs.on_error(rslt);
	ih->in.sts = N_S_IDLE;
	return rslt;
}

/*
 * Inbound stream process. The function receives a canbus frame dequeued by
 * the 'iso15765_process' and performs any needed operation to identify and
 * consume the underlying information.
 */
inline static n_rslt iso15765_process_in(iso15765_t* ih, canbus_frame_t* frame)
{
	/* Converting the canbus frame to PDU format and process it by its PCI Type */
	ih->in.fr_fmt = frame->fr_format;
	if (n_pdu_unpack(ih->addr_md, &ih->in.pdu, frame->id, (uint8_t)frame->dlc, frame->dt) == N_OK)
	{
		switch (ih->in.pdu.n_pci.pt)
		{
		case N_PCI_T_FC:
			return process_in_fc(ih);
		case N_PCI_T_CF:
			return process_in_cf(ih);
		case N_PCI_T_SF:
			return process_in_sf(ih);
		case N_PCI_T_FF:
			return process_in_ff(ih);
		default:
			break;
		}
	}

	/* According to (ref: iso15765-2 p.26) if PDU is not valid
	* we should ignore it */
	ih->clbs.on_error(N_INV_PDU);
	return N_INV_PDU;
}

/*
 * Procces the outbound stream.
 */
static n_rslt iso15765_process_out(iso15765_t* ih)
{
	/* if there is no pending action just return */
	if (ih->out.sts != N_S_TX_BUSY && ih->out.sts != N_S_TX_READY)
	{
		return N_IDLE;
	}

	uint32_t id;
	n_rslt rslt = N_ERROR;

	/* Find the PCI type of the pending outbound stream */
	ih->out.pdu.n_pci.pt = n_out_frame_type(ih);

	switch (ih->out.pdu.n_pci.pt)
	{
	case N_PCI_T_SF:
		/* Copy all the data of the SF to the outbound stream, pack and send the canbus frame */
		ih->out.pdu.n_pci.dl = ih->out.msg_sz;
		ih->out.pdu.sz = ih->out.msg_sz;

		if (n_pdu_pack(ih->addr_md, &ih->out.pdu, &id, ih->out.msg) != N_OK)
		{
			goto iso15765_process_out_cfm;
		}
			
		rslt = ih->clbs.send_frame(ih->fr_id_type, id, ih->out.fr_fmt, n_get_closest_can_dl(ih->out.pdu.sz + n_get_dt_offset(ih->addr_md, N_PCI_T_SF, ih->out.pdu.sz), ih->out.fr_fmt), ih->out.pdu.dt) == 0 ? N_OK : N_ERROR;
		goto iso15765_process_out_cfm;
		break;

	case N_PCI_T_FF:
		/* Copy all the data of the FF to the outbound stream for transmission and prepare the service
		* for a multi-frame reception */
		ih->out.pdu.n_pci.dl = ih->out.msg_sz;
		ih->out.wf_cnt = 0;
		ih->out.pdu.sz = ih->out.fr_fmt == CBUS_FR_FRM_STD ? ((ih->addr_md & 0x01) == 0 ? 6 : 5) : ((ih->addr_md & 0x01) == 0 ? 62 : 61);
		ih->out.msg_pos = ih->out.pdu.sz;
		if (n_pdu_pack(ih->addr_md, &ih->out.pdu, &id, ih->out.msg) != N_OK)
		{
			goto iso15765_process_out_cfm;
		}
		ih->out.cf_cnt = 1;

		/* after this frame we expect a Flow Control then assign the correct flag before the
		* transmission to avoid any issues and start the timer */
		ih->out.sts = N_S_TX_WAIT_FC;
		rslt = ih->clbs.send_frame(ih->fr_id_type, id, ih->out.fr_fmt, ih->out.fr_fmt == CBUS_FR_FRM_STD ? 8 : 64, ih->out.pdu.dt) == 0 ? N_OK : N_ERROR;
		ih->out.last_upd.n_bs = ih->clbs.get_ms();
		return (rslt == 0) ? N_OK : N_ERROR;

	case N_PCI_T_CF:
		/* if the minimun difference between transmissions is not reached then skip */
		if ((ih->clbs.get_ms() - ih->config.stmin) < ih->out.last_upd.n_cs)
		{
			return N_OK;
		}
			
		/* Increase the sequence number of the frame and the CF counter of the stream
		* and then pack the PDU to a CANBus frame */
		ih->out.pdu.n_pci.sn = ih->out.cf_cnt & 0x0F;
		ih->out.cf_cnt = ih->out.cf_cnt == 0xFF ? 0 : ih->out.cf_cnt + 1;
		if (ih->out.fr_fmt == CBUS_FR_FRM_STD)
		{
			uint8_t max_payload = (ih->addr_md & 0x01) == 0 ? 7 : 6;
			ih->out.pdu.sz = ih->out.msg_sz - ih->out.msg_pos;
			ih->out.pdu.sz = ih->out.pdu.sz >= max_payload ? max_payload : ih->out.pdu.sz;
		}
		else
		{
			uint8_t max_payload = (ih->addr_md & 0x01) == 0 ? 63 : 62;
			ih->out.pdu.sz = ih->out.msg_sz - ih->out.msg_pos;
			ih->out.pdu.sz = ih->out.pdu.sz >= max_payload ? max_payload : ih->out.pdu.sz;
		}

		if (n_pdu_pack(ih->addr_md, &ih->out.pdu, &id, &ih->out.msg[ih->out.msg_pos]) != N_OK)
		{
			goto iso15765_process_out_cfm;
		}
			
		/* Increase the position which indicates the remaining data in the inbound buffer */
		ih->out.msg_pos += ih->out.pdu.sz;

		/* if after this frame we expect a Flow Control then assign the correct flag before the
		* transmission to avoid any issues and start the timer */
		if (ih->out.pdu.n_pci.sn == ih->config.bs)
		{
			ih->out.sts = N_S_TX_WAIT_FC;
			ih->out.last_upd.n_bs = ih->clbs.get_ms();
		}
		/* send the canbus frame! */
		uint8_t of1 = (ih->addr_md & 0x01) == 0 ? 1 : 2;
		rslt = ih->clbs.send_frame(ih->fr_id_type, id, ih->out.fr_fmt, n_get_closest_can_dl(ih->out.pdu.sz + of1, ih->out.fr_fmt), ih->out.pdu.dt) == 0 ? N_OK : N_ERROR;
		ih->out.last_upd.n_cs = ih->clbs.get_ms();
		if (ih->out.msg_pos >= ih->out.msg_sz)
		{
			goto iso15765_process_out_cfm;
		}
		return N_OK;

	default:
		break;
	}

	return N_ERROR;

iso15765_process_out_cfm:
	ih->out.sts = N_S_IDLE;
	ih->out.cf_cnt = 0;
	ih->out.wf_cnt = 0;
	signaling(N_CONF, &ih->out, (void*)ih->clbs.cfm, 0, rslt);
	return rslt;
}

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

/*
 * Initialize and check if the configuration parameters of the ISO15765 handler are
 * correct. Not assigned callback which are not assigned will be automatically assigned
 * to the static UNUSED functions. If no error occurs, the system can start the service.
 */
n_rslt iso15765_init(iso15765_t* instance)
{
	if (instance == NULL)
	{
		return N_NULL;
	}

	/* check if parameters have correct values */
	if ((instance->fr_id_type != CBUS_ID_T_STANDARD && instance->fr_id_type != CBUS_ID_T_EXTENDED)
		|| (instance->fr_id_type & instance->addr_md) == 0)
	{
		return N_WRG_VALUE;
	}

	/* check if must-have functions are assigned */
	if (instance->clbs.send_frame == NULL || instance->clbs.get_ms == NULL)
	{
		return N_MISSING_CLB;
	}

	/* if optional functions are not assigned then use the defaults */
	if (instance->clbs.indn == NULL)
	{
		instance->clbs.indn = indn;
	}

	if (instance->clbs.ff_indn == NULL)
	{
		instance->clbs.ff_indn = ff_indn;
	}

	if (instance->clbs.cfm == NULL)
	{
		instance->clbs.cfm = cfm;
	}

	if (instance->clbs.cfg_cfm == NULL)
	{
		instance->clbs.cfg_cfm = cfg_cfm;
	}

	/* clear the in/out streams */
	memset(&instance->in, 0, sizeof(n_iostream_t));
	memset(&instance->out, 0, sizeof(n_iostream_t));
	memset(&instance->fl_pdu, 0, sizeof(n_pdu_t));
	/* init the incoming canbus frame queue(buffer) */
	if (iqueue_init(&instance->inqueue,
		I15765_QUEUE_ELMS,
		sizeof(canbus_frame_t),
		instance->inq_buf) != I_OK)
		{
			return N_INV;
		}

	ISO_15675_UNUSED(sgn_chg_cfm);

	instance->init_sts = N_OK;
	return instance->init_sts;
}

/*
 * Enqueues an incoming frame from the lower level (CANBus) to a buffer. The service
 * will process the frames during the call of the 'iso15765_process' function. Usually
 * this function should be called when a canbus frame is received.
 */
n_rslt iso15765_enqueue(iso15765_t* instance, canbus_frame_t* frame)
{
	if (instance == NULL)
	{
		return N_NULL;
	}

	if (instance->init_sts != N_OK)
	{
		return N_ERROR;
	}

	return iqueue_enqueue(&instance->inqueue, frame) == I_OK
		? N_OK : N_BUFFER_OVFLW;
}

/*
 * Request to send a message. Depending on the message a call to 'iso15765_process'
 * may be required. The service can send one message per time as long as the
 * communication is syncronous
 */
n_rslt iso15765_send(iso15765_t* instance, n_req_t* frame)
{
	if (instance == NULL)
	{
		return N_NULL;
	}

	if (instance->init_sts != N_OK)
	{
		return N_ERROR;
	}

	/* Make sure that there is no transmission in progress */
	if (instance->out.sts != N_S_IDLE)
	{
		return N_TX_BUSY;
	}
	/* and the requested size is fitting in our outbound buffer */
	if (frame->msg_sz > I15765_MSG_SIZE)
	{
		return N_BUFFER_OVFLW;
	}
	/* or there is not actual message to be sent */
	if (frame->msg_sz == 0)
	{
		return N_INV_REQ_SZ;
	}
	/* check if frame type is correct */
	if (frame->fr_fmt != CBUS_FR_FRM_STD && frame->fr_fmt != CBUS_FR_FRM_FD)
	{
		return N_INV;
	}
	/* check if Target Address Type is correct */
	if (frame->n_ai.n_tt != N_TA_T_PHY && frame->n_ai.n_tt != N_TA_T_FUNC)
	{
		return N_INV;
	}

	/* copy all the info and data to the outbound buffer */
	instance->out.fr_fmt = frame->fr_fmt;
	instance->out.msg_sz = frame->msg_sz;
	memmove(instance->out.msg, frame->msg, frame->msg_sz);
	memmove(&instance->out.pdu.n_ai, &frame->n_ai, sizeof(n_ai_t));
	instance->out.sts = N_S_TX_BUSY;

	return N_OK;
}

/*
 * Process the inbound/outbound streams of the service. For optimal operation
 * this function should be called continiously with a minimal delay. It is
 * suggested to be used in a thread.
 */
n_rslt iso15765_process(iso15765_t* instance)
{
	if (instance == NULL)
	{
		return N_NULL;
	}

	if (instance->init_sts != N_OK)
	{
		return N_ERROR;
	}

	/* First check if a timeout is occured. Only for the inbound stream */
	n_rslt rslt = process_timeouts(instance);
	canbus_frame_t frame;

	/* Dequeue all the incoming frames and process them */
	while (iqueue_dequeue(&instance->inqueue, &frame) != I_EMPTY)
	{
		rslt |= iso15765_process_in(instance, &frame);
	}

	/* Process the outbound stream */
	rslt |= iso15765_process_out(instance);
	return rslt;
}

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
