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

* David Vine
* James O'Hea (for the cs700 support that precedes this)
* Pete Jemian

## files

file | description
---- | -----
documentation/Oxford_CS800.md | this file
iocsh/Oxford_CS800.iocsh | startup commands
ipApp/Db/Oxford_CS800.db | PV database
ipApp/Db/Oxford_CS800.proto | streams protocol
ipApp/Db/Oxford_CS800_settings.req | autosave request file
ipApp/op/adl/Oxford_CS800.adl | MEDM screen

## vendor

* product: https://www.oxcryo.com/product/cryostream-800
* brochure: https://www.oxcryo.com/sites/default/files/2018-08/cryostream_800_brochure_af.pdf
* manual: https://www.sfu.ca/~mpaetzel/manuals/Cryo/Cryostream_800_series.pdf

## IOC configuration

### IOC startup directory

Instructions for EPICS base release 3.15 or later.

#### `st.cmd`

Add these lines to the IOC's `st.cmd` file before the call to `iocInit`:

    # assume the IOC prefix has been set
    epicsEnvSet("PREFIX", "xxx:")

    # assume that IP stream protocols will be found
    epicsEnvSet("STREAM_PROTOCOL_PATH", "$(IP)/ipApp/Db")

    # Oxford CryoStream 800 cooler
    epicsEnvSet("CS800_IP_ADDR", "10.0.0.173")
    epicsEnvSet("STATUS_ADDR", "$(CS800_IP_ADDR):30304:30304 UDP")
    epicsEnvSet("COMMAND_ADDR", "$(CS800_IP_ADDR):30305 UDP")
    iocshLoad("$(IP)ipApp/Db/Oxford_CS800.iocsh", "PREFIX=$(PREFIX),INSTANCE=CS")

#### `auto_settings.req`

By convention to enable automatic discovery of *autosave* requests,
the autosave request file is in the same directory with
the PV database file.  Both files share the same filename, 
up to the final part of the name (`_settings.req` and 
`.db`, respectively).

Thus, no additional configuration is needed here.

### MEDM display

    Oxford_CS800.adl CS=xxx:CS
