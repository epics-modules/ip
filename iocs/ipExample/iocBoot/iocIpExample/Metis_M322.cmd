
# Serial port setup
drvAsynSerialPortConfigure("serial1", "/dev/ttyUSB1", 0, 0, 0)
asynSetOption("serial1",0,"baud","19200")
asynSetOption("serial1",0,"parity","even")
asynSetOption("serial1",0,"bits","8")
asynSetOption("serial1",0,"stop","1")
asynOctetSetOutputEos("serial1",0,"\r")
asynOctetSetInputEos("serial1",0,"\r")

#
dbLoadRecords("$(TOP)/9idcPYROApp/Db/Metis_M322.db","P=$(PREFIX),R=m322,A=00,PORT=serial1")

#
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PREFIX),R=m322:asyn,PORT=serial1,ADDR=0,OMAX=0,IMAX=0")
