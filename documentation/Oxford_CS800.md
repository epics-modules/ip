# Oxford CryoStream 800 cooler

**Table of Contents**

- [Oxford CryoStream 800 cooler](#oxford-cryostream-800-cooler)
  - [authors](#authors)
  - [files](#files)
  - [vendor](#vendor)
  - [IOC configuration](#ioc-configuration)
    - [IOC startup directory](#ioc-startup-directory)
      - [`st.cmd`](#stcmd)
      - [`auto_settings.req`](#autosettingsreq)
    - [MEDM display](#medm-display)

## authors

* Pete Jemian
* David Vine
* James O'Hea (for the cs700 support that precedes this)

## files

file | description
---- | -----
documentation/Oxford_CS800.md | this file
iocsh/Oxford_CS800_status.iocsh | IOC startup commands for status server
iocsh/Oxford_CS800.iocsh | IOC startup commands for each controller
ipApp/Db/Oxford_CS800_settings.req | autosave request file
ipApp/Db/Oxford_CS800_status_settings.req | autosave request file
ipApp/Db/Oxford_CS800_status.db | PV database for status server
ipApp/Db/Oxford_CS800.db | PV database for each controller
ipApp/Db/Oxford_CS800.proto | stream protocol
ipApp/op/adl/Oxford_CS800_admin.adl | MEDM screen
ipApp/op/adl/Oxford_CS800_hourplot.adl | MEDM screen
ipApp/op/adl/Oxford_CS800_program.adl | MEDM screen
ipApp/op/adl/Oxford_CS800_sensors.adl | MEDM screen
ipApp/op/adl/Oxford_CS800_status_full.adl | MEDM screen
ipApp/op/adl/Oxford_CS800_status.adl | MEDM screen
ipApp/op/adl/Oxford_CS800.adl | MEDM screen
ipApp/op/ui/autoconvert/Oxford_CS800_admin.ui | caQtDM screen
ipApp/op/ui/autoconvert/Oxford_CS800_hourplot.ui | caQtDM screen
ipApp/op/ui/autoconvert/Oxford_CS800_program.ui | caQtDM screen
ipApp/op/ui/autoconvert/Oxford_CS800_sensors.ui | caQtDM screen
ipApp/op/ui/autoconvert/Oxford_CS800_status_full.ui | caQtDM screen
ipApp/op/ui/autoconvert/Oxford_CS800_status.ui | caQtDM screen
ipApp/op/ui/autoconvert/Oxford_CS800.ui | caQtDM screen
ipApp/op/ui/Oxford_CS800_hourplot.ui | caQtDM screen
ipApp/op/ui/Oxford_CS800.ui | caQtDM screen
ipApp/src/Oxford_CS800.st | sequence program for each controller


## vendor

* product: https://www.oxcryo.com/product/cryostream-800
* brochure: https://www.oxcryo.com/sites/default/files/2018-08/cryostream_800_brochure_af.pdf
* manual: https://www.sfu.ca/~mpaetzel/manuals/Cryo/Cryostream_800_series.pdf

## IOC configuration

Each CS800 controller broadcasts its status data by UDP to network port 30304.
One piece of information in that data packet is the unique controller number 
of the CS800 as set at the factory.  All broadcasts are collected by a single
asyn record, configured from the `Oxford_CS800_status.db` database.  Because 
`asyn` will bind to these sockets during the IOC startup, they are
not available for any other applications to use on the workstation running 
the IOC.  

The status of a controller is selected from the common status asyn record 
input.  (This is provided by an EPICS sequence program.)  Commands are sent 
to the controller's IP address at port 30305 by UDP.  (This is provided by 
an EPICS database and stream protocol.)  Since this UDP port cannot
be opened by multiple processes on a single workstation, only one IOC on 
a workstation can run the controller databases.

NOTE:  A [request](https://github.com/epics-modules/asyn/issues/108) has 
been made to the *asyn* repository to support the 
[`SO_REUSEPORT`](https://lwn.net/Articles/542629/) socket option
that allows multiple controllers to be run from the same IOC (and host).  If
you received a message that `UDP&` is not supported when starting the IOC,
you need the patch which provides the `SO_REUSEPORT` socket option.

SUGGESTION: Run the common status database and all controller databases 
from the same IOC.

### IOC startup directory

Instructions for EPICS base release 3.15 or later.

For the common status broadcasts, these macros apply:

MACRO         | EXAMPLE         | DEFINITION
------------- | --------------- | ----------------------
PREFIX        | xxx:            | IOC prefix
R             | CS:             | CS800 shared asyn status database prefix
STATUS_IP     | 192.168.0.176   | IP number of this IOC


For each CS800 controller, these macros apply:

MACRO         | EXAMPLE               | DEFINITION
------------- | --------------------- | ----------------------
PREFIX        | xxx:                  | IOC prefix
R             | CS0:                  | CS800 controller database prefix
CID           | 411                   | Controller Number (use 0 if unknown and only 1 CS800)
COMMAND_IP    | 192.168.0.144         | IP number of CS800 controller
CS800_STATUS  | xxx:CS:status_packets | asyn PV with status packet data

Note:  If you only have one CS800 controller on the network,
you can specify CID=0 and the software will automatically find
the status data.  For two or more CS800 controllers, you must 
specify the controller number of each.

#### `st.cmd`

Add these lines to the IOC's `st.cmd` file before the call to `iocInit`:

    # assume the IOC prefix has been set
    epicsEnvSet("PREFIX", "xxx:")

    # prepare to receive status broadcasts
    # use IOC's IP number for STATUS_IP
    iocshLoad("$(IP)/iocsh/Oxford_CS800_status.iocsh", "PREFIX=$(PREFIX), R=CS:, STATUS_IP=192.168.0.176")
    # CS800_STATUS is defined in previous command

    # for each CS800 (here, we support two controllers:  CS0 & CS1)
    # CS0: #411 at 192.168.0.144
    iocshLoad("$(IP)/iocsh/Oxford_CS800.iocsh", "PREFIX=$(PREFIX), R=CS0:, CID=411, COMMAND_IP=192.168.0.144, CS800_STATUS=$(CS800_STATUS)")
    # CS1: #369 at 192.168.0.113
    iocshLoad("$(IP)/iocsh/Oxford_CS800.iocsh", "PREFIX=$(PREFIX), R=CS1:, CID=369, COMMAND_IP=192.168.0.113, CS800_STATUS=$(CS800_STATUS)")

#### `auto_settings.req`

By convention, to enable automatic discovery of *autosave* requests,
the autosave request file is in the same directory with
the PV database file.  Both files share the same filename, 
up to the final part of the name (`_settings.req` and 
`.db`, respectively).

Thus, no additional configuration is needed here.

### MEDM display

    Oxford_CS800.adl CS=xxx:CS0   (no trailing `:`)
