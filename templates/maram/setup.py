'''
Plugin handler for maRam Weatherstation V1.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.maram.comport')
    except: seedlink.setParam('sources.maram.comport', '/dev/meteo')

    try: seedlink.param('sources.maram.baudrate')
    except: seedlink.setParam('sources.maram.baudrate', 9600)

    try: seedlink.param('sources.maram.proc')
    except: seedlink.setParam('sources.maram.proc', 'maram')

    return seedlink.param('sources.maram.comport')


  # Flush does nothing
  def flush(self, seedlink):
    pass
