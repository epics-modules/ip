// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2001/07/04 20:04:04  sluiter
// Creating
//
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
//

// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 10/28/95


// Revised: 10/22/99 for MPF by Timothy Graber

#define MAKE_DEBUG devAiHeidND261Debug

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
 
#include <semLib.h>
#include <tickLib.h>
#include <taskLib.h>
 
extern "C" {
#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "epicsPrint.h"
#include "dbCommon.h"
#include "aiRecord.h"
#include "recSup.h"
}
 
#include "Message.h"
#include "Char8ArrayMessage.h"
#include "DevMpf.h"
#include "serialServer.h"
 
#include "ctype.h"
 
extern "C"
{
#ifdef NODEBUG
#define DEBUG(l,f,v...) ;
#else
#define DEBUG(l,f,v...) { if(l<=devAiHeidND261Debug) printf(f,## v); }
#endif
volatile int devAiHeidND261Debug = 0;
}
 
#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')
 
#define BUFSZ 19 

class AiHeidND261 : public DevMpf 
{
public:
	AiHeidND261(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);
	long convert(dbCommon* pr,int pass);

	static long dev_init(void*);
private:
	char outbuf[BUFSZ], inbuf[BUFSZ];
	char term[4]; //terminator string
	char format[10]; //format of readout
	int termlen;  //terminator length
	int nchar;    //length of readout
	int timeout;
};

MAKE_LINCONV_DSET(devAiHeidND261,AiHeidND261::dev_init)

long AiHeidND261::dev_init(void* v)
{
	aiRecord* pr = (aiRecord*)v;
	AiHeidND261 *pAiHeidND261 = new AiHeidND261((dbCommon*)pr,&(pr->inp));
      pAiHeidND261->bind();
	return 0;
}

AiHeidND261::AiHeidND261(dbCommon* pr, DBLINK* l) : DevMpf(pr,l,false)
{
	aiRecord* ai = (aiRecord*)pr;
	vmeio* pio = (vmeio*)&(l->value);

	DEBUG(2,"AiHeidND261::AiHeidND261(pr,l)\n");

// initialize configurable parameters
	strcpy(format, "%lf");  // format string for sscanf
	strcpy(term, "\r\n\n"); // terminator appended by ND261 to return string
	termlen=3;
	timeout=1; // 1 sec
	nchar=18;
 	outbuf[0] = 2;	// Character ^B to read out ND261
	outbuf[1] = '\n'; // Terminator for write to ND261
 
	DEBUG(2,"AiHeidND261::AiHeidND261 term = '%s'\n", term);
	DEBUG(2,"AiHeidND261::AiHeidND261 timeout = %d\n", timeout);
	DEBUG(2,"AiHeidND261::AiHeidND261 format = %s\n", format);
	DEBUG(2,"AiHeidND261::AiHeidND261 nchar = %d\n", nchar);

}

long AiHeidND261::startIO(dbCommon* pr)
{

	DEBUG(2,"AiHeidND261::StartIO(pr)\n");

	Char8ArrayMessage  *message = new Char8ArrayMessage;
 
	message->cmd = cmdWriteRead | cmdFlush | cmdSetEom;
	message->eomLen = termlen;
//	message->extra = nchar;  // read number of characters will ignore terminator
	message->allocValue(2);
	message->setSize(2);
	message->timeout = timeout;

	::memcpy(message->eomString, term, termlen);
	::memcpy(message->value, outbuf, 2);


	return(sendReply(message));

}

long AiHeidND261::completeIO(dbCommon* pr,Message* pm)
{
	Char8ArrayMessage *pcm = (Char8ArrayMessage *) pm;
	aiRecord* ai = (aiRecord*) pr;

	unsigned char *p;
	double x;
 	int sz=0;
	int rc=0;
	int dlen=0;
	int i;

	if((pm->getType()) != messageTypeChar8Array) {
		epicsPrintf("%s DevAiStrParm::completeIO illegal message type %d\n", ai->name,pm->getType());
		rc = -1;
	} else {
		sz = pcm->getSize();
		::memcpy(inbuf,pcm->value, sz);
		dlen = termlen;
		DEBUG(2,"AiStrParm::CompleteIO(%s)\n",pr->name);
		DEBUG(2," - data size=%d\n",sz);
		DEBUG(2," - return code=%d\n",pcm->status);
		DEBUG(3," - buffer=>%s<\n",inbuf);
		DEBUG(2," - <"); 
			for (i=0, p=inbuf; i<BUFSZ; p++, i++) { DEBUG(2,"%2.2x ", (unsigned int) *p); }
		DEBUG(2,">\n",0); 
 
// if no delimiter, timeout is expected
		if (dlen && (pcm->status!=0)) rc=-1;
	}
 
 
	if (sz && !rc) {
		inbuf[sz]='\0';
// remove delimiter (of length dlen) unless timeout
		if (!pcm->status && ((unsigned int)sz < sizeof(inbuf)) && (sz >= dlen))
			inbuf[sz-dlen] = '\0';
		i = 0;
		if (sz >= 15) {
			if (inbuf[0] == '+') i=1;
			if (inbuf[0] == '-') i=-1;
		}
		if (i) {
			sscanf(&inbuf[1], format, &x);
			ai->val = x*i;
			ai->udf = 0;
			DEBUG(2," - ai->val=%f\n",ai->val);
		}
		rc = 2;	
	} else {
		ai->val=99999.0;
		ai->udf = 1;
		rc= 2; 
	}
 
	delete pcm;
	return rc;
}


/* Provide this routine for the AI record */
long AiHeidND261::convert(dbCommon* pr,int pass)
{
	aiRecord* ai = (aiRecord*)pr;
	ai->eslo=1.0;
	ai->roff=0;
	DEBUG(5,"aiStrParm:Convert roff=%ld, eslo=%f\n",ai->roff,ai->eslo);
	return 0;
}

