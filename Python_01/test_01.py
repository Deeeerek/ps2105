#!/usr/bin/env python

import PicoScope;

print "OPEN"
myUnit = PicoScope.open_unit();
print "Done."

print "SET_CHANNEL"
PicoScope.set_channel( myUnit, 0, 1, 1, 7 );
print "Done."

print "RUN_BLOCK"
PicoScope.run_block( myUnit, 512, 14, 0);
print "Done."

print "DELAY"
PicoScope.delay(2000);
print "Done."

print "READY"
print PicoScope.ready(myUnit)
print "Done."

print "GET_VALUES"
values = PicoScope.get_values(myUnit,512);
print "Done."

print "CLOSE_UNIT"
PicoScope.close_unit(myUnit);
print "Done."

print values
