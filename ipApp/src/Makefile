TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += ip

DBD += ipSupport.dbd

ip_SRCS += devAiStrParm.cc
ip_SRCS += devAoStrParm.cc
ip_SRCS += devBiStrParm.cc
ip_SRCS += devLiStrParm.cc
ip_SRCS += devLoStrParm.cc
ip_SRCS += devSiStrParm.cc
ip_SRCS += devSoStrParm.cc
ip_SRCS += devAoEurotherm.cc
ip_SRCS += devSoEurotherm.cc
ip_SRCS += devAoEurotherm2k.cc
ip_SRCS += devSoEurotherm2k.cc
ip_SRCS += devSerial.cc
ip_SRCS += devAiHeidND261.cc
ip_SRCS += devAiMKS.cc
ip_SRCS += Keithley2kDMM.st
ip_SRCS += devMPC.cc
#ip_SRCS += mpcServer.cc
ip_SRCS += ipRegister.c

ip_LIBS += mpf mpfEpics 
ip_LIBS += seq pv
ip_LIBS += $(EPICS_BASE_IOC_LIBS)

#BIN_INSTALLS += mpcServer.o

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

