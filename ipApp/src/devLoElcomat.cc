// Author: Tim Mooney (based on Hideos code written by Jim Kowalkowski)
// Converted to MPF 9/13/02  tmm

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
#include "longoutRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devLoElcomatDebug) printf(f,## v); }
#endif
volatile int devLoElcomatDebug = 0;
}

#define BUFSZ 10

class LoElcomat : public DevMpf
{
public:
	LoElcomat(dbCommon*, DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr, Message* m);

	static long dev_init(void*);
private:
	char buffer[BUFSZ];
};

MAKE_DSET(devLoElcomat, LoElcomat::dev_init)

long LoElcomat::dev_init(void* v)
{
	longoutRecord* pr = (longoutRecord*)v;
	LoElcomat *pLoElcomat = new LoElcomat((dbCommon*)pr,&(pr->out));
	pLoElcomat->bind();
	return(0);
}

LoElcomat::LoElcomat(dbCommon* pr, DBLINK* l) : DevMpf(pr,l,false)
{
	DEBUG(2,"LoElcomat::LoElcomat(pr,l)\n");
}

long LoElcomat::startIO(dbCommon* pr)
{
	longoutRecord* lo = (longoutRecord*)pr;

	DEBUG(2,"LoElcomat::StartIO(pr)\n");

	Char8ArrayMessage *message = new Char8ArrayMessage;
	message->cmd = cmdWrite | cmdFlush;
	if (lo->val <= 0xc0) {
		buffer[0] = (char)2;
		buffer[1] = (char)lo->val;
		buffer[2] = (char)3;
	}
	message->allocValue(3);
	::memcpy(message->value, buffer, 3);
	message->setSize(3);
	return sendReply(message);
}

long LoElcomat::completeIO(dbCommon* pr, Message* pm)
{
	Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;
	long rc=0;

	DEBUG(2,"LoElcomat::completeIO(pr,m)\n");
	if (pcm->status != 0) rc=-1;

	delete pm;
	return rc;
}

