
// $Log: not supported by cvs2svn $
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
//

// Author: Jim Kowalkowski
// Revised: 2/15/95

// Mark Rivers 9/1/99
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
#define DEBUG(l,f,v...) { if(l<=devLoStrParmDebug) printf(f,## v); }
#endif
volatile int devLoStrParmDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class Lo : public DevMpf
{
public:
	Lo(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);

	static long dev_init(void*);
private:
	char buffer[100];
	int buffer_start_index;
	char term[10];
	char format[32];
	int termlen;
	int timeout;
};

MAKE_DSET(devLoStrParm,Lo::dev_init)

long Lo::dev_init(void* v)
{
	longoutRecord* pr = (longoutRecord*)v;
	new Lo((dbCommon*)pr,&(pr->out));
	return(0);
}

Lo::Lo(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	longoutRecord* lo = (longoutRecord*)pr;
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"Lo::Lo(%s)\n",pr->name);

	// initialize configurable parameters
	strcpy(term, "\r\n");
	termlen=2;
	buffer_start_index = pio->signal;
	timeout=3000;
	if (lo->desc[0]) {
		strncpy(format, lo->desc, 31);
	} else {
		strcpy(format, "%d");
	}

	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"LoStrParm::LoStrParm pio->parm = '%s'\n", pio->parm);

		if ((p = strstr(pio->parm, "TERM="))) {
			termlen = 0;
			for (p+=5,i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++,p+=2) {
				term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
				DEBUG(2,"LoStrParm::LoStrParm term[i] = 0x%x\n", term[i]);
				termlen++;
			}
			term[i] = 0;
		}

		if ((p = strstr(pio->parm, "IX="))) {
			buffer_start_index = atoi(&p[3]);
		}

		if ((p = strstr(pio->parm, "FMT="))) {
			for (i=0, p+=4; i<31 && *p && *p != ','; i++, p++) format[i] = *p;
			format[i] = 0;
		}

		if ((p = strstr(pio->parm, "TO="))) {
			timeout = atoi(&p[3]);
		}
	
        }
	DEBUG(2,"LoStrParm::LoStrParm term = '%s'\n", term);
	DEBUG(2,"LoStrParm::LoStrParm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"AoStrParm::AoStrParm timeout = %d\n", timeout);
	DEBUG(2,"LoStrParm::LoStrParm format = %s\n", format);
}

long Lo::startIO(dbCommon* pr)
{
	longoutRecord* lo = (longoutRecord*)pr;

	DEBUG(2,"Lo::StartIO(%s)\n",pr->name);

        Char8ArrayMessage *message = new Char8ArrayMessage;

        message->cmd = cmdWrite;
        message->timeout = timeout/1000;
        int max_size = sizeof(buffer);
        message->allocValue(max_size);
	sprintf(buffer, format, lo->val);
	strcpy(message->value, buffer);
	strncat(message->value, term, termlen);
        message->setSize(strlen(message->value));

	DEBUG(2,"value='%s'\n", message->value);

        return(sendReply(message));
}

long Lo::completeIO(dbCommon* pr,Message* pm)
{
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	int rc=0;

	DEBUG(2,"LoStrParm::CompleteIO(%s)\n",pr->name);
        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevLoStrParm::completeIO illegal message type %d\n",
                    pr->name,pm->getType());
            rc = -1;
        } else {
	    if(pcm->status!=0) rc=-1;
        }
	if (rc != 0) DEBUG(2,"LoStrParm::completeIO(%s), error=%d\n",
                                            pr->name, rc);

	delete pm;
	return rc;
}
