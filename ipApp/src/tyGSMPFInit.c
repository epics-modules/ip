#include <epicsExport.h>
#include <iocsh.h>

/* There are no include files and/or they are not installed for the following definitions.  Must be careful to make
 * sure these following remain consistent with the source files
 */

int tyGSOctalDevCreate(char *, int, int, int, int);
void tyGSOctalConfig(int, unsigned int, char,
                     int, int, char);
int initTtyVxPort(const char *portName, const char *fileName,
    int baud, int bufsize);
int initSerialServer(const char *serverName,const char *portName,
    int bufsize, int queueSize, const char *eomstr);

int tyGSMPFInitBuffsize = 1000;
epicsExportAddress(int, tyGSMPFInitBuffsize);
int tyGSMPFInitQueuesize = 50;
epicsExportAddress(int, tyGSMPFInitQueuesize);

int tyGSMPFInit(char *server, int uart, int channel, int baud, char parity, int sbits, int dbits, char handshake, char *eomstr)
{

int port;
int status;

port  = tyGSOctalDevCreate(server, uart, channel, tyGSMPFInitBuffsize, tyGSMPFInitBuffsize);
if (port == ERROR) return(ERROR);
tyGSOctalConfig(port,  baud, parity, sbits, dbits, handshake);
status = initTtyVxPort(server, server,  baud, tyGSMPFInitBuffsize);
if (status != 0) return(status);
status = initSerialServer(server, server, tyGSMPFInitBuffsize, tyGSMPFInitQueuesize, eomstr);
if (status != 0) return(status);
return 0;
}


static const iocshArg tyGSMPFInitArg0 = { "serverName",iocshArgString};
static const iocshArg tyGSMPFInitArg1 = { "UART index",iocshArgInt};
static const iocshArg tyGSMPFInitArg2 = { "UART channel",iocshArgInt};
static const iocshArg tyGSMPFInitArg3 = { "baud",iocshArgInt};
static const iocshArg tyGSMPFInitArg4 = { "parity",iocshArgInt};
static const iocshArg tyGSMPFInitArg5 = { "stop bits",iocshArgInt};
static const iocshArg tyGSMPFInitArg6 = { "data bits",iocshArgInt};
static const iocshArg tyGSMPFInitArg7 = { "handshake",iocshArgInt};
static const iocshArg tyGSMPFInitArg8 = { "EOM string",iocshArgString};
static const iocshArg * const tyGSMPFInitArgs[9] = {
    &tyGSMPFInitArg0,&tyGSMPFInitArg1,&tyGSMPFInitArg2,
    &tyGSMPFInitArg3,&tyGSMPFInitArg4,&tyGSMPFInitArg5,
    &tyGSMPFInitArg6,&tyGSMPFInitArg7,&tyGSMPFInitArg8};
static const iocshFuncDef tyGSMPFInitFuncDef = {
    "tyGSMPFinit",9,tyGSMPFInitArgs};
static void tyGSMPFInitCallFunc(const iocshArgBuf *args)
{
    tyGSMPFInit(
        args[0].sval,args[1].ival,args[2].ival,args[3].ival,
        args[4].ival,args[5].ival,args[6].ival,args[7].ival,args[8].sval);
}

static void tyGSMPFInitRegister(void)
{
    iocshRegister(&tyGSMPFInitFuncDef,tyGSMPFInitCallFunc);
}
epicsExportRegistrar(tyGSMPFInitRegister);

