TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application for testing

PROD_IOC = ipExample

DBD += ipExample.dbd
ipExample_DBD += base.dbd

# Modules without cfg support -- manual declarations
ifdef ASYN
ipExample_DBD += asyn.dbd
ipExample_DBD += drvAsynSerialPort.dbd
ipExample_DBD += drvAsynIPPort.dbd
ipExample_DBD += devEpics.dbd
ipExample_DBD += devGpib.dbd
ipExample_LIBS += asyn
endif

ifdef SNCSEQ
ipExample_LIBS += seq pv
endif

# ip module -- cfg-based declarations
ipExample_DBD += $(IP_IOC_DBDS)
ipExample_LIBS += $(IP_IOC_LIBS)

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
ipExample_SRCS += ipExample_registerRecordDeviceDriver.cpp
ifneq ($(OS_CLASS),vxWorks)
ipExample_SRCS += ipExampleMain.cpp
endif

#The following adds support from base/src/vxWorks
ipExample_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

ipExample_LIBS += $(EPICS_BASE_IOC_LIBS)

ipExample_SYS_LIBS_solaris += socket

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

