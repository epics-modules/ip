/* gpibRecord.c - Record Support Routines for Generic GPIB record */
/*
 *      Author: 	Mark Rivers
 *      Date:   	3/22/96
 *
 * Modification Log:
 * -----------------
 * .01  06-03-97 tmm  Conversion to EPICS 3.13.
 * .02  02-Mar-2004 mlr  Conversion to EPICS 3.14 and asyn
 */


#include        <string.h>
#include        <stdio.h>
#include        <stdlib.h>

#include        <asynDriver.h>
#include        <asynOctet.h>
#include        <asynGpibDriver.h>
#include        <alarm.h>
#include        <dbDefs.h>
#include        <dbEvent.h>
#include        <dbAccess.h>
#include        <dbFldTypes.h>
#include        <devSup.h>
#include        <drvSup.h>
#include        <errMdef.h>
#include        <recSup.h>
#include        <recGbl.h>
#include        <epicsExport.h>
#define GEN_SIZE_OFFSET
#include        "gpibRecord.h"
#undef GEN_SIZE_OFFSET


/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
static long init_record();
static long process();
#define special NULL
#define get_value NULL
static long cvt_dbaddr();
static long get_array_info();
static long put_array_info();
#define get_units NULL
#define get_precision NULL
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
#define get_graphic_double NULL
#define get_control_double NULL
#define get_alarm_double NULL

#define REMEMBER_STATE(FIELD) pPvt->old.FIELD = pgpib->FIELD
#define POST_IF_NEW(FIELD) \
    if (pgpib->FIELD != pPvt->old.FIELD) { \
        db_post_events(pgpib, &pgpib->FIELD, monitor_mask); \
        pPvt->old.FIELD = pgpib->FIELD; }

struct rset gpibRSET={
	RSETNUMBER,
	report,
	initialize,
	init_record,
	process,
	special,
	get_value,
	cvt_dbaddr,
	get_array_info,
	put_array_info,
	get_units,
	get_precision,
	get_enum_str,
	get_enum_strs,
	put_enum_str,
	get_graphic_double,
	get_control_double,
	get_alarm_double };
epicsExportAddress(rset, gpibRSET);

/* The following creates a dummy DSET.  This is required because we have defined
 * this record to be devGPIB so we could get the fields for GPIB_IO
 */
struct dset devGenericGpib = {4,NULL,NULL,NULL,NULL};
epicsExportAddress(dset, devGenericGpib);


struct old_field_values {              /* Used in monitor() */
    int   nowt;               
    int   nrrd;               
    int   nord;
    int   spr;               
    int   ucmd;
    int   acmd;
    int   addr;
};

struct gpibRecPvt {
    struct gpibRecord *prec;      /* Pointer to record */
    char *portName;
    asynUser *pasynUser;
    asynOctet *pasynOctet;
    void *asynOctetPvt;
    asynCommon *pasynCommon;
    void *asynCommonPvt;
    asynGpib *pasynGpib;
    void *asynGpibPvt;
    struct old_field_values old;
};

/* These really belong in a header file, they used to be in drvHwGpibInterface.h */
#define TADBASE    0x40   /* offset to GPIB listen address 0 */
#define LADBASE    0x20   /* offset to GPIB talk address 0 */
#define SADBASE    0x60   /* offset to GPIB secondary address 0 */

static void  gpibCallback(asynUser *pasynUser);
static int connectDevice(gpibRecord *pgpib);
static void monitor();


static long init_record(pgpib,pass)
    struct gpibRecord	*pgpib;
    int pass;
{
    struct gpibRecPvt *pPvt;
    asynUser *pasynUser;
    struct link *plink=&pgpib->inp;
    int link_addr;
    int status;

    if (pass != 0) return(0);

    /* Allocate and initialize private structure used by this record */
    pPvt = (struct gpibRecPvt*) malloc(sizeof(struct gpibRecPvt)); 
    pgpib->dpvt = pPvt;

    pPvt->prec = pgpib;

    /* Get the port name and the address from the input link specification */
    if (plink->type != GPIB_IO) {
        printf("gpibRecord::init_record, incorrect link type\n");
        return(-1);
    }
    link_addr = plink->value.gpibio.addr;    /* gpib dev address */
    pPvt->portName = plink->value.gpibio.parm;     /* parm field is port name */
    /* If the link address is non-zero and the record addr field is zero, use link address */
    if ((link_addr != 0) && (pgpib->addr == 0)) pgpib->addr = link_addr;
    REMEMBER_STATE(addr);
 
    /* Initialize asyn, connect to device */
    pasynUser = pasynManager->createAsynUser(gpibCallback,0);
    pPvt->pasynUser = pasynUser;
    pasynUser->userPvt = pPvt;
    status = connectDevice(pgpib);
    if (status) {
       printf("gpibRecord, error connecting to device, status=%d\n", status);
    }

    /* Allocate the space for the binary output and binary input arrays */
    if (pgpib->omax <= 0) pgpib->omax=1;
    if (pgpib->imax <= 0) pgpib->imax=1;
    pgpib->optr = (char *)calloc(pgpib->omax, sizeof(char));
    pgpib->iptr = (char *)calloc(pgpib->imax, sizeof(char));

    return(0);
}


static int connectDevice(gpibRecord *pgpib)
{
    asynInterface *pasynInterface;
    struct gpibRecPvt *pPvt=pgpib->dpvt;
    asynUser *pasynUser = pPvt->pasynUser;
    int status;

    status = pasynManager->connectDevice(pasynUser,pPvt->portName,pgpib->addr);
    if(status!=asynSuccess) {
        printf("%s\n",pasynUser->errorMessage);
        pasynManager->freeAsynUser(pasynUser);
        return(status);
    }

    /* Get asynCommon interface */
    pasynInterface = pasynManager->findInterface(pasynUser,asynCommonType,1);
    if(!pasynInterface) {
        printf("%s %s\n",asynCommonType,pasynUser->errorMessage);
        pasynManager->freeAsynUser(pasynUser);
        return(status);
    }
    pPvt->pasynCommon = (asynCommon *)pasynInterface->pinterface;
    pPvt->asynCommonPvt = pasynInterface->drvPvt;

    /* Get asynOctet interface */
    pasynInterface = pasynManager->findInterface(pasynUser,asynOctetType,1);
    if(!pasynInterface) {
        asynPrint(pasynUser,ASYN_TRACE_ERROR,"%s %s\n",
            asynOctetType,pasynUser->errorMessage);
        pasynManager->freeAsynUser(pasynUser);
        return(status);
    }
    pPvt->pasynOctet = (asynOctet *)pasynInterface->pinterface;
    pPvt->asynOctetPvt = pasynInterface->drvPvt;

    /* Get asynGpib interface */
    pasynInterface = pasynManager->findInterface(pasynUser,asynGpibType,1);
    if(pasynInterface) {
        /* This device has an asynGpib interface, not serial or socket */
        pPvt->pasynGpib = (asynGpib *)pasynInterface->pinterface;
        pPvt->asynGpibPvt = pasynInterface->drvPvt;
    }
    return(asynSuccess);
}


static long cvt_dbaddr(paddr)
struct dbAddr *paddr;
{
   struct gpibRecord *pgpib=(struct gpibRecord *)paddr->precord;
   int fieldIndex = dbGetFieldIndex(paddr);

   if (fieldIndex == gpibRecordBOUT) {
      paddr->pfield = (void *)(pgpib->optr);
      paddr->no_elements = pgpib->omax;
      paddr->field_type = DBF_CHAR;
      paddr->field_size = sizeof(char);
      paddr->dbr_field_type = DBF_CHAR;
   } else if (fieldIndex == gpibRecordBINP) {
      paddr->pfield = (unsigned char *)(pgpib->iptr);
      paddr->no_elements = pgpib->imax;
      paddr->field_type = DBF_CHAR;
      paddr->field_size = sizeof(char);
      paddr->dbr_field_type = DBF_CHAR;
   }
   return(0);
}

static long get_array_info(paddr,no_elements,offset)
struct dbAddr *paddr;
long *no_elements;
long *offset;
{
   struct gpibRecord *pgpib=(struct gpibRecord *)paddr->precord;

   *no_elements =  pgpib->nord;  /* Is this correct? */
   *offset = 0;
   return(0);
}

static long put_array_info(paddr,nNew)
struct dbAddr *paddr;
long nNew;
{
struct gpibRecord *pgpib=(struct gpibRecord *)paddr->precord;

   pgpib->nowt = nNew;
   if (pgpib->nowt > pgpib->omax) pgpib->nowt = pgpib->omax;
   return(0);
}


static long process(pgpib)
	struct gpibRecord	*pgpib;
{
        struct gpibRecPvt* pPvt = pgpib->dpvt;

        /* If pact is FALSE then queue message to driver and return */
        if (!pgpib->pact)
        {
            /* Remember current state of fields for monitor() */
            REMEMBER_STATE(nrrd);
            REMEMBER_STATE(nord);
            REMEMBER_STATE(nowt);
            REMEMBER_STATE(spr);
            REMEMBER_STATE(ucmd);
            REMEMBER_STATE(acmd);
            /* Make sure nrrd and nowt are valid */
            if (pgpib->nrrd > pgpib->imax) pgpib->nrrd = pgpib->imax;
            if (pgpib->nowt > pgpib->omax) pgpib->nowt = pgpib->omax;
            /* If GPIB address has changed disconnect and reconnect */
            if (pgpib->addr != pPvt->old.addr) {
               pasynManager->disconnect(pPvt->pasynUser);
               connectDevice(pgpib);
            } 
            pasynManager->queueRequest(pPvt->pasynUser, asynQueuePriorityLow, 0.0);
            pgpib->pact = TRUE;
            return(0);
        }

        /* pact was TRUE, so we were called from gpibCallback.
         * The I/O is complete, so finish up.
         */

	recGblGetTimeStamp(pgpib);

	/* check event list */
	monitor(pgpib);
	/* process the forward scan link record */
	recGblFwdLink(pgpib);

	pgpib->pact=FALSE;
	return(0);
}



static void monitor(pgpib)
    struct gpibRecord   *pgpib;
{
    unsigned short  monitor_mask;
    struct gpibRecPvt* pPvt = pgpib->dpvt;

    monitor_mask = recGblResetAlarms(pgpib) | DBE_VALUE | DBE_LOG;
    
    if(strncmp(pgpib->oinp,pgpib->ainp,sizeof(pgpib->ainp))) 
    {
       db_post_events(pgpib,pgpib->ainp, monitor_mask);
       strncpy(pgpib->oinp,pgpib->ainp,sizeof(pgpib->ainp));
    }
    if (pgpib->ifmt == gpibOFMT_Binary)
        db_post_events(pgpib, pgpib->iptr, monitor_mask);

    POST_IF_NEW(nrrd);
    POST_IF_NEW(nord);
    POST_IF_NEW(nowt);
    POST_IF_NEW(spr);
    POST_IF_NEW(ucmd);
    POST_IF_NEW(acmd);
    POST_IF_NEW(addr);
}


/* gpibCallback ******************
 * This routine is called by the driver when the message gets to the head of
 * the queue.  It calls the driver to do the actual GPIB I/O and then calls
 * the record process() function again to signal that the I/O is complete
 */
static void gpibCallback(struct asynUser *pasynUser)
{
   struct gpibRecPvt *pPvt=pasynUser->userPvt;
   struct  gpibRecord *pgpib = pPvt->prec;
   int     nout;
   int     ninp;
   int     status;
   double  timeout;
   char    *inptr;
   char    *outptr;
   char    cmd_char=0;
   char    acmd[6];
   int     inlen;
   int     outlen;
   char    eos;

   dbScanLock( (struct dbCommon*) pgpib);       /* Lock the record */
   timeout = pgpib->tmot / 1000.;
   if (timeout < 1) timeout = 1;
   pPvt->pasynUser->timeout = timeout;

   /* See if a Universal Command is to be done (only if pasynGpib interface exists)*/
   /* See gpibRecord.dbd for definitions of constants gpibXXXX_Abcd */
   if ((pPvt->pasynGpib) && (pgpib->ucmd != gpibUCMD_None))
   {
      switch (pgpib->ucmd)
      {
        case gpibUCMD_Device_Clear__DCL_:
            cmd_char = 20;
            break;
        case gpibUCMD_Local_Lockout__LL0_:
            cmd_char = 17;
            break;
        case gpibUCMD_Serial_Poll_Disable__SPD_:
            cmd_char = 25;
            break;
        case gpibUCMD_Serial_Poll_Enable__SPE_:
            cmd_char = 24;
            break;
        case gpibUCMD_Unlisten__UNL_: 
            cmd_char = 63;
            break;
        case gpibUCMD_Untalk__UNT_:
            cmd_char = 95;
            break;
      }
      status = pPvt->pasynGpib->universalCmd(pPvt->asynGpibPvt, pPvt->pasynUser, cmd_char);
      if (status)
         /* Something is wrong if we couldn't write */
         recGblSetSevr(pgpib, WRITE_ALARM, MAJOR_ALARM);
      pgpib->ucmd = gpibUCMD_None;  /* Reset to no Universal Command */
      goto finish;
   }

   /* See if an Addressed Command is to be done */
   if ((pPvt->pasynGpib) && (pgpib->acmd != gpibACMD_None))
   {
      acmd[0] = 95; /* Untalk */
      acmd[1] = 63; /* Unlisten */
      acmd[2] = pgpib->addr + LADBASE;  /* GPIB address + Listen Base */
      acmd[4] = 95; /* Untalk */
      acmd[5] = 63; /* Unlisten */
      switch (pgpib->acmd)
      {
        case gpibACMD_Group_Execute_Trig___GET_:
            acmd[3] = 8;
            break;
        case gpibACMD_Go_To_Local__GTL_:
            acmd[3] = 1;
            break;
        case gpibACMD_Selected_Dev__Clear__SDC_:
            acmd[3] = 4;
            break;
        case gpibACMD_Take_Control__TCT_:
            /* This command requires Talker Base */
            acmd[2] = pgpib->addr + TADBASE;
            acmd[3] = 9;
            break;
        case gpibACMD_Serial_Poll:
            /* Serial poll. Requires 3 operations */
            /* Serial Poll Enable */
            cmd_char = IBSPE;
            status = pPvt->pasynOctet->write(pPvt->asynGpibPvt, 
                     pPvt->pasynUser, &cmd_char, 1, &nout);
            if (status)
                /* Something is wrong if we couldn't write */
                recGblSetSevr(pgpib, WRITE_ALARM, MAJOR_ALARM);
            /* Read the response byte  */
            status = pPvt->pasynOctet->read(pPvt->asynGpibPvt, 
                        pPvt->pasynUser, &pgpib->spr, 1, &ninp);
            if (ninp < 0) /* Is this right? */
                /* Something is wrong if we couldn't read */
                recGblSetSevr(pgpib, READ_ALARM, MAJOR_ALARM);
            /* Serial Poll Disable */
            cmd_char = IBSPD;
            status = pPvt->pasynOctet->write(pPvt->asynGpibPvt, 
                           pPvt->pasynUser, &cmd_char, 1, &nout);
            if (status)
                /* Something is wrong if we couldn't write */
                recGblSetSevr(pgpib, WRITE_ALARM, MAJOR_ALARM);
            pgpib->acmd = gpibACMD_None;  /* Reset to no Addressed Command */
            goto finish;
            break;
      }
      status = pPvt->pasynGpib->addressedCmd(pPvt->asynGpibPvt, pPvt->pasynUser, acmd, 6);
      if (status)
         /* Something is wrong if we couldn't write */
         recGblSetSevr(pgpib, WRITE_ALARM, MAJOR_ALARM);
      pgpib->acmd = gpibACMD_None;  /* Reset to no Addressed Command */
      goto finish;
   }
      

   if (pgpib->ofmt == gpibOFMT_ASCII)
   {    /* ASCII output mode */
        outptr = pgpib->aout;
        outlen = strlen(pgpib->aout);
   } else 
   {     /* Binary output mode */
        outptr = pgpib->optr;
        outlen = pgpib->nowt;
   }

   if (pgpib->ifmt == gpibOFMT_ASCII)
   {    /* ASCII input mode */
        inptr = pgpib->ainp;
        if (pgpib->nrrd != 0)
            inlen = pgpib->nrrd;
        else
            inlen = sizeof(pgpib->ainp);
   } else 
   {     /* Binary input mode */
        inptr = pgpib->iptr;
        if (pgpib->nrrd != 0)
            inlen = pgpib->nrrd;
        else
            inlen = pgpib->imax;
   }

   if (pgpib->tmod != gpibTMOD_Read)
   {
      /* write the message to the GPIB listen adrs */
      status = pPvt->pasynOctet->write(pPvt->asynOctetPvt, pPvt->pasynUser, 
                                     outptr, outlen, &nout);
      if (nout != outlen) 
         /* Something is wrong if we couldn't write everything */
         recGblSetSevr(pgpib, WRITE_ALARM, MAJOR_ALARM);
   }
    
   memset(inptr, 0, inlen);     /* Set the response string to zeros */
   if (pgpib->tmod != gpibTMOD_Write)
   {
      /* read the instrument  */
      eos = pgpib->eos & 0xff;
      status = pPvt->pasynOctet->setEos(pPvt->asynOctetPvt, pPvt->pasynUser, &eos, 1);
      if (status) 
         /* Something is wrong if we didn't get any response */
         recGblSetSevr(pgpib, READ_ALARM, MAJOR_ALARM);
      status = pPvt->pasynOctet->read(pPvt->asynOctetPvt, pPvt->pasynUser, 
                                    inptr, inlen, &ninp);
      if (status) 
         /* Something is wrong if we didn't get any response */
         recGblSetSevr(pgpib, READ_ALARM, MAJOR_ALARM);
      if (((pgpib->ifmt == gpibOFMT_ASCII)  && (ninp > sizeof(pgpib->ainp)-1)) ||
          ((pgpib->ifmt == gpibOFMT_Binary) && (ninp > pgpib->imax)))
      {
         /* Input buffer overflow */
         recGblSetSevr(pgpib, READ_ALARM, MINOR_ALARM);
         if (pgpib->ifmt == gpibOFMT_ASCII)  /* terminate response with \0 */
             inptr[sizeof(pgpib->ainp)-1] = '\0';
      }
      else
      {
         /* If the string is terminated by the requested terminator */
         /* remove it. */
         if ((ninp != 0) && (pgpib->eos != -1) && (inptr[ninp-1] == pgpib->eos))
            inptr[ninp-1] = '\0';
         else
            /* Add a null terminator - there is room */
	    inptr[ninp] = '\0';
      } 
      pgpib->nord = ninp; /* Number of bytes read */
   }

   /* Call process, and unlock the record */
finish:
   process(pgpib);
   dbScanUnlock( (struct dbCommon*) pgpib);
   return;
}
