
// Author: Tim Mooney (based on Hideos code written by Jim Kowalkowski)
// Converted to MPF 9/13/02  tmm

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <semLib.h>
#include <tickLib.h>
#include <taskLib.h>
#include <math.h>

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
#define DEBUG(l,f,v...) { if(l<=devAiElcomatDebug) printf(f,## v); }
#endif
volatile int devAiElcomatDebug = 0;
}

#define BUFSZ 512

class AiElcomat : public DevMpf
{
public:
	AiElcomat(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);
	virtual long convert(dbCommon* pr,int pass);

	static long dev_init(void*);
private:
	char buffer[BUFSZ];
};

MAKE_LINCONV_DSET(devAiElcomat,AiElcomat::dev_init)

long AiElcomat::dev_init(void* v)
{
	aiRecord* pr = (aiRecord*)v;
	AiElcomat *pAiElcomat = new AiElcomat((dbCommon*)pr,&(pr->inp));
	pAiElcomat->bind();
	return 0;
}

AiElcomat::AiElcomat(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	//vmeio* io = (vmeio*)&(l->value);

	DEBUG(2,"AiElcomat::AiElcomat(pr,l)\n");
}

long AiElcomat::startIO(dbCommon* pr)
{
	DEBUG(2,"AiElcomat::StartIO(pr)\n");

	Char8ArrayMessage  *message = new Char8ArrayMessage;

	message->cmd = cmdRead | cmdSetEom;
	message->eomLen = 0;
	message->extra = BUFSZ;  // Maximum number of characters to read
	return(sendReply(message));
}

long AiElcomat::completeIO(dbCommon* pr,Message* pm)
{
	aiRecord* ai = (aiRecord*)pr;
	Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;
	int rc, sz, i, j, k, *ip;
	unsigned int ix;
	unsigned char *p;
	float x, y, xs, ys;

	DEBUG(2,"AiElcomat::completeIO(pr,pm)\n");
	if ((pm->getType()) != messageTypeChar8Array) {
		epicsPrintf("%s DevAiElcomat::completeIO: illegal message type %d\n",
			ai->name,pm->getType());
		rc = -1;
		delete pm;
		return rc;
	}

	sz = pcm->getSize();
	::memcpy(buffer,pcm->value, sz);
	DEBUG(2," - data size=%d\n",sz);
	DEBUG(1," - return code=%d\n",pcm->status);
	DEBUG(2," - <"); 
	for (i=0, p=(unsigned char *)buffer; i<20; p++, i++) {
		DEBUG(2,"%2.2x ", (unsigned int)*p);
	}
	DEBUG(2,">\n"); 
	rc = 0;
	if (pcm->status != 0) rc=-1;

	if (sz >= 8) {
		// Advance pointer until we see something that looks like a data packet
		for (i = 0, p = (unsigned char *)buffer;
				i <= sz-8 && ((p[0] != 2) || (p[7] != 3));
				i++, p++);
		DEBUG(3," -  found data packet after %d bytes\n",i); 
		if (i <= sz-8) {
			x = y = xs = ys = 0.0;
			for (j=0; i<=sz-8 && p[0]==2 && p[7]==3; j++, i+=8, p+=8) {
				DEBUG(4," data: <"); 
				for (k=0; k<8; k++) DEBUG(4,"%2.2x ", (unsigned int)p[k]);
				DEBUG(4,"> "); 
				ix = p[3];
				ix = (ix<<8) | p[2];
				ix = (ix<<8) | p[1];
				if (ix & 0x800000) ix |= 0xff000000;
				ip = (int *)&ix;
				x += *ip/100.0;
				xs += (*ip/100.0) * (*ip/100.0);
	
				ix = p[6];
				ix = (ix<<8) | p[5];
				ix = (ix<<8) | p[4];
				if (ix & 0x800000) ix |= 0xff000000;
				ip = (int *)&ix;
				y += *ip/100.0;
				ys += (*ip/100.0) * (*ip/100.0);
				DEBUG(4," x_ave=%.2f  ",x/(j+1));
				DEBUG(4," y_ave=%.2f\n",y/(j+1));
			}
			ai->val = x/j;
			ai->sval = y/j;
			if (j>1) {
				ai->hopr = sqrt(xs - x*x/j)/(j-1);
				ai->lopr = sqrt(ys - y*y/j)/(j-1);
			} else {
				ai->hopr = 0;
				ai->lopr = 0;
			}
			ai->rval = j;
			DEBUG(2," - averaged %d measurements\n", j);
			ai->udf = 0;
		} else {
			DEBUG(3," -  skipped %d bytes; didn't find any data\n",i); 
			ai->val=0.0;
			ai->sval=0.0;
			ai->hopr = 0.0;
			ai->lopr = 0.0;
			ai->rval = 0;
			ai->udf = 1;
		}
		rc = 2;
	} else {
		ai->val=0.0;
		ai->sval=0.0;
		ai->udf = 1;
		ai->rval = 0;
		ai->hopr = 0.0;
		ai->lopr = 0.0;
		rc = 2;
	}
	delete pm;
	return rc;
}

/* Provide this routine for the AI record */
long AiElcomat::convert(dbCommon* pr,int pass)
{
	aiRecord* ai = (aiRecord*)pr;
	ai->eslo=1.0;
	ai->roff=0;
	DEBUG(5,"aiStrParm:Convert roff=%ld, eslo=%f\n",ai->roff,ai->eslo);
	return 0;
}

