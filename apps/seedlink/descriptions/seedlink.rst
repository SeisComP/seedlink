SeedLink serves miniSEED data to clients in real times. It is a real-time data
acquisition protocol and a client-server software that implements this protocol.
The SeedLink protocol is based on TCP. All
connections are initiated by the client. During handshaking phase the client can
subscribe to specific stations and streams using simple commands in ASCII coding.
When handshaking is completed, a stream of SeedLink "packets" consisting of a
8-byte SeedLink header (containing the sequence number) followed by a 512-byte
miniSEED record, is sent to the client. The packets of each individual station
are always transferred in timely (FIFO) order. The SeedLink implementation used
in SeisComP is the oldest and most widely used, however, other implementations
exist. Another well-known implementation is deployed in IRIS DMC and some
manufacturers have implemented SeedLink in their digitizer firmware. All
implementations are generally compatible, but not all of them support the full
SeedLink protocol. On the other hand IRIS DMC implements some extensions which
are not supported by other servers. In the following we use "SeedLink" to denote
the SeedLink implementation used in SeisComP.

The data source of a SeedLink server can be anything which is supported by a
:ref:`SeedLink plugin <seedlink-sources>` - a small program
that sends data to the SeedLink server. Plugins are controlled by the SeedLink
server, e.g., a plugin is automatically restarted if it crashes or a timeout
occurs. Data supplied by a plugin can be a form of miniSEED packets or just
raw integer samples with accompanying timing information. In the latter case,
the SeedLink server uses an integrated "Stream Processor" to create the desired
data streams and assemble miniSEED packets.

.. note::

   SeedLink supports by default miniSEED records with a size of 512 bytes which
   is hardcoded. For serving records with other sizes, e.g., 4096 bytes,
   SeedLink must be recompiled with a modified variable *MSEED-RECLEN* located
   in :file:`seedlink/apps/seedlink/iosystem.h` of the SeedLink repository.


.. _seedlink-sources:

Supported data sources
----------------------

The table below lists digitizers and data acquisition systems and the SeedLink
plugins supporting these system. More plugins (Kinemetrics K2, Lennartz MARS-88,
Lennartz PCM 5800, etc.) have been implemented by various users, but are not
(yet) included in the package. The included C language plugin interface is
described in section 5.1.1.5. Antelope, Earthworm and NAQS can also import data
from SeisComP. In SeisComP the class :ref:`RecordStream <global_recordstream>`
is implemented that supports both
SeedLink and ArcLink sources; this class is used by all SeisComP modules that
work with waveform data. On a lower level, SeedLink clients can be implemented
using the :cite:t:`libslink` software library or its Java counterpart,
JSeedLink. Libslink supports Linux/UNIX, Windows and MacOS X platforms, and
comes with an exhaustive documentation in form of UNIX manual pages.

.. csv-table::
   :widths: 2 3 5
   :header: Plugin name, Source or Digitizer/DAS, Plugin Implementer
   :align: left

   :ref:`seedlink-sources-antelope-label` * ,  Antelope, Chad Trabant (IRIS)
   :ref:`seedlink-sources-caps-label`       ,  CAPS server :cite:p:`caps` , `gempa GmbH <https://www.gempa.de>`_
   :ref:`seedlink-sources-chain-label`      ,  SeedLink, GFZ
   dm24 **                                  ,  Guralp DM24, GFZ; based on libgcf2 from Guralp
   :ref:`seedlink-sources-dr24-label`       ,  Geotech DR24, GFZ
   :ref:`seedlink-sources-echopro_3ch100hz-label` /  :ref:`seedlink-sources-echopro_6ch200hz-label`, Kelunji Echo/EchoPro, Oyvind Natvik (UiB)
   :ref:`seedlink-sources-edata-label`      ,  Earth Data PS2400/PS6-24, GFZ
   :ref:`seedlink-sources-ewexport-label`   ,  Earthworm export server (TCP/IP), Chad Trabant (IRIS)
   :ref:`seedlink-sources-ewexport_pasv-label`,  Earthworm passive export server (TCP/IP), Chad Trabant (IRIS)
   :ref:`seedlink-sources-fs_mseed-label`   ,  miniSEED file plugin, GFZ
   :ref:`seedlink-sources-gdrt-label`       ,  GDRT server, GFZ
   :ref:`seedlink-sources-gmeteo-label`     ,  GFZ meteo protocol, GFZ
   :ref:`seedlink-sources-hrd24-label`      ,  Nanometrics HRD24, GFZ; Recai Yalgin
   :ref:`seedlink-sources-liss-label`       ,  LISS, Chad Trabant (IRIS)
   :ref:`seedlink-sources-m24-label` *      ,  Lennartz M24, Lennartz Electronic GmbH
   :ref:`seedlink-sources-maram-label`      ,  maRam Weatherstation V1, GFZ
   :ref:`seedlink-sources-minilogger-label` ,  SEP064 USB Seismometer Interface, GFZ; Anthony Lomax
   :ref:`seedlink-sources-mk6-label` *      ,  MK6, Jan Wiszniowski (IGPAS)
   :ref:`seedlink-sources-mppt-label` *     ,  SunSaver MPPT via Modbus TCP/IP, GFZ
   :ref:`seedlink-sources-mseedfifo-label`  ,  Generic, GFZ
   :ref:`seedlink-sources-mseedscan-label`  ,  Transfers miniSEED files from a directory, Chad Trabant (IRIS)
   :ref:`seedlink-sources-mws-label`        ,  Reinhardt MWS5/MWS9 Weather Station, GFZ
   :ref:`seedlink-sources-naqs-label`       ,  NAQS, "Chad Trabant (IRIS); based on sample code from Nanometrics, Inc."
   :ref:`seedlink-sources-nmxp-label` *     ,  NAQS, Matteo Quintiliani (INGV)
   nrts **    ,  NRTS, GFZ; based on ISI toolkit from David E. Chavez
   :ref:`seedlink-sources-optodas-label`    ,  OptoDAS interrogator via ZeroMQ, GFZ
   :ref:`seedlink-sources-ps2400_eth-label` ,  Earth Data PS2400/PS6 Ethernet, GFZ; `gempa GmbH <https://www.gempa.de>`_
   :ref:`seedlink-sources-q330-label`       ,  Quanterra Q330, "GFZ; based on lib330 maintained by ISTI, Inc."
   comserv ** ,  "Quanterra Q380/Q680, Q4120, Q720", "GFZ; based on Comserv by Quanterra, Inc."
   :ref:`seedlink-sources-reftek-label`     ,  RefTek RTPD, "GFZ; based on software library provided by RefTek, Inc."
   :ref:`seedlink-sources-sadc-label`       ,  SARA SADC10/18/20/30, GFZ
   :ref:`seedlink-sources-scream-label`     ,  SCREAM, Reinoud Sleeman (KNMI)
   :ref:`seedlink-sources-scream_ring-label`,  SCREAM, "Reinoud Sleeman (KNMI), This is the second revision of the scream plugin which supports buffering for short-term completeness."
   :ref:`seedlink-sources-vaisala-label`    ,  Vaisala ASCII protocol (serial plugin), GFZ
   :ref:`seedlink-sources-wago-label`       ,  WAGO MODBUS/TCP devices, GFZ
   :ref:`seedlink-sources-wave24-label` *   ,  Wave24, MicroStep-MIS
   :ref:`seedlink-sources-win-label`        ,  WIN, GFZ; based on source code of WIN system
   ws2300 **  ,  Lacrosse 2300 Weather Station, GFZ; based on open2300 library from Kenneth Lavrsen

\* Third-party plugin, not included in SeisComP distribution

\*\* No longer supported


Telnet interface
----------------

:program:`seedlink` provides a telnet interface accepting the commands set out in
:ref:`seedlink_commands` through the seedlink :confval:`port` ::

   telnet [host] [port]

Example fetching the SeedLink version: ::

   $ telnet localhost 18000
   Trying 127.0.0.1...
   Connected to localhost.gempa.de.
   Escape character is '^]'.
   hello
   SeedLink v3.3 (2020.122)
   bye
   Connection closed by foreign host.


Queries
-------

:program:`seedlink` provides a query interface.
Use :program:`slinktool` to send queries
for fetching:

* Station and stream information
* Waveform data


Protocol
========

A SeedLink session starts with opening the TCP/IP connection and ends with
closing the TCP/IP connection. During the session the following steps are
performed in order:

* Opening the connection
* Handshaking
* Transferring SeedLink packets

We will take a closer look at the protocol. Note, the details are normally
hidden from the clients by the libslink software library; therefore it is not
necessary to be familiar with the protocol in order to implement clients.


Handshaking
-----------

When the TCP/IP connection has been established the server will wait for the
client to start handshaking without initially sending any data to the client.
During handshaking the client sends SeedLink commands to the server. The
commands are used to set the connection into a particular mode, setup stream
selectors, request a packet sequence number to start with and eventually start
data transmission. SeedLink commands consist of an ASCII string followed by
zero or several arguments separated by spaces and terminated with carriage
return (<cr>, ASCII code 13) followed by an optional linefeed
(<lf>, ASCII code 10). The commands can be divided into two categories: "action
commands" and "modifier commands". Action commands perform a function such as
starting data transfer. Modifier commands are used to specialize or modify the
function performed by the action commands that follow. When a server receives a
modifier command it responds with the ASCII string "OK" followed by a carriage
return and a line feed to acknowledge that the command has been accepted. If
the command was not recognized by the server or has invalid parameters, then
the ASCII string "ERROR" is sent as a response to the client followed by a
carriage return and a line feed. The client should not send any further
commands before it has received a response to the previous modifier command. If
a network error or timeout occurs the client should close the connection and
start a new session. Data transmission is started when the server receives the
commands DATA, FETCH, TIME or END as described in section 5.1.1.3. Once the data
transfer has been started no more commands, except INFO, should be sent to the
server. The flow diagram of handshaking in uni-station vs. multi-station mode
is shown in :ref:`seedlink-handshaking`.

.. _seedlink-handshaking:

.. figure::  media/seedlink/Handshaking_uni_multi_station_mode.*
   :width: 10cm

   Handshaking in uni-station vs. multi-station mode.


Data Transfer
-------------

When handshaking has been completed the server starts sending data packets, each
consisting of an 8-byte SeedLink header followed by a 512-byte miniSEED record.
The SeedLink header is an ASCII string consisting of the letters "SL" followed
by a six-digit hexadecimal packet sequence number. Each station has its own
sequence numbers. If multiple stations are requested using a single TCP channel
the client should look at the contents of the miniSEED header to determine the
station name (or to maintain the current sequence numbers for each station). A
sequence number in the same format is used as an argument to the commands "DATA"
or "FETCH" to start the data transfer from a particular packet. Each SeedLink
node re-assigns sequence numbers for technical reasons. It is not possible to
use the same sequence numbers when communicating with alternative servers.
Within a particular node the sequence numbers of a single station are
consecutive and wrap around at FFFFFF. This can be used by the client to detect
"sequence gaps" (e.g., some data has been missed by the client due to long
network outage or a software bug). However, if stream selectors are used the
sequence numbers are only guaranteed to be in increasing order (with wrap)
because some packets might be filtered out by the server. In this case the
first packet is not necessarily the one requested, but the nearest packet (not
older than requested) that matches installed selectors.
The data is transferred as a continuous stream without any error detections or
flow control because these functions are performed by the TCP protocol. This
guarantees the highest data transfer rate that is possible with the particular
hardware and TCP/IP implementation.
Obviously, the average data transfer rate must be greater than the rate at
which new data becomes ready to send at the server. If this is the case, sooner
or later the server has sent all data available to the client. When this
happens, depending on the SeedLink mode, the server sends new data as soon as
it arrives or appends ASCII string "END" to the last packet and waits for the
client to close connection. The latter mode is called "dial-up mode" because
it is normally used in conjunction with dial-up lines to open the connection
periodically for a short time and download all data available. A SeedLink
packet can never start with "END" thus no ambiguity arises.


.. _seedlink_commands:

Commands
--------

HELLO
    responds with a two-line message (both lines terminated with <cr><lf>). The first line contains the version number of the SeedLink daemon, the second  line contains station or data center description specified in the configuration. HELLO is used mainly for testing a SeedLink server with "telnet". It is also used by libslink to determine the server version.

CAT
    shows the station list. Used mainly for testing a SeedLink server with "telnet".

BYE
    closes the connection. Used mainly for testing a SeedLink server with "telnet".

STATION station code [network code]
    turns on multi-station mode, used to transfer data of multiple stations over a single TCP channel. The STATION command, followed by SELECT (optional) and FETCH, DATA or TIME commands is repeated for each station and the handshaking is finished with END. STATION is a modifier command (it modifies the function of subsequent SELECT, and DATA, FETCH or TIME commands) so it responds with "OK" on success, "ERROR" otherwise.

END
    end of handshaking in multi-station mode. This is an action command, because it starts data transfer. No explicit response is sent.

SELECT [pattern]
    when used without pattern, all selectors are canceled. Otherwise, the pattern is a positive selector to enable matching miniSEED stream transfer. The pattern can be used as well as a negative selector with a leading "!" to prevent the transfer of some miniSEED streams. Only one selector can be used in a single SELECT request. A SeedLink packet is sent to the client if it matches any positive selector and doesn’t match any negative selectors.

General format of selectors is LLCCC.T where LL is location, CCC is channel, and T is type (one of DECOTL for data, event, calibration, blockette, timing, and log records). "LL", ".T", and "LLCCC." can be omitted, meaning "any". It is also possible to use "?" in place of L and C. Some examples can be found in table 3-1 in section 3.3.3.2.
SELECT is a modifier command (it modifies the function of subsequent DATA, FETCH or TIME commands) so a response follows with "OK" on success, "ERROR" otherwise.

DATA [n [begin time]]
    in multi-station mode this sets the current station into real-time mode and (optionally) the current sequence number to n; in uni-station mode this starts data transfer in real-time mode from packet n or from the next packet available if used without arguments. If begin time is used, any older packets are filtered out. begin time should be in the form of 6 decimal numbers separated by commas in the form: year,month,day,hour,minute,second, e.g. ’2002,08,05,14,00,00’. DATA is a modifier command in multi-station mode (responds with "OK" or "ERROR"); in uni-station mode it is an action command (no explicit response is sent).

FETCH [n [begin time]]
    works like DATA but sets the station to dial-up mode instead of real-time mode.

TIME [begin time [end time]]
    extracts the time window from begin time to end time. The times are specified in the form of 6 decimal numbers separated by commas in the form: year,month,day,hour,minute,second, e.g. ’2002,08,05,14,00,00’.

INFO level
    requests an INFO packet containing XML data embedded in a miniSEED log record. level should be one of the following: ID, CAPABILITIES, STATIONS, STREAMS, GAPS, CONNECTIONS, ALL. The XML document conforms to the Document Type Definition (DTD) shown in section ???. The amount of info available depends on the configuration of the SeedLink server.


Plugin Interface
================

In order to implement a SeedLink plugin a developer needs two files included in the SeisComP distribution: :file:`plugin.h` and :file:`plugin.c`.
In these files the following public functions are defined:

.. c:function:: int send_raw3(const char *station, const char *channel, const struct ptime *pt, int usec_correction, int timing_quality, const int32_t *dataptr, int number_of_samples)

is used to send a raw packet (array of 32-bit integer samples) to SeedLink. The parameters are:

station
    station ID, must match one of the defined stations in :file:`seedlink.ini`. (Up to 10 characters.)

channel
    channel ID, referenced by the "input" element in :file:`streams.xml`. (Up to 10 characters.)

pt
    time of the first sample in the array. If NULL then time is calculated relative to the previous send_raw3() call. struct ptime is defined in :file:`plugin.h`.

usec_correction
    time correction in microseconds to be written in the SEED data header. Can be useful if the digitizer is not phase locked to GPS.

timing_quality
    timing quality in percent (0-100). The number is directly written into miniSEED header (blockette 1001). Semantics is implementation-defined. Usually 100 means that GPS is in lock and 0 means there never was a GPS lock, so the timing is completely unreliable. When GPS goes out of lock, the value can slowly decrease reflecting a possible time drift.

dataptr
    Array of signed 32-bit samples.

Number_of_samples
    Length of the sample array.

Special cases:

* If timing_quality = -1, blockette 1001 is omitted.
* If number_of_samples = 0 & pt = NULL set new time without sending any data.
* If dataptr = NULL send a gap (advance time as if number of samples was sent without sending any actual data).

.. c:function:: int send_raw_depoch(const char *station, const char *channel, double depoch, int usec_correction, int timing_quality, const int32_t dataptr, int number_of_samples)

same as send_raw3() except time is measured in seconds since 1 January 1970 (depoch). Leap seconds are ignored.

.. c:function:: int send_flush3(const char *station, const char *channel)

flushes all miniSEED data streams associated with a channel. All buffered data is sent out creating "unfilled" miniSEED records if necessary. The parameters are:

station
    station ID.

Channel
    channel ID.

.. c:function:: int send_mseed(const char *station, const void *dataptr, int packet_size)

is used to send a miniSEED packet to SeedLink. Such packets are not further processed. The parameters are:

station
    station ID.

dataptr
    pointer to 512-byte miniSEED packet.

packet size
    must be 512.


.. c:function:: int send_log3(const char *station, const struct ptime *pt, const char *fmt, ...)

is used to send a log message to SeedLink (LOG stream). It must be noted that encapsulating log messages in miniSEED records is relatively inefficient because each message takes at least one record (512 bytes), regardless of message size. Due to 64-byte miniSEED header, up to 448 bytes per record can be used \*  The parameters are:

station
    station ID.

pt
    the timestamp of the message.

fmt
    format string, as used by printf(), followed by a variable number of arguments.


Compatibility with Earlier Versions
===================================

It is possible to determine the version of the plugin interface by looking at the C macro PLUGIN_INTERFACE_VERSION. The current version is 3, therefore all functions that have changed since earlier versions end with "3". It is possible to enable full backward compatibility with earlier versions of the plugin interface by defining the C macro PLUGIN_COMPATIBILITY. In this case the old functions are also defined.


SeedLink configuration files
============================

The following configuration files are used by SeedLink and its plugins.

.. warning::

   Some files such as seedlink.ini, plugin.ini and chain\*.xml are generated by the :program:`seiscomp` tool according
   to the configuration in :file:`etc/seedlink.cfg` and its bindings. They live in :file:`var/lib/seedlink` and should
   not be modified. If modifications are necessary then the generator needs to be changed to better support
   the desired user options.

plugins.ini
    Configuration file for SeedLink plugins. Used by serial_plugin, fs_plugin and comserv_plugin.

seedlink.ini
    Main configuration file for SeedLink. For more details see below.

filters.fir
    Coefficients of SeedLink’s decimating FIR filters. If a filter’s name ends with "M", it is a minimum-phase filter – causal filter with minimized (non-constant) phase delay; since the filter is non-symmetric all coefficients must be given. Otherwise the filter is a zero-phase filter, i.e. a non-causal filter with zero phase delay; in this case the filter is symmetric and so only half of the coefficients must be given (it is not possible to use a zero-phase filter with an odd number of coefficients).

.. warning::

   The coefficients for non-symmetric (minimum-phase) FIR filters in the filters.fir file are stored
   in reverse order. It is important to reverse the order of coefficients if the operator adds
   a new minimum-phase filter or uses the included minimum-phase filters for another application.
   The coefficients for symmetric (zero-phase) FIR filters are not stored in reverse order. As a
   sanity check for symmetric filters the largest coefficient is always in the middle of the symmetry.

streams.xml
    SeedLink stream configuration file for the internal stream processor, referenced from seedlink.ini. For details see below.


\*.ini files have a somewhat obscure syntax. They contain zero or more sections, each beginning with a section name
in squared brackets which should appear on a line of its own. Section name cannot contain spaces and squared brackets,
but it can be optionally surrounded by spaces. Each section consists of zero or more entries – definitions and assignments.
A definition consists of a keyword and a name separated by spaces (e.g. "station EDD"). An assignment consists of a
parameter and a value separated by the "=" sign and optionally surrounded by spaces (e.g. "network = GE").

The set of assignments that immediately follow a definition is in the scope of that definition. Assignments in the
beginning of a section are "global" – they are used to set some generic parameters and provide default values
(e.g. "network = GE" in the beginning of the section sets the default network that can be overridden in the scope
of a station definition).

Parameters and keywords are case insensitive and must not contain the symbols "=", "[", "]" or spaces. Names must
not contain "=" signs or spaces. Values must not contain "=" signs or spaces, unless enclosed in double quotes.
Double quotes that are part of the value itself must be preceded by "\".

Each assignment must be complete on a single line, but several assignments can appear on one line, separated by spaces.
Any line beginning with a "#" or "*" character is regarded as a comment and ignored.


seedlink.ini
------------

seedlink.ini may contain several sections, but only one having the same name as
the executable to be used. A section in seedlink.ini has the following structure
(parameters are shown in courier, default values are shown in squared brackets,
but relying on them is not recommended):

seedlink.ini is generated from :file:`\~/.seiscomp/seedlink.cfg` and
:file:`etc/seedlink.cfg`.


streams.xml
-----------

This file, like all XML documents, has a tree-like structure. The root element
is called "stream" and it in turn contains "proc" elements which are referenced
by name in seedlink.ini. A "proc" element contains one or more "tree" elements,
which in turn contain "input" and "node" elements. There should be one "input"
element per plugin channel; if an "input" element is missing, the channel is
ignored and you will see a message like::

    Jun 24 12:56:28 st55 seedlink: EDD channel X ignored

Here is the description of all elements and attributes:

**element** streams
    root element, has no attributes.

**element** proc
    defines a "proc" object (set of "stream trees"), referenced from seedlink.ini.

**attribute** name
    name of "proc" object, for reference.

**element** using
    used to include all "stream trees" defined by one "proc" object in another "proc" object.

**attribute** name
    the name of referenced "proc" object.

**element** tree
    defines a "stream tree" – a downsampling scheme of an input channel.

**element** input
    associates an input channel with the stream tree.

**attribute** name
    name of the input channel; depends on the configuration of the particular plugin (usual channel names are "Z", "N" and "E").

**attribute** channel
    name of the output channel (last letter of a miniSEED stream name).

**attribute** location
    miniSEED location code of the output channel (up to two characters).

**attribute** rate
    sampling rate of the input channel (must match the actual sampling rate, which is dependent on the configuration of the plugin and digitizer).

**element** node
    defines a node of a stream tree; this element is recursive, meaning that it may contain one or more "node" elements itself.

**attribute** filter
    use the named filter for decimation; filters are defined in file filters.fir.

**attribute** stream
    create miniSEED output stream at this node. The value of the attribute should be a miniSEED stream name without the last character (which is taken from the attribute "channel" of element "input").

:file:`streams.xml` is generated into :file:`var/lib/seedlink/streams.xml`. Each
data plugin provides templates with predefined procs. If e.g. the *chain* plugin
is configured with proc *stream100* then :file:`share/templates/seedlink/chain/streams_stream100.tpl`
is being read and generated into the final :file:`streams.xml`. Own proc definitions
can be added by creation a corresponding template file.

Again, the **source** and **proc** definition is used to resolve the streams proc
template file at :file:`share/templates/seedlink/[source]/streams_[proc].tpl`.
