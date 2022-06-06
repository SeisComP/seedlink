* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/gdrt_plugin --udpport $sources.gdrt.udpport --stations-from $sources.gdrt.stationsFrom"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

