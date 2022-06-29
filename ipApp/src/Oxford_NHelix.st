program nhelix("name=nhelix,P=jiao:,C=NH:")

option +r;
option -c;

#include "seqPVmacros.h"

#define debug_flag CS_Debug
#define PACKETS_LENGTH 46


PVA(char,   CSstatus,    PACKETS_LENGTH,     "{P}{C}Status.BINP",      EvFlag);
PVA(char,   CScmd4,      3,                  "{P}{C}Cmd4.BOUT",        NoMon);
PVA(char,   CScmd3,      6,                  "{P}{C}Cmd3.BOUT",        NoMon);
PVA(char,   CScmd2,      4,                  "{P}{C}Cmd2.BOUT",        NoMon);
PVA(char,   CScmd1,      2,                  "{P}{C}Cmd1.BOUT",        NoMon);

PV(short,   CSstart,     "{P}{C}start",         Monitor);
PV(short,   CSend,       "{P}{C}End",           Monitor);
PV(short,   CSramp,      "{P}{C}setTemp",       Monitor);
PV(short,   CSforce,     "{P}{C}ForceHe",       Monitor);
PV(short,   CSpurge,     "{P}{C}Purge",         Monitor);
PV(short,   RampRate_HI, "{P}{C}Rate.C",        Monitor);
PV(short,   RampRate_LO, "{P}{C}Rate.D",        Monitor);
PV(short,   EndRate_HI,  "{P}{C}EndRateChar.C", Monitor);
PV(short,   EndRate_LO,  "{P}{C}EndRateChar.D", Monitor);
PV(short,   Setpoint_HI, "{P}{C}Setpoint.D",    Monitor);
PV(short,   Setpoint_LO, "{P}{C}Setpoint.E",    Monitor);
PV(short,   CS_Debug,    "{P}{C}Debug",         Monitor);

evflag CSforce_mon;
sync CSforce CSforce_mon;

PV(double,   RD_GasSetPoint,  "{P}{C}GasSetPoint",  NoMon);
PV(double,   RD_GasTemp,      "{P}{C}GasTemp",      NoMon);
PV(double,   RD_GasError,     "{P}{C}GasError",     NoMon);
PV(short,    RD_PhaseID,      "{P}{C}PhaseID",      Monitor);
PV(short,    RD_RunMode,      "{P}{C}RunMode",      NoMon);
PV(short,    RD_AlarmCode,    "{P}{C}AlarmCode1.A", NoMon);
PV(short,    RD_RampRate,     "{P}{C}RampRate",     NoMon);
PV(double,   RD_TargetTemp,   "{P}{C}TargetTemp",   NoMon);
PV(double,   RD_ShieldTemp,   "{P}{C}ShieldTemp",   NoMon);
PV(double,   RD_NozzleTemp,   "{P}{C}NozzleTemp",   NoMon);
PV(short,    RD_Remaining,    "{P}{C}Remaining",    NoMon);
PV(short,    RD_CryoSpeed,    "{P}{C}CryoSpeed",    NoMon);
PV(short,    RD_GasHeat,      "{P}{C}GasHeat",      NoMon);
PV(short,    RD_ShieldHeat,   "{P}{C}ShieldHeat",   NoMon);
PV(short,    RD_AverageNozzleHeat, "{P}{C}AverageNozzleHeat", NoMon);
PV(short,    RD_CryoStatus,   "{P}{C}CryoStatus",   NoMon);
PV(double,   RD_RunTime,      "{P}{C}RunTime",      NoMon);
PV(double,   RD_GasFlow,      "{P}{C}GasFlow",      NoMon);
PV(double,   RD_LinePressure, "{P}{C}LinePressure", NoMon);
PV(short,    RD_CryoAdjust,   "{P}{C}CryoAdjust",   NoMon);
PV(short,    RD_OuterFlow,    "{P}{C}OuterFlow",    NoMon);
PV(short,    RD_DetectedGasType, "{P}{C}DetectedGasType", NoMon);
PV(short,    RD_SelectedGasType, "{P}{C}SelectedGasType", NoMon);
PV(double,   RD_WaterTemp,    "{P}{C}WaterTemp",     NoMon);
PV(short,    RD_AverageGasHeat,  "{P}{C}AverageGasHeat",  NoMon);
PV(short,    RD_NozzleHeat,   "{P}{C}NozzleHeat",   NoMon);


#define CS_END_CMD 15
#define CS_RAMP_CMD 11
#define CS_START_CMD 10
#define CS_FORCEHE_CMD 20
#define CS_PURGE_CMD 16

#define CS_CMD1_LEN 2
#define CS_CMD2_LEN 4
#define CS_CMD3_LEN 6
#define CS_CMD4_LEN 3
#define END_PHASE 4
#define RUN 3

%% #include <stdlib.h>

char *SNLtaskName;


ss NHelix {
         
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
               when(efTestAndClear(CSforce_mon)) {
                   CScmd4[0] = CS_CMD4_LEN;
                   CScmd4[1] = CS_FORCEHE_CMD;
                   CScmd4[2] = CSforce;
                   pvPut(CScmd4);
                   DEBUG_PRINT(1, "ForceHe changed\n");
               } state idle
               when(CSpurge) {
                   CScmd1[0] = CS_CMD1_LEN;
                   CScmd1[1] = CS_PURGE_CMD;
                   pvPut(CScmd1);
                   PVPUT(CSpurge,0);
                   DEBUG_PRINT(1, "send purge\n");
               } state idle
               when(efTestAndClear(CSstatus_mon)) {
                   if(CSstatus[0]==46 && CSstatus[1]==200) {
                     PVPUT(RD_GasSetPoint,0.01*( 65535&(((CSstatus[2]<<8)| (255&CSstatus[3])))) );
                     PVPUT(RD_GasTemp,0.01*( 65535&( ((CSstatus[4]<<8) | (255&CSstatus[5])))) );
                     PVPUT(RD_GasError,0.01*( -256*(CSstatus[6]&128) + (32767&( (CSstatus[6]<<8)|(255&CSstatus[7]))) ) );
                     PVPUT(RD_RunMode, 255&CSstatus[8]);
                     PVPUT(RD_PhaseID, 255&CSstatus[9]);
                     PVPUT(RD_RampRate, 65535&( (CSstatus[10]<<8)|(255&CSstatus[11]))  );
                     PVPUT(RD_TargetTemp,0.01*(65535&( ((CSstatus[12]<<8)|(255&CSstatus[13])))));
                     PVPUT(RD_ShieldTemp,0.01*(65535&( ((CSstatus[14]<<8)|(255&CSstatus[15])))));
                     PVPUT(RD_NozzleTemp, 0.01*(65535&( ((CSstatus[16]<<8)|(255&CSstatus[17])))));
                     PVPUT(RD_Remaining, (65535&(((CSstatus[18]<<8)| (255&CSstatus[19]))) )/60.);
                     PVPUT(RD_CryoSpeed, 255&CSstatus[20]);
                     PVPUT(RD_GasHeat, 255&CSstatus[21]);   
                     PVPUT(RD_ShieldHeat, 255&CSstatus[22]);
                     PVPUT(RD_AverageNozzleHeat, 255&CSstatus[23]);   
                     PVPUT(RD_CryoStatus, 255&CSstatus[24]);
                     PVPUT(RD_AlarmCode, 255&CSstatus[25]);
                     PVPUT(RD_RunTime,(65535&(((CSstatus[26]<<8)| (255&CSstatus[27]))) )/60.);
                     PVPUT(RD_GasFlow, 0.1*(255&CSstatus[31]));
                     PVPUT(RD_LinePressure, 0.01*(255&CSstatus[32]));
                     PVPUT(RD_CryoAdjust, 255&CSstatus[33]);
                     PVPUT(RD_OuterFlow, 0.1*(255&CSstatus[34]));
                     PVPUT(RD_DetectedGasType, 255&CSstatus[35]);
                     PVPUT(RD_SelectedGasType, 255&CSstatus[36]);
                     PVPUT(RD_WaterTemp,0.01*(255&CSstatus[38]));
                     PVPUT(RD_AverageGasHeat, 255&CSstatus[40]);
                     PVPUT(RD_NozzleHeat, 255&CSstatus[41]);
                     DEBUG_PRINT(1, "status changed\n");
                }
               } state idle
               
        }

        state endSend {
	       when( (255&CSstatus[8]) != RUN) {
	           DEBUG_PRINT(1, "endSend:  Not in Run phase,ignored end\n");
	       } state idle 
						 

               when(efTestAndClear(CSstatus_mon)) {
                   if(CSstatus[0]==46 && CSstatus[1]==200) {
                     PVPUT(RD_GasSetPoint,0.01*( 65535&(((CSstatus[2]<<8)| (255&CSstatus[3])))) );
                     PVPUT(RD_GasTemp,0.01*( 65535&( ((CSstatus[4]<<8) | (255&CSstatus[5])))) );
                     PVPUT(RD_GasError,0.01*( -256*(CSstatus[6]&128) + (32767&( (CSstatus[6]<<8)|(255&CSstatus[7]))) ) );
                     PVPUT(RD_RunMode, 255&CSstatus[8]);
                     PVPUT(RD_PhaseID, 255&CSstatus[9]);
                     PVPUT(RD_RampRate, 65535&( (CSstatus[10]<<8)|(255&CSstatus[11]))  );
                     PVPUT(RD_TargetTemp,0.01*(65535&( ((CSstatus[12]<<8)|(255&CSstatus[13])))));
                     PVPUT(RD_ShieldTemp,0.01*(65535&( ((CSstatus[14]<<8)|(255&CSstatus[15])))));
                     PVPUT(RD_NozzleTemp, 0.01*(65535&( ((CSstatus[16]<<8)|(255&CSstatus[17])))));
                     PVPUT(RD_Remaining, (65535&(((CSstatus[18]<<8)| (255&CSstatus[19]))) )/60.);
                     PVPUT(RD_CryoSpeed, 255&CSstatus[20]);
                     PVPUT(RD_GasHeat, 255&CSstatus[21]);   
                     PVPUT(RD_ShieldHeat, 255&CSstatus[22]);
                     PVPUT(RD_AverageNozzleHeat, 255&CSstatus[23]);   
                     PVPUT(RD_CryoStatus, 255&CSstatus[24]);
                     PVPUT(RD_AlarmCode, 255&CSstatus[25]);
                     PVPUT(RD_RunTime,(65535&(((CSstatus[26]<<8)| (255&CSstatus[27]))) )/60.);
                     PVPUT(RD_GasFlow, 0.1*(255&CSstatus[31]));
                     PVPUT(RD_LinePressure, 0.01*(255&CSstatus[32]));
                     PVPUT(RD_CryoAdjust, 255&CSstatus[33]);
                     PVPUT(RD_OuterFlow, 0.1*(255&CSstatus[34]));
                     PVPUT(RD_DetectedGasType, 255&CSstatus[35]);
                     PVPUT(RD_SelectedGasType, 255&CSstatus[36]);
                     PVPUT(RD_WaterTemp,0.01*(255&CSstatus[38]));
                     PVPUT(RD_AverageGasHeat, 255&CSstatus[40]);
                     PVPUT(RD_NozzleHeat, 255&CSstatus[41]);
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


