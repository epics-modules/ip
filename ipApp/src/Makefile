TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += ip

DBD += ipSupport.dbd
DBD += ipVXSupport.dbd
DBD += ip.dbd
DBD += ipVX.dbd

ip_SRCS += devXxStrParm.c
ip_SRCS += devXxEurotherm.c
ip_SRCS += devAiHeidND261.c
ip_SRCS += devAiMKS.c
ip_SRCS += Keithley2kDMM.st
ip_SRCS += Keithley65xxEM.st
ip_SRCS += Keithley2kDMM_mf40.st
ip_SRCS += Federal.st
ip_SRCS += devMPC.c
ip_SRCS += devGP307gpib.c  
ip_SRCS += devXxHeidenhainGpib.c
ip_SRCS += devXxAX301.c    
ip_SRCS += devXxKeithleyDMM199Gpib.c 
ip_SRCS_vxWorks += tyGSAsynInit.c
ip_SRCS += devTelevac.c
ip_SRCS += devTPG261.c

ip_LIBS += asyn
ip_LIBS += seq pv
ip_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

