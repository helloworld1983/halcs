# Set your cross compile prefix with CROSS_COMPILE variable
CROSS_COMPILE ?=

CMDSEP = ;

CC =		$(CROSS_COMPILE)gcc
AR =		$(CROSS_COMPILE)ar
LD =		$(CROSS_COMPILE)ld
OBJDUMP =	$(CROSS_COMPILE)objdump
OBJCOPY =	$(CROSS_COMPILE)objcopy
SIZE =		$(CROSS_COMPILE)size
MAKE =		make

PWD = 		$(shell pwd)

# Select board in which we will work. Options are: ml605 or afcv3
BOARD ?= ml605

INSTALL_DIR ?= /usr/local
export INSTALL_DIR

LIBBPMCLIENT = libbpmclient

# Config variables suitable for creating shared libraries
LIB_VER_MAJOR = 1
LIB_VER_MINOR = 0
LIB_VER_REVISION = 0
LIB_VER = $(LIB_VER_MAJOR).$(LIB_VER_MINOR).$(LIB_VER_REVISION)

# General C flags
CFLAGS = -std=gnu99 -O2 -fPIC

ifeq ($(BOARD),ml605)
CFLAGS += -D__BOARD_ML605__
endif

ifeq ($(BOARD),afcv3)
CFLAGS += -D__BOARD_AFCV3__
endif

CFLAGS_DEBUG =

# To enable this option, use: make ERRHAND_DBG=y
ifneq ($(ERRHAND_DBG),)
CFLAGS_DEBUG += -DERRHAND_DBG=$(ERRHAND_DBG)
endif

# To enable this option use: make ERRHAND_MIN_LEVEL=DBG_MIN_TRACE
ifneq ($(ERRHAND_MIN_LEVEL),)
CFLAGS_DEBUG += -DERRHAND_MIN_LEVEL=$(ERRHAND_MIN_LEVEL)
endif

# To enable this option use: make ERRHAND_SUBSYS_ON='"(DBG_DEV_MNGR | \
# DBG_DEV_IO | DBG_SM_IO | DBG_LIB_CLIENT  | DBG_SM_PR | DBG_SM_CH | DBG_LL_IO | DBG_HAL_UTILS)"'
#
# You can also OR the available subsytems to enable debug messages in just the
# those subsytems. See file errhand_opts.h for more information
ifneq ($(ERRHAND_SUBSYS_ON),)
CFLAGS_DEBUG += -DERRHAND_SUBSYS_ON=$(ERRHAND_SUBSYS_ON)
endif

# Debug flags -D<flasg_name>=<value>
CFLAGS_DEBUG += -g

# Specific platform Flags
CFLAGS_PLATFORM = -Wall -Wextra -Werror
LDFLAGS_PLATFORM =

# Libraries
LIBS = -lerrhand -lzmq -lczmq -lmdp

# General library flags -L<libdir>
LFLAGS =

# Specific board objects
OBJS_BOARD = ../../sm_io/modules/acq/ddr3_map.o
# Specific platform objects
OBJS_PLATFORM =
# Objects that are not specificaly related to the bpm_client, but composes
# our library, as it eases some tasks for us
OBJS_EXTERNAL = ../../sm_io/modules/sm_io_codes.o \
                ../../sm_io/modules/acq/sm_io_acq_exports.o \
                ../../sm_io/modules/dsp/sm_io_dsp_exports.o \
                ../../sm_io/modules/fmc130m_4ch/sm_io_fmc130m_4ch_exports.o \
                ../../sm_io/modules/swap/sm_io_swap_exports.o \
                ../../sm_io/modules/rffe/sm_io_rffe_exports.o \
                ../../sm_io/modules/afc_diag/sm_io_afc_diag_exports.o

# Project boards
boards_INCLUDE_DIRS = -I../../../include/boards/$(BOARD)

# Include directories
INCLUDE_DIRS = -I. -I../../../include -I../liberrhand -I../libhutils -I../libdisptable \
	       $(boards_INCLUDE_DIRS) \
	       -I../../sm_io/modules \
	       -I../../sm_io/modules/fmc130m_4ch \
	       -I../../sm_io/modules/acq \
	       -I../../sm_io/modules/swap \
	       -I../../sm_io/modules/dsp \
	       -I../../sm_io/modules/rffe \
	       -I../../sm_io/modules/afc_diag \
	       -I/usr/local/include

# Merge all flags. Optimize for size (-Os)
CFLAGS += $(CFLAGS_PLATFORM) $(CFLAGS_DEBUG)

LDFLAGS = $(LDFLAGS_PLATFORM)

# Output library names
OUT = $(LIBBPMCLIENT)

.SECONDEXPANSION:

# Library objects
$(LIBBPMCLIENT)_OBJS_LIB = bpm_client.o bpm_client_err.o bpm_client_rw_param.o

# Objects common for both server and client libraries.
common_OBJS = $(OBJS_BOARD) $(OBJS_PLATFORM) $(OBJS_EXTERNAL)

# Objects for each version of library
$(LIBBPMCLIENT)_OBJS = $(common_OBJS) $($(LIBBPMCLIENT)_OBJS_LIB)
$(LIBBPMCLIENT)_CODE_HEADERS = ../../sm_io/modules/fmc130m_4ch/sm_io_fmc130m_4ch_codes.h \
	../../sm_io/modules/acq/sm_io_acq_codes.h \
	../../sm_io/modules/dsp/sm_io_dsp_codes.h \
	../../sm_io/modules/swap/sm_io_swap_codes.h \
	../../sm_io/modules/rffe/sm_io_rffe_codes.h \
	../../sm_io/modules/afc_diag/sm_io_afc_diag_codes.h \
	../../sm_io/modules/sm_io_codes.h \
	../../../include/acq_chan_gen_defs.h \
	../libdisptable/disp_table.h \
	../libdisptable/disp_table_err.h \
	../libhutils/hutils.h \
	../libhutils/hutils_err.h \
	../libhutils/hutils_math.h \
	../libhutils/hutils_utils.h \
	bpm_client_codes.h

$(LIBBPMCLIENT)_FUNC_EXPORTS = ../../sm_io/modules/fmc130m_4ch/sm_io_fmc130m_4ch_exports.h \
	../../sm_io/modules/acq/sm_io_acq_exports.h \
	../../sm_io/modules/dsp/sm_io_dsp_exports.h \
	../../sm_io/modules/swap/sm_io_swap_exports.h \
	../../sm_io/modules/rffe/sm_io_rffe_exports.h \
	../../sm_io/modules/afc_diag/sm_io_afc_diag_exports.h

$(LIBBPMCLIENT)_ACQ_HEADERS += ../../../include/acq_chan.h

$(LIBBPMCLIENT)_HEADERS = $($(LIBBPMCLIENT)_OBJS_LIB:.o=.h) $($(LIBBPMCLIENT)_CODE_HEADERS) \
    $($(LIBBPMCLIENT)_FUNC_EXPORTS)

# Installation headers
$(LIBBPMCLIENT)_INST_HEADERS = $($(LIBBPMCLIENT)_HEADERS)

# Save a git repository description
GIT_REVISION = $(shell git describe --dirty --always)
GIT_USER_NAME = $(shell git config --get user.name)
GIT_USER_EMAIL = $(shell git config --get user.email)

REVISION_NAME = bpm_client_revision
OBJ_REVISION = $(addsuffix .o, $(REVISION_NAME))

OBJS_all = $(common_OBJS) $($(LIBBPMCLIENT)_OBJS) \
	   $(OBJ_REVISION)

# Libraries suffixes
LIB_STATIC_SUFFIX = .a
LIB_SHARED_SUFFIX = .so

# Generate suitable names for static libraries
TARGET_STATIC = $(addsuffix $(LIB_STATIC_SUFFIX), $(OUT))
TARGET_SHARED = $(addsuffix $(LIB_SHARED_SUFFIX), $(OUT))
TARGET_SHARED_VER = $(addsuffix $(LIB_SHARED_SUFFIX).$(LIB_VER), $(OUT))