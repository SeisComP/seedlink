  <proc name="vaisala">
    <tree>
      <input name="KO" channel="KO" location="" rate="1"/>
      <input name="IO" channel="IO" location="" rate="1"/>
      <input name="DO" channel="DO" location="" rate="1"/>
      <input name="WD" channel="WD" location="" rate="1"/>
      <input name="W1" channel="W1" location="" rate="1"/>
      <input name="W2" channel="W2" location="" rate="1"/>
      <input name="WS" channel="WS" location="" rate="1"/>
      <input name="W3" channel="W3" location="" rate="1"/>
      <input name="W4" channel="W4" location="" rate="1"/>
      <input name="RA" channel="RA" location="" rate="1"/>
      <input name="RD" channel="RD" location="" rate="1"/>
      <input name="RI" channel="RI" location="" rate="1"/>
      <input name="RP" channel="RP" location="" rate="1"/>
      <input name="HA" channel="HA" location="" rate="1"/>
      <input name="HD" channel="HD" location="" rate="1"/>
      <input name="HI" channel="HI" location="" rate="1"/>
      <input name="HP" channel="HP" location="" rate="1"/>
      <node stream="W"/>
    </tree>
    <tree>
      <input name="EV" channel="EV" location="" rate="1"/>
      <node stream="A"/>
    </tree>
  </proc>
