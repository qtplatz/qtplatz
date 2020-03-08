#!/usr/bin/env python3
import sys
from PyQt5.QtWidgets import QMainWindow, QApplication, QWidget, QAction, qApp, QTableWidget, QTableWidgetItem, QVBoxLayout, QFileDialog
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot
from datetime import datetime
import sqlite3
import matplotlib.pyplot as plt
from matplotlib.backends.qt_compat import QtCore, QtWidgets, is_pyqt5
from matplotlib.backends.backend_qt5agg import FigureCanvas, NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure

times = []
counts = []

def count_frequency( file ):
    print (file);
    conn = sqlite3.connect( file )
    c = conn.cursor()
    c.execute( "SELECT peak_time,count(*) FROM trigger,peak where id=idTrigger GROUP by peak_time" )
    result_set = c.fetchall()
    for row in result_set:
        times.append( -row[ 0 ] )
        counts.append( row[ 1 ] )

if __name__ == '__main__':
    app = QApplication(sys.argv)
    filenames, f = QFileDialog.getOpenFileNames(None
                                                , "Open file for export in python"
                                                , "/data/data/wspc/2019-08-02"
                                                , "QtPlatz (*.adfs);;All Files (*)" )

    for file in filenames:
        count_frequency ( file );

        plt.ylabel('Frequency(counts)')
        plt.xlabel('Peak Height(mV)')

        plt.grid( True )

        plt.scatter( times, counts )
        plt.plot( times, counts )
        plt.show()


"""
#conn = sqlite3.connect('/disk2/data/mouse/2018-09-07/Soil_Closed15_0100.adfs')

c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, COUNT(*) FROM peak WHERE peak_time > 205.98e-6 and peak_time < 206.01 GROUP BY threshold" )

result_set = c.fetchall()

V = []
counts = []

for row in result_set:
    V.append( -row[ 0 ] )
    counts.append( row[ 1 ] )


"""
