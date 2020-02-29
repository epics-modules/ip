### Jena NV40/1CLE support

# serial configuration
drvAsynSerialPortConfigure("serial1", "/dev/ttyUSB0", 0, 0, 0)
asynSetOption("serial1",0,baud,9600)
asynSetOption("serial1",0,bits,"8")
asynSetOption("serial1",0,parity,"None")
asynSetOption("serial1",0,stop,"1")
#!asynSetOption("serial1",0,clocal,"Y")
#!asynSetOption("serial1",0,crtscts,"N")
#!drvAsynSerialPortConfigure("serial1", "/dev/ttyS1", 0, 0, 0)
#!drvAsynSerialPortConfigure("serial1", "/dev/ttyS2", 0, 0, 0)

dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PREFIX),R=jena1:asyn,PORT=serial1,ADDR=0,OMAX=0,IMAX=0")
#!dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PREFIX),R=jena2:asyn,PORT=serial2,ADDR=0,OMAX=0,IMAX=0")
#!dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PREFIX),R=jena3:asyn,PORT=serial3,ADDR=0,OMAX=0,IMAX=0")

asynSetOption("serial1", 0, "ixon", "Y")
#!asynSetOption("serial2", 0, "ixon", "Y")
#!asynSetOption("serial3", 0, "ixon", "Y")

# ASYN trace masks
#!asynSetTraceMask(serial1,   0, 0x9)
#!asynSetTraceIOMask(serial1, 0, 0x2)
#!asynSetTraceMask(serial2,   0, 0x9)
#!asynSetTraceIOMask(serial2, 0, 0x2)
#!asynSetTraceMask(serial3,   0, 0x9)
#!asynSetTraceIOMask(serial3, 0, 0x2)

dbLoadTemplate("JenaNV40_1CLE.substitutions")
