import os

'''
Plugin handler for the GDRT plugin.
'''
class SeedlinkPluginHandler:
    def __init__(self):
        self.instances = {}
        self.stations = {}

    def push(self, seedlink):
        try: gdrtStation = seedlink.param('sources.gdrt.station')
        except: gdrtStation = seedlink.param('seedlink.station.code')

        seedlink.setParam('sources.gdrt.station', gdrtStation);

        try: locationCode = seedlink.param('sources.gdrt.locationCode')
        except: locationCode = ""

        seedlink.setParam('sources.gdrt.locationCode', locationCode);

        try: sampleRate = float(seedlink.param('sources.gdrt.sampleRate'))
        except: sampleRate = 1.0

        seedlink.setParam('sources.gdrt.sampleRate', sampleRate)

        try: udpport = int(seedlink.param('sources.gdrt.udpport'))
        except: udpport = 9999

        seedlink.setParam('sources.gdrt.udpport', udpport);

        try:
            n = self.instances[udpport]

        except KeyError:
            n = len(self.instances)
            self.instances[udpport] = n

        stationsFrom = os.path.join(seedlink.config_dir, "gdrt%d.stations" % n)
        seedlink.setParam('sources.gdrt.stationsFrom', stationsFrom)

        try:
            stationList = self.stations[stationsFrom]

        except KeyError:
            stationList = []
            self.stations[stationsFrom] = stationList

        stationList.append((gdrtStation,
                            seedlink.param('seedlink.station.network'),
                            seedlink.param('seedlink.station.code'),
                            locationCode if locationCode else "--",
                            sampleRate))

        return udpport


    def flush(self, seedlink):
        for stationsFrom, stations in self.stations.items():
            with open(stationsFrom, "w") as fd:
                for s in stations:
                    fd.write("%s %s %s %s %f\n" % s)
 
