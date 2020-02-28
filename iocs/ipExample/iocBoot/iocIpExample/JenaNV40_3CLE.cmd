### Jena NV40/3CLE support

# serial configuration

dbLoadRecords("$(ASYN)/db/asynRecord.db","P=IOC:,R=jena:asyn1,PORT=serial7,ADDR=0,OMAX=0,IMAX=0")

asynSetOption("serial7", 0, "ixon", "Y")

# ASYN trace masks
#!asynSetTraceMask(serial7,   0, 0x9)
#!asynSetTraceIOMask(serial7, 0, 0x2)

dbLoadTemplate("templates/JenaNV40_3CLE.substitutions")

