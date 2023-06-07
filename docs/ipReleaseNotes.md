---
layout: default
title: Release Notes
nav_order: 3
---


ip Release Notes
================

Release 2-22 June, 2023
-----------------------

- New Support for:
  - SRS PTC10 programmable temperature controller.
  - Lakeshore 218
  - Protura P201 BissC Adapter
  - Oxford ILM 200 and Cryojet 2
    
- tyGSAsynInit function arguments 'parity' and 'handshake' changed to accept strings in IOC shell
    
- Documentation moved to github pages

Release 2-21-1 June, 2021
-------------------------

- New Support for:
  - Teledyne Syringe Pump
  - HP Agilent Power Supplies
 

Release 2-21 October, 2020
--------------------------

- Fixed 3 bugs in CPSyringe.db
- Changing units (.EGU fields) did not update EGU readbacks. Fixed this by changing .EGU inputs in scalcout records from NPP to CP.
- Changing the Diameter reset the rates and volumes to defaults. Fixed this by adding a SetAll seq record that rewrites the rates and volumes when the diameter is changed.
 
- New Support for:
  - BK 9173B Power Supply
    
  - Jena NV 40/3CLE Amplifier
  - ADAM 4015 6-ch RTD module
  - Advantech ADAM 4018 8-ch RTD Module
  - Metis M322 Pyrometer
  - Stanford S6390 Vector Signal Generator
  - Varian Dual Ion Pump Controller
 
- CS800 Support updated to allow multiple devices on one subnet

Release 2-20 May, 2019
----------------------

- Fixed potential string buffer overflow and 0 termination problems in SNL programs
- New Support for the BK 9173B Power Supply

Release 2-19 Dec, 2017
----------------------

- iocsh boot scripts
- Added support for Lakeshore DRC 93CA and loading communications channels on serial ports.
- Fixed protocol file PHD2000.proto
- Added support for the ADAM-4018
- Added support for MKS 651C valve system
- Added op/ui/autoconvert directory
- New version of \*Main.cpp file from base 3.15.5; contains epicsExit() after iocsh(NULL) which is needed for epicsAtExit() to work on Windows
- Update CONFIG to use CONFIG\_SITE. Update RULES and Makefile.

Release 2-18 Apr, 2016
----------------------

- BK9130: BK Precision 9130 triple output power supply
- LakeShore336: updated support for several new LS336 parameters
- CPSyringe: Added ReadStatus; Read state and infusion volume at different rate from rest of settings; read rest of settings whenever any setting is changed so ReadSettings.SCAN can be Passive

Release 2-17 Mar, 2015
----------------------

- misc doc fixes
- removed picos.db
- SR830 improvements

Release 2-16 Dec, 2014
----------------------

- New support for Harvard PHD 2000 syringe pump
- New caQtDM translations to fix related display buttons
- Restructured the module so exported stuff is in ip, and example/test stuff is in ip/iocs/ipExample. Also, made ip.dbd and ipSupport.dbd the same. Ditto for ipVX.dbd and ipVXSupport.dbd.

Release 2-15 Jul, 2014
----------------------

- New support for Tabor 8024 aribtrary waveform generator
- New support for USdigital T7 inclinometer
- New support for Cole-Palmer Syringe Pumps
- Significant update to Stanford SR630 Thermocouple support
- New support for LakeShore 335 Temperature Controller
- New support for Kepco BOP HVPS
- picoMotor.db: calc expression "a?b:c" syntax now requires both b and c

Release 2-14 Apr, 2013
----------------------

- supplied missing SR830.proto file
- minor mods to deviceCmdReply.db
- ipDoc.html - list what's in the module and what it does
- Keithley2kDMM.st - Added event flag on done\_read; previously it would read the meter when ANY monitor occurred and done\_read was 1, not just when done\_read changed to 1.
- modified .opi files (CSS-BOY); added .ui files (caQtDM)
- Added support for GE PACE5000 pressure controller
- devMPC.c - Added sanity checks for the response from the device; thanks to Larry Hoff from BNL

Release 2-13 Nov 14, 2011
-------------------------

- devXxStrParm.c fixed for stringin records when device response is longer than 40 characters; previously was not null terminating.
- Added jenaNV40 and NCD\_R2X databases and MEDM-display files.

Release 2-12 Oct 25, 2011
-------------------------

- Added support for National Control Devices R2X series of RS-232 single/dual relay controllers.
- Added support for Piezo Systems Jena NV40/1 CLE piezo controller

Release 2-11 Aug. 25, 2011
--------------------------

- Added support for LakeShore 336 TC
- Added support for Alcatel 2000 vacuum controller
- Added .opi display files for CSS-BOY
- Added support for US Digital Inclinometer
- Moved LoveController.adl to the love module
- Modified RELEASE; deleted RELEASE.arch
- Fixed Keithley2kDMM for 64-bit arch.

Release 2-10 Mar. 30, 2010
--------------------------

- devAiMKS - Don't print error messages on SYNTAX! or NotCMD! responses, since these seem to be due to a bug in the device (assuming we are really only sending good commands) and fill the log files.
- Omega\_DP41.txt - docs for Omega DP41 panel meter
- Lakeshore331 - new support for Lakeshore 331 temperature controller
- SR830 - Stanford Research Systems lockin amplifier
- SR810 - new support for Stanford Research Systems lockin amplifier
- Agilent\_E3631A\_settings.req - renamed to agree with database
- tyGSAsynInit.c - improved error reporting

Release 2-9 Dec. 19, 2008
-------------------------

- pico\_small.adl - use asyn record instead of serial
- devMPC.c - increased timeout, and defend against timeout; fixed bug when reading volts and return string length &lt; 4

Release 2-8 Sept. 10, 2008
--------------------------

- new device US Digital Serial Encoder
- new device Keithley 8517 Electrometer
- new device LakeShore 340 Temperature controller
- new device Omega DP41 Digital Panel Meter
- new device Pfeiffer TPG261 RS232 Vacuum Controller.
- new file ip/ipApp/Db/asynRecord\_settings.req (Ideally, this would be in the asyn module, but settings files that correspond with database files is a synApps notion, and asyn is not part of synApps.)
- ip/ipApp/Db/deviceCmdReply.req replaced by deviceCmdReply\_settings.req, and modified to correctly include asynRecord\_settings.req
- Updated Eurotherm 2k support
- ip/ipApp/op/adl/deviceCmdReply\_full.adl: asynRecord TMOT is in seconds, not ms.

Release 2-7 Sept. 11, 2006
--------------------------

- Added support for Newport LAE500 autocollimator
- Added support for additional thermocouple types for Keithley 2700 multimeter
- Databases using asyn serial support no longer set input/output terminator. This must now be done in asyn-record configuration.
- HeidVRZ460\_encoder.db converted to use asyn
- Keithley2kDMM\_mf.db -- Modified by Arun 08-Feb-2006 to include R-type thermocouple
- Keithley2kDMM\_mf20\_settings.req -- new file
- Keithley2kDMM\_mf\_channelN\_settings.req now just includes calcout\_settings.req
- pzt3.db, pzt\_3id.db -- BitBus-based databases deleted.
- Keithley2kDMM.st -- Added support for additional thermocouple types for 2700
- devAiHeidND261.c -- converted to use asyn
- devXxEurotherm.c -- converted to use asyn
- deviceCmdReply.db -- Now is useable for binary data (i.e., strings that may contain unprintable characters, including the null character), and for devices that require CRC's or checksums that are implemented in the string-calc software (in the calc module). However, the string-length limitation is still 39 bytes.
- Added documentation for deviceCmdReply

Release 2-6 Sept. 16, 2005
--------------------------

Fixed bugs in devAiMKS.c. Number of characters to send and receive was incorrect since terminators no longer are counted.

Fixed bugs in devMPC.c.

- The IOC was crashing if the MPC was not responding, i.e. powered off or disconnected. It now handles write and read errors gracefully.
- The Lock and Start commands had bugs and were not working. Thanks to Mohan Ramanthan for fixing these

Added support for Pelco 6700 video switch.

Added support for Stanford Research Systems SR630 thermocouple scanner

Fixed support for Agilent E3631A

Release 2-5 March 29, 2005
--------------------------

Fixed bugs in devXxStrParm.c that prevented it working with the asyn address != 0. This meant the previous version would not work with GPIB. In previous versions the VME "signal" parameter was used to indicate a buffer start address for I/O. No existing databases seem to use this feature, and it will no longer work, since the address field is needed for device addressing.

Minor change to appearance of Keithley2kDMM\_mf.adl.

Release 2-4 December 21, 2004
-----------------------------

Tested against base 3.14.7; asyn 4.1+; ipac 2.8; seq 2.0.8 This version of ip will not work with ipac versions lower than 2.8, or asyn versions lower than 4.0. Everything that used serial or GPIB records has been converted to use the asyn record. Everything that used MPF has been converted to use asyn. Some databases have been converted from gdct to VDCT.

st.cmd requires changes. In ipac 2.8, the argument list for tyGSOctalDevCreate() changed. tyGSAsynInit() is a wrapper around this and related calls, and its arg list must also change. Here's an example of the correct syntax:

```
tyGSAsynInit("serial1",  "UART_0", 0, 9600,'N',2,8,'N',"")  /* SRS570 */
```

serial\_OI\_block and GPIB\_OI\_block have been replaced by deviceCmdReply. Several devices previously supported in the std module have been moved here, including Heidenhain VRZ460, Keithley 199 DMM, and the Queensgate Piezo controller.

 [cvs log](cvsLog.txt)

Release 2-3, March 3, 2004
--------------------------

Tested against base 3.14.5; mpf 2.4.2; ipac 2.7a; seq 2.0.8 New support:

- Agilent\_E3631A power supply
- Federal digital encoder
- picoMotor piezo-driven screw

New display files:

- MPC\_TSP.adl, MPC.adl
- Federal\*.adl
- picoMotor\*.adl
- Agilent\*.adl

Generic serial database no longer sets serial port parameters.

New documentation:

- Agilent\_E3631A.txt
- Federal.txt
- devXxStrParm.README
- picoMotor.txt

Release 1-2, March 6, 2002
--------------------------

Additions to ipApp/Db: - DAC.db
- Eurotherm.db, Eurotherm
- MKS\_single.db
- Updated PI500.db

Additions to ipApp/src: - devAoEurotherm.cc
- devSoEurotherm.cc

Modifications in ipApp/src: - Added Eurotherm to ipLIBOBJS and ipShare.dbd.
- Makefile.Vx *depends* fix for \*.cc source code.

Modification in ipApp/op/adl: - Updated PI500.adl

December 4, 1999
----------------

Changes made to ipApp since synApps\_R3.13.1.1. The most important change is the addition of support for the Message Passing Facility, MPF.

- Renamed Makefile.Vx.do\_build to Makefile.Vx.buildHideos
- Added Makefile.Vx.buildMPF
- Makefile.Vx should be a soft link to one of the above two files. When building MPF this points to Makefile.Vx.buildMPF
- Minor modifications to Makefile.Host.
    
    
    - Removed drvMM400 stuff, since this is now either in stdApp or wherever the motor stuff is ultimately put.
- Added ipLIBOBJS, so when building the MPF version of ipApp it now works like stdApp and mcaApp, i.e. it copies all of the files in ipLIBOBJS to SHARE\_BIN and it makes ipLib.
- Added mpfServerLIBOBJS. This creates a library called mpfServLib which contains all of the MPF server code which need to be loaded onto an MPF server. The only exception is mpfLib, which must be loaded separately before mpfServLib.
- Renamed ipShare.dbd to be ipShare.dbd.Hideos. (Actually I seem to have deleted this file, but it is just the version from synAppsR3.13.1.1)
- Created ipShare.dbd.MPF
- Created softlink ipShare.dbd which should be a soft link to one of the above two files. These files should ultimately be identical, but MPF device support is not yet available for all devices. When building MPF this points to ipShare.dbd.MPF
- Added following device support to ipShare.dbd.MPF
    - MKS vacuum gauge controller
    - Greenspring IP-Unidig digital I/O modules from MPF
    - EPID record fast feedback from MPF

- Created following MPF device support files:
    - devAiHeidND261MPF.cc - Heidenhein encoder
    - devAiMKSMPF.cc - HPS/MKS vacuum gauge controller (*new*)
    - devAiStrParmMPF.cc - analog input string parm
    - devAoStrParmMPF.cc - analog output string parm
    - devBiStrParmMPF.cc - binary input string parm
    - devLiStrParmMPF.cc - longin string parm
    - devLoStrParmMPF.cc - longout string parm
    - devSerialMPF.cc - generic serial record device support
    - devSiStrParmMPF.cc - string in string parm
    - devSoStrParmMPF.cc - string out string parm

> Most of these are simple conversions of the corresponding Hideos device support. The only exception is devAiMKSMPF.cc which is new device support for the HPS MKS vacuum gauge controller.
> 
> Note that the DTYP string defined for the MPF device support is the same as the old Hideos DTYPE, for example "Hideos lo stringParm" or "Hideos ai HeidND261". This was done to allow conversion to MPF without having to modify the databases at all. At some point it would be good to modify ipShare.dbd to give the DTYPs names which reflect the fact that they are for *both* MPF and Hideos, not just Hideos. This will require changing all of the databases as well.

- The device support which I have *not* converted to MPF includes:
    - Love controller (but Mohan has converted this)
    - Elcomat autocollimator
    - Huber DMC 9200
    - Eurotherm temperature controller
    - GE Fanuc PLC

- Added multifunction device support for Keithley 2000 multimeter, Keithley2kDMM.st. This SNL program and the database ipApp/Db/Keithley2kDMM\_mf.db can be used instead of the ipApp/Db/Keithley2kDMM.db. It provides the capability to have each of the scanner card inputs use a different function (DCV, ACV, Ohms, etc.).

- Added serialIOMPF.cc. This is the MPF version of the Hideos file serialIO.cc. serialIO is a simple serial communication package. It is used, for example, by the MM4000 motor controller driver.

- Added the following databases to ipApp/Db
    - MKS.db - Support for HPS/MKS vacuum gauge controller. Device support is in ipApp/src.
    - Digitel.db - Support for PE Digitel 500/1500 ion pump controller. This does not have specific device support, but uses the generic serial record.
    - Keithley2kDMM\_mf.db - Support for the "multifunction" Keithley 2000 support. This uses the SNL program ipApp/src/ Keithley2kDMM.st.
    - IpUnidigLi.db - longin record support for the Greenspring IP-Unidig family of digital I/O modules. Device support is in mpf/ipUnidigApp.
    - IpUnidigBi.db - binary input record support for the Greenspring IP-Unidig family of digital I/O modules. Device support is in mpf/ipUnidigApp.
    - IpUnidigBo.db - binary output record support for the Greenspring IP-Unidig family of digital I/O modules. Device support is in mpf/ipUnidigApp.
    - ip330Scan.db - support for the Acromag IP-330 ADC as a scanning A/D converter. Device support is in mpf/ip330App.
    - fast\_pid\_control.db - support for fast feedback using the EPID record (in stdApp). Device support is in mpf/ip330App.

- Modified the following databases in ipApp/Db
    - generic\_serial.db. Added PRTY, BAUD, DBIT and SBIT parameters so that the record can be configured from st.cmd to communicate correctly.

- Added the following MEDM .adl files ipApp/op/adl
    - IpUnidig.adl - graphical screen with 24 binary inputs and outputs for the Greenspring IP-Unidig family of digital I/O modules.
    - Ip330Scan.adl - screen for 16 A/D inputs for the Acromag IP330 ADC using the Ip330Scan device support.
    - Keithley2kDMM\_mf.adl and Keithley2kDMM\_full\_mf.adl - screens for the Keithley 2000 multifunction support.
