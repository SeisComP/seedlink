slinktool connects to a :ref:`seedlink` server and queries the server for information
or requests data using uni-station or multi-station mode and prints information
about the packets received. All received packets can optionally be dumped to a
single file or saved in custom directory and file layouts.


Examples
========


All-station/Uni-station mode
----------------------------

The following would connect to a SeedLink server at slink.host.com port 18000 and
configure the link in all-station/uni-station mode, exactly which data are received
depends on the data being served by the SeedLink server on that particular port.
Additionally, all of the received packets are appended to the file 'data.mseed'
and each packet received is reported on the standard output. ::

   slinktool -v -o data.mseed slink.host.com:18000

The '-s' argument could be used to indicate selectors to limit the type of packets
sent by the SeedLink server (without selectors all packet types are sent). The
following would limit this connection to BHZ channel waveform data with a location
code of 10 (see an explanation of SeedLink selectors below). Additionally another
verbose flag is given, causing slinktool to report detailed header information
from data records. ::

   slinktool -vv -s 10BHZ.D -o data.mseed slink.host.com:18000


Multi-station mode
------------------

The following example would connect to a SeedLink server on localhost port 18010
and configure the link in multi-station mode. Each station specified with the '-S'
argument will be requested, optionally specifying selectors for each station. ::

   slinktool -v -S GE\_WLF,MN\_AQU:00???,IU\_KONO:BHZ.D :18010

This would request all data from the GEOFON station WLF as no selectors were indicated,
MedNet station AQU with location code 00 and all streams and waveform data from the
IU network station KONO from stream BHZ.
A variety of different data selections can be made simultaneously.

Examples:

* Horizontal BH channels, data only: ::

     -s 'BHE.D BHN.D' -S 'GE\_STU,GE\_MALT,GE\_WLF'

* Vertical channels only: ::

     -s BHZ -S GE\_STU,GE\_WLF,GE\_RUE,GE\_EIL


Wildcarding network and station codes
-------------------------------------

Some SeedLink implementations support wildcarding of the network and station codes.
If this is the case, the only two wildcard characters recognized are '\*' for
one or more characters and '?' for any single character.

As an example, all US network data can be requested using the following syntax ::

   -S 'US\_\*'


Seedlink Selectors
==================

SeedLink selectors are used to request specific types of data within a given data
stream, in effect limiting the default action of sending all data types.
A data packet is sent to the client if it matches any positive selector
(without leading "!") and doesn't match any negative selectors (with a leading "!").
The general format of selectors is LLSSS.T, where LL is location, SSS is channel
and T is type (one of [DECOTL] for Data, Event, Calibration, Blockette, Timing,
and Log records). "LL", ".T", and "LLSSS." can be omitted, implying anything in
that field. It is also possible to use "?" in place of L and S as a single character
wildcard. Multiple selectors are separated by space(s).

Examples: ::

   BH?          - BHZ, BHN, BHE (all record types)
   00BH?.D      - BHZ, BHN, BHE with location code '00' (data records)
   BH? !E       - BHZ, BHN, BHE (excluding detection records)
   BH? E        - BHZ, BHN, BHE & detection records of all channels
   !LCQ !LEP    - exclude LCQ and LEP channels
   !L !T        - exclude log and timing records


Archiving Data
==============

Using the '-A format' option received data can be saved in a custom directory and
file structure. The archive format argument is expanded for each packet processed
using the following flags: ::

   n : network code, white space removed
   s : station code, white space removed
   l : location code, white space removed
   c : channel code, white space removed
   Y : year, 4 digits
   y : year, 2 digits zero padded
   j : day of year, 3 digits zero padded
   H : hour, 2 digits zero padded
   M : minute, 2 digits zero padded
   S : second, 2 digits zero padded
   F : fractional seconds, 4 digits zero padded
   % : the percent (%) character
   # : the number (#) character
   t : single character type code:
         D - waveform data packet
         E - detection packet
         C - calibration packet
         T - timing packet
         L - log packet
         O - opaque data packet
         U - unknown/general packet
         I - INFO packet
         ? - unidentifiable packet

The flags are prefaced with either the % or # modifier. The % modifier indicates
a defining flag while the # indicates a non-defining flag. All received packets
with the same set of defining flags will be saved to the same file. Non-defining
flags will be expanded using the values in the first packet received for the
resulting file name.

Time flags are based on the start time of the given packet.

For example, the format string: ::

   /archive/%n/%s/%n.%s.%l.%c.%Y.%j

would be expanded to day length files named something like: ::

   /archive/NL/HGN/NL.HGN..BHE.2003.055

Using non-defining flags the format string: ::

   /data/%n.%s.%Y.%j.%H:#M:#S.miniseed

would be expanded to: ::

   /data/NL.HGN.2003.044.14:17:54.miniseed

resulting in hour length files because the minute and second are specified with the non-defining modifier. The minute and second fields are from the first packet in the file.


Stream List File
=================

The stream list file used with the '-l' option is expected to define a data stream
on each line. The format of each line is: ::

   Network Station [selectors]

The selectors are optional. If default selectors are also specified (with the '-s' option),
they they will be used when no selectors are specified for a given stream.

Example: ::

   ----  Begin example file -----
   # Comment lines begin with a '#' or '\*'
   # Example stream list file for use with the -l argument of slclient or
   # with the sl\_read\_streamlist() libslink function.
   GE ISP  BH?.D
   NL HGN
   MN AQU  BH? HH?
   ----  End example file -----


.. note::

   All diagnostic output from slinktool is printed to standard error (stderr).
   Exceptions are when

   * Printing miniSEED packet details with the *-p* option.
   * Printing unpacked samples with the *-u* option.
   * Printing the raw or formatted responses to INFO requests.


Author of slinktool
===================

Chad Trabant

ORFEUS Data Center/EC-Project MEREDIAN

IRIS Data Management Center

Original source code: https://github.com/iris-edu/slinktool/tree/master/doc
