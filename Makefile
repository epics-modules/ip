#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS += configure ipApp
ipApp_DEPEND_DIRS   = configure

ifeq ($(BUILD_IOCS), YES)
DIRS += iocs
iocs_DEPEND_DIRS += ipApp
endif

include $(TOP)/configure/RULES_TOP

uninstall: uninstall_iocs
uninstall_iocs:
	$(MAKE) -C iocs uninstall
.PHONY: uninstall uninstall_iocs

realuninstall: realuninstall_iocs
realuninstall_iocs:
	$(MAKE) -C iocs realuninstall
.PHONY: realuninstall realuninstall_iocs
