* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for maRam Weatherstation V1

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=maram

* Serial port
port=$sources.maram.comport

* Baud rate
bps=$sources.maram.baudrate

* Time interval in minutes when weather information is logged, 0 (default)
* means "disabled". Weather channels can be used independently of this
* option.
statusinterval=60

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled).
zero_sample_limit = -1

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = -1

* Keyword 'channel' is used to map input channels to symbolic channel
* names. Channel names are arbitrary 1..10-letter identifiers which should
* match the input names of the stream processing scheme in streams.xml,
* which is referenced from seedlink.ini

* Temperature (C * 100)
channel KI source_id=TD scale=100

* Humidity (%RH * 10)
channel II source_id=HR scale=10

* Air Pressure (hPa * 1000)
channel DI source_id=PR scale=1000

