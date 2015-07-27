/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QDialog>
#include <adcontrols/metric/prefix.hpp>

namespace Ui {
class Dialog;
}

class QStringList;

namespace adtextfile {

    class Dialog : public QDialog  {
        Q_OBJECT

    public:
        explicit Dialog(QWidget *parent = 0);
        ~Dialog();
        enum data_type { data_chromatogram, data_spectrum };
        enum scan_type { time_squared_scan_law, linear_scan_law };
        
        void setDataType( data_type t );
        data_type dataType() const;

        void setAcceleratorVoltage( double );
        void setLength( double );

        double acceleratorVoltage() const;
        double length() const;

        void setHasDataInterpreer( bool );
        bool hasDataInterpreter() const;

        void setDataInterpreterClsids( const QStringList& );
        QString dataInterpreterClsid() const;

        bool invertSignal() const;
        bool correctBaseline() const;
        adcontrols::metric::prefix dataPrefix() const;

        void appendLine( const QStringList& );

        void setIsCentroid( bool );
        bool isCentroid() const;

        bool isMassIntensity() const;
        bool isTimeIntensity() const;
        size_t columnCount() const;

    private:
        Ui::Dialog *ui;
    };

}


#endif // DIALOG_HPP
