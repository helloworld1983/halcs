/*
 * Copyright (C) 2020 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _FMC_100M_4CH_H_
#define _FMC_100M_4CH_H_

/* Known modules IDs (from SDB records defined in FPGA) */
#define FMC_100M_4CH_SDB_DEVID       0x00000608
#define FMC_100M_4CH_SDB_NAME        "FMC_100M_4CH"

extern const smio_bootstrap_ops_t fmc100m_4ch_bootstrap_ops;

#endif


