/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef SCANLAWDIALOG_HPP
#define SCANLAWDIALOG_HPP

#include "adwidgets_global.hpp"
#include <QDialog>
#include <memory>

class QKeyEvent;
class QContextMenuEvent;

namespace adcontrols { class ScanLaw; class MassSpectrometer; }

namespace adwidgets {

    namespace Ui {
        class ScanLawDialog;
    }

    class ADWIDGETSSHARED_EXPORT ScanLawDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ScanLawDialog(QWidget *parent = 0);
        ~ScanLawDialog();

        void setScanLaw( const adcontrols::ScanLaw& );
        void setScanLaw( std::shared_ptr< const adcontrols::MassSpectrometer >, int mode );
        const adcontrols::ScanLaw& scanLaw() const;
        
        double tDelay() const;
        double acceleratorVoltage() const;
        double fLength() const;
        void setValues( double fLength, double accVoltage, double tDelay, int mode );
        QString formula() const;
        void setFormula( const QString& );
        void setMass( double );
        double mass() const;

        void setData( const std::vector< std::pair<double, double> >& time_mass_array );

    private:
        Ui::ScanLawDialog *ui;
        class impl;
        impl * impl_;
        void setCalculator();
        void estimate();
        void keyPressEvent( QKeyEvent * ) override;

    public slots:
        void handleCopyToClipboard();
        void handlePaste();
    };

}

#endif // SCANLAWDIALOG_HPP
