
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/07/08 19:53:52  rivers
// Removed vxWorks specific code
//
// Revision 1.2  2002/10/24 02:58:30  rivers
// Added bind call for DevMpf
//
// Revision 1.1.1.1  2001/07/04 20:04:05  sluiter
// Creating
//
// Revision 1.4  1995/04/12  16:23:44  jbk
// new stuff added
//
// Revision 1.3  1995/03/23  17:43:11  jbk
// fix to give data even if time out occurred
//
// Revision 1.2  1995/03/21  19:44:06  jbk
// added comments and such
//
// 1999/08/15  Mark Rivers - Converted to MPF
// 1999/10/7   Mark Rivers - Enhanced debugging
//

// Author: Jim Kowalkowski
// Revised: 2/15/95

#include <string.h>
#include <stdio.h>

#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "epicsPrint.h"
#include "dbCommon.h"
#include "stringinRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devSiStrParmDebug) printf(f ,## v); }
#else
#ifdef __SUNPRO_CC
#define DEBUG(l,...) { if(l<=devSiStrParmDebug) printf(__VA_ARGS__); }
#else
#define DEBUG(l,f,v) { if(l<=devSiStrParmDebug) printf(f,v); }
#endif
#endif
#endif
volatile int devSiStrParmDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class SiStrParm : public DevMpf
{
public:
	SiStrParm(dbCommon*,DBLINK*);

	long startIO(dbCommon* pr);
	long completeIO(dbCommon* pr,Message* m);

	static long dev_init(void*);
private:
	char buffer[100];
	int buffer_start_index;
	char term[10];
	int termlen;
	int timeout;
	int nchar;
};

MAKE_DSET(devSiStrParm,SiStrParm::dev_init)

long SiStrParm::dev_init(void* v)
{
	stringinRecord* pr = (stringinRecord*)v;
	SiStrParm *pSiStrParm = new SiStrParm((dbCommon*)pr,&(pr->inp));
	pSiStrParm->bind();
	return(0);
}

SiStrParm::SiStrParm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"SiStrParm::SiStrParm(%s)\n",pr->name);

	// initialize configurable parameters
	::strcpy(term, "\r\n");
	termlen=2;
	buffer_start_index = pio->signal;
	timeout=1000;
	nchar=100;

	// get any configurable parameters from parm field
        if (pio->parm) {
		DEBUG(2,"SiStrParm::SiStrParm pio->parm = '%s'\n", pio->parm);
		if ((p = strstr(pio->parm, "TERM="))) {
			termlen = 0;
			for (p+=5,i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++,p+=2) {
				term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
				DEBUG(2,"SiStrParm::SiStrParm term[i] = 0x%x\n", term[i]);
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

		if ((p = strstr(pio->parm, "N="))) {
			nchar = atoi(&p[2]);
		}
	}

	DEBUG(2,"SiStrParm::SiStrParm term = '%s'\n", term);
	DEBUG(2,"SiStrParm::SiStrParm buffer_start_index = %d\n", 
                                                    buffer_start_index);
	DEBUG(2,"SiStrParm::SiStrParm timeout = %d\n", timeout);
	DEBUG(2,"SiStrParm::SiStrParm nchar = %d\n", nchar);
}

long SiStrParm::startIO(dbCommon* pr)
{
	DEBUG(2,"SiStrParm::StartIO(%s)\n", pr->name);
        Char8ArrayMessage *message = new Char8ArrayMessage;

        message->cmd = cmdRead | cmdSetEom;
        message->eomLen = termlen;
        message->extra = nchar;  // Maximum number of characters to read
	::strncpy(message->eomString, term, 2);
        message->timeout = timeout/1000;
        return(sendReply(message));

}

long SiStrParm::completeIO(dbCommon* pr,Message* pm)
{
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
        int sz=0;
        int rc=0;
        int dlen=0;

	stringinRecord* si = (stringinRecord*)pr;
        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevSiStrParm::completeIO illegal message type %d\n",
                    si->name,pm->getType());
            rc = -1;
        } else {
            sz = pcm->getSize();
            ::memcpy(buffer,pcm->value, sz);
      	    dlen = termlen;
	    DEBUG(2,"SiStrParm::CompleteIO(%s), size=%d, status=%d\n",
                        pr->name, sz, pcm->status);
	    DEBUG(3," - buffer=>%s<\n",buffer);

	    // if no delimiter, timeout is expected
	    if (dlen && (pcm->status!=0)) rc=-1; 
        }


	if (sz && !rc && (buffer_start_index < sz)) {
		buffer[sz]='\0';
		// remove delimiter (of length dlen) unless timeout
		if (!pcm->status && ((unsigned int)sz < sizeof(buffer)) && 
                                                          (sz >= dlen))
			buffer[sz-dlen] = '\0';
		strncpy(si->val,&buffer[buffer_start_index],sizeof(si->val));
		si->udf=0;
	}
	else {
		si->val[0]='\0';
		si->udf=1;
	}

	delete pcm;
	return rc;
}

