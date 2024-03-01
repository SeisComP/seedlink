"""
Plugin handler for the OptoDAS plugin.
"""
class SeedlinkPluginHandler:
    def __init__(self):
        pass

    def push(self, seedlink):
        try: seedlink.param("sources.optodas.address")
        except: seedlink.setParam("sources.optodas.address", "tcp://localhost:3333")

        try: seedlink.param("sources.optodas.sampleRate")
        except: seedlink.setParam("sources.optodas.sampleRate", "100")

        try: seedlink.param("sources.optodas.gain")
        except: seedlink.setParam("sources.optodas.gain", "1.0")

        try: seedlink.param("sources.optodas.networkCode")
        except: seedlink.setParam("sources.optodas.networkCode", "XX")

        try: seedlink.param("sources.optodas.stationCode")
        except: seedlink.setParam("sources.optodas.stationCode", "{channel:05d}")

        try: seedlink.param("sources.optodas.locationCode")
        except: seedlink.setParam("sources.optodas.locationCode", "")

        try: seedlink.param("sources.optodas.channelCode")
        except: seedlink.setParam("sources.optodas.channelCode", "HSF")

        try: seedlink.param("sources.optodas.proc")
        except: seedlink.setParam("sources.optodas.proc", "auto")

        if seedlink.param("sources.optodas.proc") == "auto":
            seedlink.setParam("sources.optodas.proc", "auto:optodas_%s_%s_%s" % (
                              seedlink.param("sources.optodas.locationCode"),
                              seedlink.param("sources.optodas.channelCode"),
                              seedlink.param("sources.optodas.sampleRate")))

        return seedlink.param("sources.optodas.address")

    def flush(self, seedlink):
        pass

