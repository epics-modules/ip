// mpcSever.cc

// Author: Mohan Ramanathan
// Date: 27 April 1999
// Modified: Mark Rivers, 3-Oct-2001, increased stack size in taskSpawn 
// because it was tight on PPC
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

extern "C" {
#include <vxWorks.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <taskLib.h>
#include <semLib.h>
}
 
#include "Message.h"
#include "Int32Message.h"
#include "Char8ArrayMessage.h"
#include "SerialPort.h"

long mpcDebug=0;

static const int BufferSize = 50;
static const int timeoutSeconds = 5;
 
enum readStatus
{
    readSuccess=0, readTimeout, readOverrun, readFailure
};

static const char * const readStatusString[] = 
{
    "readSuccess", "readTimeout", "readOverrun", "readFailure"
};

enum readState { readIdle, readStart, readGotDelim};


class MPC
{
public:
    MPC(const char *name,const char *portName, int queueSize);
    static void mpcServer(MPC *);
    bool getStartOk() const {return(startOk);}
    static byteHandlerRC byteHandler(void*,unsigned char);
    void talk(Char8ArrayMessage *pmessage);
private:
    MessageServer *pMessageServer;
    SerialPort* pSerialPort;
    bool startOk;

    readStatus status;
    readState state;

    unsigned char cmdBuf[BufferSize];
    int nextRead;
    char readBuf[BufferSize];
    int nextResponse;
    char responseBuf[BufferSize];
};

static char taskname[] = "mpc";

int initMPCServer(char *serverName,char *portName,int queueSize)
{
    MPC *pMPC = new MPC(serverName,portName,queueSize);
    if(!pMPC->getStartOk()) return(0);
    int taskId = taskSpawn(taskname,100,VX_FP_TASK,4000,
	(FUNCPTR)MPC::mpcServer,(int)pMPC,0,0,0,0,0,0,0,0,0);
    if(taskId==ERROR) 
	printf("mpcServer taskSpawn Failure\n");
    return(0);
}
	

MPC:: MPC(const char *name,const char *portName, int queueSize)
: startOk(false), status(readFailure),
  state(readIdle), nextRead(0),  nextResponse(0)
{
    pSerialPort = SerialPort::bind(portName,this,byteHandler);
    if(pSerialPort==0) {
	printf("%s: could not bind to SerialPort %s\n",name,portName);
        return;
    }
    pMessageServer = new MessageServer(name,queueSize);
    startOk = true;
}

void MPC::mpcServer(MPC *pMPC)
{
    while(true) {
        pMPC->pMessageServer->waitForMessage();
	Message *message;
        while((message = pMPC->pMessageServer->receive())) {
            if(message->getType()!=messageTypeChar8Array) {
                printf("mpcServer got illegal message type %d\n",
                    message->getType());
		continue;
            }
            Char8ArrayMessage  *preceive = (Char8ArrayMessage *)message;
	    pMPC->talk(preceive);
	    Message *psend;
	    if(pMPC->status!=readSuccess) {
		Int32Message *pm = (Int32Message *)pMPC->pMessageServer->
			allocReplyMessage(preceive,messageTypeInt32);
		pm->status = -1;
		psend = pm;
	    } else {
                Char8ArrayMessage *pm =
			(Char8ArrayMessage *)pMPC->pMessageServer->
                        allocReplyMessage(preceive,messageTypeChar8Array);
		pm->allocValue(pMPC->nextResponse);
		::memcpy(pm->value,pMPC->responseBuf,pMPC->nextResponse);
		pm->setSize(pMPC->nextResponse);
		psend = pm;
	    }
            delete preceive;
            pMPC->pMessageServer->reply(psend);
	}
    }
    return;
}

byteHandlerRC MPC::byteHandler(void* v, unsigned char indata)
{
    register char data = (char)indata;
    MPC* pMPC = (MPC*)v;
    switch(pMPC->state) {
    case readIdle:
        break;
    case readStart:
	if(data=='\r') {
            pMPC->state=readIdle;
	    pMPC->status = readSuccess;
	    return(byteHandlerEndRead);
	} else {
	    if(pMPC->nextRead >= (BufferSize-1)) {
		pMPC->state=readIdle;
	        pMPC->status = readOverrun;
	        return(byteHandlerEndRead);
	    } else {
                pMPC->readBuf[pMPC->nextRead++]=data;
	    }
	}
        break;

    default:
        break;
    }
    return(byteHandlerOK);
}

void MPC::talk(Char8ArrayMessage* pinmessage)
{
    serialStatus ioStatus;
    
    ::memset(cmdBuf,0,BufferSize);
    ::memset(readBuf,0,BufferSize);
    
    int32 cmdSize = pinmessage->getSize();
    ::memcpy(cmdBuf,pinmessage->value,cmdSize);
    
    nextRead = 0;
    state = readStart;

    if(mpcDebug>1) 
            printf("MPC::talk sending (%d): %s\n",cmdSize, cmdBuf);

    ioStatus = pSerialPort->write(cmdBuf,cmdSize,timeoutSeconds);
    ioStatus = pSerialPort->read(timeoutSeconds);
    if(ioStatus==serialTimeout) status = readTimeout;

    nextRead -=3;  // strip of the space and checksum
    readBuf[nextRead] = 0;
  
    if((mpcDebug) || (status!=readSuccess && mpcDebug)) { 
	printf(" %d %s %s Cmd: %s \n Reply: %s \n",ioStatus,
	    pMessageServer->getName(),readStatusString[status],
	    cmdBuf,  readBuf);
	    
    } 

    if(status!=readSuccess) return;
    if(readBuf[3]=='O' && readBuf[4] == 'K') { 
        ::memset(responseBuf,0,BufferSize);
        if ( nextRead < 9 ) {
 	    ::strcpy(responseBuf,"OK");
        } else {      
            char *pdata = &readBuf[9]; // strip off the header cmds.
 	    ::strcpy(responseBuf,pdata);
 	}
	nextResponse = ::strlen(responseBuf);

        if(mpcDebug) 
            printf("MPC ResponseBuf (%d): %s\n",nextResponse,responseBuf);

	status = readSuccess;
	return;
    } else {    
	printf("MPC: error from controller %s\n",readBuf);
	status = readFailure;
	return;
    }
}
