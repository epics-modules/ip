< envPaths
dbLoadDatabase("../../dbd/ipExample.dbd")
ipExample_registerRecordDeviceDriver(pdbbase)


iocInit

seq &Keithley2kDMM, "P=13Keithley1:, Dmm=DMM1, channels=22, model=2700, stack=10000"

