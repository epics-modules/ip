/* ipRegister.c */
/* This file forces the Keithley2kDMM.st object file to be included when the ip library is linked with */

#include "epicsExport.h"
#include "seqCom.h"
extern struct seqProgram Keithley2kDMM;

void ipRegister(void)
{
    seqRegisterSequencerProgram(&Keithley2kDMM);
    seqRegisterSequencerCommands();
}

epicsExportRegistrar(ipRegister);
