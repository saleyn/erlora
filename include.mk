## -*- makefile -*-

######################################################################
## C

CC	:= gcc
CFLAGS	:= -g -O2 -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DCPU_VENDOR_OS=\"i686-pc-linux-gnu\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_LINUX_IF_TUN_H=1 -DLINUX=1

LD_SHARED	:= ld -shared
SLANG_INCLUDE	:=

######################################################################
## Erlang

# On windows $(OS) is set, on UNIX $(OSTYPE) is set but must be exported
ifneq (,$(findstring Windows,$(OS)))
  ERLPATH := C:/Progra~1/erl5.4.9/bin
  ERL_TOP := C:/Progra~1/erl5.4.9
else
  ERLPATH := /usr/local/bin
  ERL_TOP := /usr/local/lib/erlang
endif

ERL		:= $(ERLPATH)/erl
ERLC	:= $(ERLPATH)/erlc


ERL_C_INCLUDE_DIR := $(ERL_TOP)/usr/include

ERLC_FLAGS        := -W -I../include
ERLC_BOOT_FLAGS   ?=

ifndef no_debug_info
  ERLC_FLAGS += +debug_info
endif

ifdef debug
  ERLC_FLAGS += -Ddebug
endif

ifdef native
  ERLC_FLAGS += +native
endif

APP_ROOT      ?= ../../..
BIN_DIR       ?= ../bin
EBIN_DIR      ?= ../ebin
DOC_DIR       ?= ../doc
PRIV_DIR      ?= ../priv
MIBS_SRC_DIR  ?= ../mibs
MIBS_BIN_DIR  ?= ../priv/mibs
INSTALL_DIR   ?= $(APP_ROOT)/make
LIB_DIR       ?= $(APP_ROOT)/lib
EMULATOR      := beam
ALL_EBIN_DIRS ?= $(addprefix -pa , $(wildcard $(LIB_DIR)/*/ebin))
ERL_SOURCES   ?= $(filter-out $(ERL_EXCLUDE_MODULES), $(wildcard *.erl))
ERL_HEADERS   ?= $(wildcard *.hrl) $(wildcard ../include/*.hrl)
ERL_OBJECTS   ?= $(ERL_SOURCES:%.erl=$(EBIN_DIR)/%.$(EMULATOR))
ERL_DOCUMENTS ?= $(ERL_SOURCES:%.erl=$(DOC_DIR)/%.html)
ERL_RELEASE   ?= $(firstword $(wildcard *.rel))

ERL_MIBS      ?= $(wildcard $(MIBS_SRC_DIR)/*.mib)
ERL_MIBS_BIN  ?= $(ERL_MIBS:$(MIBS_SRC_DIR)/%.mib=$(MIBS_BIN_DIR)/%.bin)

APPNAME       ?= $(patsubst %.app.src,%,$(wildcard *.app.src))

# This ugly variable is needed to generate the some_application.app file
# from some_application.app.src containing %VSN% and %MODULES% macros.
# This variable is used in targets.mk
APPSCRIPT = '$$vsn=shift; $$mods=""; \
  while(@ARGV){ $$_=shift; s/^([A-Z].*)$$/\'\''$$1\'\''/; $$mods.=", " if $$mods; $$mods .= $$_; } \
  while(<>)   { s/%VSN%/$$vsn/; s/%MODULES%/$$mods/; print; }'

# ALL_TARGET sets default tdependencies for the "all" target in targets.mk
ALL_TARGET    ?= $(ERL_OBJECTS) $(EBIN_DIR)/$(APPNAME).app $(ERL_MIBS_BIN)

# Set CLEAN_TARGET to force cleanup of additional files (used in targets.mk)
CLEAN_TARGET  ?= $(EBIN_DIR)/$(APPNAME).app
