/*
 * Copyright (C) 2015 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU LGPL, version 3 or any later version.
 */

#include <libgen.h>
#include "bpm_server.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...)  \
    ASSERT_HAL_TEST(test_boolean, DEV_IO, "[dev_io]",               \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)       \
    ASSERT_HAL_ALLOC(ptr, DEV_IO, "[dev_io]",                       \
            devio_err_str (DEVIO_ERR_ALLOC),                        \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                    \
    CHECK_HAL_ERR(err, DEV_IO, "[dev_io]",                          \
            devio_err_str (err_type))

#ifdef __CFG_DIR__
#define CFG_DIR                     STRINGIFY(__CFG_DIR__)
#else
#error "Config directory not defined!"
#endif

#ifdef __CFG_FILENAME__
#define CFG_FILENAME                STRINGIFY(__CFG_FILENAME__)
#else
#error "Config filename not defined!"
#endif

#define LOG_FILENAME_LEN            50
/* This composes the log filename as "dev_io%u_fe%u.log" or
 * "dev_io%u_be%u.log" */
#define DEVIO_LOG_RADICAL_PATTERN   "dev_io%u"
#define DEVIO_LOG_DEVIO_MODEL_TYPE  "%s"
#define DEVIO_LOG_INST_TYPE         "%u"
#define DEVIO_LOG_SUFFIX            "log"
#define DEVIO_LOG_FILENAME_PATTERN \
                                    DEVIO_LOG_RADICAL_PATTERN "_" \
                                    DEVIO_LOG_DEVIO_MODEL_TYPE \
                                    DEVIO_LOG_INST_TYPE "." \
                                    DEVIO_LOG_SUFFIX

/* Arbitrary hard limit for the maximum number of AFE DEVIOs
 * for each DBE DEVIO */
#define DEVIO_MAX_FE_DEVIOS         16

#define DEVIO_SERVICE_LEN           50
#define DEVIO_NAME                  "dev_io"
#define DEVIO_CFG_NAME              "dev_io_cfg"

#define DEVIO_LIBBPMCLIENT_LOG_MODE    "a"
#define DEVIO_KILL_CFG_SIGNAL       SIGINT

static devio_err_e _spawn_assoc_devios (devio_t *devio, uint32_t dev_id,
        devio_type_e devio_type, char *broker_endp, char *log_file_name,
        zhash_t *hints);
static devio_err_e _spawn_rffe_devios (devio_t *devio, uint32_t dev_id,
        char *broker_endp, char *log_file_name, zhash_t *hints);
static devio_err_e _spawn_platform_smios (devio_t *devio, devio_type_e devio_type,
        uint32_t smio_inst_id);
static devio_err_e _spawn_be_platform_smios (devio_t *devio);
static devio_err_e _spawn_fe_platform_smios (devio_t *devio, uint32_t smio_inst_id);

void print_help (char *program_name)
{
    printf( "BPM Device I/O\n"
            "Usage: %s [options]\n"
            "\t-h This help message\n"
            "\t-d Daemon mode.\n"
            "\t-v Verbose output\n"
            "\t-n <devio_type = [be|fe]> Devio type\n"
            "\t-t <device_type = [eth|pcie]> Device type\n"
            "\t-e <dev_entry = [ip_addr|/dev entry]> Device entry\n"
            "\t-i <dev_id> Device ID\n"
            "\t-s <fe_smio_id> FE SMIO ID (only valid for devio_type = fe)\n"
            "\t-l <log_filename> Log filename\n"
            "\t-b <broker_endpoint> Broker endpoint\n", program_name);
}

int main (int argc, char *argv[])
{
    int verbose = 0;
    int daemonize = 0;
    char *devio_type_str = NULL;
    char *dev_type = NULL;
    char *dev_entry = NULL;
    char *dev_id_str = NULL;
    char *fe_smio_id_str = NULL;
    char *broker_endp = NULL;
    char *log_file_name = NULL;
    char **str_p = NULL;
    int i;

    if (argc < 5) {
        print_help (argv[0]);
        exit (1);
    }

    /* FIXME: This is rather buggy! */
    /* Simple handling of command-line options. This should be done
     * with getopt, for instance*/
    for (i = 1; i < argc; i++)
    {
        if (streq (argv[i], "-v")) {
            verbose = 1;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Verbose mode set\n");
        }
        else if (streq (argv[i], "-d")) {
            daemonize = 1;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Demonize mode set\n");
        }
        else if (streq (argv[i], "-n")) {
            str_p = &devio_type_str;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set devio_type parameter\n");
        }
        else if (streq (argv[i], "-t")) {
            str_p = &dev_type;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set dev_type parameter\n");
        }
        else if (streq (argv[i], "-e")) {
            str_p = &dev_entry;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set dev_entry parameter\n");
        }
        else if (streq (argv[i], "-i")) {
            str_p = &dev_id_str;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set dev_id_str parameter\n");
        }
        else if (streq (argv[i], "-s")) {
            str_p = &fe_smio_id_str;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set fe_smio_id_str parameter\n");
        }
        else if (streq (argv[i], "-b")) {
            str_p = &broker_endp;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set broker_endp parameter\n");
        }
        else if (streq (argv[i], "-l")) {
            str_p = &log_file_name;
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Will set log filename\n");
        }
        else if (streq (argv[i], "-h")) {
            print_help (argv[0]);
            exit (1);
        }
        /* Fallout for options with parameters */
        else {
            if (str_p) {
                *str_p = strdup (argv[i]);
                DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Parameter set to \"%s\"\n", *str_p);
            }
        }
    }

    /* Daemonize dev_io */
    if (daemonize != 0) {
        int rc = daemon(0, 0);

        if (rc != 0) {
            perror ("[dev_io] daemon");
            goto err_exit;
        }
    }

    llio_type_e llio_type = llio_str_to_type (dev_type);
    /* Parse command-line options */
    if (llio_type == INVALID_DEV) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Dev_type parameter is invalid\n");
        goto err_exit;
    }

    devio_type_e devio_type = devio_str_to_type (devio_type_str);
    /* Parse command-line options */
    if (devio_type == INVALID_DEVIO) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Devio_type parameter is invalid\n");
        goto err_exit;
    }

    /* Check for device entry */
    if (dev_entry == NULL) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Dev_entry parameter was not set. Exiting ...\n");
        goto err_exit;
    }

    uint32_t dev_id = 0;
    /* Check for device ID */
    if (dev_id_str == NULL) {
        switch (llio_type) {
            case ETH_DEV:
                DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Dev_id parameter was not set. Exiting ...\n");
                goto err_exit;
                break;

            case PCIE_DEV:
                DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Dev_id parameter was not set.\n"
                        "\tDefaulting it to the /dev file number ...\n");

                int matches = sscanf (dev_entry, "/dev/fpga%u", &dev_id);
                if (matches == 0) {
                    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Dev_entry parameter is invalid.\n"
                            "\tIt must be in the format \"/dev/fpga<device_number>\". Exiting ...\n");
                    goto err_exit;
                }

            default:
                DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Dev_id parameter was not set. Exiting ...\n");
                goto err_exit;
        }
    }
    /* Use the passed ID */
    else {
        dev_id = strtoul (dev_id_str, NULL, 10);
    }

    uint32_t fe_smio_id = 0;
    /* Check for FE SMIO ID */
    if (devio_type == FE_DEVIO && fe_smio_id_str == NULL) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Fe_smio_id parameter was not set. Exiting ...\n");
        goto err_exit;
    }
    else {
        fe_smio_id = strtoul (fe_smio_id_str, NULL, 10);
    }

    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Dev_id parameter was set to %u.\n",
            dev_id);

    /* Spawn the Configure DEVIO to get the uTCA slot number. This is only
     * available in AFCv3 */

#if defined (__BOARD_AFCV3__) && (__WITH_DEVIO_CFG__)
    int child_devio_cfg_pid = 0;
    if (llio_type == PCIE_DEV) {
        /* Argument options are "process name", "device type" and
         *"dev entry" */
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Spawing DEVIO Config\n");
        char *argv_exec [] = {DEVIO_CFG_NAME, "-n", devio_type_str,"-t", dev_type,
            "-i", dev_id_str, "-e", dev_entry, "-b", broker_endp, NULL};
        /* Spawn Config DEVIO */
        child_devio_cfg_pid = hutils_spawn_chld (DEVIO_CFG_NAME, argv_exec);

        if (child_devio_cfg_pid < 0) {
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Could not create "
                    "DEVIO Config instance\n");
            goto err_exit;
        }
    }

    /* At this point, the Config DEVIO is ready to receive our commands */
    char devio_config_service_str [DEVIO_SERVICE_LEN];
    snprintf (devio_config_service_str, DEVIO_SERVICE_LEN-1, "BPM%u:DEVIO:AFC_DIAG%u",
            dev_id, 0);
    devio_config_service_str [DEVIO_SERVICE_LEN-1] = '\0'; /* Just in case ... */

    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Creating libclient for DEVIO config\n");
    bpm_client_err_e client_err = BPM_CLIENT_SUCCESS;

    bpm_client_t *client_cfg = bpm_client_new_log_mode (broker_endp, 0,
            log_file_name, DEVIO_LIBBPMCLIENT_LOG_MODE);

    if (client_cfg == NULL) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Could not create "
                "DEVIO Config libclient instance\n");
        goto err_client_cfg;
    }

    /* Get uTCA card slot number */
    client_err = bpm_get_afc_diag_card_slot (client_cfg, devio_config_service_str,
            &dev_id);

    if (client_err != BPM_CLIENT_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Could not retrieve "
                "slot number. Unsupported board?\n");
        goto err_card_slot;
    }

    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Slot number: 0x%08X\n", dev_id);

    if (llio_type == PCIE_DEV) {
        /* FIXME: give some time for the process to terminate gracefully */
        sleep (5);
        /* Kill DEVIO cfg as we've already got our slot number */
        if (child_devio_cfg_pid > 0) {
            kill (child_devio_cfg_pid, DEVIO_KILL_CFG_SIGNAL);
        }
        /* Wait child */
        hutils_wait_chld ();
    }
    /* Destroy libclient */
    bpm_client_destroy (&client_cfg);
#endif

    /* We don't need it anymore */
    str_p = &fe_smio_id_str;
    free (*str_p);
    fe_smio_id_str = NULL;
    str_p = &dev_type;
    free (*str_p);
    dev_type = NULL;
    str_p = &dev_id_str;
    free (*str_p);
    dev_id_str = NULL;

    /* Initilialize dev_io */
    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_TRACE, "[dev_io] Creating DEVIO instance ...\n");

    char devio_service_str [DEVIO_SERVICE_LEN];
    snprintf (devio_service_str, DEVIO_SERVICE_LEN-1, "BPM%u:DEVIO", dev_id);
    devio_service_str [DEVIO_SERVICE_LEN-1] = '\0'; /* Just in case ... */
    devio_t *devio = devio_new (devio_service_str, dev_id, dev_entry, llio_type,
            broker_endp, verbose, log_file_name);

    if (devio == NULL) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_new error!\n");
        goto err_card_slot;
    }

    /* We don't need it anymore */
    str_p = &dev_entry;
    free (*str_p);
    dev_entry = NULL;
    str_p = &broker_endp;
    free (*str_p);
    broker_endp = NULL;

    /* Print SDB devices */
    devio_print_info (devio);

    zhash_t *devio_hints = zhash_new ();
    if (devio_hints == NULL) {
        DBE_DEBUG (DBG_DEV_MNGR | DBG_LVL_FATAL, "[dev_io] Could allocate "
                "hints hash table\n");
        goto err_devio_hints_alloc;
    }

    /**************************************************************************/
    /************ Read configuration variables from config file ***************/
    /**************************************************************************/

    /* Check for field not found */
    zconfig_t *root_cfg = zconfig_load (CFG_DIR "/" CFG_FILENAME);
    if (root_cfg == NULL) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Could not load "
                "configuration file\n");
        goto err_cfg_load;
    }

    /* Read DEVIO suggested bind endpoints and fill the hash table with
     * the corresponding keys */
    hutils_err_e herr = hutils_get_hints (root_cfg, devio_hints);
    if (herr != HUTILS_SUCCESS) {
        DBE_DEBUG (DBG_DEV_MNGR | DBG_LVL_FATAL, "[dev_mngr] Could not get hints "
                "from configuration file\n");
        goto err_cfg_get_hints;
    }

    /* Spawn associated DEVIOs */
    devio_err_e err = _spawn_assoc_devios (devio, dev_id, devio_type,
            broker_endp, log_file_name, devio_hints);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Could not spawn "
                "associated DEVIOs!\n");
        goto err_assoc_devio;
    }

    /* Spawn platform SMIOSs */
    err = _spawn_platform_smios (devio, devio_type, fe_smio_id);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] _spawn_platform_smios error!\n");
        goto err_devio;
    }

    /* Step 1: Loop though all the SDB records and intialize (boot) the
     * smio modules*/
    /* Step 2: Optionally, register the necessary smio modules specifying
     * its ID and calling devio_register_sm */
    /* Step 3: Poll all PIPE's sockets to determine if we have something to
     * handle, like messages from smios */
    /*      Step 3.5: If we do, call devio_handle_smio () and treat its
     *      request as appropriate */
    err = devio_loop (devio);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_loop error: %s\n",
                devio_err_str (err));
        goto err_devio;
    }

err_devio:
err_assoc_devio:
err_cfg_get_hints:
    zconfig_destroy (&root_cfg);
err_cfg_load:
    zhash_destroy (&devio_hints);
err_devio_hints_alloc:
    /* wait child, if any */
    hutils_wait_chld ();
    devio_destroy (&devio);
err_card_slot:
#if defined (__BOARD_AFCV3__) && (__WITH_DEVIO_CFG__)
    bpm_client_destroy (&client_cfg);
err_client_cfg:
    /* Kill DEVIO Config process */
    kill (child_devio_cfg_pid, DEVIO_KILL_CFG_SIGNAL);
#endif
err_exit:
    str_p = &log_file_name;
    free (*str_p);
    str_p = &fe_smio_id_str;
    free (*str_p);
    str_p = &broker_endp;
    free (*str_p);
    str_p = &dev_id_str;
    free (*str_p);
    str_p = &dev_entry;
    free (*str_p);
    str_p = &dev_type;
    free (*str_p);
    str_p = &devio_type_str;
    free (*str_p);
    return 0;
}

static devio_err_e _spawn_assoc_devios (devio_t *devio, uint32_t dev_id,
        devio_type_e devio_type, char *broker_endp, char *log_file_name,
        zhash_t *hints)
{
    assert (devio);

    devio_err_e err = DEVIO_SUCCESS;

    switch (devio_type) {
        case BE_DEVIO:
            err = _spawn_rffe_devios (devio, dev_id, broker_endp, log_file_name,
                    hints);
            break;

        case FE_DEVIO:
            /* Nothing to do */
            break;

        default:
            /* FIXME: increase the error clarity? */
            err = DEVIO_ERR_NO_SMIO_ID;
    }

    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] spawn_assoc_devios error!\n");
        goto err_spwan_assoc_devios;
    }

err_spwan_assoc_devios:
    return err;
}

static devio_err_e _spawn_rffe_devios (devio_t *devio, uint32_t dev_id,
        char *broker_endp, char *log_file_name, zhash_t *hints)
{
    assert (devio);
    assert (hints);

    devio_err_e err = DEVIO_SUCCESS;
    char *dev_id_c = NULL;
    char *smio_inst_id_c = NULL;

    /* For each DEVIO, spawn up to 2 RFFE DEVIOs. Do a lookup in our
     * hints hash to look for endpoints to bind to */
    uint32_t j;
    for (j = 0; j < DEVIO_MAX_FE_DEVIOS; ++j) {
        char hints_key [HUTILS_CFG_HASH_KEY_MAX_LEN];
        uint32_t smio_inst_id = j;
        int errs = snprintf (hints_key, sizeof (hints_key),
                HUTILS_CFG_HASH_KEY_PATTERN_COMPL, dev_id, smio_inst_id,
                HUTILS_CFG_AFE);

        /* Only when the number of characters written is less than the whole buffer,
         * it is guaranteed that the string was written successfully */
        ASSERT_TEST (errs >= 0 && (size_t) errs < sizeof (hints_key),
                "Could not generate AFE bind address from configuration "
                "file", err_cfg_exit, DMNGR_ERR_CFG);

        char *endpoint_fe = zhash_lookup (hints, hints_key);
        /* If key is not found, assume we don't have any more AFE to
         * prepare */
        if (endpoint_fe == NULL || streq (endpoint_fe, "")) {
            continue;
        }

        /* Set up logdir */
        char devio_log_filename[LOG_FILENAME_LEN] = "stdout";
        /* Get dirname of the parent DEVIO. Don't free it as written in the
         * manpage */
        char *devio_log_prefix = dirname (log_file_name);

        /* TODO: Check for the validity of the log filename */
        if (devio_log_prefix != NULL) {
            snprintf (devio_log_filename, LOG_FILENAME_LEN,
                    "%s/"DEVIO_LOG_FILENAME_PATTERN, devio_log_prefix,
                    dev_id, FE_DEVIO_STR, smio_inst_id);
        }

        /* Stringify parameters */
        dev_id_c = hutils_stringify_dec_key (dev_id);
        ASSERT_ALLOC (dev_id_c, err_dev_id_c_alloc, DEVIO_ERR_ALLOC);
        smio_inst_id_c = hutils_stringify_dec_key (smio_inst_id);
        ASSERT_ALLOC (smio_inst_id_c, err_smio_inst_id_c_alloc, DEVIO_ERR_ALLOC);

        /* Spawn DEVIO RFFE */
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Spawing DEVIO RFFE\n");
        char *argv_exec [] = {DEVIO_NAME, "-n", FE_DEVIO_STR,"-t", ETH_DEV_STR,
            "-i", dev_id_c, "-e", endpoint_fe, "-s", smio_inst_id_c,
            "-b", broker_endp, "-l", devio_log_filename, NULL};
        /* Spawn Config DEVIO */
        int child_devio_cfg_pid = hutils_spawn_chld (DEVIO_NAME, argv_exec);

        if (child_devio_cfg_pid < 0) {
            DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] Could not create "
                    "DEVIO RFFE instance\n");
            goto err_spawn;
        }

        free (dev_id_c);
        dev_id_c = NULL;
        free (smio_inst_id_c);
        smio_inst_id_c = NULL;
    }

err_spawn:
    free (smio_inst_id_c);
err_smio_inst_id_c_alloc:
    free (dev_id_c);
err_dev_id_c_alloc:
err_cfg_exit:
    return err;
}

static devio_err_e _spawn_platform_smios (devio_t *devio, devio_type_e devio_type,
        uint32_t smio_inst_id)
{
    assert (devio);

    devio_err_e err = DEVIO_SUCCESS;

    switch (devio_type) {
        case BE_DEVIO:
            err = _spawn_be_platform_smios (devio);
            break;

        case FE_DEVIO:
            err = _spawn_fe_platform_smios (devio, smio_inst_id);
            break;

        default:
            /* FIXME: increase the error clarity? */
            err = DEVIO_ERR_NO_SMIO_ID;
    }

    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

err_register_sm:
    return err;
}

static devio_err_e _spawn_be_platform_smios (devio_t *devio)
{
    uint32_t fmc130m_4ch_id = 0x7085ef15;
    uint32_t acq_id = 0x4519a0ad;
    uint32_t dsp_id = 0x1bafbf1e;
    uint32_t swap_id = 0x12897592;
    devio_err_e err = DEVIO_SUCCESS;

    /* ML605 or AFCv3 */
#if defined (__BOARD_ML605__) || (__BOARD_AFCV3__)
    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Spawning default SMIOs ...\n");
    err = devio_register_sm (devio, fmc130m_4ch_id, FMC1_130M_BASE_ADDR, 0);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, acq_id, WB_ACQ1_BASE_ADDR, 0);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, dsp_id, DSP1_BASE_ADDR, 0);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, swap_id, DSP1_BASE_ADDR, 0);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }
    /* AFCv3 spefific */
#if defined (__BOARD_AFCV3__)
    uint32_t afc_diag_id = 0x51954750;

    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io] Spawning AFCv3 specific SMIOs ...\n");

    err = devio_register_sm (devio, fmc130m_4ch_id, FMC2_130M_BASE_ADDR, 1);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, acq_id, WB_ACQ2_BASE_ADDR, 1);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, dsp_id, DSP2_BASE_ADDR, 1);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, swap_id, DSP2_BASE_ADDR, 1);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }

    err = devio_register_sm (devio, afc_diag_id, WB_AFC_DIAG_BASE_ADDR, 0);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io] devio_register_sm error!\n");
        goto err_register_sm;
    }
#endif
#else
#error "BE FPGA Board not supported!"
#endif

err_register_sm:
    return err;
}

static devio_err_e _spawn_fe_platform_smios (devio_t *devio, uint32_t smio_inst_id)
{
    uint32_t rffe_id = 0x7af21909;
    devio_err_e err = DEVIO_SUCCESS;

    /* RFFE V2 only */
    /* #if defined (__AFE_RFFE_V1__) */
#if defined (__AFE_RFFE_V2__)
    DBE_DEBUG (DBG_DEV_IO | DBG_LVL_INFO, "[dev_io_fe] Spawning default SMIOs ...\n");
    err = devio_register_sm (devio, rffe_id, 0, smio_inst_id);
    if (err != DEVIO_SUCCESS) {
        DBE_DEBUG (DBG_DEV_IO | DBG_LVL_FATAL, "[dev_io_fe] devio_register_sm error!\n");
        goto err_register_sm;
    }
#else
#error "FE RFFE Board not supported!"
#endif

err_register_sm:
    return err;
}
