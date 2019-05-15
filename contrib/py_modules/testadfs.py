#!/usr/bin/python3

from qtplatz import adProcessor, adControls

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
        print( '\treader {}, {}, it has {} spectra'.format( r.objuuid(), r.objtext(), r.size() ) )
        sp = r.readSpectrum();
        print( '\t\tspectrum size = {}, protocols = {}'.format( sp.size(), sp.numProtocols() ) );
        #        for i in range( 0, sp.size() ):
        #            print( '{}, {}, {}, {}'.format( i, sp.getTime(i), sp.getMass(i), sp.getIntensity(i) ) )
        for proto in range ( 1, sp.numProtocols() + 1 ):
            sub = sp.getProtocol( proto )
            print ( '\tprotocol {}, spectrum length: {}'.format( proto, sub.size() ) )
