#include <epicsExport.h>
#include <string.h>
#include <stdio.h>
#include <asynShellCommands.h>
#include <drvAsynSerialPort.h>
#include <iocsh.h>

/* There are no include files and/or they are not installed for the following 
 * definitions.  
 * Must be careful to make sure these following remain consistent with the 
 * source files
 */

int tyGSOctalDevCreate(char *, int, int, int, int);
int initSerialServer(const char *serverName,
                     const char *portName, int addr, int bufsize, 
                     int queueSize, 
                     const char *eomstr);

int tyGSMPFInitBuffsize = 1000;
epicsExportAddress(int, tyGSMPFInitBuffsize);
int tyGSMPFInitQueuesize = 50;
epicsExportAddress(int, tyGSMPFInitQueuesize);

int tyGSMPFInit(char *port, int uart, int channel, int baud, char parity, 
                int sbits, int dbits, char handshake, char *eomstr)
{

   int index;
   int status;
   #define SIZE 80
   char buffer[SIZE];

   index  = tyGSOctalDevCreate(port, uart, channel, tyGSMPFInitBuffsize, 
                               tyGSMPFInitBuffsize);
   if (index == ERROR) return(ERROR);
   drvAsynSerialPortConfigure(port, port, 0, 0, 0);

   asynConnect(port, port, 0, eomstr, eomstr,  1, 128);

   /* Port options */
   sprintf(buffer, "%d", baud);
   status = asynSetOption(port, 0, "baud", buffer);
   if (status != 0) return(status);

   if (parity == 'N') strcpy(buffer,"none");
   if (parity == 'E') strcpy(buffer,"even");
   if (parity == 'O') strcpy(buffer,"odd");
   status = asynSetOption(port, 0, "parity", buffer);
   if (status != 0) return(status);

   sprintf(buffer, "%d", sbits);
   status = asynSetOption(port, 0, "stop", buffer);
   if (status != 0) return(status);

   sprintf(buffer, "%d", dbits);
   status = asynSetOption(port, 0, "bits", buffer);
   if (status != 0) return(status);

   if (handshake == 'H') strcpy(buffer,"N");
   if (handshake == 'N') strcpy(buffer,"Y");
   status = asynSetOption(port, 0, "clocal", buffer);
   if (status != 0) return(status);

   status = initSerialServer(port, port, 0, tyGSMPFInitBuffsize, 
                             tyGSMPFInitQueuesize, eomstr);
   if (status != 0) return(status);
   return 0;
}


static const iocshArg tyGSMPFInitArg0 = { "serverName",iocshArgString};
static const iocshArg tyGSMPFInitArg1 = { "UART index",iocshArgInt};
static const iocshArg tyGSMPFInitArg2 = { "UART channel",iocshArgInt};
static const iocshArg tyGSMPFInitArg3 = { "baud",iocshArgInt};
static const iocshArg tyGSMPFInitArg4 = { "parity('N'=none,'E'=even,'O'=odd)",
                                          iocshArgInt};
static const iocshArg tyGSMPFInitArg5 = { "stop bits(1 or 2)",iocshArgInt};
static const iocshArg tyGSMPFInitArg6 = { "data bits(5-8)",iocshArgInt};
static const iocshArg tyGSMPFInitArg7 = { "handshake('N'=none,'H'=hardware)",
                                          iocshArgInt};
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

