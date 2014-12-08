TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application for testing

PROD_IOC = ipExample

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
ipExample_SRCS_DEFAULT += ipExample_registerRecordDeviceDriver.cpp ipExampleMain.cpp
ipExample_SRCS_vxWorks += ipExampleVX_registerRecordDeviceDriver.cpp

#The following adds support from base/src/vxWorks
ipExample_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

ipExample_LIBS += ip
ipExample_LIBS += asyn
ipExample_LIBS += seq pv
ipExample_LIBS += $(EPICS_BASE_IOC_LIBS)

ipExample_SYS_LIBS_solaris += socket

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

