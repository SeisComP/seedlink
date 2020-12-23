import os

'''
Plugin handler for the Antelope ORB plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Default host is localhost
    try: seedlink.param('sources.antelope.address')
    except: seedlink.setParam('sources.antelope.address', 'localhost')

    # Default port is 39136
    try: seedlink.param('sources.antelope.port')
    except: seedlink.setParam('sources.antelope.port', 39136)

    # Select defaults to '*'
    try: seedlink.param('sources.antelope.select')
    except: seedlink.setParam('sources.antelope.select', '*')

    # key is station (one instance per station)
    return seedlink.net + "." + seedlink.sta

  # Flush does nothing
  def flush(self, seedlink):
    pass
