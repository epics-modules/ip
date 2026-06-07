---
layout: default
title: Home
nav_order: 1
---

# ip
{: .no_toc}

## Table of contents
{: .no_toc .text-delta}

- TOC
{:toc}

## Overview

The ip module provides EPICS support for message-based instruments
that communicate via serial (RS-232), GPIB, or socket (TCP/IP, UDP)
interfaces. It is part of the
[synApps](https://www.aps.anl.gov/BCDA/synApps) collection of EPICS
modules maintained by BCDA at Argonne National Laboratory.

The module supports a wide range of devices including temperature
controllers, vacuum gauge controllers, power supplies, digital
multimeters, lock-in amplifiers, piezo controllers, encoders, syringe
pumps, and other laboratory instruments. See the
[Supported Devices](ipDoc.md) page for the full list.

The name "ip" is a legacy from the use of Industry Pack cards in a VME
environment to provide serial or GPIB communication channels.

## Device support styles

Devices in this module use one of two approaches:

- **Compiled device support** -- Custom C source code or SNL (State
  Notation Language) programs that implement the device communication
  protocol directly using [asyn](https://github.com/epics-modules/asyn)
  interfaces.

- **StreamDevice protocols** -- Protocol files (`.proto`) used with
  [StreamDevice](https://github.com/paulscherrerinstitute/StreamDevice)
  to define command formatting and reply parsing in a configuration
  file rather than compiled code.

Additionally, the [deviceCmdReply](deviceCmdReply.md) database provides
a way to communicate with serial/GPIB/socket devices at run time
without writing any device support code.

## Dependencies

- [asyn](https://github.com/epics-modules/asyn) -- Serial, IP, and
  GPIB communication
- [seq](https://github.com/epics-modules/sequencer) -- State Notation
  Language runtime (for SNL-based device support)
- [ipac](https://github.com/epics-modules/ipac) -- Industry Pack
  carrier support (for vxWorks with IP serial/GPIB modules)

## Building

### DBD files

Include `ipSupport.dbd` in the IOC's DBD file to get the compiled
device support registrations. For vxWorks targets, also include
`ipVXSupport.dbd` for additional GPIB device support.

### Library

Link the IOC against the `ip` library.

### StreamDevice databases

Databases that use StreamDevice protocols require that the `.proto`
file be accessible to the IOC at runtime. Set the
`STREAM_PROTOCOL_PATH` environment variable to include the directory
containing the protocol files (typically `$(IP)/db`).
