
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/10/29 23:12:30  rivers
// Added bind call for DevMpf
//
// Revision 1.1.1.1  2001/07/04 20:04:05  sluiter
// Creating
//
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
//

// Author: Tim Mooney (based on code written by Jim Kowalkowski)
// Revised: 10/28/95
// Converted to MPF, Mark Rivers 9/5/99

#include <string.h>
#include <stdio.h>

#include "dbAccess.h"
#include "dbDefs.h"
#include "link.h"
#include "epicsPrint.h"
#include "dbCommon.h"
#include "aiRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devAiStrParmDebug) printf(f,## v); }
#endif
volatile int devAiStrParmDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class AiStrParm : public DevMpf
{
public:
        AiStrParm(dbCommon*,DBLINK*);

        long startIO(dbCommon* pr);
        long completeIO(dbCommon* pr,Message* m);
        virtual long convert(dbCommon* pr,int pass);

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

MAKE_LINCONV_DSET(devAiStrParm,AiStrParm::dev_init)

long AiStrParm::dev_init(void* v)
{
        aiRecord* pr = (aiRecord*)v;
        AiStrParm *pAiStrParm = new AiStrParm((dbCommon*)pr,&(pr->inp));
        pAiStrParm->bind();
        return 0;
}

AiStrParm::AiStrParm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
        aiRecord* ai = (aiRecord*)pr;
        int i;
        vmeio* pio = (vmeio*)&(l->value);
        char *p;

        DEBUG(2,"AiStrParm::AiStrParm(%s)\n",pr->name);

        // initialize configurable parameters
        strcpy(term, "\r\n");
        termlen=2;
        buffer_start_index = pio->signal;
        timeout=1000;
        nchar=100;
        if (ai->desc[0]) {
                strncpy(format, ai->desc, 31);
        } else {
                strcpy(format, "%lf");
        }

        // get any configurable parameters from parm field
        if (pio->parm) {
                DEBUG(2,"AiStrParm::AiStrParm pio->parm = '%s'\n", pio->parm);

                if ((p = strstr(pio->parm, "TERM="))) {
                        termlen = 0;
                        for (p+=5,i=0; i<9 && isxdigit(p[0]) && isxdigit(p[1]); i++,p+=2) {
                                term[i] = HEXCHAR2INT(p[0])*16 + HEXCHAR2INT(p[1]);
                                DEBUG(2,"AiStrParm::AiStrParm term[i] = 0x%x\n", term[i]);
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

        DEBUG(2,"AiStrParm::AiStrParm term = '%s'\n", term);
        DEBUG(2,"AiStrParm::AiStrParm buffer_start_index = %d\n", buffer_start_index);
        DEBUG(2,"AiStrParm::AiStrParm timeout = %d\n", timeout);
        DEBUG(2,"AiStrParm::AiStrParm format = %s\n", format);
        DEBUG(2,"AiStrParm::AiStrParm nchar = %d\n", nchar);
}

long AiStrParm::startIO(dbCommon* pr)
{

        DEBUG(2,"AiStrParm::startIO(%s)\n",pr->name);

        Char8ArrayMessage  *message = new Char8ArrayMessage;

        message->cmd = cmdRead | cmdSetEom;
        message->eomLen = termlen;
        message->extra = nchar;  // Maximum number of characters to read
        ::memcpy(message->eomString, term, 2);
        message->timeout = timeout/1000;
        return(sendReply(message));
}

long AiStrParm::completeIO(dbCommon* pr,Message* pm)
{
        Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
        aiRecord* ai = (aiRecord*)pr;
        int sz=0;
        int rc=0;
        int dlen=0;
        int i;

        if((pm->getType()) != messageTypeChar8Array) {
            epicsPrintf("%s DevAiStrParm::completeIO illegal message type %d\n",
                    ai->name,pm->getType());
            rc = -1;
        } else {
            sz = pcm->getSize();
            ::memcpy(buffer,pcm->value, sz);
            dlen = termlen;
            DEBUG(2,"AiStrParm::CompleteIO(%s)\n",pr->name);
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
                i = sscanf(&buffer[buffer_start_index], format, &ai->val);
                DEBUG(2," - sscanf returns=%d\n",i);
                DEBUG(2," - ai->val=%f\n",ai->val);
                ai->udf = (i!=1);
                rc = 2;
        }
        else {
                ai->val=0.0;
                ai->udf = 1;
                rc = 2;
        }

        delete pcm;
        return rc;
}

long AiStrParm::convert(dbCommon* pr,int pass)
{
        aiRecord* ai = (aiRecord*)pr;
        ai->eslo=1.0;
        ai->roff=0;
        DEBUG(5,"aiStrParm:Convert roff=%d, eslo=%f\n",ai->roff,ai->eslo);
        return 0;
}
