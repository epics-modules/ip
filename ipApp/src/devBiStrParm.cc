
// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2001/07/04 20:04:05  sluiter
// Creating
//
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
//

// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 10/28/95
// Converted to MPF, Mark Rivers 9/14/99

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
#include "biRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devBiStrParmDebug) printf(f,## v); }
#endif
volatile int devBiStrParmDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class BiStrParm : public DevMpf
{
public:
	BiStrParm(dbCommon*,DBLINK*);

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
	char zerostring[32];
	char onestring[32];
};

MAKE_DSET(devBiStrParm,BiStrParm::dev_init)

long BiStrParm::dev_init(void* v)
{
	biRecord* pr = (biRecord*)v;
	BiStrParm *pBiStrParm = new BiStrParm((dbCommon*)pr,&(pr->inp));
	pBiStrParm->bind();
	return 0;
}

BiStrParm::BiStrParm(dbCommon* pr, DBLINK* l) : DevMpf(pr,l,false)
{
	biRecord* bi = (biRecord*)pr;
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"BiStrParm::BiStrParm(%s)\n",pr->name);

	// initialize configurable parameters
	strcpy(term, "\r\n");
	termlen=2;
	buffer_start_index = pio->signal;
	timeout=1000;
	nchar=100;
	if (bi->desc[0]) {
		strncpy(format, bi->desc, 31);
	} else {
		strcpy(format, "%d");
	}
	zerostring[0] = 0;
	onestring[0] = 0;

	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"BiStrParm::BiStrParm pio->parm = '%s'\n", pio->parm);

		if ((p = strstr(pio->parm, "TERM="))) {
			termlen = 0;
			for (p+=5,i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++,p+=2) {
				term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
				DEBUG(2,"BiStrParm::BiStrParm term[i] = 0x%x\n", term[i]);
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

		if ((p = strstr(pio->parm, "0STR="))) {
			for (i=0, p+=5; i<31 && *p && *p != ','; i++, p++) {
				zerostring[i] = *p;
			}
			zerostring[i] = 0;
		}

		if ((p = strstr(pio->parm, "1STR="))) {
			for (i=0, p+=5; i<31 && *p && *p != ','; i++, p++) {
				onestring[i] = *p;
			}
			onestring[i] = 0;
		}
	}

	DEBUG(2,"BiStrParm::BiStrParm term = '%s'\n", term);
	DEBUG(2,"BiStrParm::BiStrParm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"BiStrParm::BiStrParm timeout = %d\n", timeout);
	DEBUG(2,"BiStrParm::BiStrParm format = %s\n", format);
	DEBUG(2,"BiStrParm::BiStrParm nchar = %d\n", nchar);
	DEBUG(2,"BiStrParm::BiStrParm zerostring = %s\n", zerostring);
	DEBUG(2,"BiStrParm::BiStrParm onestring = %s\n", onestring);
}

long BiStrParm::startIO(dbCommon* pr)
{

	DEBUG(2,"BiStrParm::startIO(%s)\n",pr->name);

        Char8ArrayMessage  *message = new Char8ArrayMessage;

        message->cmd = cmdRead | cmdSetEom;
        message->eomLen = termlen;
        message->extra = nchar;  // Maximum number of characters to read
        ::memcpy(message->eomString, term, 2);
        message->timeout = timeout/1000;
        return(sendReply(message));
}

long BiStrParm::completeIO(dbCommon* pr,Message* pm)
{
	biRecord* bi = (biRecord*)pr;
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
        int sz=0;
        int rc=0;
        int dlen=0;
        int i;

        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevBiStrParm::completeIO illegal message type %d\n",
                    bi->name,pm->getType());
            rc = -1;
        } else {
            sz = pcm->getSize();
            ::memcpy(buffer,pcm->value, sz);
            dlen = termlen;
            DEBUG(2,"BiStrParm::completeIO(%s)\n",pr->name);
            DEBUG(2," - data size=%d\n",sz);
            DEBUG(2," - return code=%d\n",pcm->status);
            DEBUG(3," - buffer=>%s<\n",buffer);

            // if no delimiter, timeout is expected
            if (dlen && (pcm->status!=0)) rc=-1;
        }

	if (sz && !rc && (buffer_start_index < sz)) {
		buffer[sz]='\0';
		// remove delimiter (of length dlen) unless timeout
		if (!pcm->status && ((unsigned int)sz < sizeof(buffer)) &&
                                         (sz >= dlen))
			(buffer[sz-dlen]) = '\0';

		if (*zerostring &&
			!strncmp(&buffer[buffer_start_index], zerostring, strlen(zerostring))) {
			bi->val = 0;
			bi->udf = 0;
		}
		if (bi->udf && *onestring &&
			!strncmp(&buffer[buffer_start_index], onestring, strlen(onestring))) {
			bi->val = 1;
			bi->udf = 0;
		}
		if (bi->udf) {
			i = sscanf(&buffer[buffer_start_index], format, &bi->val);
			DEBUG(2," - sscanf returns=%d\n",i);
			DEBUG(2," - bi->val=%d\n",bi->val);
			if (bi->val) bi->val = 1;
			bi->udf = (i!=1);
		}
		rc = 0;
	}
	else {
		bi->val=0;
		bi->udf = 1;
		rc = 0;
	}
        delete pcm;
        return rc;
}
