* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for Vaisala weather station

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=vaisala

* Serial port
port=$sources.vaisala.comport

* Baud rate
bps=$sources.vaisala.baudrate

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

* Outdoor temperature (C * 100)
channel KO source_id=Ta scale=100

* Outdoor humidity (%RH)
channel IO source_id=Ua scale=1

* Air pressure (hPa * 10)
channel DO source_id=Pa scale=10

* Wind direction (deg)
channel WD source_id=Dm scale=1

* Wind direction minimum (deg)
channel W1 source_id=Dn scale=1

* Wind direction maximum (deg)
channel W2 source_id=Dx scale=1

* Wind speed (m/s * 10)
channel WS source_id=Sm scale=10

* Wind speed minimum (m/s * 10)
channel W3 source_id=Sn scale=10

* Wind speed maximum (m/s * 10)
channel W4 source_id=Sx scale=10

* Rain accumulation (mm * 100)
channel RA source_id=Rc scale=100

* Rain duration (s)
channel RD source_id=Rd scale=1

* Rain intensity (mm/h * 10)
channel RI source_id=Ri scale=10

* Rain peak intensity (mm/h * 10)
channel RP source_id=Rp scale=10

* Hail accumulation (hits/cm^2 * 10)
channel HA source_id=Hc scale=10

* Hail duration (s)
channel HD source_id=Hd scale=1

* Hail intensity (hits/cm^2h * 10)
channel HI source_id=Hi scale=10

* Hail peak intensity (hits/cm^2h * 10)
channel HP source_id=Hp scale=10

* Supply voltage (V * 10)
channel EV source_id=Vs scale=10

