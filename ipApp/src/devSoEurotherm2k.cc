
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/02/10 17:07:11  bcda
// Add support for Eurotherm 2000 series temperature controllers -KAG
//
// Revision 1.3  1995/04/12  16:23:45  jbk
// new stuff added
//

// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 09/08/97
// Revised 09/05/01 Tim Mooney, converted to MPF
// Revised 12/17/02 Kurt Goetze, modified to work with model 2408

#include <string.h>
#include <stdio.h>

#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "epicsPrint.h"
#include "dbCommon.h"
#include "stringoutRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devSoEurotherm2kDebug) printf(f,## v); }
#endif
volatile int devSoEurotherm2kDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class SoEurotherm2k : public DevMpf
{
public:
	SoEurotherm2k(dbCommon*,DBLINK*);

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

MAKE_DSET(devSoEurotherm2k,SoEurotherm2k::dev_init)

long SoEurotherm2k::dev_init(void* v)
{
	stringoutRecord* pr = (stringoutRecord*)v;
	new SoEurotherm2k((dbCommon*)pr,&(pr->out));
	return 0;
}

SoEurotherm2k::SoEurotherm2k(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"SoEurotherm2k::SoEurotherm2k(%s)\n",pr->name);

	// initialize configurable parameters
	term[0] = 5; // ENQ
	term[1] = 0;
	termlen=1;
	buffer_start_index = pio->signal;
	timeout=3000;

	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"SoEurotherm2k::SoEurotherm2k pio->parm = '%s'\n", pio->parm);
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
	DEBUG(2,"SoEurotherm2k::SoEurotherm2k term = '%s'\n", term);
	DEBUG(2,"SoEurotherm2k::SoEurotherm2k buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"SoEurotherm2k::SoEurotherm2k timeout = %d\n", timeout);
}

long SoEurotherm2k::startIO(dbCommon* pr)
{
	stringoutRecord* so = (stringoutRecord*)pr;

	DEBUG(2,"SoEurotherm2k::startIO(%s)\n",pr->name);

	Char8ArrayMessage *message = new Char8ArrayMessage;

	message->cmd = cmdWrite;
	message->timeout = timeout/1000;
	int max_size = sizeof(so->val) + sizeof(term);
	message->allocValue(max_size);
	buffer[0] = 4; // EOT
	buffer[1] = '0'; // group address
	buffer[2] = '0'; // group address repeated
	buffer[3] = '1'; // local address
	buffer[4] = '1'; // local address repeated
	buffer[5] = 0;

	strncat(buffer, &(so->val[buffer_start_index]),
		sizeof(buffer)-strlen(buffer));
	strncat(buffer, term, sizeof(buffer)-strlen(buffer));

	strcpy(message->value, buffer);
	message->setSize(strlen(message->value));
	DEBUG(3,"SoEurotherm2k::StartIO(%s), sent message %s\n", 
                                        pr->name, message->value);
	return(sendReply(message));
}

long SoEurotherm2k::completeIO(dbCommon* pr,Message* pm)
{
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	stringoutRecord* so = (stringoutRecord*)pr;
	int rc=0;

	DEBUG(2,"SoEurotherm2k::CompleteIO(%s)\n", pr->name);
        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevSoEurotherm2k::completeIO illegal message type %d\n",
                    so->name,pm->getType());
            rc = -1;
        } else {
	    if(pcm->status!=0) rc=-1;
        }
	if (rc != 0) DEBUG(2,"SoEurotherm2k::completeIO(%s), error=%d\n",
                                            so->name, rc);

	delete pm;
	return rc;
}
