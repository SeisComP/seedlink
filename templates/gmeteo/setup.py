import os

'''
Plugin handler for GFZ meteo.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.gmeteo.comport')
    except: seedlink.setParam('sources.gmeteo.comport', '/dev/meteo')

    try: seedlink.param('sources.gmeteo.baudrate')
    except: seedlink.setParam('sources.gmeteo.baudrate', 19200)

    try: seedlink.param('sources.gmeteo.proc')
    except: seedlink.setParam('sources.gmeteo.proc', 'gmeteo')

    return seedlink.param('sources.gmeteo.comport')


  # Flush does nothing
  def flush(self, seedlink):
    pass
