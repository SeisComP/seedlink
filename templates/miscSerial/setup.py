import re

"""
Plugin handler for the miscSerial plugin.
"""
class SeedlinkPluginHandler:
    # Create defaults
    def __init__(self):
        self.instances = {}

    def push(self, seedlink):
        sta = seedlink.param("seedlink.station.id")

        try:
            key = sta + "." + str(self.instances[sta])
            self.instances[sta] += 1
        except KeyError:
            key = sta + ".0"
            self.instances[sta] = 1

        # Check and set defaults
        try:
            seedlink.param("sources.miscSerial.comport")
        except:
            seedlink.setParam("sources.miscSerial.comport", "/dev/data")

        try:
            seedlink.param("sources.miscSerial.baudrate")
        except:
            seedlink.setParam("sources.miscSerial.baudrate", 9600)

        try:
            seedlink.param("sources.miscSerial.proc")
        except:
            seedlink.setParam("sources.miscSerial.proc", "auto")

        try:
            seedlink.param("sources.miscSerial.sample_frequency")
        except:
            seedlink.setParam("sources.miscSerial.sample_frequency", "1")

        freq = seedlink.param("sources.miscSerial.sample_frequency")

        if re.match("[0-9]+$", freq) != None:
            seedlink.setParam("sources.miscSerial.sample_period", str(1.0 / int(freq)))
        else:
            res = re.match("([0-9]+)/([0-9]+)$", freq)
            if res != None:
                seedlink.setParam(
                    "sources.miscSerial.sample_period",
                    str(float(res.group(2)) / float(res.group(1))),
                )
            else:
                print("Sample frequency invalid !!!")
                raise Exception

        try:
            seedlink.param("sources.miscSerial.channels")
        except:
            seedlink.setParam("sources.miscSerial.channels", "HHZ,HHN,HHE")

        splitted_chans = seedlink.param("sources.miscSerial.channels").split(",")
        seedlink.setParam("sources.miscSerial.channelsNumber", len(splitted_chans))

        try:
            seedlink.param("sources.miscSerial.flush_period")
        except:
            seedlink.setParam("sources.miscSerial.flush_period", "0")

        try:
            seedlink.param("sources.miscSerial.serial_clock_period")
        except:
            seedlink.setParam("sources.miscSerial.serial_clock_period", "0")

        ##### Auto-generate proc conf and channel/source_id mapping

        if seedlink.param("sources.miscSerial.proc") == "auto":
            seedlink.setParam("sources.miscSerial.proc", "auto:miscSerial_%s" % key)
            trees = ""
            channels = ""
            idx = 0

            for chan in splitted_chans:
                chan = chan.strip()

                if chan == "none":
                    idx += 1
                    continue

                elif len(chan) == 3:
                    location_val = "00"
                    stream_val = chan[0:2]
                    channel_val = chan[2]

                elif len(chan) == 5:
                    location_val = chan[0:2]
                    stream_val = chan[2:4]
                    channel_val = chan[4]

                else:
                    print("Invalid channel name")
                    raise Exception

                trees += "    <tree>\n"
                trees += """      <input name="{}" channel="{}" location="{}" rate="{}"/>\n""".format(
                    idx,
                    channel_val,
                    location_val,
                    seedlink.param("sources.miscSerial.sample_frequency"),
                )
                trees += """      <node stream="{}"/>\n""".format(stream_val)
                trees += "    </tree>\n"

                channels += "channel {} source_id={}\n".format(idx, idx)

                idx += 1

            seedlink.setParam("sources.miscSerial.trees", trees)
            seedlink.setParam("sources.miscSerial.channels", channels)

        return key

    # Flush does nothing
    def flush(self, seedlink):
        pass
