TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += ip

DBDINC += serialRecord
DBD += ipSupport.dbd
DBD += ipVXSupport.dbd
DBD += ip.dbd
DBD += ipVX.dbd

ip_SRCS += serialRecord.c
ip_SRCS += devAiStrParm.cc
ip_SRCS += devAoStrParm.cc
ip_SRCS += devBiStrParm.cc
ip_SRCS += devLiStrParm.cc
ip_SRCS += devLoStrParm.cc
ip_SRCS += devSiStrParm.cc
ip_SRCS += devSoStrParm.cc
ip_SRCS += devAoEurotherm.cc
ip_SRCS += devSoEurotherm.cc
ip_SRCS += devSerial.cc
ip_SRCS += devAiHeidND261.cc
ip_SRCS += devAiMKS.cc
ip_SRCS += Keithley2kDMM.st
ip_SRCS += devMPC.cc
ip_SRCS += ipRegister.c
ip_SRCS_vxWorks += tyGSMPFInit.c

ip_LIBS += mpf mpfEpics 
ip_LIBS += seq pv
ip_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# build an ioc application for testing

PROD_IOC = ipApp

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
ipApp_SRCS += ip_registerRecordDeviceDriver.cpp
ipApp_SRCS_DEFAULT += ipMain.cpp
ipApp_SRCS_vxWorks += -nil-
#The following adds support from base/src/vxWorks
ipApp_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

ipApp_LIBS += ip
ipApp_LIBS += mpf mpfEpics mpfSerial
ipApp_LIBS += seq pv
ipApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

