#
drvAsynIPPortConfigure("tcp390","192.168.1.2:5025",0,0,0)
# Don't set the terminator, the protocol file will handle it
#!asynOctetSetInputEos( "tcp390",0,"\n")
#!asynOctetSetOutputEos("tcp390",0,"\n")
# Turn on asyn traces
asynSetTraceMask("tcp390",0,3)
asynSetTraceIOMask("tcp390",0,1)

#
dbLoadRecords("$(IP)/ipApp/Db/SRS_SG390.db", "P=IOC:,R=sg390,PORT=tcp390")

# Turn off asyn traces
asynSetTraceMask("tcp390",0,1)
asynSetTraceIOMask("tcp390",0,0)

dbLoadRecords("$(ASYN)/db/asynRecord.db","P=IOC:,R=sg390:asyn,PORT=tcp390,ADDR=0,OMAX=256,IMAX=256")
