// devSerial.cc
// Device support for the Generic Serial Record
// Author: Mark Rivers
// Date: 3/22/96  Created
//       08/14/99 Converted to MPF
//       10/29/99 Added ConnectIO, modified send_port
//       9/18/00  Added support for new Hybrid I/O format which does I/O to the
//                long binary buffer but uses ASCII conventions, i.e. output buffer
//                size is determined with strlen().

#include <string.h>
#include <stdio.h>

#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "epicsPrint.h"
#include "dbCommon.h"
#include "serialRecord.h"
#include "recGbl.h"
#include "recSup.h"
#include "menuScan.h"
#include "alarm.h"
#include "epicsExport.h"

#include "Message.h"
#include "Char8ArrayMessage.h"
#include "SerialConfigMessage.h"
#include "DevMpf.h"
#include "serialServer.h"

// This device support module cannot use the standard MPF macros for creating
// DSETS because we want to define additional functions which those macros do
// not allow.

typedef struct {
        long        number;
        DEVSUPFUN   report;
        DEVSUPFUN   init;
        DEVSUPFUN   init_record;
        long (*get_ioint_info)(int,dbCommon*,IOSCANPVT*);
        DEVSUPFUN   read_write;
        long (*conv)(void*,int);
        long (*port_setup)(serialRecord *pr,
                            int baud, int data_bits, int stop_bits,
                            char parity, char flow_control);
} DEVSERIAL_DSET;

#define VERY_LONG_WAIT 1000000000 // 1 billion seconds is a long time
extern "C"
{
#ifdef NODEBUG
#define DEBUG(l,f,v) ;
#else
#ifdef __GNUG__
#define DEBUG(l,f,v...) { if(l<=devSerialDebug) printf(f,## v); }
#else
#ifdef __SUNPRO_CC
#define DEBUG(l,...) { if(l<=devSerialDebug) printf(__VA_ARGS__); }
#else
#define DEBUG(l,f,v) { if(l<=devSerialDebug) printf(f,v); }
#endif
#endif
#endif
volatile int devSerialDebug = 0;
}

class Serial : public DevMpf
{
public:
        Serial(dbCommon*,DBLINK*);

        long startIO(dbCommon* pr);
        long completeIO(dbCommon* pr,Message* m);
        long cancelIO();
        virtual void receiveReply(dbCommon* pr, Message* m);
        static long dev_init(void*);
        static long port_setup(serialRecord *pr,
                                    int baud, int data_bits, int stop_bits,
                                    char parity, char flow_control);
        static long get_ioint_info(int cmd, struct dbCommon *pr,
                                        IOSCANPVT *ppvt);
private:
        char outbuff[100];
        char inbuff[100];
        IOSCANPVT ioscanpvt;
};

extern "C" {DEVSERIAL_DSET devSerial =
  { 5,NULL,NULL,Serial::dev_init,Serial::get_ioint_info,
        DevMpf::read_write,NULL,Serial::port_setup };};
epicsExportAddress(DEVSERIAL_DSET, devSerial);

long Serial::dev_init(void* v)
{
        serialRecord* pr = (serialRecord*)v;
        DEBUG(1, "Serial::dev_init, record=%s\n", pr->name);
        Serial *pSerial = new Serial((dbCommon*)pr,&(pr->inp));
        pSerial->bind();
        return (0);
}

Serial::Serial(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
       DEBUG(1, "Serial::Serial, record=%s\n", pr->name);
}

long Serial::get_ioint_info(int cmd, struct dbCommon *pr, IOSCANPVT *ppvt)
{
        DEBUG(1, "Entering get_ioint_info,cmd=%d\n", cmd);
        //serialRecord* soi = (serialRecord*)pr;
        //Serial* ser = (Serial*)pr->dpvt;
        // If this record was just added to a scan list then send a message
        // However, only do this if the TMOD field is serialTMOD_Read. No other
        // transaction mode makes sense to be I/O Event scanned
        // THIS NEEDS TO BE CHECKED
        //if ((cmd == 0) && (soi->tmod == serialTMOD_Read))  ser->startIO(pr);
        //else if (cmd == 1) ser->cancelIO();
        //*ppvt = ser->ioscanpvt;
        return((long)0);
}

long Serial::startIO(dbCommon* pr)
{
        serialRecord* soi = (serialRecord*)pr;
        char *outptr=NULL;
        int outlen=0;

        Char8ArrayMessage *pDataMessage = new Char8ArrayMessage;

        DEBUG(5,"Entering Serial::startIO\n");

        // If the record is in IO_EVENT scan mode then set the timeout
        // to be infinite, else set it to the TMOT field.
        if (soi->scan == menuScanI_O_Intr)
                pDataMessage->timeout = VERY_LONG_WAIT;
        else
        	pDataMessage->timeout = soi->tmot/1000;  // Record is msec, MPF is sec
                // Minimum timeout in MPF is 1 second.  If the tmot field is
                // non-zero then set the timeout to at least 1 second
                if ((soi->tmot > 0) && (pDataMessage->timeout < 1)) 
                        pDataMessage->timeout=1;
        // Set operation to be performed depends upon the TMOD field in the record
        switch(soi->tmod) {
                case serialTMOD_Write_Read:
                        pDataMessage->cmd = cmdWriteRead | cmdFlush | cmdSetEom;
                        break;
                case serialTMOD_Write:
                        pDataMessage->cmd = cmdWrite;
                        break;
                case serialTMOD_Read:
                        pDataMessage->cmd = cmdRead | cmdSetEom;
                        break;
        }

        //  Set things up depending upon ASCII or binary modes
        if (soi->tmod != serialTMOD_Read)
        {
            if (soi->ofmt == serialOFMT_ASCII)
            {
                ::strncpy(outbuff, soi->aout, sizeof(outbuff));
                //  strncpy appends nulls to the end of outbuff
                //  Add the output delimiter
                if (soi->odel != -1) outbuff[strlen(outbuff)]=soi->odel;
                outptr = outbuff;
                outlen = strlen(outbuff);
            }
            else // Binary or Hybrid output mode
            {
                outptr = (char *)soi->optr;
                outlen = soi->nowt;
                if (soi->ofmt == serialOFMT_Hybrid)
                    outlen = strlen(( char *) soi->optr);
                if (outlen > soi->nowt) outlen = soi->nowt;
                if (outlen > soi->omax) outlen = soi->omax;
                // Append delimiter if there is room
                if ((soi->odel != -1) && (outlen < soi->omax))
                {
                    outptr[outlen]=soi->odel;
                    outlen = outlen + 1;
                }
            }
        }

        if (soi->tmod != serialTMOD_Write)
        {
            // Set the delimiter
            if (soi->idel == -1)
                pDataMessage->eomLen=0;  // This means no terminator
            else
                pDataMessage->eomLen=1;
                pDataMessage->eomString[0] = soi->idel;
        }

        pDataMessage->allocValue(outlen);
        pDataMessage->setSize(outlen);
        ::memcpy(pDataMessage->value,outptr,outlen);
        // Set the requested number of characters to read.  0 means read till
        // terminator or timeout, positive number will terminate read on that
        // number of characters.
        pDataMessage->extra = soi->nrrd;
        return(sendReply(pDataMessage));
}

long Serial::completeIO(dbCommon* pr,Message* pm)
{
        serialRecord* soi = (serialRecord*)pr;
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;

        DEBUG(5,"Entering devSerial::completeIO\n");

        if((pm->getType()) == messageTypeInt32) {
            // This is a response to a serialConfig message.
            // Nothing to be done.
            goto finish;
        }

        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevSerial::completeIO illegal message type %d\n",
                    soi->name,pm->getType());
            goto finish;
        }

        soi->udf=0;

        switch (soi->tmod)
        {
        case serialTMOD_Write:
            break;

        default:  // serialTMOD_Write_Read or serialTMOD_Read
            if (pcm->status != 0)
                recGblSetSevr(pr,READ_ALARM,INVALID_ALARM);
            soi->nord = pcm->getSize();
            // Action depends upon input format (ASCII or binary)
            DEBUG(1,"devSerial::completeIO, status=%d\n", pcm->status)
            DEBUG(1,"devSerial::completeIO, getSize()=%d\n", soi->nord);
            switch (soi->ifmt)
            {
            case serialOFMT_ASCII:
                // Copy the string to the record
                if (soi->nord > MAX_STRING_SIZE-1) soi->nord=MAX_STRING_SIZE-1;
                ::strncpy(soi->ainp,pcm->value,soi->nord);
                // If the string is terminated by the requested terminator
                // replace with NULL.
                if ((soi->nord != 0) &&
                    (soi->idel != -1) &&
                    (soi->ainp[soi->nord-1] == soi->idel))
                            soi->ainp[soi->nord-1] = '\0';
                else
                    // Make sure input string is null terminated.
                    soi->ainp[soi->nord]='\0';
                break;
            case serialOFMT_Binary:
            case serialOFMT_Hybrid:
                char *inptr = (char *)soi->iptr;
                if (soi->nord > soi->imax) soi->nord = soi->imax;
                ::memcpy(inptr,pcm->value,soi->nord);
                // If the string is terminated by the requested terminator
                // remove it.
                if ((soi->nord != 0) &&
                    (soi->idel != -1) &&
                    (inptr[soi->nord-1] == soi->idel))
                            inptr[soi->nord-1] = '\0';
                else
                    // Add a null terminator if there is room
                    if (soi->nord < soi->imax) inptr[soi->nord]='\0';
                break;
            }
        }

finish:
        delete pm;

        // If the record is in IO_EVENT scan mode then call startIO again
        // However, only do this if the TMOD field is serialTMOD_Read. No other
        // transaction mode makes sense to be I/O Event scanned.
        //if ((soi->scan == menuScanI_O_Intr) && (soi->tmod == serialTMOD_Read))
        //        startIO(pr);
        return MPF_OK;
}


long Serial::cancelIO()
{
/* THIS NEEDS WORK
        InterruptMsg* lm =
                (InterruptMsg*)GetMessageBuffer(InterruptMsgType);
        lm->status = SerialCancel;
        return Send(lm);
*/
    return(0);
}




long Serial::port_setup(serialRecord *pr,
                            int baud, int data_bits, int stop_bits,
                            char parity, char flow_control)
{
        DEBUG(5,"Entering Serial::port_setup\n");
        Serial* ser = (Serial*)pr->dpvt;
        SerialConfigMessage* sc=new SerialConfigMessage;

        // We need to check here that the server is connected, because
        // this function can be called directly from the record and it does
        // not know if the server is connected or not.  It will be called
        // from connectIO when the server does connect.
        if (!ser->isConnected()) return -1;
        
        sc->baud = baud;
        sc->stopBits = stop_bits;
        sc->bitsPerChar = data_bits;
  	sc->parity = parity;
  	sc->flowControl = flow_control;

        // send the message
        ser->send(sc, replyTypeReceiveReply); 

        return 0;
}

void Serial::receiveReply(dbCommon* pr,Message* pmessage)
{
        // This function is called in response to reply from
        // SerialConfigMessage.  Nothing needs to be done.
        DEBUG(1, "%s Serial::receiveReply, enter\n", pr->name);
        delete pmessage;
}
