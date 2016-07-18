/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#pragma once

#include "adwidgets_global.hpp"

#include <QDialog>
#include <memory>

class QMenu;

namespace adcontrols { class MSPeaks; }
namespace boost { namespace uuids { struct uuid; } }

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT ScanLawDialog2 : public QDialog {
        
        Q_OBJECT
        
    public:
        explicit ScanLawDialog2(QWidget *parent = 0);
        ~ScanLawDialog2();

        void setSpectrometerData( const boost::uuids::uuid&, const QString& desc, double length );
        void setLength( double );
        void setAcceleratorVoltage( double );
        void setTDelay( double );

        double length() const;
        double acceleratorVoltage() const;
        double tDelay() const;
        
        void addPeak( uint32_t id, const QString& formula, double time, double matchedMass, int mode );
        bool commit();
        size_t peakCount() const;

        void addObserver( const boost::uuids::uuid&, const QString& objtext, double va, double t0, bool checked = true );
        QVector< QString > checkedObservers() const;

    public slots:
        void handleCopyToClipboard();
        void handlePaste();
        void handleAddPeak();
    private:
        void handleLengthChanged();
        void handleAcceleratorVoltageChanged();
        void handleTDelayChanged();
        bool read( adcontrols::MSPeaks& ) const;
        bool estimateAcceleratorVoltage( double& t0, double& v, const adcontrols::MSPeaks& ) const;
        bool estimateLength( double& t0, double& L, const adcontrols::MSPeaks& ) const;
        void updateMassError();
        void updateObservers( double t0, double acclVolts );
        void handlePeakTableMenu( const QPoint& );
        
        class impl;
        std::unique_ptr< impl > impl_;
        friend class ScanLawDialog2_archive;
    };

}

