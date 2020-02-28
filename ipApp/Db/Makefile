TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
#DB += DB += xxx.db

DB += $(patsubst ../%, %, $(wildcard ../*.template))
DB += $(patsubst ../%, %, $(wildcard ../*.db))
DB += $(patsubst ../%, %, $(wildcard ../*.vdb))

DB += $(patsubst ../%, %, $(wildcard ../*.proto))
DB += $(patsubst ../%, %, $(wildcard ../*.protocol))

REQ += $(patsubst ../%, %, $(wildcard ../*.req))

#----------------------------------------------------
# If <anyname>DB += .db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

ifeq ($(BASE_3_14),YES)
# Only needed for 3.14 build rules:
vpath %.proto $(USR_VPATH) $(GENERIC_SRC_DIRS)
#vpath %.protocol $(USR_VPATH) $(GENERIC_SRC_DIRS)
#vpath %.req $(USR_VPATH) $(GENERIC_SRC_DIRS)
endif
