* template: $template
plugin $seedlink.source.id cmd = "$seedlink.plugin_dir/optodas_plugin -a '$sources.optodas.address' -r $sources.optodas.sampleRate -g $sources.optodas.gain -n '$sources.optodas.networkCode' -s '$sources.optodas.stationCode'"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.optodas.proc"

