
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/10/24 02:58:30  rivers
// Added bind call for DevMpf
//
// Revision 1.1.1.1  2001/07/04 20:04:05  sluiter
// Creating
//
//

// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 10/27/95
//          09/14/99 Mark Rivers, converted to MPF

#include <string.h>
#include <stdio.h>

#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "epicsPrint.h"
#include "dbCommon.h"
#include "aoRecord.h"
#include "recSup.h"

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
#define DEBUG(l,f,v...) { if(l<=devAoStrParmDebug) printf(f,## v); }
#endif
volatile int devAoStrParmDebug = 0;
}


#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class AoStrParm : public DevMpf
{
public:
	AoStrParm(dbCommon*,DBLINK*);

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

MAKE_LINCONV_DSET(devAoStrParm,AoStrParm::dev_init)

long AoStrParm::dev_init(void* v)
{
	aoRecord* pr = (aoRecord*)v;
	AoStrParm *pAoStrParm = new AoStrParm((dbCommon*)pr,&(pr->out));
	pAoStrParm->bind();
	return 0;
}

AoStrParm::AoStrParm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	aoRecord* ao = (aoRecord*)pr;
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"AoStrParm::AoStrParm(%s)\n",pr->name);

	// initialize configurable parameters
	strcpy(term, "\r\n");
	termlen=2;
	buffer_start_index = pio->signal;
	timeout=3000;
	if (ao->desc[0]) {
		strncpy(format, ao->desc, 31);
	} else {
		strcpy(format, "%f");
	}


	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"AoStrParm::AoStrParm pio->parm = '%s'\n", pio->parm);

		if ((p = strstr(pio->parm, "TERM="))) {
			termlen = 0;
			for (p+=5, i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++, p+=2) {
				term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
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

	DEBUG(2,"AoStrParm::AoStrParm term = '%s'\n", term);
	DEBUG(2,"AoStrParm::AoStrParm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"AoStrParm::AoStrParm timeout = %d\n", timeout);
	DEBUG(2,"AoStrParm::AoStrParm format = %s\n", format);
}

long AoStrParm::startIO(dbCommon* pr)
{
	aoRecord* ao = (aoRecord*)pr;
        Char8ArrayMessage *message = new Char8ArrayMessage;

	DEBUG(2,"AoStrParm::StartIO(%s)\n",pr->name);
        message->cmd = cmdWrite;
        message->timeout = timeout/1000;
        int max_size = sizeof(buffer);
        message->allocValue(max_size);
	sprintf(buffer, format, ao->val);
	strcpy(message->value, buffer);
	strncat(message->value, term, termlen);
        message->setSize(strlen(message->value));

	DEBUG(2,"value='%s'\n", message->value);
        return(sendReply(message));


}

long AoStrParm::completeIO(dbCommon* pr,Message* pm)
{
	DEBUG(2,"AoStrParm::CompleteIO(%s)\n",pr->name);
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	int rc=0;

	DEBUG(2,"AoStrParm::CompleteIO(%s)\n",pr->name);
        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevAoStrParm::completeIO illegal message type %d\n",
                    pr->name,pm->getType());
            rc = -1;
        } else {
	    if(pcm->status!=0) rc=-1;
        }
	if (rc != 0) DEBUG(2,"AoStrParm::completeIO(%s), error=%d\n",
                                            pr->name, rc);

	delete pm;
	return rc;
}
