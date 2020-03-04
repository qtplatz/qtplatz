#!/usr/bin/env python3
from qtplatz import adControls, adProcessor;
import pprint
import numpy as np
from datetime import datetime

def print_spectrum( ms, sp, indent ):
    prop = ms.property();
    #print ( "\tsamplingInfo: ", end = '' );
    print ( "\tsampInfo:\t", prop.samplingInfo() );
    dt = datetime.fromtimestamp( prop.timeSinceEpoch() // 1000000000 );
    print ( '\tepoch_time: {}\t {}'.format( dt.strftime('%Y-%m-%d %H:%M:%S'), prop.timeSinceInjection()), end = '' );
    print ( "\tnumAverage=", prop.numAverage() );
    pprint.pprint( prop.device_data( sp ), indent=indent, width=131 );
    
    lines = 0;
    for value in ms:
        lines += 1;
        print( lines, np.array(value) )  # time, mass, intensity
        if ( lines >= 10 ):
            break;
    prop = ms.property();
    print ( "\tepoch_time: {}\t {}".format( dt.strftime('%Y-%m-%d %H:%M:%S'), prop.timeSinceInjection()), end = '' );
    print ( "\tnumAverage=", prop.numAverage(), "\ttimeSinceInjection: ", prop.timeSinceInjection() );

def print_file( f, sp, indent ):
    pprint.pprint( [ f.attributes()['dataType'], f.attributes()['name'] ], indent=indent );
    if ( f.attributes()['dataType'] == 'MassSpectrum' ):
        print_spectrum( f.body(), sp, indent + 4 );

processor = adProcessor.processor()
filename = '/Users/toshi/data/z440/2020-02-27/HCOONa_0002.adfs'
if ( processor.open( filename )):
    print ( '{} Open success'.format( processor.filename() ))
else:
    print ( '{} Open failed'.format( processor.filename() ))
    exit()

sp = processor.massSpectrometer();
print ( "\tmass spectrometer clsid: ", sp.massSpectrometerClsid(), "\t", sp.massSpectrometerName() );
print ( "\tdata interpreter uuid:   ", sp.dataInterpreterUuid(), "\t", sp.dataInterpreterText() );

folder = processor.findFolder('/Processed/Spectra')

for f in folder.files():
    print ( "---- subfolder: " , f.name() );
    print_file( f, sp, 4 );
    for a in f.attachments():
        print_file( a, sp, 8 )



print ( "Supported mass spectrometers:" );
clsid = "";
for model in adControls.MassSpectrometer_list():
    if ( model[1] == 'AccuTOF' ):
        clsid = model[0];
    
if ( clsid != "" ):
    accutof = adControls.MassSpectrometer_create( clsid );
    print ( "\taccutof: mass spectrometer clsid: ", accutof.massSpectrometerClsid(), "\t", accutof.massSpectrometerName() );
    print ( "\taccutof: data interpreter uuid:   ", accutof.dataInterpreterUuid(), "\t", accutof.dataInterpreterText() );

