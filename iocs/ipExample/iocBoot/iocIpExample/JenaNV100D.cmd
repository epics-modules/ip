### Jena NV100/D support

# serial configuration
drvAsynSerialPortConfigure("serial1", "/dev/ttyUSB0", 0, 0, 0)
asynSetOption("serial1",0,baud,115200)
asynSetOption("serial1",0,bits,"8")
asynSetOption("serial1",0,parity,"None")
asynSetOption("serial1",0,stop,"1")
#!asynSetOption("serial1",0,clocal,"Y")
#!asynSetOption("serial1",0,crtscts,"N")
# ethernet configuration
#!drvAsynIPPortConfigure("eth1","192.168.1.???:23",0,0,0)
# Don't set the terminator, the protocol file will handle it
#!asynOctetSetInputEos( "eth1",0,"\r")
#!asynOctetSetOutputEos("eth1",0,"\r")

dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PREFIX),R=jena1:asyn,PORT=serial1,ADDR=0,OMAX=0,IMAX=0")

asynSetOption("serial1", 0, "ixon", "Y")

# ASYN trace masks
#!asynSetTraceMask(serial1,   0, 0x9)
#!asynSetTraceIOMask(serial1, 0, 0x2)

dbLoadTemplate("JenaNV100D.substitutions")
