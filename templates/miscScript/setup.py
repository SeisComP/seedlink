import re

"""
Plugin handler for the miscScript plugin.
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
            seedlink.param("sources.miscScript.script_path")
        except:
            seedlink.setParam("sources.miscScript.script_path", "")

        try:
            seedlink.param("sources.miscScript.script_args")
        except:
            seedlink.setParam(
                "sources.miscScript.script_args", "default"
            )  # If no argument, set 'default'

        try:
            seedlink.param("sources.miscScript.proc")
        except:
            seedlink.setParam("sources.miscScript.proc", "auto")

        try:
            seedlink.param("sources.miscScript.sample_frequency")
        except:
            seedlink.setParam("sources.miscScript.sample_frequency", "1")

        freq = seedlink.param("sources.miscScript.sample_frequency")

        if re.match("[0-9]+$", freq) != None:
            seedlink.setParam("sources.miscScript.sample_period", str(1.0 / int(freq)))
        else:
            res = re.match("([0-9]+)/([0-9]+)$", freq)
            if res != None:
                seedlink.setParam(
                    "sources.miscScript.sample_period",
                    str(float(res.group(2)) / float(res.group(1))),
                )
            else:
                print("Sample frequency invalid !!!")
                raise Exception

        try:
            seedlink.param("sources.miscScript.channels")
        except:
            seedlink.setParam("sources.miscScript.channels", "HHZ,HHN,HHE")

        splitted_chans = seedlink.param("sources.miscScript.channels").split(",")
        seedlink.setParam("sources.miscScript.channelsNumber", len(splitted_chans))

        try:
            seedlink.param("sources.miscScript.flush_period")
        except:
            seedlink.setParam("sources.miscScript.flush_period", "0")

        ##### Auto-generate proc conf and channel/source_id mapping

        if seedlink.param("sources.miscScript.proc") == "auto":
            seedlink.setParam("sources.miscScript.proc", "auto:miscScript_%s" % key)
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
                    seedlink.param("sources.miscScript.sample_frequency"),
                )
                trees += """      <node stream="{}"/>\n""".format(stream_val)
                trees += "    </tree>\n"

                channels += "channel {} source_id={}\n".format(idx, idx)

                idx += 1

            seedlink.setParam("sources.miscScript.trees", trees)
            seedlink.setParam("sources.miscScript.channels", channels)

        return key

    # Flush does nothing
    def flush(self, seedlink):
        pass
