* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for miscSerial "csv" source

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=miscSerial

* Serial port 
port=$sources.miscSerial.comport

* Baud rate
bps=$sources.miscSerial.baudrate

* specific miscSerial entries
channelsNumber=$sources.miscSerial.channelsNumber
flush_period=$sources.miscSerial.flush_period
sample_period=$sources.miscSerial.sample_period
serial_clock_period=$sources.miscSerial.serial_clock_period

* lsb (defaults to 8): least significant bit (relative to 32-bit samples),
*   normally 8 for 24-bit samples, but can be set for example to 7 to get
*   25-bit samples;
* statusinterval (defaults to 0): time interval in minutes when "state of
*   health" information is logged, 0 means "disabled". State of health
*   channels can be used independently of this option.
*
* If you set 'checksum' to a wrong value then the driver will not work and
* you will get error messages like "bad SUM segment" or "bad MOD segment".
lsb=8
statusinterval=60

* Parameter 'time_offset' contains the amount of microseconds to be added
* to the time reported by the digitizer.

* 1.389 sec is possibly the correct offset if you have a version of the
* Earth Data digitizer with external GPS unit.
* time_offset=1389044

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled).
zero_sample_limit = -1

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = -1

* Timing quality to use when GPS is out of lock
unlock_tq = 10

* Keyword 'channel' is used to map input channels to symbolic channel
* names. Channel names are arbitrary 1..10-letter identifiers which should
* match the input names of the stream processing scheme in streams.xml,
* which is referenced from seedlink.ini

$sources.miscSerial.channels
