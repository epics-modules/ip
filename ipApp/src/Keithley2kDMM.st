program Keithley2kDMM ("P=13LAB:, Dmm=DMM1")

/*  This sequencer program works with Keithley2kDMM.db.  This version of the
 *  Keithley2kDMM support has been significantly rearranged so that the database
 *  contains largely unlinked records and the flow control is done by this
 *  sequence program.  The support has been significantly enhanced to permit
 *  each channel to have a different mode (V DC, Ohms, etc.).
 *
 *  Mark Rivers
 *  March 17, 1998
 *
 *  Based on original database by Kurt Goetze
 *  Modifications:
 *  22-APR-1999 MLR  Added logic to work around bug with AC volts on scanner
 *  31-MAY-1999 MLR  Fixed logic so it does not close relay repeated if only
 *                   one channel being used in multi-channel mode
*/

/* This program must be compiled with the recursive option so that multiple copies
 * can run at once */
option +r;

#define NUM_CHANNELS 10
#define NUM_FRONT_MODES 10
#define NUM_REAR_MODES 8

int i;
int scan_chan;
long previous_mode;
long previous_chan;

long    ch_mode[NUM_CHANNELS];
assign  ch_mode to 
           {"{P}{Dmm}ch1_mode_sel.VAL",
            "{P}{Dmm}ch2_mode_sel.VAL",
            "{P}{Dmm}ch3_mode_sel.VAL",
            "{P}{Dmm}ch4_mode_sel.VAL",
            "{P}{Dmm}ch5_mode_sel.VAL",
            "{P}{Dmm}ch6_mode_sel.VAL",
            "{P}{Dmm}ch7_mode_sel.VAL",
            "{P}{Dmm}ch8_mode_sel.VAL",
            "{P}{Dmm}ch9_mode_sel.VAL",
            "{P}{Dmm}ch10_mode_sel.VAL"};
monitor ch_mode;
evflag  ch_mode_mon;
sync    ch_mode ch_mode_mon;

long    ch_close[NUM_CHANNELS];
assign  ch_close to 
           {"{P}{Dmm}ch1_close.PROC",
            "{P}{Dmm}ch2_close.PROC",
            "{P}{Dmm}ch3_close.PROC",
            "{P}{Dmm}ch4_close.PROC",
            "{P}{Dmm}ch5_close.PROC",
            "{P}{Dmm}ch6_close.PROC",
            "{P}{Dmm}ch7_close.PROC",
            "{P}{Dmm}ch8_close.PROC",
            "{P}{Dmm}ch9_close.PROC",
            "{P}{Dmm}ch10_close.PROC"};

long    ch_on_off[NUM_CHANNELS];
assign  ch_on_off to 
           {"{P}{Dmm}Ch1_on_off.VAL",
            "{P}{Dmm}Ch2_on_off.VAL",
            "{P}{Dmm}Ch3_on_off.VAL",
            "{P}{Dmm}Ch4_on_off.VAL",
            "{P}{Dmm}Ch5_on_off.VAL",
            "{P}{Dmm}Ch6_on_off.VAL",
            "{P}{Dmm}Ch7_on_off.VAL",
            "{P}{Dmm}Ch8_on_off.VAL",
            "{P}{Dmm}Ch9_on_off.VAL",
            "{P}{Dmm}Ch10_on_off.VAL"};
monitor ch_on_off;

long    ch_raw[NUM_CHANNELS];
assign  ch_raw to 
           {"{P}{Dmm}Ch1_raw.PROC",
            "{P}{Dmm}Ch2_raw.PROC",
            "{P}{Dmm}Ch3_raw.PROC",
            "{P}{Dmm}Ch4_raw.PROC",
            "{P}{Dmm}Ch5_raw.PROC",
            "{P}{Dmm}Ch6_raw.PROC",
            "{P}{Dmm}Ch7_raw.PROC",
            "{P}{Dmm}Ch8_raw.PROC",
            "{P}{Dmm}Ch9_raw.PROC",
            "{P}{Dmm}Ch10_raw.PROC"};

/* Note:  dci and aci must be the last 2 modes.  These are defined for
   front panel use only. */
long dmm_modes[NUM_FRONT_MODES];
assign dmm_modes to 
           {"{P}{Dmm}conf_dcv.PROC",
            "{P}{Dmm}conf_acv.PROC",
            "{P}{Dmm}conf_ohm2w.PROC",
            "{P}{Dmm}conf_ohm4w.PROC",
            "{P}{Dmm}conf_freq.PROC",
            "{P}{Dmm}conf_tempj.PROC",
            "{P}{Dmm}conf_tempk.PROC",
            "{P}{Dmm}conf_tempt.PROC",
            "{P}{Dmm}conf_dci.PROC",
            "{P}{Dmm}conf_aci.PROC"};

long ch_mode_sel;
assign ch_mode_sel to "{P}{Dmm}ch_mode_sel.VAL";
monitor ch_mode_sel;
evflag  ch_mode_sel_mon;
sync    ch_mode_sel ch_mode_sel_mon;


long done_read;
assign  done_read to "{P}{Dmm}done_read.VAL";
monitor done_read;

long dmm_chan;
assign  dmm_chan to "{P}{Dmm}dmm_chan.VAL";
monitor dmm_chan;
evflag  dmm_chan_mon;
sync    dmm_chan dmm_chan_mon;

long dmm_delay_read;
assign  dmm_delay_read to "{P}{Dmm}dmm_delay_read.PROC";

long dmm_read;
assign  dmm_read to "{P}{Dmm}dmm_read.PROC";

long Dmm_raw;
assign  Dmm_raw to "{P}{Dmm}Dmm_raw.PROC";

long read_complete;
assign  read_complete to "{P}{Dmm}read_complete.VAL";
monitor read_complete;
evflag  read_complete_mon;
sync    read_complete read_complete_mon;

long single_multi;
assign  single_multi to "{P}{Dmm}single_multi.VAL";
monitor single_multi;

long init_dmm;
assign init_dmm to "{P}{Dmm}init_dmm.PROC";

char *unit_strings[NUM_FRONT_MODES];
char *P;
char *Dmm;
string pvname;

string dmm_units; assign dmm_units to "{P}{Dmm}units.VAL";
string ch_units; assign ch_units to "";

ss mode_change {
    state init {
        when() {
            /* We set the following event flags to force these modes and
               labels to be set correctly initially */
            efSet(ch_mode_mon);
            efSet(ch_mode_sel_mon);
            efSet(dmm_chan_mon);
            unit_strings[0] = "VDC";
            unit_strings[1] = "VAC";
            unit_strings[2] = "OHM";
            unit_strings[3] = "OHM";
            unit_strings[4] = "HZ";
            unit_strings[5] = "DegC";
            unit_strings[6] = "DegC";
            unit_strings[7] = "DegC";
            unit_strings[8] = "ADC";
            unit_strings[9] = "AAC";
            P = macValueGet("P");
            Dmm = macValueGet("Dmm");
        } state monitor_mode_changes
    }
    state monitor_mode_changes {
        when(efTestAndClear(ch_mode_mon)) {
            for (i=0; i<NUM_CHANNELS; i++) {
                /* Note: We have detected that one of the channel modes has
                   changed.  For now we update all the channel labels.
                   Although inefficient, it is done infrequently and the lack
                   of string arrays in SNC makes any other approach a real
                   mess. 
                */
                sprintf(ch_units, "%s", unit_strings[ch_mode[i]]);
                sprintf(pvname, "%s%sch%d_units.VAL", P, Dmm, i+1);
                pvAssign(ch_units, pvname);
                pvPut(ch_units);
            }
        } state monitor_mode_changes

        when(efTestAndClear(ch_mode_sel_mon)) {
            sprintf(dmm_units, "%s", unit_strings[ch_mode_sel]);
            pvPut(dmm_units);
            pvPut(dmm_modes[ch_mode_sel]);
            previous_mode = ch_mode_sel;
        } state monitor_mode_changes

        when(efTestAndClear(dmm_chan_mon)) {
            pvPut(ch_close[dmm_chan]);
            previous_chan = dmm_chan;
        } state monitor_mode_changes
    }
}

ss read_meter {
    state init {
        when() {
            scan_chan = 0;
            previous_mode = -1;
            previous_chan = -1;
            efClear(read_complete_mon);
            pvPut(init_dmm);
        } state wait_read
    }
    state wait_read {
        when(done_read == 1) {
        } state read_channel
    }
    state read_channel {
        when (single_multi == 0) {
            if (dmm_chan != previous_chan) {
                pvPut(ch_close[dmm_chan]);
                previous_chan = dmm_chan;
            }
            if (ch_mode_sel != previous_mode) {
                sprintf(dmm_units, "%s", unit_strings[ch_mode_sel]);
                pvPut(dmm_units);
                pvPut(dmm_modes[ch_mode_sel]);
                previous_mode = ch_mode_sel;
            }
            read_complete=0;
            pvPut(read_complete);
            efClear(read_complete_mon);
            pvPut(dmm_read);
        } state single_read_response

        when((single_multi != 0) && (ch_on_off[scan_chan] == 0)) {
            if (scan_chan != previous_chan) {
                pvPut(ch_close[scan_chan]);
                previous_chan = scan_chan;
            }
            if (ch_mode[scan_chan] != previous_mode) {
                pvPut(dmm_modes[ch_mode[scan_chan]]);
                previous_mode = ch_mode[scan_chan];
            }
            read_complete=0;
            pvPut(read_complete);
            efClear(read_complete_mon);
            /* This logic adds a delay between the ch_clos and the
             * dmm_read if the function is AC volts, to work around bug in
             * Keithley 2000. */
            if (ch_mode[scan_chan] == 1)
                pvPut(dmm_delay_read);
            else
                pvPut(dmm_read);
        } state multi_read_response

        when((single_multi != 0) && (ch_on_off[scan_chan] != 0)) {
        } state next_channel
    }
    state single_read_response {
        when(efTestAndClear(read_complete_mon)) {
            pvPut(Dmm_raw);
            done_read = 0;
            pvPut(done_read);
        } state wait_read
    }
    state multi_read_response {
        when(efTestAndClear(read_complete_mon)) {
            pvPut(ch_raw[scan_chan]);
        } state next_channel
    }
    state next_channel {
        when (scan_chan < (NUM_CHANNELS-1)) {
            scan_chan++;
        } state read_channel
        when (scan_chan >= (NUM_CHANNELS-1)) {
            scan_chan = 0;
            done_read = 0;
            pvPut(done_read);
        } state wait_read
    }
}
