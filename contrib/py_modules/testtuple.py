#!/usr/bin/env python3

import os
if os.name == 'nt':
    os.add_dll_directory( 'c:/QtPlatz/bin' )

from qtplatz import py_adcontrols, py_adprocessor

t = py_adprocessor.tuple();
print ( t );

v = py_adprocessor.tuples();
print ( v );
for tt in v:
    print(tt)
