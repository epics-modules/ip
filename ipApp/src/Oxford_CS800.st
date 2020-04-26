program cs800 ("name=cs0, CID=0, P=xxx:, R=CS0:, SP=xxx:CS:status_packet")

/* option -c;  /* Begin execution before PV connections complete */
option +d;  /* turn ON run-time debugging messages */
option +r;  /* re-usable code, can load more than once */
option -w;  /* Suppress SNC warnings */

/*
 * usage:
   
   VxWorks-only: (re)load the compiled sequence program:
    ld < /usr/local/epics/synApps_6_1__base_7_0_3_1/support/xxx-R6-1/xxxApp/src/O.linux-arm/cs800.o

 seq &cs800, "name=cs113, CID=113, P=xxx:, R=CS0:, SP=xxx:CS:status_packet"
 seq &cs800, "name=cs144, CID=144, P=xxx:, R=CS1:, SP=xxx:CS:status_packet"
 
 * to kill the running sequence program:
 td "cs113"
 
 Parameters
 
 name | this instance of running SNL program
 ---- | ------------------------------------
 CID  | controller ID
 P    | IOC prefix
 R    | controller's database prefix
 SP   | PV of status packet (asyn record)

 */

/*------------------------------------------------ 
  ------------------------------------------------   general purpose
  ------------------------------------------------ */

%%#include <math.h>
%%#include <stdio.h>
%%#include <stdlib.h>
%%#include <string.h>
%%#include <epicsThread.h>
#include "seqPVmacros.h"


/*------------------------------------------------ 
  ------------------------------------------------   macros
  ------------------------------------------------ */

#define STATUS_PACKET_VALID_LENGTH    928
#define NUMBER_OF_PARAMETERS          230
#define IDLE_POLL_INTERVAL            (30)
#define DEBUGLEVEL_OFF                (0)
#define DEBUGLEVEL_ERROR              (1)
#define DEBUGLEVEL_WARNING            (2)
#define DEBUGLEVEL_INFO               (3)
#define DEBUGLEVEL_DEBUG              (4)
#define NOT_FOUND                     (-1)

#define CHANGE_STATE(_STATE_)    \
      DEBUG_PRINT2(DEBUGLEVEL_DEBUG, "state change: %s -> %s", current_state, _STATE_); \
      sprintf(current_state, _STATE_);

#define CHECK_PVPUT(_PV_, _PARAM_ID_)     \
      value = getParam(_PARAM_ID_, param_id, param_value);       \
      if (value != NOT_FOUND) PVPUT(_PV_, value);

#define PARAM_CONTROLLER_NUMBER       (1028)
#define PARAM_CONTROLLER_SETPOINT     (1050)
#define PARAM_CONTROLLER_TEMPERATURE  (1051)
#define PARAM_CONTROLLER_RUNMODE      (1053)
#define PARAM_CONTROLLER_PHASE        (1054)


/****
 **** local variables 
 ****/
char current_state[256];
char scratch[256];
int i;
int cid;
int s_cid;
int value;
int s_checksum;
int param_id[NUMBER_OF_PARAMETERS];
int param_value[NUMBER_OF_PARAMETERS];
char*   SNLtaskName;

%% int getParam(int id, int *pid, int *pval);

/*------------------------------------------------ 
  ------------------------------------------------   assignments
  ------------------------------------------------ */

/**** 
 **** asyn record
 ****/
PV(int, s_nrrd, "{SP}.NRRD", Monitor);       /* number bytes received */
PVA(short, s_binp, STATUS_PACKET_VALID_LENGTH, "{SP}.BINP", Monitor);    /* Binary input */


/**** 
 **** CS800 controller database 
 ****/
PV(int, disable,      "{P}{R}disable",          Monitor);
PV(int, setpoint,     "{P}{R}setpoint.RVAL",    NoMon);
PV(int, temperature,  "{P}{R}temperature.RVAL", NoMon);
PV(int, run_mode,     "{P}{R}runmode.RVAL",     NoMon);
PV(int, phase,        "{P}{R}phase.RVAL",       NoMon);

PV(int, debug_flag, "{P}{R}debug", Monitor);    /* SNL diagnostics level */


/* 
 * state set:  cs800
 */
ss cs800 {

  state startup {
    when () {
      sprintf(current_state, "startup");
      pvGet(s_binp);
      pvGet(s_nrrd);
      
      cid = atoi(macValueGet("CID"));

      DEBUG_PRINT1(DEBUGLEVEL_INFO, "controller ID: %d", cid);
      DEBUG_PRINT1(DEBUGLEVEL_INFO, "IOC prefix: %s", macValueGet("P"));
      DEBUG_PRINT1(DEBUGLEVEL_INFO, "controller database: %s", macValueGet("R"));
      DEBUG_PRINT1(DEBUGLEVEL_INFO, "status packet PV: %s", macValueGet("SP"));
    } state idle
  }


  state idle {
    when (!strcmp(current_state, "idle")) {
      CHANGE_STATE("idle")
    } state idle

    when (disable) {} state ignore

    when (s_nrrd == STATUS_PACKET_VALID_LENGTH) {
      /* interpret binary input buffer

         ignore ...
           first two bytes: 0xaaab
           second two bytes: length of data packet
           second last two bytes: 16-bit checksum
           last two bytes: 0xabaa
       */
      s_checksum = s_binp[924]*256 + s_binp[925];

      for (i=4; i < STATUS_PACKET_VALID_LENGTH-4; i+=4) {
        param_id[i/4] = s_binp[i]*256 + s_binp[i+1];
        param_value[i/4] = s_binp[i+2]*256 + s_binp[i+3];
      }

      /*
       * If CID was given as 0 (or negative), then look at the first packet
       * and reassign cid to the controller ID found in that packet.
       * This is likely the default case when only one CS800 controller
       * is on the network.  No need to identify the controller by 
       * number since it is the only one.
       */
      s_cid = getParam(PARAM_CONTROLLER_NUMBER, param_id, param_value);
      if (cid < 1) {
        cid = s_cid;
        DEBUG_PRINT1(DEBUGLEVEL_INFO, "choosing controller ID: %d", cid);
      }
    } state validate

    when (delay(IDLE_POLL_INTERVAL)) {} state idle
  }


  /* user can disable this software */
  state ignore {
    when (!strcmp(current_state, "ignore")) {
      CHANGE_STATE("ignore")
    } state ignore

    when (!disable) {} state idle

    when (delay(IDLE_POLL_INTERVAL)) {} state ignore
  }


  state validate { 
    when (!strcmp(current_state, "validate")) {
      CHANGE_STATE("validate")
    } state validate

    when (disable) {} state ignore

    when (cid < 1) {} state idle

    when (cid != s_cid) {
        DEBUG_PRINT2(DEBUGLEVEL_DEBUG, "controller ID mismatch: expected=%d received=%d", cid, s_cid);
    } state idle

    when () {} state process
  }


  state process { 
    when (!strcmp(current_state, "process")) {
      CHANGE_STATE("process")
    } state process

    when (disable) {} state ignore

    when () {
      CHECK_PVPUT(setpoint,     PARAM_CONTROLLER_SETPOINT);
      CHECK_PVPUT(temperature,  PARAM_CONTROLLER_TEMPERATURE);
      CHECK_PVPUT(run_mode,     PARAM_CONTROLLER_RUNMODE);
      CHECK_PVPUT(phase,        PARAM_CONTROLLER_PHASE);
    } state idle
  }

}


%{

int getParam(int id, int *param_id, int *param_value) {
  for (int i=0; i<NUMBER_OF_PARAMETERS; i++)
    if (param_id[i] == id)
      return param_value[i];

  return NOT_FOUND;
}

}%
