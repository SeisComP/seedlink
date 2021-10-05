slarchive connects to a SeedLink server, requests data streams and writes received
packets into directory/file structures (archives). The precise layout
of the directories and files is defined in a format string.

The implemented layouts are:

- :ref:`SDS <slarchive-section-sds>`: The SeisComP Data Structure, default in |scname|
- BUD: Buffer of Uniform Data structure
- DLOG: The old SeisComP/datalog structure for backwards compatibility

The duration for which the data are kept in archive is controlled by the bindings
parameter :confval:`keep`. slarchive itself does not clean the archive. For removing
old data execute :file:`$SEISCOMP_ROOT/var/lib/slarchive/purge_datafiles`. A
regular clean-up is suggested by ::

   seiscomp print crontab

The resulting line, e.g. ::

   20 3 * * * /home/sysop/seiscomp/var/lib/slarchive/purge_datafiles >/dev/null 2>&1

can be adjusted and added to crontab.


Background Execution
====================

When starting slarchive in |scname| as a daemon module in the background SDS is
considered and the packets are written without modification: ::

   $ seiscomp start slarchive


Command-Line Execution
======================

Writing to **other layouts** or to **multiple archives** and other options are
supported when executing slarchive on the command line.
E.g. to write to more than one archive simply specify multiple format definitions
(or presets).

For more command-line option read the help: ::

   $ slarchive -h


Multiple Instances
==================

slarchive allows generating aliases, e.g. for running in multiple instances with
different module and bindings configurations. For creating/removing aliases use the
:ref:`seiscomp script <sec-management-commands>`, e.g. ::

   $ seiscomp alias create slarchive2 slarchive


.. _slarchive-section-sds:

SDS definition
==============

SDS is the basic directory and file layout in |scname| for waveform archives. The
archive base directory is defined by :confval:`archive`. The SDS layout is defined
as:

.. code-block:: sh

   <SDSdir>
     + year
       + network code
         + station code
           + channel code
             + one file per day and location, e.g. NET.STA.LOC.CHAN.D.YEAR.DOY

File example: :file:`<SDSdir>/Year/NET/STA/CHAN.TYPE/NET.STA.LOC.CHAN.TYPE.YEAR.DAY`.

+-----------+-----------------------------------------------+
| Field     | Description                                   |
+===========+===============================================+
| SDSdir    | Arbitrary base directory                      |
+-----------+-----------------------------------------------+
| YEAR      | 4 digit YEAR                                  |
+-----------+-----------------------------------------------+
| NET       | Network code/identifier, 1-8 characters,      |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| STA       | Station code/identifier, 1-8 characters,      |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| CHAN      | Channel code/identifier, 1-8 characters,      |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| TYPE      | 1 character, indicating the data type,        |
|           | provided types are:                           |
|           |                                               |
|           | | **D** Waveform data                         |
|           | | **E** Detection data                        |
|           | | **L** Log data                              |
|           | | **T** Timing data                           |
|           | | **C** Calibration data                      |
|           | | **R** Response data                         |
|           | | **O** Opaque data                           |
|           |                                               |
+-----------+-----------------------------------------------+
| LOC       | Location identifier, 1-8 characters,          |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| DAY       | 3 digit day of year, padded with zeros        |
+-----------+-----------------------------------------------+
