#!/usr/bin/env python3
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog
from PyQt5.QtGui import QIcon
from qtplatz import adProcessor, adControls
from datetime import datetime

app = QApplication(sys.argv)
filename, f = QFileDialog.getOpenFileName(None, "Open file for export in python", "","QtPlatz (*.adfs);;All Files (*)" )

if filename:

    file = adProcessor.processor();

    if ( file.open( filename ) ):
        print ( '{} Open Success'.format( filename ) )
        tuples = file.dataReaderTuples();
        for t in tuples:
            readers = file.dataReaders();
            print ( readers )
            for r in readers:
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
                                    # for i in range( 0, sp.size() ):
                                    #print( '\t\t\t{}, {}, {}, {}'.format( i, sub.getTime(i), sub.getMass(i), sub.getIntensity(i) ) )
                        if reader.next() == False:
                            break
