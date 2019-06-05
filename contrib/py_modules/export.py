#!/usr/bin/env python3
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog
from PyQt5.QtGui import QIcon
from qtplatz import adProcessor, adControls

app = QApplication(sys.argv)
filename, f = QFileDialog.getOpenFileName(None, "Open file for export in python", "","QtPlatz (*.adfs);;All Files (*)" )

print ( 'filename: {}'.format( filename ) )
print ( 'filter: {}'.format(f))
print ( '********************' )

if filename:
    file = adProcessor.processor();

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
