---
layout: default
title: Supported Devices
nav_order: 3
has_children: true
---

# Supported Devices
{: .no_toc}

## Table of contents
{: .no_toc .text-delta}

- TOC
{:toc}

This page lists all devices supported by the ip module, grouped by
category. Each entry lists the associated database, protocol, display,
and source files. Devices with more detailed documentation have their
own pages linked from the sidebar.

## Temperature Controllers

### Eurotherm 800 and 2000 series

Custom device support using asyn.

- devXxEurotherm.c
- Eurotherm.db
- Eurotherm2k.db, Eurotherm2k_settings.req
- Eurotherm.adl, Eurotherm2k.adl, Eurotherm2k_more.adl

### LakeShore 218

StreamDevice support for the LakeShore 218 temperature monitor.

- LakeShore218.vdb

### LakeShore 330

Custom device support using asyn.

- LakeShore330.db, LakeShore330_settings.req
- LakeShore330.adl, LakeShore330_more.adl

Serial connection uses a straight-through RJ-11 (6-conductor) cable
with XM-Octal default jumpers. Pinout: pins 1,2 = RxD (to DB9 pin 3),
pins 3,4 = GND (to DB9 pin 5), pins 5,6 = TxD (to DB9 pin 2).
1200 or 300 baud, 7 data bits, 1 stop bit, odd parity.

### LakeShore 331

StreamDevice support.

- LakeShore331.db, LakeShore331.req
- LakeShore331.adl, LakeShore331_more.adl

### LakeShore 335

StreamDevice support.

- LakeShore335.db, LakeShore335.proto, LakeShore335.req
- LakeShore335.adl

### LakeShore 336

StreamDevice support.

- LakeShore336.db, LakeShore336.proto, LakeShore336.req
- LakeShore336.adl, LakeShore336_full.adl, LakeShore336_more.adl

IOC example (Ethernet):

```
drvAsynIPPortConfigure("ether1", "<TC336 IP>:7777", 0, 0, 0)
asynOctetSetInputEos("ether1",0,"\n")
asynOctetSetOutputEos("ether1",0,"\n")
dbLoadRecords("$(IP)/db/LakeShore336.db", "P=xxx:,Q=TC1,PORT=ether1,ADDR=0")
```

{: .note}
> The "Manual" setting (percentage override of the power output) sets
> the output power in Open Loop mode; the SetPoint parameter is ignored.
> Zone Mode is Closed Loop with an internal table that changes control
> settings based on crossing setpoint thresholds. Loops 3 and 4 are
> voltage outputs for external power supplies and do not perform closed
> loop control.

### LakeShore 340

StreamDevice support.

- LakeShore340.db, LakeShore340.req
- LakeShore340.adl, LakeShore340_more.adl

Serial connection uses a straight-through RJ-11 (6-conductor) cable
with the same pinout as the LakeShore 330.

### LakeShore DRC-93CA

- LakeShoreDRC-93CA.db
- LakeShore.adl

### Oxford CryoStream 800

SNL-based support for the Oxford CryoStream 800 cooler. See the
[Oxford CryoStream 800](Oxford_CS800.md) page for full documentation.

### Oxford ILM200 / ILM202

Liquid level monitors.

- Oxford_ILM202.db
- Oxford_ILM202.adl

### Oxford CryoJet

StreamDevice support.

- Oxford_CryoJet.db, Oxford_CryoJet.proto

### Oxford Cyberstar X1000

Scintillator detector.

- Oxford_X1k.db, Oxford_X1k.adl
- cyberAmp.db, cyberAmp.adl

### Thermo Neslab EX

StreamDevice support. Written by Kurt Goetze.

- NeslabEX.db, NeslabEX.proto
- NeslabEX.adl

Serial: 19200 baud, 8N1. Set front panel to Computer mode.

IOC example:

```
tyGSAsynInit("serial3", "UART_0", 2, 19200, 'N', 1, 8, 'N', "", "")
dbLoadRecords("$(IP)/db/NeslabEX.db", "P=xxx:,S=1,PORT=serial3")
```

### SRS PTC10

StreamDevice support for the Stanford Research Systems PTC10
programmable temperature controller.

- PTC10.db, PTC10.proto, PTC10_settings.req
- PTC10_tc_chan.db, PTC10_rtd_chan.db, PTC10_aio_chan.db
- PTC10_coldj_chan.db, PTC10_tec_in_chan.db, PTC10_tec_out_chan.db

### Omega iSeries

StreamDevice support.

- iSeries.db, iSeries.proto

### Advantech ADAM 4015

6-channel RTD module.

- ADAM_4015.db, ADAM_4015_chan.db
- ADAM_4015.proto

### Advantech ADAM 4018 / 4018+

8-channel thermocouple input module. StreamDevice support.

- ADAM_4018.db, ADAM_4018_chan.db
- ADAM_4018.proto, ADAM_4018_settings.req, ADAM_4018_chan_settings.req
- ADAM_4018.adl, ADAM_4018_chan.adl, ADAM_4018_setup.adl
- ADAM_4018_temp.adl, ADAM_4018_temp_chan.adl

## Vacuum Controllers

### MKS 937 Vacuum Gauge Controller

Custom device support using asyn.

- devAiMKS.c
- MKS.db, MKS_single.db

### GP307 Vacuum Controller

GPIB device support.

- devGP307gpib.c

### MPC Ion Pump Controller

Custom device support using asyn.

- devMPC.c, devMPC.h
- MPC.db, MPC_settings.req
- MPC.adl, MPC_TSP.adl

### Pfeiffer TPG261 Vacuum Controller

Custom device support using asyn.

- devTPG261.c, devTPG261.h
- TPG261.db, TPG261.adl

### Televac Vacuum Controller

Custom device support using asyn.

- devTelevac.c, devTelevac.h

### Alcatel ACS 1000

StreamDevice support.

- Alcatel_ACS1000.db, Alcatel_ACS1000.protocol

### Alcatel ACS 2000

StreamDevice support.

- Alcatel_ACS2000.db, Alcatel_ACS2000.protocol

### Physical Electronics Digitel 500/1500

Ion pump controller using generic serial record support.

- Digitel.db

### TSP Ion Pump Controller

- TSP.db, TSP_settings.req

### Varian Dual Ion Pump Controller

- VarianDualIP.db, VarianDualIP.protocol

## Power Supplies

### BK Precision 9130

StreamDevice support for the BK Precision 9130 triple output power supply.

- BK9130.db, BK9130.proto, BK9130_settings.req
- BK9130.adl

Serial: 9600 baud, 8N1, LF terminator. The device's DB9 connector is
TTL, not RS-232. An adapter from BK Precision is required; for
low-power UARTs like the IP-Octal, supply +12 VDC to the adapter on
DB9 pins 4 (+) and 5 (-).

IOC example:

```
dbLoadRecords("$(IP)/db/BK9130.db", "P=xxx:,PS=1,PORT=serial2")
```

### BK Precision 9173B

- BK9173B.db, BK9173B.proto

### Agilent E3631A

Programmable triple output power supply.

- Agilent_E3631A.db, Agilent_E3631A_settings.req, Agilent_settings.req
- Agilent_E3631A.adl, Agilent_E3631A_A.adl

Serial: 9600 baud, 8N2. Requires a jumper tying pin 6 (DSR) to
Space (+12 V) on the XM-Octal breakout. Cable is straight-through
with 2/3 swap and 7/8 swap. Messages terminate with CR (out) and
LF (in).

IOC example:

```
tyGSAsynInit("serial1", 0, 0, 9600, 'N', 2, 8, 'N', "")
dbLoadRecords("$(IP)/db/Agilent_E3631A.db", "P=xxx:,S=ps1,PORT=serial1")
```

### HP/Agilent PS66xxA

- HP_Agilent_PS66xxA.db, HP_Agilent_PS66xxA.proto

### Instek GPP

- InstekGPP.db, InstekGPP.proto

### Kepco BOP

High-voltage power supply.

- Kepco_BOP.db

## Encoders and Position

### Heidenhain ND261

Serial encoder reader. Custom device support using asyn.

- devAiHeidND261.c
- heidND261.db, HeidND261.adl

### Heidenhain AWE1024

GPIB encoder interpolator (vxWorks only).

- devXxHeidenhainGpib.c
- HeidAWE1024.db
- HeidEncoder.adl, 2HeidEncoder.adl

### Heidenhain VRZ460

Encoder reader using asyn.

- HeidVRZ460_encoder.db, HeidVRZ460_encoder.adl

### US Digital AD4

Quadrature to serial adaptor. StreamDevice support.

- Encoder_AD4.db, Encoder_AD4.proto, Encoder_AD4_settings.req
- Encoder_AD4.adl

Serial: 9600 baud, 8N1, no EOS (binary protocol).

IOC example:

```
drvAsynSerialPortConfigure("serial1", "/dev/ttyS0", 0, 0, 1)
asynSetOption(serial1, 0, baud, 9600)
dbLoadRecords("$(IP)/db/Encoder_AD4.db", "P=xxx:,N=1,PORT=serial1")
```

### US Digital X3

Multi-axis absolute MEMS inclinometer. StreamDevice support.

- USdigital_X3.vdb, USdigital_X3.proto, USdigital_X3_settings.req
- USdigital_X3.adl, USdigital_X3_more.adl

Serial: 19200 baud (must be changed from default 115200 using the
vendor's demo program). Straight-through cable with XM-Octal default
jumpers.

IOC example:

```
tyGSAsynInit("serial5", "UART_0", 4, 19200, 'N', 1, 8, 'N', "", "")
dbLoadRecords("$(IP)/db/USdigital_X3.vdb", "P=xxx:,S=1,PORT=serial5")
```

### US Digital T7

Single-axis absolute inclinometer. StreamDevice support.

- USdigital_T7.vdb, USdigital_T7.proto, USdigital_T7_settings.req
- USdigital_T7.adl, USdigital_T7_more.adl

Use the vendor's "T7Demo" program to change the device address to 1
and optionally change the baud rate from the default 115200.

### Federal ERO-1071

Digital encoder reader. SNL support.

- Federal.st
- Federal.db, Federal.adl, Federal_more.adl, Federal_setup.adl

Serial: 19200 baud, even parity, 2 stop bits, 7 data bits. The device
expects STX (0x02) for readings with no CR/LF terminator.

Federal box settings: probe = "scan", x.xxx = match display precision,
show = all "yes", units = "mm", rs232 = 19200/2/even/no/no.

IOC example:

```
tyGSAsynInit("serial1", 0, 0, 19200, 'E', 2, 7, 'N', "")
dbLoadRecords("$(IP)/db/Federal.db", "P=xxx:,S=s1,C=0,SERVER=serial1")
```

### New Focus 8750 Picomotor Controller

- picoMotor.db, picoMotor.substitutions
- picoMotor_positions.req, picoMotor_settings.req
- picoStop12.db, picoStop8.db
- picoMotor.adl, picoMotor_18.adl, picoMotor_9.adl
- picoMotor_help.adl, picoMotor_more.adl
- picoTop_example.adl, pico_small.adl, topPicoMotors12.adl

Serial: 19200 baud, 8N1, 2-3 swap cable.

{: .note}
> Minimum velocity must be less than velocity. E-Stop causes the motor
> to lose position. Joystick must be off and Driver must be on for EPICS
> operation. If the controller is power-cycled while the IOC is running,
> "joystick off" and "driver on" must be re-asserted, and dynamics
> parameters must be re-sent if not hardware-saved.

### Huber DMC9200

Motor controller.

- HuberDMC9200.db, HuberDMC9200Init.db
- HuberDMC.adl

### Newport LAE500

Autocollimator.

- Newport_LAE500.db, Newport_LAE500_settings.req
- Newport_LAE500.adl, Newport_LAE500_help.adl

### Protura P201

BissC adapter.

- Protura_P201.db, Protura_P201.proto

### MicroE SS350

- MicroE_SS350.vdb, MicroE_SS350.proto

## Multimeters and Electrometers

### Keithley 2000 / 2700

SNL-based device support for the Keithley 2000 series digital
multimeters. Supports standard and multifunction (per-channel
function selection) modes.

- Keithley2kDMM.st, Keithley2kDMM_mf40.st
- Keithley2kDMM.db, Keithley2kDMM20.db, Keithley2kDMM_mf.db, Keithley2kDMM_mf40.db
- Keithley2kDMM_settings.req, Keithley2kDMM20_settings.req
- Keithley2kDMM_channelN_settings.req, Keithley_10CH_settings.req
- Keithley2kDMM_mf10_settings.req, Keithley2kDMM_mf20_settings.req
- Keithley2kDMM_mf22_settings.req, Keithley2kDMM_mf40_settings.req
- Keithley2kDMM_mf_channelN_settings.req
- Keithley2kDMM.adl, Keithley2kDMM20.adl, Keithley2kDMM20_full.adl
- Keithley2kDMM_full.adl, Keithley2kDMM_mf.adl, Keithley2kDMM_full_mf.adl
- Keithley2kDMM_mf40.adl, Keithley2kDMM_10chan_mf.adl
- Keithley2kDMM_11_22_mf.adl, Keithley2kDMM_calc.adl
- Keithley2kDMM_scpi.adl, scpi_commands.adl

### Keithley 199

GPIB digital multimeter (vxWorks only).

- devXxKeithleyDMM199Gpib.c
- KeithleyDMM.db
- KeithleyDMM.adl, KeithleyDMM_full.adl

### Keithley 65xx Electrometer

SNL-based device support.

- Keithley65xxEM.st, Keithley65xxEM.db
- Keithley65xxEM.adl

### Omega DP41 Panel Meter

StreamDevice support. Written by Mark Rivers.

- Omega_DP41.db, OmegaDP41.db
- Omega_DP41.proto, Omega_DP41_settings.req
- OmegaDP41.adl, OmegaDP41_more.adl
- Omega_DP41.adl, Omega_DP41_settings.adl

Serial: 9600 baud, 7 data bits, 2 stop bits. The device sets bit 7
(MSB) = 1 on all output bytes; use 7 data bits and 2 stop bits on
the terminal server as a workaround.

{: .important}
> The third bit of BUS FT ("ECHO response") must be on (1) for
> StreamDevice support. Set it from the DP41 front panel under BUS FT.

IOC example:

```
tyGSAsynInit("serial1", 0, 0, 9600, 'N', 2, 7, 'N', "\r", "\r")
dbLoadTemplate("Omega_DP41.substitutions")
```

### Metis M322 Pyrometer

- Metis_M322.db, Metis_M322.proto

## Lock-in Amplifiers

### Stanford SR810

- SR810.vdb, SR810.adl

### Stanford SR830

StreamDevice support.

- SR830.vdb, SR830.proto, SR830_settings.req
- SR830.adl

Serial: 9600 baud, 8 data bits, 2 stop bits, no parity, no
handshaking. Terminators: CR in both directions.

Cable: DB9 pin 3 to DB25 pin 2, DB9 pin 2 to DB25 pin 3, DB9 pin 5
to DB25 pin 7.

IOC example:

```
dbLoadRecords("$(IP)/db/SR830.vdb", "P=xxx:,N=1,PORT=serial9")
```

## Signal Generators and Waveform Generators

### Stanford SRS SG390

Vector signal generator.

- SRS_SG390.db, SRS_SG390.proto

### Tabor 8024

Arbitrary waveform generator.

- Tabor8024.db, Tabor8024_settings.req, Tabor8024.adl

### Rigol DG4000

Function/arbitrary waveform generator. StreamDevice support.

- Rigol_DG4000.db, Rigol_DG4000_ChN.db
- Rigol_DG4000.proto, Rigol_DG4000_settings.req, Rigol_DG4000_ChN_settings.req

### Rigol DS1000Z

Digital oscilloscope. StreamDevice support.

- Rigol_DS1000Z.db, Rigol_DS1000Z_ChN.db
- Rigol_DS1000Z.proto, Rigol_DS1000Z_settings.req, Rigol_DS1000Z_ChN_settings.req

## Piezo Controllers

### Queensgate AX301

GPIB-based piezo controller. Custom device support.

- devXxAX301.c
- pzt.db, pzt.adl, pzt_1id.adl, pzt_3id.adl

Serial cable: DB9 pin 2 to DB25 pin 2, DB9 pin 3 to DB25 pin 3,
DB9 pin 5 to DB25 pin 7.

### Piezosystems Jena NV 40/1 CLE

StreamDevice support.

- JenaNV40.db, JenaNV40.proto
- jenaNV40.adl, jenaNV40_genTweak.adl

### PI 500

- PI500.db, PI500.adl

## Syringe Pumps

### Cole-Palmer Syringe Pump

StreamDevice support.

- CPSyringe.db, CPSyringe.proto, CPSyringe_settings.req
- CPSyringe.adl

### Harvard PHD 2000

StreamDevice support.

- PHD2000.db, PHD2000.proto

### Teledyne Syringe Pump

- teledynePumpD.db

## Preamplifiers and Signal Conditioning

### Stanford SR570

Current preamplifier.

- SR570.db, SR570preamp_settings.req
- SR570.adl, SR570_tiny.adl

### Stanford SR630

Thermocouple reader. StreamDevice support.

- SR630.db, SR630.proto, SR630_settings.req
- SR630.adl

## Pressure Controllers

### GE PACE5000

StreamDevice support.

- PACE5000.db, PACE5000.proto, PACE5000_settings.req
- PACE5000.adl

### MKS 651C

Valve system. StreamDevice support.

- MKS651C.db, MKS651C.proto

## Optical and Shutter Controls

### Pelco CM6700

Video switch.

- Pelco_CM6700.db, Pelco_CM6700_names.db, Pelco_CM6700_settings.req
- Pelco_CM6700.adl

### Thorlabs SC10

Shutter controller.

- Thorlabs_SC10.db, Thorlabs_SC10.proto

### Uniblitz D122 / VCM-D1

Shutter controllers.

- uniblitz_d122.db, uniblitz_vcm-d1.db, uniblitz.proto

### XIA PFCU Filters

- XIA_pfcu_filters.db, XIA_pfcu_filters.proto

## Data Acquisition and I/O

### Advantech ADAM 4018+

See [Advantech ADAM 4018 / 4018+](#advantech-adam-4018--4018) in the
Temperature Controllers section.

### KeyenceCL3000

- KeyenceCL3000_main.db, KeyenceCL3000_chan.db
- KeyenceCL3000.proto

### Mitutoyo MIG2

Multi-input gathering unit.

- MitutoyoMIG2.db
- MitutoyoMIG2.adl, MitutoyoMIG2_more.adl

### NCD R2X

Single/Dual RS-232 relay controllers.

- NCD_R2X.db, NCD_R2X.proto, NCD_R2X.adl

### Synaccess NetBooter

Network power switch.

- Synaccess_np05b.db, Synaccess_np1601du.db
- Synaccess_netBooter.proto

### HG-100

- HG-100.db, HG-100.proto

### AMI 420

- AMI420.vdb

## Generic Support

### deviceCmdReply

Run-time programmable database for building commands, sending them to
serial/GPIB/socket devices, and parsing replies without writing device
support code. See the [deviceCmdReply](deviceCmdReply.md) page for
full documentation.

- deviceCmdReply.db, deviceCmdReply_settings.req
- deviceCmdReply.adl, deviceCmdReply_full.adl

### devXxStrParm (deprecated)

Generic command build and parsing device support using the asyn
`parm` string to specify format, terminators, timeout, and buffer
size. StreamDevice is the preferred replacement.

- devXxStrParm.c
- serialFmt.db

Supported record types: ai, ao, bi, longin, longout, stringin,
stringout. Parameters are specified in the INP/OUT link parm field:

```
field(INP, "#C1 S0 @SERVER=serial1,TERM=0d0a,TO=100,FMT=%lf")
```

| Parameter | Description | Default |
| - | - | - |
| TERM | Terminator as hex bytes (e.g., `0d0a` = CR/LF) | `0d0a` |
| IX | Input buffer index where conversion begins | signal element |
| FMT | sprintf/sscanf format string (max 31 chars) | record DESC field |
| TO | Timeout in milliseconds | 1000 (input), 3000 (output) |
| N | Input buffer size | 100 |
| 0STR | String interpreted as binary 0 (bi only) | empty |
| 1STR | String interpreted as binary 1 (bi only) | empty |

### tyGSAsynInit

Shorthand for a common collection of asyn commands used to configure
GreenSpring serial ports.

- tyGSAsynInit.c (vxWorks only)

### serial and GPIB utilities

Backward-compatibility and test displays.

- serial.db, serialFmt.db, serialtest.adl
- Serial_IO.adl, Serial_Port_Setup.adl
- serial_OI_block.adl, serial_OI_block_full.adl

### Autosave request for asyn record

- asynRecord_settings.req
