// devMPC.cc

// Author: Mohan Ramanathan
// Date: 29 April 1999
// Modifications:
// Mark Rivers  17-Feb-2001  Added support for TSP and auto-restart
// Mark Rivers  26-Oct-2002  Fixed problem with reading AMPS on recent MPC
//                           controllers, they don't send AMPS in the response
// Mark Rivers  1-Sep-2003   Changed software to use normal mpfSerial server, 
//                           rather than custom server
/*
 *****************************************************************
 *                         COPYRIGHT NOTIFICATION
 *****************************************************************

 * THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
 * AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
 * AND IN ALL SOURCE LISTINGS OF THE CODE.
 
 * (C)  COPYRIGHT 1999 UNIVERSITY OF CHICAGO
 
 * Argonne National Laboratory (ANL), with facilities in the States of 
 * Illinois and Idaho, is owned by the United States Government, and
 * operated by the University of Chicago under provision of a contract
 * with the Department of Energy.

 * Portions of this material resulted from work developed under a U.S.
 * Government contract and are subject to the following license:  For
 * a period of five years from May 30, 1999, the Government is
 * granted for itself and others acting on its behalf a paid-up,
 * nonexclusive, irrevocable worldwide license in this computer
 * software to reproduce, prepare derivative works, and perform
 * publicly and display publicly.  With the approval of DOE, this
 * period may be renewed for two additional five year periods. 
 * Following the expiration of this period or periods, the Government
 * is granted for itself and others acting on its behalf, a paid-up,
 * nonexclusive, irrevocable worldwide license in this computer
 * software to reproduce, prepare derivative works, distribute copies
 * to the public, perform publicly and display publicly, and to permit
 * others to do so.

 *****************************************************************
 *                               DISCLAIMER
 *****************************************************************

 * NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
 * THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
 * MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
 * LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
 * USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
 * DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
 * OWNED RIGHTS.  

 *****************************************************************
 * LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
 * DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (708-252-2000).
 *****************************************************************
*/

#include <alarm.h>
#include <string.h>
#include <stdio.h>
 
#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "errlog.h"
#include "dbCommon.h"
#include "aiRecord.h"
#include "aoRecord.h"
#include "biRecord.h"
#include "boRecord.h"
#include "mbboRecord.h"
#include "stringinRecord.h"
#include "stringoutRecord.h"
#include "recGbl.h"
#include "recSup.h"

#include "Message.h"
#include "Char8ArrayMessage.h"
#include "DevMpf.h"
#include "serialServer.h"
#include "devMPC.h"

#ifndef vxWorks
#define ERROR -1
#endif

#define READ_TIMEOUT 5;

long devMPCDebug = 0;

/*
 INP or OUT  has form Cx Sx @server,address,parameter
 Cx is for processor number, Sx is signal ( different commands)
 @server, address, parameter - server is a string identifier used
 between the message passing systems, address is used for pump address
 and parameter is either 1 or 2 depending on which of the two we use.
 S0  -	SI	Pump Status  
 S1  -	AI	Read Pressure
 S2  -	AI	Read Current
 S3  -	AI	Read Voltage
 S4  -	AI	Read Pump Size
 S5  -	AI	Read Setpoint value 1 or 2
 S6  -	BI	Read On/Off of setpoint 1 or 2
 S7  -	AI	Read Setpoint value 3 or 4
 S8  -	BI	Read On/Off of setpoint 3 or 4
 S9  -	AI	Read Setpoint value 5 or 6
 S10 -	BI	Read On/Off of setpoint 5 or 6
 S11 -	AI	Read Setpoint value 7 or 8
 S12 -	BI	Read On/Off of setpoint 7 or 8
 S13 -	BI	Read auto-restart status
 S14 -	SI	Read TSP status
 
 S20 -	MBBO	Set Pressure Units  
 S21 -	MBBO	Set Display
 S22 -	AO	Set Pump Size
 S23 -	AO	Set Setpoint 1 or 2
 S24 -	AO	Set Setpoint 3 or 4
 S25 -	AO	Set Setpoint 5 or 6
 S26 -	AO	Set Setpoint 7 or 8
 S27 -	BO	Start /Stop Pump
 S28 -	        Used by start/stop pump
 S29 -	BO	Keyboard lock/unlock
 S30 -          Used by keyboard lock/unlock
 S31 -  BO      Auto-restart on/off
 S32 -  SO      TSP timed mode on
 S33 -  BO      TSP off
 S34 -  MBBO    Select TSP filament
 S35 -  BO      TSP filament clear
 S36 -  BO      TSP filament auto-advance on/off
 S37 -  BO      TSP continuous on/off
 S38 -  SO      Set TSP sublimation parameters
 S39 -  BO      TSP degass
*/

static const int BufferSize = 50;

class DevMPC : public DevMpf
{
public:
    DevMPC( dbCommon*,DBLINK*);

    virtual long startIO(dbCommon* pr);
    virtual long completeIO(dbCommon* pr,Message* m);
    long BuildCommand(int,char*);
protected:
    char recBuf[BufferSize];
    char sendBuf[BufferSize];
    int command;
    char address[3];
    char parameter[2];
};

class DevAiMPC : public DevMPC
{
public:
    DevAiMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};

class DevAoMPC : public DevMPC
{
public:
    DevAoMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};

class DevBiMPC : public DevMPC
{
public:
    DevBiMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};

class DevBoMPC : DevMPC
{
public:
    DevBoMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};

class DevMbboMPC : public DevMPC
{
public:
    DevMbboMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};

class DevSiMPC : public DevMPC
{
public:
    DevSiMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};

class DevSoMPC : public DevMPC
{
public:
    DevSoMPC( dbCommon*,DBLINK*);
    long startIO(dbCommon* pr);
    long completeIO(dbCommon* pr,Message* m);
    static long dev_init(void*);
   
};


MAKE_LINCONV_DSET(devAiMPC,DevAiMPC::dev_init)
MAKE_LINCONV_DSET(devAoMPC,DevAoMPC::dev_init)
MAKE_DSET(devBiMPC,DevBiMPC::dev_init)
MAKE_DSET(devBoMPC,DevBoMPC::dev_init)
MAKE_DSET(devMbboMPC,DevMbboMPC::dev_init)
MAKE_DSET(devSiMPC,DevSiMPC::dev_init)
MAKE_DSET(devSoMPC,DevSoMPC::dev_init)


long DevAiMPC::dev_init(void* v)
{
    aiRecord* pr = (aiRecord*)v;
    DevMPC* pDevMPC = new DevAiMPC((dbCommon*)pr,&(pr->inp));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}

long DevAoMPC::dev_init(void* v)
{
    aoRecord* pr = (aoRecord*)v;
    DevMPC* pDevMPC = new DevAoMPC((dbCommon*)pr,&(pr->out));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}

long DevBiMPC::dev_init(void* v)
{
    biRecord* pr = (biRecord*)v;
    DevMPC* pDevMPC = new DevBiMPC((dbCommon*)pr,&(pr->inp));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}

long DevBoMPC::dev_init(void* v)
{
    boRecord* pr = (boRecord*)v;
    DevMPC* pDevMPC = new DevBoMPC((dbCommon*)pr,&(pr->out));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}

long DevMbboMPC::dev_init(void* v)
{
    mbboRecord* pr = (mbboRecord*)v;
    DevMPC* pDevMPC = new DevMbboMPC((dbCommon*)pr,&(pr->out));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}

long DevSiMPC::dev_init(void* v)
{
    stringinRecord* pr = (stringinRecord*)v;
    DevMPC* pDevMPC = new DevSiMPC((dbCommon*)pr,&(pr->inp));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}

long DevSoMPC::dev_init(void* v)
{
    stringoutRecord* pr = (stringoutRecord*)v;
    DevMPC* pDevMPC = new DevSoMPC((dbCommon*)pr,&(pr->out));
    pDevMPC->bind();
    return(pDevMPC->getStatus());
}


DevMPC::DevMPC(dbCommon* pr,link* l)
: DevMpf(pr,l,false)
{
    vmeio* io = (vmeio*)&(l->value);
    const char* pv = getUserParm();
    if(pv==0 ) {
        errlogPrintf("%s DevMPC Illegal INP field\n",pr->name);
        pr->pact = TRUE;
        return;
    }
    int var1,var2;
    ::sscanf(pv,"%d,%d",&var1,&var2);
    sprintf(address,"%02X",var1);
    sprintf(parameter,"%d",var2);

    if (devMPCDebug)
        errlogPrintf(" name =%s; address =%s; parameter =%s;\n", 
        	pr->name,address,parameter);

    command = io->signal;
    if (command<0 || (command >GetTSPStat && command < SetUnit) || 
        command>SetTSPDegas) {
        errlogPrintf("%s DevMPC Illegal INP field\n",pr->name);
        pr->pact = TRUE;
        return;
    }

}

DevAiMPC::DevAiMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

DevAoMPC::DevAoMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

DevBiMPC::DevBiMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

DevBoMPC::DevBoMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

DevMbboMPC::DevMbboMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

DevSiMPC::DevSiMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

DevSoMPC::DevSoMPC(dbCommon* pr,link* l)
: DevMPC(pr,l)
{
}

long DevMPC::BuildCommand(int hexCmd, char *pvalue)
{
/*  
    The MPC commands are of the form :  "~  AA XX d cc\r"
    AA = Address from 00 - FF
    XX = 2 character Hex Command
    d  =  parameter or data comma seperated
    cc = 2 character checksum Hex values  

    The checksum is to be calculated starting from the character after the
    start character and ending with the space after the data/parm field.
    Add the sum and divide by 0x100 or decimal 256. The reminder in hex is 
    two character checksum. Follow the checksum with a terminator of CR only.
    At the current time we are not calculating the checksum due to the fact
    the device is happy with just "00"
*/
    char tempBuf[10];

    ::memset(sendBuf,0,BufferSize);
    ::strcpy(sendBuf,"~ ");
    ::strcat(sendBuf,address);
    ::sprintf(tempBuf, " %2.2X ", hexCmd);
    ::strcat(sendBuf,tempBuf);
    ::strcat(sendBuf,pvalue);
    ::strcat(sendBuf," 00"); // checksum is set to 00 now !!!!

    if (devMPCDebug)
	errlogPrintf("Command %x sent (%d) |%s|\n",hexCmd,::strlen(sendBuf),sendBuf);

    ::strcat(sendBuf,"\r");  // command terminator
    return(0);
}

long DevMPC::startIO(dbCommon* )
{
/*
      Having packed the signal now perform the actual send!!!!
*/
    Char8ArrayMessage* message = new Char8ArrayMessage;
    int lenSend = ::strlen(sendBuf) +1;
    message->allocValue(lenSend);
    ::memcpy(message->value,sendBuf,lenSend);
    message->setSize(lenSend);
    message->timeout = READ_TIMEOUT;
    message->eomLen = 1;
    message->eomString[0] = '\r';
    message->cmd = cmdWriteRead | cmdFlush | cmdSetEom;
    int status = sendReply(message);
    return(status);
}


long DevAiMPC::startIO(dbCommon*pr )
{
    aiRecord* record = (aiRecord*)pr;

    char tempparameter[2];
    int stptNo;
    int hexCmd=0;
/*
   For setpoint readback set the correct setpoint number.
   All odd setpoints are for pump 1 and even for pump 2
*/    
    switch (command)
    {
        case GetSpVal12:
        case GetSpVal34:
        case GetSpVal56:
        case GetSpVal78:
            hexCmd = 0x3c;
            stptNo = (command - GetSpVal12)/2 ;
            stptNo = stptNo * 2 + ::atoi(parameter);
            sprintf(tempparameter,"%d",stptNo);
            break;
        case GetPres:
            hexCmd = 0x0b;
            ::strcpy(tempparameter,parameter);
            break;
        case GetCur:
            hexCmd = 0x0a;
            ::strcpy(tempparameter,parameter);
            break;
        case GetVolt:
            hexCmd = 0x0c;
            ::strcpy(tempparameter,parameter);
            break;
        case GetSize:
            hexCmd = 0x11;
            ::strcpy(tempparameter,parameter);
            break;
        default:
            errlogPrintf(" %s Wrong record type \n",record->name);
            record->pact = TRUE;
            break;
    }
    DevMPC::BuildCommand(hexCmd,tempparameter);

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}

long DevAoMPC::startIO(dbCommon*pr )
{
    aoRecord* record = (aoRecord*)pr;

    char tempparameter[25]="";
    char pvalue[10]="";
    int stptNo;
    int hexCmd=0;
/*
   For setting setpoint the command is of the form :n,s,x.xE-yy,x.xE-yy
   n - setpoint number.
   s - supply here 1 for pump1 and 2 for pump 2
   values are for On and Off and will be set the same as record->val field.
   All odd setpoints are for pump 1 and even for pump 2
*/    
        
    switch (command)
    {
        case SetSize:
            hexCmd = 0x12;
            ::strcpy(tempparameter,parameter);  
            ::strcat(tempparameter,",");
            sprintf(pvalue,"%d",(int) record->val);
            ::strcat(tempparameter,pvalue);  // Pump Size
            break;
        case SetSp12:
        case SetSp34:
        case SetSp56:
        case SetSp78:
            hexCmd = 0x3d;
            stptNo = (command - SetSp12) *2 + ::atoi(parameter);
            sprintf(tempparameter,"%1.1d",stptNo);
            ::strcat(tempparameter,",");
            ::strcat(tempparameter,parameter);  // for suppy number
            ::strcat(tempparameter,",");
            sprintf(pvalue,"%7.1E",record->val);
            ::strcat(tempparameter,pvalue);  // for On pressure
            ::strcat(tempparameter,",");
            ::strcat(tempparameter,pvalue);  // for Off pressure
            break;
        default:
            errlogPrintf(" %s Wrong record type \n",record->name);
            record->pact = TRUE;
            break;
    }  
    DevMPC::BuildCommand(hexCmd,tempparameter);

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}

long DevBiMPC::startIO(dbCommon*pr )
{
    biRecord* record = (biRecord*)pr;

    char tempparameter[2];
    int stptNo;
    int hexCmd=0;

/*
   For setpoint readback set the correct setpoint number.
   All odd setpoints are for pump 1 and even for pump 2
*/    
    switch (command)
    {
        case GetSpS12:
        case GetSpS34:
        case GetSpS56:
        case GetSpS78:
            hexCmd = 0x3c;
            stptNo = (command - GetSpS12) + ::atoi(parameter);
            sprintf(tempparameter,"%d",stptNo);
            break;
        case GetAutoRestart:
            hexCmd = 0x34;
            ::strcpy(tempparameter,"");
            break;
        default:
            errlogPrintf(" %s Wrong record type \n",record->name);
            record->pact = TRUE;
            break;
    }
    DevMPC::BuildCommand(hexCmd,tempparameter);

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}

long DevBoMPC::startIO(dbCommon*pr )
{
    boRecord* record = (boRecord*)pr;
    int hexCmd=0;

    switch (command)
    {
        case SetStart:
            if (record->val == 0) hexCmd = 0x37; else hexCmd = 0x38;
            DevMPC::BuildCommand(hexCmd,parameter);
            break;
        case SetStop:
            hexCmd = 0x38;
            DevMPC::BuildCommand(hexCmd,parameter);
            break;
        case SetLock:
            if (record->val == 0) hexCmd = 0x44; else hexCmd = 0x45;
            DevMPC::BuildCommand(hexCmd,parameter);
            break;
        case SetUnlock:
            hexCmd = 0x45;
            DevMPC::BuildCommand(hexCmd,parameter);
            break;
        case SetAutoRestart:
            hexCmd = 0x33;
            if (record->val) 
                DevMPC::BuildCommand(hexCmd,"YES");
            else 
                DevMPC::BuildCommand(hexCmd,"NO");
            break;
        case SetTSPAutoAdv:
            hexCmd = 0x2c;
            if (record->val) 
                DevMPC::BuildCommand(hexCmd,"YES");
            else 
                DevMPC::BuildCommand(hexCmd,"NO");
            break;
        case SetTSPOff:
            hexCmd = 0x28;
            DevMPC::BuildCommand(hexCmd,"");
            break;
        case SetTSPClear:
            hexCmd = 0x2b;
            DevMPC::BuildCommand(hexCmd,"");
            break;
        case SetTSPContinuous:
            hexCmd = 0x2d;
            DevMPC::BuildCommand(hexCmd,"");
            break;
        case SetTSPDegas:
            hexCmd = 0x2f;
            DevMPC::BuildCommand(hexCmd,"");
            break;
        default:
           errlogPrintf(" %s Wrong record type \n",record->name);
           record->pact = TRUE;
           break;
    }

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}

long DevMbboMPC::startIO(dbCommon*pr )
{
    mbboRecord* record = (mbboRecord*)pr;
    char tempparameter[10];
    int hexCmd=0;

    switch (command)
    {
        case SetUnit:
            hexCmd = 0x0e;
            ::strcpy(tempparameter,parameter);
            ::strcat(tempparameter,DisplayStr[record->rval]);
            break;
        case SetDis:
            hexCmd = 0x25;
           ::strcpy(tempparameter,parameter);
           ::strcat(tempparameter,DisplayStr[record->rval]);
           break;
        case SetTSPFilament:
            hexCmd = 0x29;
            ::sprintf(tempparameter,"%ld", record->rval);
            break;
        default:
           errlogPrintf(" %s Wrong record type \n",record->name);
           record->pact = TRUE;
           break;
    }
    DevMPC::BuildCommand(hexCmd,tempparameter);

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}

long DevSiMPC::startIO(dbCommon*pr )
{
    stringinRecord* record = (stringinRecord*)pr;
    int hexCmd=0;

    switch (command)
    {
        case GetStatus:
            hexCmd = 0x0d;
            DevMPC::BuildCommand(hexCmd, parameter);
            break;
        case GetTSPStat:
            hexCmd = 0x2a;
            DevMPC::BuildCommand(hexCmd, "");
            break;
        default:
           errlogPrintf(" %s Wrong record type \n",record->name);
           record->pact = TRUE;
           break;
    }

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}

long DevSoMPC::startIO(dbCommon*pr )
{
    stringoutRecord* record = (stringoutRecord*)pr;
    int hexCmd=0;

    switch (command)
    {
        case SetTSPTimed:
            hexCmd = 0x27;
            break;
        case SetTSPSublimation:
            hexCmd = 0x2e;
            break;
        default:
           errlogPrintf(" %s Wrong record type \n",record->name);
           record->pact = TRUE;
           break;
    }
    DevMPC::BuildCommand(hexCmd,record->val);

    int status = DevMPC::startIO((dbCommon*) pr);
    return(status);
}


long DevMPC::completeIO(dbCommon* pr, Message* m)
{
/*
   This function gets the message back from the server and packs the buffer
*/
    if(m->getType() != messageTypeChar8Array) {
	if(m->getType() != messageTypeInt32)
	    if (devMPCDebug) errlogPrintf("DevMPC::completeIO illegal message. %s\n",
		pr->name);
	recGblSetSevr(pr,READ_ALARM,INVALID_ALARM);
	delete m;
	return(ERROR);
    }
    Char8ArrayMessage *message = (Char8ArrayMessage *)m;
    int32 rtnSize = message->getSize();
    if(rtnSize>=BufferSize) {
	if (devMPCDebug) errlogPrintf("DevMPC::completeIO message too big=%d\n", rtnSize);
	recGblSetSevr(pr,READ_ALARM,INVALID_ALARM);
	delete m;
	return(ERROR);
    }
    if(rtnSize < 4) {
	if (devMPCDebug) errlogPrintf("DevMPC::completeIO message too small=%d\n", rtnSize);
	recGblSetSevr(pr,READ_ALARM,INVALID_ALARM);
	delete m;
	return(ERROR);
    }

    ::memset(recBuf,0,BufferSize);
    if (devMPCDebug)
	errlogPrintf("%s command (%d) received (before processing) len=%d, |%s|\n",
                     pr->name,command,rtnSize,message->value);
    if(message->value[3]=='O' && message->value[4] == 'K') {
        if (rtnSize < 13 ) {
            ::strcpy(recBuf,"OK");
        } else {
            char *pdata = &message->value[9]; // strip off the header cmds.
            ::strncpy(recBuf,pdata,rtnSize-13);  // strip off 4 trailing character (space, checksum \r)
        }
    }
    if (devMPCDebug)
	errlogPrintf("%s command (%d) received (after processing) |%s|\n",pr->name,command,recBuf);

    delete m;
    return(0);
}



long DevAiMPC::completeIO(dbCommon* pr, Message* m)
{
    aiRecord* record = (aiRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_NoConvert);
    int rtnSize = strlen(recBuf);        

    char *ploc;
    char pvalue[10]="";
    float value=0;
    char *pdata = recBuf;
    char *llen = pdata;

    switch (command)
    {
        case GetPres:
            ::strncpy(pvalue,recBuf,7);
            pvalue[7] =0;
            value = strtod(pvalue,NULL);
            ploc=&recBuf[8];
            ::strncpy(pvalue,ploc,rtnSize-8);
            pvalue[::strlen(ploc)] =0;
            break;

        case GetCur:
            ::strncpy(pvalue,recBuf,7);
            pvalue[7] =0;
            value = strtod(pvalue,NULL);
            ::strcpy(pvalue,"AMPS");
            break;

        case GetVolt:
            ::strncpy(pvalue,recBuf,4);
            pvalue[4] =0;
            value = strtod(pvalue,NULL);
            ::strcpy(pvalue,"VOLTS");
            break;

        case GetSize:
            llen = ::strchr(pdata,'L');
            *llen = 0;
            ::strcpy(pvalue,pdata);
            value = strtod(pvalue,NULL);
            ::strcpy(pvalue,"L/S");
            break;

        case GetSpVal12:
        case GetSpVal34:
        case GetSpVal56:
        case GetSpVal78:
            ploc=&recBuf[4];
            ::strncpy(pvalue,ploc,7);
            pvalue[7] =0;
            value = strtod(pvalue,NULL);
            ::strcpy(pvalue,"TORR");
            break;

        default:
           errlogPrintf(" %s Wrong record type \n",record->name);
           record->pact = TRUE;
           break;
    }
    record->val = value;
    ::strcpy(record->egu,pvalue);
    record->udf=0;
    return(MPF_NoConvert);
}

long DevAoMPC::completeIO(dbCommon* pr, Message* m)
{
    aoRecord* record = (aoRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_NoConvert);
    int rtnSize = strlen(recBuf);        

   if (rtnSize > 2) {
        recGblSetSevr(record,READ_ALARM,INVALID_ALARM);
	errlogPrintf("DevAoMPC::completeIO message too big in %s\n",
		record->name);
	return(0);
    }
    record->udf=0;
    return(MPF_NoConvert);
}

long DevBiMPC::completeIO(dbCommon* pr, Message* m)
{
    biRecord* record = (biRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_OK);
    int rtnSize = strlen(recBuf);
    char *pvalue;
    int value=0;
    switch (command)
    {
        case GetSpS12:
        case GetSpS34:
        case GetSpS56:
        case GetSpS78:
            pvalue = &recBuf[rtnSize-1];
            ::sscanf(pvalue,"%d",&value);
            break;
        case GetAutoRestart:
            if (::strcmp(recBuf,"YES") == 0) value=1; else value=0;
            break;
        default:
            errlogPrintf(" %s Wrong record type \n",record->name);
            record->pact = TRUE;
            break;
    }

    record->rval = value;
    record->udf=0;
    return(MPF_OK);
}

long DevBoMPC::completeIO(dbCommon* pr, Message* m)
{
    boRecord* record = (boRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_OK);
    int rtnSize = strlen(recBuf);
    if (rtnSize > 2) {
        recGblSetSevr(record,READ_ALARM,INVALID_ALARM);
	errlogPrintf("DevBoMPC::completeIO message too big in %s\n",
		record->name);
	return(0);
    }
    record->udf=0;
    return(MPF_OK);
}

long DevMbboMPC::completeIO(dbCommon* pr, Message* m)
{
    mbboRecord* record = (mbboRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_OK);
    int rtnSize = strlen(recBuf);
    if (rtnSize > 2) {
        recGblSetSevr(record,READ_ALARM,INVALID_ALARM);
	errlogPrintf("DevMbboMPC::completeIO message too big in %s\n",
		record->name);
	return(0);
    }
    record->udf=0;
    return(MPF_OK);
}


long DevSiMPC::completeIO(dbCommon* pr, Message* m)
{
    stringinRecord* record = (stringinRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_OK);
    int rtnSize = strlen(recBuf);        
    if (rtnSize > 39) {
        recGblSetSevr(record,READ_ALARM,INVALID_ALARM);
	errlogPrintf("DevSiMPC::completeIO message too big in %s\n",
		record->name);
	return(0);
    }

    if (devMPCDebug)
        errlogPrintf("%s Value decoded as : %s \n\n", record->name,recBuf);

    ::strcpy(record->val,recBuf);
    record->udf=0;
    return(MPF_OK);
}

long DevSoMPC::completeIO(dbCommon* pr, Message* m)
{
    stringoutRecord* record = (stringoutRecord*)pr;

    if (DevMPC::completeIO((dbCommon*) pr, m)) 
        return(MPF_OK);
    int rtnSize = strlen(recBuf);
    if (rtnSize > 2) {
        recGblSetSevr(record,READ_ALARM,INVALID_ALARM);
	errlogPrintf("DevMbboMPC::completeIO message too big in %s\n",
		record->name);
	return(0);
    }
    record->udf=0;
    return(MPF_OK);
}
