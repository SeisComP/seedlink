* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for SunSaver MPPT

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=modbus

* Serial port 
port=tcp://$sources.mppt.address:$sources.mppt.port

* Baud rate
bps=0

* Time interval in minutes when status information is logged, 0 (default)
* means "disabled". Status channels can be used independently of this
* option.
statusinterval=60

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled).
zero_sample_limit = -1

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = -1

* Modbus base address
baseaddr = 8

* Keyword 'channel' is used to map input channels to symbolic channel
* names. Channel names are arbitrary 1..10-letter identifiers which should
* match the input names of the stream processing scheme in streams.xml,
* which is referenced from seedlink.ini

* Battery voltage
channel SA source_id=${sources.mppt.unit_id}.${sources.mppt.channels.a.sid} realscale=0.003052 realunit=V precision=2

* Array voltage
channel SB source_id=${sources.mppt.unit_id}.${sources.mppt.channels.b.sid} realscale=0.003052 realunit=V precision=2

* Load voltage
channel SC source_id=${sources.mppt.unit_id}.${sources.mppt.channels.c.sid} realscale=0.003052 realunit=V precision=2

* Charging current
channel SD source_id=${sources.mppt.unit_id}.${sources.mppt.channels.d.sid} realscale=0.002416 realunit=A precision=2

* Load current
channel SE source_id=${sources.mppt.unit_id}.${sources.mppt.channels.e.sid} realscale=0.002416 realunit=A precision=2

