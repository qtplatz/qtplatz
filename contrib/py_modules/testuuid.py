#!/usr/bin/env python3

from qtplatz import adControls, adProcessor

uuid = adProcessor.gen_uuid();
print ( '---------- got: {} --------'.format( uuid ) );
print ( '---------- str: {}'.format( str( uuid ) ) )

adProcessor.set_uuid( uuid );

uuids = adProcessor.uuids();

print ( len( uuids ) );

for id in uuids:
    print( id )
