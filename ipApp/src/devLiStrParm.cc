
// $Log: not supported by cvs2svn $
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
//

// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 10/28/95
// Converted to MPF, Mark Rivers 9/5/99

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
#include "longinRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devLiStrParmDebug) printf(f,## v); }
#endif
volatile int devLiStrParmDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class LiStrParm : public DevMpf
{
public:
	LiStrParm(dbCommon*,DBLINK*);

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
	int nchar;
};

MAKE_DSET(devLiStrParm,LiStrParm::dev_init)

long LiStrParm::dev_init(void* v)
{
	longinRecord* pr = (longinRecord*)v;
	new LiStrParm((dbCommon*)pr,&(pr->inp));
	return 0;
}

LiStrParm::LiStrParm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	longinRecord* li = (longinRecord*)pr;
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"LiStrParm::LiStrParm(%s)\n",pr->name);

	// initialize configurable parameters
	strcpy(term, "\r\n");
	termlen=2;
	buffer_start_index = pio->signal;
	timeout=1000;
	nchar=100;

	if (li->desc[0]) {
		strncpy(format, li->desc, 31);
	} else {
		strcpy(format, "%ld");
	}

	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"LiStrParm::LiStrParm pio->parm = '%s'\n", pio->parm);

		if ((p = strstr(pio->parm, "TERM="))) {
			termlen = 0;
			for (p+=5,i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++,p+=2) {
				term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
				DEBUG(2,"LiStrParm::LiStrParm term[i] = 0x%x\n", term[i]);
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

		if ((p = strstr(pio->parm, "N="))) {
			nchar = atoi(&p[2]);
		}
	}

	DEBUG(2,"LiStrParm::LiStrParm term = '%s'\n", term);
	DEBUG(2,"LiStrParm::LiStrParm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"LiStrParm::LiStrParm timeout = %d\n", timeout);
	DEBUG(2,"LiStrParm::LiStrParm format = %s\n", format);
	DEBUG(2,"LiStrParm::LiStrParm nchar = %d\n", nchar);
}

long LiStrParm::startIO(dbCommon* pr)
{

	DEBUG(2,"LiStrParm::startIO(%s')\n",pr->name);

        Char8ArrayMessage  *message = new Char8ArrayMessage;

        message->cmd = cmdRead | cmdSetEom;
        message->eomLen = termlen;
        message->extra = nchar;  // Maximum number of characters to read
        ::memcpy(message->eomString, term, 2);
        message->timeout = timeout/1000;
        return(sendReply(message));
}

long LiStrParm::completeIO(dbCommon* pr,Message* pm)
{
	longinRecord* li = (longinRecord*)pr;
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
        int sz=0;
        int rc=0;
        int dlen=0;
        int i;

        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevLiStrParm::completeIO illegal message type %d\n",
                    li->name,pm->getType());
            rc = -1;
        } else {
            sz = pcm->getSize();
            ::memcpy(buffer,pcm->value, sz);
            dlen = termlen;
            DEBUG(2,"LiStrParm::completeIO(%s)\n",pr->name);
            DEBUG(2," - data size=%d\n",sz);
            DEBUG(2," - return code=%d\n",pcm->status);
            DEBUG(3," - buffer=>%s<\n",buffer);

            // if no delimiter, timeout is expected
            if (dlen && (pcm->status!=0)) rc=-1;
        }

        if (sz && !rc && (buffer_start_index < sz)) {
                buffer[sz]='\0';
                DEBUG(2," - buffer='%s'\n",buffer);
                // remove delimiter (of length dlen) unless timeout
                if (!pcm->status && ((unsigned int)sz < sizeof(buffer)) &&
                                                          (sz >= dlen))
                        buffer[sz-dlen] = '\0';
		i = sscanf(&buffer[buffer_start_index], format, &li->val);
		DEBUG(2," - sscanf returns=%d\n",i);
		DEBUG(2," - li->val=%ld\n",li->val);
		li->udf = (i!=1);
		rc = 0;
        }
        else {
		li->val=0;
		li->udf = 1;
		rc = 0;
        }
        delete pcm;
        return rc;
}
