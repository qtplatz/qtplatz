#!/usr/bin/env python3

from qtplatz import adProcessor, adControls
from datetime import datetime
file = adProcessor.processor();
filename = '/media/toshi/data/data/hpk-mcp/2019-05-31/Ar_10-90laps_1680V(1790)_3850mA_0001.adfs';

if ( file.open( filename ) ):
    print ( '{} Open Success'.format( filename ) )
    tuples = file.dataReaderTuples();
    for t in tuples:
        print( '\treader {}'.format( t ) )
    readers = file.dataReaders();
    print ( readers )
    for r in readers:
        print( '\treader {}, {}, it has {} spectra'.format( r.objuuid(), r.objtext(), r.size() ) )
    reader = file.dataReader( 'f26ed645-7e3f-508e-b880-83812dc7594c' )
    if reader:
        while True:
            sp = reader.readSpectrum();
            if sp:
                dt = datetime.fromtimestamp( reader.epoch_time() // 1000000000 )
                print( '\tepoch_time: {}'.format( dt.strftime('%Y-%m-%d %H:%M:%S') + '.' + str( reader.epoch_time() % 1000000000 ).zfill(9) ), end = '' )
                print ( '\t\tprotocol: {}, length: {}, {} protocols in the waveform.'.format( 0, sp.size(), sp.numProtocols() ) );
                for i in range( 0, sp.size() ):
                    print( '\t\t\t{}, {}, {}, {}'.format( i, sp.getTime(i), sp.getMass(i), sp.getIntensity(i) ) )
                for proto in range ( 1, sp.numProtocols() + 1 ):
                    sub = sp.getProtocol( proto )
                    if sub:
                        print ( '\t\tprotocol: {}, length: {}'.format( proto, sub.size() ) )
                        for i in range( 0, sp.size() ):
                            print( '\t\t\t{}, {}, {}, {}'.format( i, sub.getTime(i), sub.getMass(i), sub.getIntensity(i) ) )
            if reader.next() == False:
                break
