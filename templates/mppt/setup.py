import os

'''
Plugin handler for SunSaver MPPT.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.mppt.port')
    except: seedlink.setParam('sources.mppt.port', 502)

    try: seedlink.param('sources.mppt.proc')
    except: seedlink.setParam('sources.mppt.proc', 'mppt')

    try:
      mppt_chan = dict(zip(seedlink.param('sources.mppt.channels').lower().split(','), range(26)))

    except:
      mppt_chan = dict()
      
    for letter in range(ord('a'), ord('z') + 1):
      try: seedlink.param('sources.mppt.channels.%s.sid' % chr(letter))
      except: seedlink.setParam('sources.mppt.channels.%s.sid' % chr(letter), mppt_chan.get(chr(letter), 256))

    return seedlink.net + "." + seedlink.sta

  # Flush does nothing
  def flush(self, seedlink):
    pass
