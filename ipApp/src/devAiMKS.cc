// devAiMKS.cc
//
// Author: Mark Rivers
//
// Modifications:  1995     mlr     Original version
//                 11/1/97  mlr     Added debugging statements
//                 08/15/99 mlr     Converted to MPF
//
// This module provides EPICS AI (Analog Input) device support for the 
//  MKS 937 Vacuum Gauge Controller uing HiDEOS.
//  It presently uses only RS-232 communication. RS-485 may be added in the
//  future.
//  The PARM field is specified as follows:
//  x-Serial[n],m
//      where 
//          "x" is the IP slot (a-d)
//          "n" is the serial port on that IP module (0-7)
//          "m" is the vacuum gauge number on the MKS 937 (1-5)
//
//  Remember to set the serial port settings in the vxWorks startup file to
//  use the correct baud rate and even parity.  For example:
//      ConfigureHideosSerialPort("a-Serial[1]",1,19200,'E',8,1,'N')
//
//  This device support package does the following:
//      - On record initialization:
//          - If rec.PREC=0 then it sets rec.PREC=1.  PREC=1 will display
//            the actual number of digits reported by the 937.
//          - Sends queries the 937 to read the current gauge units (Torr, 
//            Pascal, etc.) and sets rec.EGU to this string.  This query is via
//            send(pm, replyTypeReceiveReply) in connectIO(), and the reply is 
//            received in receiveReply().
//          - Queries the 937 to determine the gauge type (cold-cathode,
//            Pirani, etc).  This information is used to determine the lower 
//            and upper limits of this gauge.
//          - If rec.LOPR=0 and rec.HOPR=0 sets these to the upper and lower
//            limits of this gauge type.
//      - On record processing
//          - Reads the current pressure from the gauge.
//          - If the pressure is above or below the range of this gauge, sets
//            a minor alarm and sets rec.VAL= minimum or maximum value for
//            this gauge type.
//          - If the gauge returns any other error or if the read request
//            times out, then it sets a major alarm and sets rec.VAL=0.
//          - If there are no errors then rec.VAL is set to the pressure
//            and device support returns 2 to tell record support not to
//            convert the value, since we wrote directly to the VAL field.
//      

#include <string.h>
#include <stdio.h>

#include "dbAccess.h"
#include "aiRecord.h"
#include "recGbl.h"
#include "recSup.h"
#include <alarm.h>

#include "Message.h"
#include "Char8ArrayMessage.h"
#include "DevMpf.h"
#include "serialServer.h"

#include "ctype.h"

#define TYPE_CC   0   // Cold-cathode gauge
#define TYPE_PR   1   // Pirani gauge
#define TYPE_CM   2   // Capacitance manometer
#define TYPE_TC   3   // Thermocouple
#define TYPE_CV   4   // Convection
#define TIMEOUT   2   // Timeout in sec

extern "C"
{
#ifdef NODEBUG
#define DEBUG(l,f,v) ;
#else
#define DEBUG(l,f,v...) { if(l<=devAiMKSDebug) printf(f,## v); }
#endif
volatile int devAiMKSDebug = 0;
}

typedef enum {replyReadUnits, replyReadGaugeType, replyReadPressure} MKSReplyType;

class aiMKS : public DevMpf
{
public:
	aiMKS(dbCommon*,DBLINK*);
    
        virtual void connectIO(dbCommon *pr, Message *message);
	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);
        virtual void receiveReply(dbCommon* pr, Message* m);
	static long dev_init(void*);

private:
        char read_command[4];
	char response[10];
        int  gauge_type;
        char gauge_number;
        double gauge_high;
        double gauge_low;
};

MAKE_LINCONV_DSET(devAiMKS,aiMKS::dev_init)

long aiMKS::dev_init(void* v)
{
	aiRecord* pr = (aiRecord*)v;
	aiMKS *paiMKS = new aiMKS((dbCommon*)pr,&(pr->inp));
      paiMKS->bind();
	return(0);
}

aiMKS::aiMKS(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	aiRecord* ai = (aiRecord*)pr;

        DEBUG(5,"aiMKS::aiMKS, enter, record=%s\n", pr->name);
        // Parse the parm field to get the gauge number
	gauge_number=getUserParm()[0];
	if(gauge_number==0 || gauge_number<'1' || gauge_number>'5')
	{
		recGblRecordError(S_db_badField,(void *)pr,
			"devAiMKS (init_record) Illegal INP parm field");
		// status=S_db_badField;
        }
	else
	{
                ::memcpy(read_command, "Rn\r", 3);
		read_command[1]=gauge_number;
        }

        // If the .PREC field is 0, set it to 1, since this is the actual
        //   resolution of the device.
        if (ai->prec == 0) ai->prec = 1;
                      
}

void aiMKS::connectIO(dbCommon *pr, Message *message)
{
        // This connection routine is called whenever the server connects or
        // disconnects.  When it connects we want to query the gauge type and
        // units.  2 query messages are sent here, the reponses are handled
        // in receiveReply().
        Char8ArrayMessage *pm;

        ConnectMessage *pConnectMessage = (ConnectMessage *)message;
        DEBUG(2,"aiMKS::connectIO, enter, record=%s, status=%d\n", 
                            pr->name, pConnectMessage->status);

        if (pConnectMessage->status != connectYes) goto finish;


        // Read the pressure units (Torr, Pascal, etc.) from the controller
        // This read is started here, and completes in receiveReply.
        pm = new Char8ArrayMessage;
        pm->allocValue(3);
        pm->setSize(3);
        ::memcpy(pm->value,"SU\r", 3);
        pm->eomLen = 1;
        pm->eomString[0] = '\r';
        pm->timeout = TIMEOUT;
        pm->cmd = cmdWriteRead | cmdFlush | cmdSetEom;
        pm->setClientExtra(replyReadUnits);
        send(pm, replyTypeReceiveReply);

        // Read the gauge type from the controller
        // Note that three values are output, one for each slot.  The first
        // slot is either empty or a cold-cathode controller.  The other two
        // slots can hold any type of controller board, and for all types 
        // except cold-cathode each board may control one or two gauges.
        pm = new Char8ArrayMessage;
        pm->allocValue(3);
        pm->setSize(3);
        memcpy(pm->value, "SG\r", 3);
        pm->cmd = cmdWriteRead | cmdFlush;
        pm->timeout = TIMEOUT;
        pm->setClientExtra(replyReadGaugeType);
        send(pm, replyTypeReceiveReply);

    finish:
        DevMpf::connectIO(pr, message);  // Call the base class method
}
        
long aiMKS::startIO(dbCommon* pr)
{
        Char8ArrayMessage *message = new Char8ArrayMessage;
        DEBUG(5,"aiMKS::startIO, record=%s, enter\n", pr->name);
        message->allocValue(3);
        message->setSize(3);
        ::memcpy(message->value,read_command, 3);
        message->cmd = cmdWriteRead | cmdFlush;
        message->timeout = TIMEOUT;
        return(sendReply(message));
}

long aiMKS::completeIO(dbCommon* pr,Message* pmessage)
{
        aiRecord* ai = (aiRecord*)pr;
        int response_len=0;
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pmessage;

        if((pmessage->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s aiMKS::completeIO illegal message type %d\n",
                    ai->name,pmessage->getType());
            goto finish;
        } 
        ::strncpy(response, pcm->value, sizeof(response));
        response_len = pcm->getSize();
        DEBUG(5,"aiMKS::completeIO record=%s\n", ai->name);
        DEBUG(5,"aiMKS::completeIO response_len=%d\n", response_len);
        DEBUG(5,"aiMKS::completeIO response=\n%s\n", response);
        if ((pcm->status != 0)  || 
            (response_len != 8)) {
            DEBUG(1,"aiMKS::completeIO record=%s, status=%d\n", 
					ai->name, pcm->status);
            DEBUG(1,"aiMKS::completeIO response_len=%d\n", response_len);
            DEBUG(1,"aiMKS::completeIO response=\n%s\n", response);
            recGblSetSevr(pr,READ_ALARM,MAJOR_ALARM);
            goto finish;
        }

        ai->val = 0.;  // Set value to 0. in case of errors
        response[7]=0;

        // Decode pressure string
        // If the first character of the response is a digit or blank then
        // we got a number, which is a good response
        if( (isdigit(response[0])) || (response[0]==' ') )
            ai->val = atof(response);
        else
        {
            if (!strncmp(response,"H ",2) || !strncmp(response,"A ",2))
            {
                // Pressure is above valid range for gauge
                ai->val = gauge_high;
                recGblSetSevr(pr,READ_ALARM,MINOR_ALARM);
            }
            else if (!strncmp(response,"L ",2))
            {
                // Pressure is below valid range for gauge
                ai->val = gauge_low;
                recGblSetSevr(pr,READ_ALARM,MINOR_ALARM);
            }
            else if (!strncmp(response,"MI",2)  || // Misconnected 
                     !strncmp(response,"NO",2)  || // No gauge
                     !strncmp(response,"HV",2))    // High voltage off
            {
                DEBUG(5,"aiMKS::completeIO record=%s, no gauge, etc.\n", 
                                                ai->name);
                recGblSetSevr(pr,READ_ALARM,MAJOR_ALARM);
            }
            else    // Unexpected response from MKS
            {
                DEBUG(5,"aiMKS::completeIO record=%s, unknown response\n", 
                                                ai->name);
                recGblSetSevr(pr,READ_ALARM,MAJOR_ALARM);
            }
        }

    finish:
	delete(pmessage);
        ai->udf = 0;
        // Note - we always return "2" to record support.  This indicates
        //   success, but don't convert value, since this routine writes
        //   directly to the VAL field.
	return (MPF_NoConvert);
}

void aiMKS::receiveReply(dbCommon* pr,Message* pmessage)
{
        aiRecord* ai = (aiRecord*)pr;
        int response_len=0;
        char gauge_str[3];      // String to hold gauge type
        float mult;             // Multiplier for different pressure units
        int MKSReplyType;
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pmessage;

        if((pmessage->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s aiMKS::receiveReply illegal message type %d\n",
                    ai->name,pmessage->getType());
            goto finish;
        }
        ::strncpy(response, pcm->value, sizeof(response));
        response_len = pcm->getSize();
        DEBUG(5,"aiMKS::receiveReply record=%s\n", ai->name);
        DEBUG(5,"aiMKS::receiveReply response_len=%d\n", response_len);
        DEBUG(5,"aiMKS::receiveReply response=\n%s\n", response);
        if ((pcm->status != 0) || (response_len != 8)) {
            DEBUG(1,"aiMKS::receiveReply record=%s\n", ai->name);
            DEBUG(1,"aiMKS::receiveReply response_len=%d\n", response_len);
            DEBUG(1,"aiMKS::receiveReply response=\n%s\n", response);
            recGblSetSevr(pr,READ_ALARM,MAJOR_ALARM);
            goto finish;
        }

        MKSReplyType = pcm->getClientExtra();
        switch (MKSReplyType) {

        case replyReadUnits:
            response[7]=0;
            strcpy(ai->egu, response);
            break;

        case replyReadGaugeType:
            response[7]=0;
            gauge_str[2] = '\0';
            switch (gauge_number) {
                case '1':
                    strncpy(gauge_str,&response[0],2);
                    break;
                case '2':
                case '3':
                    strncpy(gauge_str,&response[2],2);
                    break;
                case '4':
                case '5':
                    strncpy(gauge_str,&response[4],2);
                    break;
            }
            if (!strcmp(gauge_str, "Cc")) 
            {
                gauge_type = TYPE_CC;
                gauge_low = 1.0e-11;
                gauge_high = 1.0e-2;
            }
            else if (!strcmp(gauge_str, "Pr")) 
            {
                gauge_type = TYPE_PR;
                gauge_low = 5.e-4;
                gauge_high = 760.;
            }
            else if (!strcmp(gauge_str, "Cm")) 
            {
                gauge_type = TYPE_CM;
                gauge_low = 1.e-3; // These values depend on head model
                gauge_high = 1.e0;
            }
            else if (!strcmp(gauge_str, "Tc")) 
            {
                gauge_type = TYPE_TC;
                gauge_low = 1.e-3;
                gauge_high = 1.e0;
            }
            else if (!strcmp(gauge_str, "Cv")) 
            {
                gauge_type = TYPE_CV;
                gauge_low = 1.e-3;
                gauge_high = 1.e+3;
            }
            else
                recGblSetSevr(pr,READ_ALARM,MAJOR_ALARM);

            // The values for gauge_low and guage_high assigned above are 
            // in Torr.  If the gauge is not reading in Torr then we need to
            // convert to the appropriate units.
            mult = 1.0;
            if (!strncmp(ai->egu, "mbar", 4)) mult=1.3;
            else if (!strncmp(ai->egu, "Pascal", 6)) mult=130.;
            else if (!strncmp(ai->egu, "micron", 6)) mult=1000.;
            gauge_low = gauge_low * mult;
            gauge_high = gauge_high * mult;
            // If LOPR and HOPR are both zero then set them to the gauge 
            // limits
            if ((ai->lopr==0.) && (ai->hopr==0.))
            {
                ai->lopr = gauge_low;
                ai->hopr = gauge_high;
            }
            DEBUG(5,"aiMKS::receiveReply record=%s, gauge_str=%s, lopr=%f, hopr=%f\n", 
                        ai->name, gauge_str, ai->lopr, ai->hopr);
            break;

            default:
            DEBUG(5,"aiMKS::receiveReply record=%s, invalid reply type=%d\n", 
                                        ai->name, MKSReplyType);
        }

    finish:
	delete(pmessage);
}
