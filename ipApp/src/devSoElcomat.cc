
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
#include "stringoutRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devSoElcomatDebug) printf(f,## v); }
#endif
volatile int devSoElcomatDebug = 0;
}

class SoElcomat : public DevMpf
{
public:
	SoElcomat(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);

	static long dev_init(void*);
private:
	char buffer[100];
};

MAKE_DSET(devSoElcomat,SoElcomat::dev_init)

long SoElcomat::dev_init(void* v)
{
	stringoutRecord* pr = (stringoutRecord*)v;
	new SoElcomat((dbCommon*)pr,&(pr->out));
	return(0);
}

SoElcomat::SoElcomat(dbCommon* pr, DBLINK* l) : DevMpf(pr,l,false)
{
	DEBUG(2,"SoElcomat::SoElcomat(pr,l)\n");
}

long SoElcomat::startIO(dbCommon* pr)
{
	stringoutRecord* so = (stringoutRecord*)pr;
	Char8ArrayMessage *message = new Char8ArrayMessage;

	DEBUG(2,"SoElcomat::StartIO(pr)\n");

	message->cmd = cmdWrite;
	message->timeout = 3;
	int max_size = sizeof(so->val) + 2;
	message->allocValue(max_size);
	strncpy(message->value, so->val, sizeof(so->val));
	strcat(message->value, "\r\n");
	message->setSize(strlen(message->value));
	DEBUG(3,"SoStrParm::StartIO(%s), sent message %s\n", 
                                        pr->name, message->value);
	return(sendReply(message));
}

long SoElcomat::completeIO(dbCommon* pr,Message* pm)
{
	Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	stringoutRecord* so = (stringoutRecord*)pr;
	int rc=0;

	DEBUG(2,"%.2f SoElcomat::completeIO(%s)\n", tickGet()/60., pr->name);
	if ((pm->getType()) != messageTypeChar8Array) {
		epicsPrintf("%s DevSoStrParm::completeIO illegal message type %d\n",
			so->name,pm->getType());
		rc = -1;
	} else {
		if (pcm->status!=0) rc=-1;
	}
	if (rc != 0) DEBUG(2,"SoElcomat::completeIO(%s), error=%d\n",
                                            so->name, rc);

	delete pm;
	return rc;
}

