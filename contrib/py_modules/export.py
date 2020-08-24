#!/usr/bin/env python3
import sys
from qtplatz import adProcessor, adControls
from datetime import datetime
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog
from PyQt5.QtGui import QIcon

def export_adfs( filename ):

    file = adProcessor.processor();
    print (filename)

    if ( file.open( filename ) ):
        print ( '{} Open Success'.format( filename ) )

        guids = file.dataReaderTuples()
        for guid in guids:
            reader = file.dataReader( guid[ 2 ] )
            if reader:
                while True:
                    sp = reader.readSpectrum();
                    if sp:
                        #dt = datetime.fromtimestamp( reader.epoch_time() // 1000000000 )
                        #print ( '\tepoch_time: {}'.format( dt.strftime('%Y-%m-%d %H:%M:%S') + '.' + str( reader.epoch_time() % 1000000000 ).zfill(9) ), end = '' )
                        #print ( '\t\tprotocol: {}, length: {}, {} protocols in the waveform.'.format( 0, sp.__len__(), sp.numProtocols() ) );
                        seconds  = reader.epoch_time() // 1000000000;
                        fraction = reader.epoch_time() % 1000000000;
                        proto = 0
                        for d in sp:
                            print( [guid[2], proto, seconds, fraction, d ] )
                        for proto in range ( 1, sp.numProtocols() + 1 ):
                            sub = sp.getProtocol( proto )
                            if sub:
                                for d in sub:
                                    print( [guid[2], proto, seconds, fraction, d ] )
                    if reader.next() == False:
                        break

    else:
        print ( "file: ", filename, " open failed." )

if __name__ == '__main__':
    app = QApplication(sys.argv)
    filename, f = QFileDialog.getOpenFileName(None, "Open file for export in python", "","QtPlatz (*.adfs);;All Files (*)" )
    if filename:
        export_adfs( filename )

#if __name__ == '__main__':
#    export_adfs( '/data/data/otsuka/2020-03-11/Hela1-0001.adfs' )
