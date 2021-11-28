import os

'''
Plugin handler for the Vaisala plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.vaisala.comport')
    except: seedlink.setParam('sources.vaisala.comport', '/dev/weatherstation')

    try: seedlink.param('sources.vaisala.baudrate')
    except: seedlink.setParam('sources.vaisala.baudrate', 19200)

    try: seedlink.param('sources.vaisala.proc')
    except: seedlink.setParam('sources.vaisala.proc', 'vaisala')

    return seedlink.param('sources.vaisala.comport')


  # Flush does nothing
  def flush(self, seedlink):
    pass
