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

int tyGSAsynInitBuffsize = 1000;
epicsExportAddress(int, tyGSAsynInitBuffsize);

int tyGSAsynInit(char *port, int uart, int channel, int baud, char parity, 
                int sbits, int dbits, char handshake, char *eomstr)
{

   int index;
   int status;
   #define SIZE 80
   char buffer[SIZE];

   index  = tyGSOctalDevCreate(port, uart, channel, tyGSAsynInitBuffsize, 
                               tyGSAsynInitBuffsize);
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
   return(status);
}


static const iocshArg tyGSAsynInitArg0 = { "portName",iocshArgString};
static const iocshArg tyGSAsynInitArg1 = { "UART index",iocshArgInt};
static const iocshArg tyGSAsynInitArg2 = { "UART channel",iocshArgInt};
static const iocshArg tyGSAsynInitArg3 = { "baud",iocshArgInt};
static const iocshArg tyGSAsynInitArg4 = { "parity('N'=none,'E'=even,'O'=odd)",
                                          iocshArgInt};
static const iocshArg tyGSAsynInitArg5 = { "stop bits(1 or 2)",iocshArgInt};
static const iocshArg tyGSAsynInitArg6 = { "data bits(5-8)",iocshArgInt};
static const iocshArg tyGSAsynInitArg7 = { "handshake('N'=none,'H'=hardware)",
                                          iocshArgInt};
static const iocshArg tyGSAsynInitArg8 = { "EOM string",iocshArgString};
static const iocshArg * const tyGSAsynInitArgs[9] = {
    &tyGSAsynInitArg0,&tyGSAsynInitArg1,&tyGSAsynInitArg2,
    &tyGSAsynInitArg3,&tyGSAsynInitArg4,&tyGSAsynInitArg5,
    &tyGSAsynInitArg6,&tyGSAsynInitArg7,&tyGSAsynInitArg8};
static const iocshFuncDef tyGSAsynInitFuncDef = {
    "tyGSAsynInit",9,tyGSAsynInitArgs};
static void tyGSAsynInitCallFunc(const iocshArgBuf *args)
{
    tyGSAsynInit(
        args[0].sval,args[1].ival,args[2].ival,args[3].ival,
        args[4].ival,args[5].ival,args[6].ival,args[7].ival,args[8].sval);
}

static void tyGSAsynInitRegister(void)
{
    iocshRegister(&tyGSAsynInitFuncDef,tyGSAsynInitCallFunc);
}
epicsExportRegistrar(tyGSAsynInitRegister);

