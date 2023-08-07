#!/usr/bin/env python3

from qtplatz import py_adcontrols, py_adprocessor
from datetime import datetime
file = py_adprocessor.processor();
filename = '/Users/toshi/data/z440/2023-08-04/SFE10sSFC_neg_L3_50ppm_0,2uL_0001.adfs';

if ( file.open( filename ) ):
    print ( '{} Open Success'.format( filename ) )
    tuples = file.dataReaderTuples();
    for t in tuples:
        print( '\treader {}'.format( t ) )
    readers = file.dataReaders();
    print ( readers )
    for r in readers:
        print( '\treader {}, {}, it has {} spectra'.format( r.objuuid(), r.objtext(), r.size() ) )
    reader = file.dataReader( 'da16704b-2d18-5c91-82c7-42a0a8b94d2b' )
    if reader:
        while True:
            sp = reader.readSpectrum();
            if sp:
                dt = datetime.fromtimestamp( reader.epoch_time() // 1000000000 )
                #print( '\tepoch_time: {}'.format( dt.strftime('%Y-%m-%d %H:%M:%S') + '.' + str( reader.epoch_time() % 1000000000 ).zfill(9) ), end = '' )
                #print ( '\t\tprotocol: {}, length: {}, {} protocols in the waveform.'.format( 0, sp.size(), sp.numProtocols() ) );
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
