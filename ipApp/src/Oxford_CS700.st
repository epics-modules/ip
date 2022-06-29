program cryostream("name=CS700,P=jiao:,C=CS:")

option +r;
option -c;

#include "seqPVmacros.h"

#define debug_flag CS_Debug
#define PACKETS_LENGTH 32


PVA(char,   CSstatus,    PACKETS_LENGTH,     "{P}{C}Status.BINP",           EvFlag);
PVA(char,    CScmd3,       6,                  "{P}{C}Cmd3.BOUT",        NoMon);
PVA(char,    CScmd2,       4,                  "{P}{C}Cmd2.BOUT",        NoMon);
PVA(char,    CScmd1,       2,                  "{P}{C}Cmd1.BOUT",        NoMon);
PV(short,   CSstart,     "{P}{C}start",    Monitor);
PV(short,   CSend,     "{P}{C}End",    Monitor);
PV(short,   CSramp,     "{P}{C}setTemp",    Monitor);
PV(short,   RampRate_HI,     "{P}{C}Rate.C",    Monitor);
PV(short,   RampRate_LO,     "{P}{C}Rate.D",    Monitor);
PV(short,   EndRate_HI,     "{P}{C}EndRateChar.C",    Monitor);
PV(short,   EndRate_LO,     "{P}{C}EndRateChar.D",    Monitor);
PV(short,   Setpoint_HI,    "{P}{C}Setpoint.D", Monitor);
PV(short,   Setpoint_LO,    "{P}{C}Setpoint.E", Monitor);
PV(short,   CS_Debug,        "{P}{C}Debug",  Monitor);


PV(double,   RD_GasSetPoint, "{P}{C}GasSetPoint", NoMon);
PV(double,   RD_GasTemp,     "{P}{C}GasTemp", NoMon);
PV(double,   RD_GasError,    "{P}{C}GasError",NoMon);
PV(short,   RD_PhaseID,     "{P}{C}PhaseID", Monitor);
PV(short,   RD_RunMode,      "{P}{C}RunMode", NoMon);
PV(short,   RD_AlarmCode,    "{P}{C}AlarmCode1.A", NoMon);
PV(short,    RD_RampRate,      "{P}{C}RampRate", NoMon);
PV(double,   RD_TargetTemp,    "{P}{C}TargetTemp", NoMon);
PV(double,   RD_EvapTemp,      "{P}{C}EvapTemp", NoMon);
PV(double,   RD_SuctTemp,       "{P}{C}SuctTemp", NoMon);
PV(double,   RD_GasFlow,        "{P}{C}GasFlow", NoMon);
PV(short,    RD_GasHeat,         "{P}{C}GasHeat", NoMon);
PV(short,    RD_EvapHeat,        "{P}{C}EvapHeat",NoMon);
PV(short,    RD_SuctHeat,        "{P}{C}SuctHeat",NoMon);
PV(short,    RD_EvapAdj,          "{P}{C}EvapAdj",NoMon);
PV(double,    RD_RunTime,        "{P}{C}RunTime",NoMon);
PV(double,    RD_LinePressure, "{P}{C}LinePressure",NoMon);

#define CS_END_CMD 15
#define CS_RAMP_CMD 11
#define CS_START_CMD 10

#define CS_CMD1_LEN 2
#define CS_CMD2_LEN 4
#define CS_CMD3_LEN 6
#define END_PHASE 4
#define RUN 3

%% #include <stdlib.h>

char *SNLtaskName;


ss CryoStream {
         
        state init {
               when (pvConnectCount() != pvChannelCount() ) {
                  DEBUG_PRINT(1,"not all pvs are connected");
               } state wait_for_PVs_to_connect    
                 
               when() {   
                 SNLtaskName = macValueGet("name");
                 efClear(CSstatus_mon);
              
               } state idle   
               
        }
        state wait_for_PVs_to_connect {
                when (pvConnectCount() == pvChannelCount()) {  
                        DEBUG_PRINT(1, "all channels connected");
                } state idle
                
        }
        state idle {
               when (pvConnectCount() != pvChannelCount() ) {
                  DEBUG_PRINT(1,"not all pvs are connected");
               } state wait_for_PVs_to_connect   
               
               when(CSstart) {               
                   CScmd1[0] = CS_CMD1_LEN;
                   CScmd1[1]= CS_START_CMD;
        	   pvPut(CScmd1);
                   PVPUT(CSstart,0);
                   DEBUG_PRINT(1,"send restart\n"); 
                 
               } state idle
               when(CSend) {
                   CScmd2[0] = CS_CMD2_LEN;
                   CScmd2[1] = CS_END_CMD;
                   CScmd2[2] = EndRate_HI;
                   CScmd2[3] = EndRate_LO;
                   pvPut(CScmd2);    
                   PVPUT(CSend,0);    
                   DEBUG_PRINT(1,"send end\n");  
               } state endSend  
               when(CSramp) {
                   CScmd3[0] = CS_CMD3_LEN;
                   CScmd3[1] = CS_RAMP_CMD;
                   CScmd3[2] = RampRate_HI;
                   CScmd3[3] = RampRate_LO;
                   CScmd3[4] = Setpoint_HI;
                   CScmd3[5] = Setpoint_LO;
                   pvPut(CScmd3);    
                   PVPUT(CSramp,0);    
                   DEBUG_PRINT(1,"setpoint changed\n");
               } state idle
               when(efTestAndClear(CSstatus_mon)) {
                   if(CSstatus[0]==32 && CSstatus[1]==1) {
		     PVPUT(RD_GasSetPoint,0.01*( 65535&(((CSstatus[2]<<8)| (255&CSstatus[3])))) );
                     PVPUT(RD_GasTemp,0.01*( 65535&( ((CSstatus[4]<<8) | (255&CSstatus[5])))) );
                     PVPUT(RD_GasError,0.01*( -256*(CSstatus[6]&128) + (32767&( (CSstatus[6]<<8)|(255&CSstatus[7]))) ) );
                     PVPUT(RD_RunMode, 255&CSstatus[8]);
                     PVPUT(RD_PhaseID, 255&CSstatus[9]);
                     PVPUT(RD_RampRate, 65535&( (CSstatus[10]<<8)|(255&CSstatus[11]))  );
                     PVPUT(RD_TargetTemp,0.01*(65535&( ((CSstatus[12]<<8)|(255&CSstatus[13])))));
                     PVPUT(RD_EvapTemp,0.01*(65535&(((CSstatus[14]<<8)| (255&CSstatus[15])))));
                     PVPUT(RD_SuctTemp,0.01*(65535&(((CSstatus[16]<<8)|(255&CSstatus[17])))));
                     PVPUT(RD_GasFlow, 0.1*(255&CSstatus[20]));
                     PVPUT(RD_GasHeat, 255&CSstatus[21]);
                     PVPUT(RD_EvapHeat, 255&CSstatus[22]);
                     PVPUT(RD_SuctHeat, 255&CSstatus[23]);
                     PVPUT(RD_LinePressure, 0.01*(255&CSstatus[24]));
                     PVPUT(RD_AlarmCode, 255&CSstatus[25]);
                     PVPUT(RD_RunTime,(65535&(((CSstatus[26]<<8)| (255&CSstatus[27]))) )/60.);
                     PVPUT(RD_EvapAdj, 255&CSstatus[31]);
                     DEBUG_PRINT(1, "status changed\n");
		   }
               } state idle
               
        }

        state endSend {
	       when( (255&CSstatus[8]) != RUN) {
	           DEBUG_PRINT(1, "endSend:  Not in Run phase,ignored end\n");
	       } state idle 
						 

               when(efTestAndClear(CSstatus_mon)) {
                   if(CSstatus[0]==32 && CSstatus[1]==1) {
		     PVPUT(RD_GasSetPoint,0.01*( 65535&(((CSstatus[2]<<8)| (255&CSstatus[3])))) );
                     PVPUT(RD_GasTemp,0.01*( 65535&( ((CSstatus[4]<<8) | (255&CSstatus[5])))) );
                     PVPUT(RD_GasError,0.01*( -256*(CSstatus[6]&128) + (32767&( (CSstatus[6]<<8)|(255&CSstatus[7]))) ) );
                     PVPUT(RD_RunMode, 255&CSstatus[8]);
                     PVPUT(RD_PhaseID, 255&CSstatus[9]);
                     PVPUT(RD_RampRate, 65535&( (CSstatus[10]<<8)|(255&CSstatus[11]))  );
                     PVPUT(RD_TargetTemp,0.01*(65535&( ((CSstatus[12]<<8)|(255&CSstatus[13])))));
                     PVPUT(RD_EvapTemp,0.01*(65535&(((CSstatus[14]<<8)| (255&CSstatus[15])))));
                     PVPUT(RD_SuctTemp,0.01*(65535&(((CSstatus[16]<<8)|(255&CSstatus[17])))));
                     PVPUT(RD_GasFlow, 0.1*(255&CSstatus[20]));
                     PVPUT(RD_GasHeat, 255&CSstatus[21]);
                     PVPUT(RD_EvapHeat, 255&CSstatus[22]);
                     PVPUT(RD_SuctHeat, 255&CSstatus[23]);
                     PVPUT(RD_LinePressure, 0.01*(255&CSstatus[24]));
                     PVPUT(RD_AlarmCode, 255&CSstatus[25]);
                     PVPUT(RD_RunTime,(65535&(((CSstatus[26]<<8)| (255&CSstatus[27]))) )/60.);
                     PVPUT(RD_EvapAdj, 255&CSstatus[31]);
                     DEBUG_PRINT(1, "endSend: status changed\n");
                     if( (255&CSstatus[8]) != END_PHASE)  {
                       CScmd2[0] = CS_CMD2_LEN;
                       CScmd2[1] = CS_END_CMD;
                       CScmd2[2] = EndRate_HI;
                       CScmd2[3] = EndRate_LO;
                       pvPut(CScmd2);    
                       PVPUT(CSend,0);    
                       DEBUG_PRINT(1,"resend end\n");     

                     }
		   }
               } state endSend
               
               when( RD_PhaseID ==  END_PHASE) {
                   DEBUG_PRINT(1, "endSend: end command is taken\n");
               } state idle
	       
             
        }


}


