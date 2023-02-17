import os

'''
Plugin handler for SunSaver MPPT.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    address = 'localhost'
    try: address = seedlink.param('sources.mppt.address')
    except: seedlink.setParam('sources.mppt.address', address)

    port = 502
    try: port = int(seedlink.param('sources.mppt.port'))
    except: seedlink.setParam('sources.mppt.port', port)

    try: int(seedlink.param('sources.mppt.unit_id'))
    except: seedlink.setParam('sources.mppt.unit_id', 1)

    try: seedlink.param('sources.mppt.proc')
    except: seedlink.setParam('sources.mppt.proc', 'mppt')

    try:
      mppt_chan = dict(zip(seedlink.param('sources.mppt.channels').lower().split(','), range(26)))

    except:
      mppt_chan = dict()
      
    for letter in range(ord('a'), ord('z') + 1):
      try: seedlink.param('sources.mppt.channels.%s.sid' % chr(letter))
      except: seedlink.setParam('sources.mppt.channels.%s.sid' % chr(letter), mppt_chan.get(chr(letter), 256))

    return address + ':' + str(port)

  # Flush does nothing
  def flush(self, seedlink):
    pass
