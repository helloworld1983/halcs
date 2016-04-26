/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _SM_IO_FMC_ACTIVE_CLK_CODES_H_
#define _SM_IO_FMC_ACTIVE_CLK_CODES_H_

/* Messaging OPCODES */
#define FMC_ACTIVE_CLK_OPCODE_TYPE                         uint32_t
#define FMC_ACTIVE_CLK_OPCODE_SIZE                         (sizeof (FMC_ACTIVE_CLK_OPCODE_TYPE))

#define FMC_ACTIVE_CLK_OPCODE_SI571_OE                     0
#define FMC_ACTIVE_CLK_NAME_SI571_OE                       "fmc_active_clk_si571_oe"
#define FMC_ACTIVE_CLK_OPCODE_PLL_FUNCTION                 1
#define FMC_ACTIVE_CLK_NAME_PLL_FUNCTION                   "fmc_active_clk_pll_function"
#define FMC_ACTIVE_CLK_OPCODE_PLL_STATUS                   2
#define FMC_ACTIVE_CLK_NAME_PLL_STATUS                     "fmc_active_clk_pll_status"
#define FMC_ACTIVE_CLK_OPCODE_CLK_SEL                      3
#define FMC_ACTIVE_CLK_NAME_CLK_SEL                        "fmc_active_clk_clk_sel"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_CFG_DEFAULTS          4
#define FMC_ACTIVE_CLK_NAME_AD9510_CFG_DEFAULTS            "fmc_active_clk_ad9510_cfg_defaults"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_PLL_A_DIV             5
#define FMC_ACTIVE_CLK_NAME_AD9510_PLL_A_DIV               "fmc_active_clk_ad9510_pll_a_div"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_PLL_B_DIV             6
#define FMC_ACTIVE_CLK_NAME_AD9510_PLL_B_DIV               "fmc_active_clk_ad9510_pll_b_div"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_PLL_PRESCALER         7
#define FMC_ACTIVE_CLK_NAME_AD9510_PLL_PRESCALER           "fmc_active_clk_ad9510_pll_prescaler"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_R_DIV                 8
#define FMC_ACTIVE_CLK_NAME_AD9510_R_DIV                   "fmc_active_clk_ad9510_r_div"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_PLL_PDOWN             9
#define FMC_ACTIVE_CLK_NAME_AD9510_PLL_PDOWN               "fmc_active_clk_ad9510_pll_pdown"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_MUX_STATUS            10
#define FMC_ACTIVE_CLK_NAME_AD9510_MUX_STATUS              "fmc_active_clk_ad9510_mux_status"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_CP_CURRENT            11
#define FMC_ACTIVE_CLK_NAME_AD9510_CP_CURRENT              "fmc_active_clk_ad9510_cp_current"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_OUTPUTS               12
#define FMC_ACTIVE_CLK_NAME_AD9510_OUTPUTS                 "fmc_active_clk_ad9510_outputs"
#define FMC_ACTIVE_CLK_OPCODE_AD9510_PLL_CLK_SEL           13
#define FMC_ACTIVE_CLK_NAME_AD9510_PLL_CLK_SEL             "fmc_active_clk_ad9510_pll_clk_sel"
#define FMC_ACTIVE_CLK_OPCODE_SI571_SET_FREQ               14
#define FMC_ACTIVE_CLK_NAME_SI571_SET_FREQ                 "fmc_active_clk_si571_set_freq"
#define FMC_ACTIVE_CLK_OPCODE_SI571_GET_DEFAULTS           15
#define FMC_ACTIVE_CLK_NAME_SI571_GET_DEFAULTS             "fmc_active_clk_si571_get_defaults"
#define FMC_ACTIVE_CLK_OPCODE_END                          16

/* Messaging Reply OPCODES */
#define FMC_ACTIVE_CLK_REPLY_TYPE                          uint32_t
#define FMC_ACTIVE_CLK_REPLY_SIZE                          (sizeof (FMC_ACTIVE_CLK_REPLY_TYPE))

#define FMC_ACTIVE_CLK_OK                                  0   /* Operation was successful */
#define FMC_ACTIVE_CLK_ERR                                 1   /* Could not set/get value */
#define FMC_ACTIVE_CLK_UNINPL                              2   /* Unimplemented function or operation */
#define FMC_ACTIVE_CLK_REPLY_END                           3   /* End marker */

#endif