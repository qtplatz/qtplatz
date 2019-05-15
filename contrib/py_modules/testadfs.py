#!/usr/bin/python3

from qtplatz import adProcessor

file = adProcessor.processor();
filename = '/home/toshi/data/hpk-mcp/2019-04-25/mcp1/Ar_36_40_5kV_20lap_1600V(1620)_110mV_3290mA_1_1_3_0010.adfs';

if ( file.open( filename ) ):
    print ( '{} Open Success'.format( filename ) )
    tuples = file.dataReaderTuples();
    for t in tuples:
        print( '\treader {}'.format( t ) )
    readers = file.dataReaders();
    print ( readers )
    for r in readers:
        print( '\treader {}, {}'.format( r.objuuid(), r.objtext() ) )
