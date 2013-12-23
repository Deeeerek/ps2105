#!/usr/bin/env python

import PicoScope;

myUnit = PicoScope.open_unit();
print "Hello, World! :", myUnit
PicoScope.close_unit(myUnit);
print "Goodbye, Cruel World!"
