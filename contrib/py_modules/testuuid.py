#!/usr/bin/env python3

from qtplatz import py_adcontrols, py_adprocessor

uuid = py_adprocessor.gen_uuid();
print ( '---------- got: {} --------'.format( uuid ) );
print ( '---------- str: {}'.format( str( uuid ) ) )

py_adprocessor.set_uuid( uuid );

uuids = py_adprocessor.uuids();

print ( len( uuids ) );

for id in uuids:
    print( id )
