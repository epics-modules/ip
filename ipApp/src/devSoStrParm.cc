
// $Log: not supported by cvs2svn $
// Revision 1.3  1995/04/12  16:23:45  jbk
// new stuff added
//
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
//
// Author: Jim Kowalkowski
// Revised: 2/15/95
//
// Mark Rivers 8/20/99
// Converted to MPF

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
#define DEBUG(l,f,v...) { if(l<=devSoStrParmDebug) printf(f,## v); }
#endif
volatile int devSoStrParmDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class SoStrParm : public DevMpf
{
public:
	SoStrParm(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);

	static long dev_init(void*);
private:
	char buffer[100];
	int buffer_start_index;
	char term[10];
	int termlen;
	int timeout;
};

MAKE_DSET(devSoStrParm,SoStrParm::dev_init)

long SoStrParm::dev_init(void* v)
{
	stringoutRecord* pr = (stringoutRecord*)v;
	new SoStrParm((dbCommon*)pr,&(pr->out));
	return(0);
}

SoStrParm::SoStrParm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"SoStrParm::SoStrParm(%s)\n",pr->name);

	// initialize configurable parameters
	strcpy(term, "\r\n");
	termlen=2;
	buffer_start_index = pio->signal;
	timeout=3000;

	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"SoStrParm::SoStrParm pio->parm = '%s'\n", pio->parm);
		if ((p = strstr(pio->parm, "TERM="))) {
			termlen = 0;
			for (p+=5,i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++,p+=2) {
				term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
				termlen++;
			}
			term[i] = 0;
		}

		if ((p = strstr(pio->parm, "IX="))) {
			buffer_start_index = atoi(&p[3]);
		}

		if ((p = strstr(pio->parm, "TO="))) {
			timeout = atoi(&p[3]);
		}
	}
	DEBUG(2,"SoStrParm::SoStrParm term = '%s'\n", term);
	DEBUG(2,"SoStrParm::SoStrParm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"SoStrParm::SoStrParm timeout = %d\n", timeout);
}

long SoStrParm::startIO(dbCommon* pr)
{
	stringoutRecord* so = (stringoutRecord*)pr;

	DEBUG(2,"%.2f SoStrParm::StartIO(%s)\n", tickGet()/60., pr->name);

        Char8ArrayMessage *message = new Char8ArrayMessage;

        message->cmd = cmdWrite;
        message->timeout = timeout/1000;
        int max_size = sizeof(so->val) + sizeof(term);
        message->allocValue(max_size);
	strncpy(message->value, &(so->val[buffer_start_index]), 
                                            sizeof(so->val));
	strncat(message->value, term, termlen);
        message->setSize(strlen(message->value));
	DEBUG(3,"SoStrParm::StartIO(%s), sent message %s\n", 
                                        pr->name, message->value);
        return(sendReply(message));

}

long SoStrParm::completeIO(dbCommon* pr,Message* pm)
{
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	stringoutRecord* so = (stringoutRecord*)pr;
	int rc=0;

	DEBUG(2,"%.2f SoStrParm::CompleteIO(%s)\n", tickGet()/60., pr->name);
        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevSoStrParm::completeIO illegal message type %d\n",
                    so->name,pm->getType());
            rc = -1;
        } else {
	    if(pcm->status!=0) rc=-1;
        }
	if (rc != 0) DEBUG(2,"SoStrParm::completeIO(%s), error=%d\n",
                                            so->name, rc);

	delete pm;
	return rc;
}
