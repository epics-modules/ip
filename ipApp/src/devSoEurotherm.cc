
// $Log: not supported by cvs2svn $
// Revision 1.5  2003/08/03 17:37:50  rivers
// Removed call to tickGet()
//
// Revision 1.4  2003/07/09 21:40:29  goetze
// Merged Mark's and Tim's changes.
//
// Revision 1.2.2.1  2003/07/09 18:33:15  goetze
// kag- general version with bind call
//
// Revision 1.2  2002/10/24 02:58:30  rivers
// Added bind call for DevMpf
//
// Revision 1.1  2002/01/07 16:06:35  sluiter
// Copied from synApps R4.3
//
// Revision 1.3  1995/04/12  16:23:45  jbk
// new stuff added
//
// This module implements the Eurotherm EI-Bisynch protocol to talk
// to selected Eurotherm temperature cntrollers.  At present, selected
// commands for 800-series and 2000-series controllers are known to
// work.  This module makes no attempt at the MODBUS protocol also
// supported by Eurotherm 2000-series temperature controllers.  You
// must ensure that the device is using bisynch.
//
// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 09/08/97
// Revised 09/05/01 Tim Mooney, converted to MPF
// Reviced 02/14/03 Tim Mooney, can specify group and local address in
//                              vmeio parm string
// Revised 07/09/03 Kurt Goetze, merged Tim's changes with Mark's

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
#ifdef __GNUG__
#define DEBUG(l,f,v...) { if(l<=devSoEurothermDebug) printf(f ,## v); }
#else
#ifdef __SUNPRO_CC
#define DEBUG(l,...) { if(l<=devSoEurothermDebug) printf(__VA_ARGS__); }
#else
#define DEBUG(l,f,v) { if(l<=devSoEurothermDebug) printf(f,v); }
#endif
#endif
#endif
volatile int devSoEurothermDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class SoEurotherm : public DevMpf
{
public:
	SoEurotherm(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);

	static long dev_init(void*);
private:
	char buffer[100];
	int buffer_start_index;
	char term[10];
	int termlen;
	int timeout;
	int group_address;
	int local_address;
};

MAKE_DSET(devSoEurotherm,SoEurotherm::dev_init)

long SoEurotherm::dev_init(void* v)
{
	stringoutRecord* pr = (stringoutRecord*)v;
/*	new SoEurotherm((dbCommon*)pr,&(pr->out)); */
	SoEurotherm *pSoEurotherm = new SoEurotherm((dbCommon*)pr,&(pr->out));
	pSoEurotherm->bind();
	return 0;
}

SoEurotherm::SoEurotherm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"SoEurotherm::SoEurotherm(%s)\n",pr->name);

	// initialize configurable parameters
	term[0] = 5; // ENQ
	term[1] = 0;
	termlen=1;
	buffer_start_index = pio->signal;
	timeout=3000;

	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"SoEurotherm::SoEurotherm pio->parm = '%s'\n", pio->parm);
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

		if ((p = strstr(pio->parm, "GAD="))) {
			group_address = atoi(&p[4]);
		}

		if ((p = strstr(pio->parm, "LAD="))) {
			local_address = atoi(&p[4]);
		}
	}
	DEBUG(2,"SoEurotherm::SoEurotherm term = '%s'\n", term);
	DEBUG(2,"SoEurotherm::SoEurotherm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"SoEurotherm::SoEurotherm timeout = %d\n", timeout);
}

long SoEurotherm::startIO(dbCommon* pr)
{
	stringoutRecord* so = (stringoutRecord*)pr;

	DEBUG(2,"SoEurotherm::startIO(%s)\n",pr->name);

	Char8ArrayMessage *message = new Char8ArrayMessage;

	message->cmd = cmdWrite;
	message->timeout = timeout/1000;
	int max_size = sizeof(so->val) + sizeof(term);
	message->allocValue(max_size);
	buffer[0] = 4; // EOT
	buffer[1] = '0' + group_address; // group address
	buffer[2] = '0' + group_address; // group address repeated
	buffer[3] = '0' + local_address; // local address
	buffer[4] = '0' + local_address; // local address repeated
	buffer[5] = 0;

	strncat(buffer, &(so->val[buffer_start_index]),
		sizeof(buffer)-strlen(buffer));
	strncat(buffer, term, sizeof(buffer)-strlen(buffer));

	strcpy(message->value, buffer);
	message->setSize(strlen(message->value));
	DEBUG(3,"SoEurotherm::StartIO(%s), sent message %s\n", 
                                        pr->name, message->value);
	return(sendReply(message));
}

long SoEurotherm::completeIO(dbCommon* pr,Message* pm)
{
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	stringoutRecord* so = (stringoutRecord*)pr;
	int rc=0;

	DEBUG(2, "SoEurotherm::CompleteIO(%s)\n", pr->name);
        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevSoEurotherm::completeIO illegal message type %d\n",
                    so->name,pm->getType());
            rc = -1;
        } else {
	    if(pcm->status!=0) rc=-1;
        }
	if (rc != 0) DEBUG(2,"SoEurotherm::completeIO(%s), error=%d\n",
                                            so->name, rc);

	delete pm;
	return rc;
}
