
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/01/07 16:06:21  sluiter
// Copied from synApps R4.3
//
// Revision 1.2  1995/03/21  19:44:08  jbk
// added comments and such
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
#include "aoRecord.h"
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
#define DEBUG(l,f,v...) { if(l<=devAoEurothermDebug) printf(f,## v); }
#endif
volatile int devAoEurothermDebug = 0;
}

#define HEXCHAR2INT(c) ((c)>'9' ? 10+tolower(c)-'a' : tolower(c)-'0')

class AoEurotherm : public DevMpf
{
public:
	AoEurotherm(dbCommon*,DBLINK*);

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
	int group_address;
	int local_address;
};

MAKE_LINCONV_DSET(devAoEurotherm,AoEurotherm::dev_init)

long AoEurotherm::dev_init(void* v)
{
	aoRecord* pr = (aoRecord*)v;
	new AoEurotherm((dbCommon*)pr,&(pr->out));
	return 0;
}

AoEurotherm::AoEurotherm(dbCommon* pr,DBLINK* l) : DevMpf(pr,l,false)
{
	aoRecord* ao = (aoRecord*)pr;
	int i;
	vmeio* pio = (vmeio*)&(l->value);
	char *p;

	DEBUG(2,"AoEurotherm::AoEurotherm(%s)\n",pr->name);

	// initialize configurable parameters
	term[0] = 3; // ETX
	term[1] = 0;
	termlen = 1;
	buffer_start_index = pio->signal;
	timeout = 3000;
	group_address = 0;
	/*
	 * Default local address for Eurotherm 800 series is 0.  For the Eurotherm 2000
	 * series, local address 0 broadcasts to all devices, and the default local
	 * address is 1.
	 */
	local_address = 0;

	if (ao->desc[0]) {
		strncpy(format, ao->desc, 31);
	} else {
		strcpy(format, "%f");
	}


	// get any configurable parameters from parm field
	if (pio->parm) {
		DEBUG(2,"AoEurotherm::AoEurotherm pio->parm = '%s'\n", pio->parm);

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

		if ((p = strstr(pio->parm, "GAD="))) {
			group_address = atoi(&p[4]);
		}

		if ((p = strstr(pio->parm, "LAD="))) {
			local_address = atoi(&p[4]);
		}
	}

	DEBUG(2,"AoEurotherm::AoEurotherm term = '%s'\n", term);
	DEBUG(2,"AoEurotherm::AoEurotherm buffer_start_index = %d\n", buffer_start_index);
	DEBUG(2,"AoEurotherm::AoEurotherm timeout = %d\n", timeout);
	DEBUG(2,"AoEurotherm::AoEurotherm format = %s\n", format);
}

long AoEurotherm::startIO(dbCommon* pr)
{
	aoRecord* ao = (aoRecord*)pr;
	Char8ArrayMessage *message = new Char8ArrayMessage;
	char checksum;
	int i;

	DEBUG(2,"AoEurotherm::startIO(%s)\n",pr->name);

	message->cmd = cmdWrite;
	message->timeout = timeout/1000;
	int max_size = sizeof(buffer);
	message->allocValue(max_size);
	// write message
	buffer[0] = 4; // EOT
	buffer[1] = '0' + group_address; // group address
	buffer[2] = '0' + group_address; // group address repeated
	buffer[3] = '0' + local_address; // local address
	buffer[4] = '0' + local_address; // local address repeated
	buffer[5] = 2; // STX
	sprintf(&buffer[6], format, ao->val);
	strncat(buffer, term, termlen);
	// add checksum
	for (i=7, checksum=buffer[6]; buffer[i] && i < 98; i++) checksum ^= buffer[i];
	buffer[i] = checksum;
	buffer[i+1] = 0;
	if (devAoEurothermDebug >= 2) { 
		printf("buffer = '"); 
		for (i=0; buffer[i]; i++) printf("%2.2x ", (unsigned int)buffer[i]);
		printf("'\n"); 
	}
	// send to MPF
	strcpy(message->value, buffer);
	message->setSize(strlen(message->value));
	DEBUG(2,"value='%s'\n", message->value);
	return(sendReply(message));
}

long AoEurotherm::completeIO(dbCommon* pr,Message* pm)
{
	DEBUG(2,"AoEurotherm::completeIO(%s)\n",pr->name);
	Char8ArrayMessage *pcm = (Char8ArrayMessage *)pm;;
	int rc=0;

	DEBUG(2,"AoEurotherm::completeIO(%s)\n",pr->name);
	if ((pm->getType()) != messageTypeChar8Array) {
		epicsPrintf("%s DevAoEurotherm::completeIO illegal message type %d\n",
			pr->name,pm->getType());
		rc = -1;
	} else {
		if(pcm->status!=0) rc=-1;
	}
	if (rc != 0) DEBUG(2,"AoEurotherm::completeIO(%s), error=%d\n",
                                            pr->name, rc);

	delete pm;
	return rc;
}

