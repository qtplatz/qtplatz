/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "infitofwidgets_global.hpp"
#include <QDialog>
#include <memory>
#include <map>
#include <set>

class QMenu;

namespace admtcontrols { class ScanLaw; }
namespace adcontrols { class MSPeaks; class MassSpectrometer; }
namespace boost { namespace uuids { struct uuid; } }

namespace infitofwidgets {

    class INFITOFWIDGETSSHARED_EXPORT ScanLawDialog : public QDialog {

        Q_OBJECT

    public:
        explicit ScanLawDialog(QWidget *parent = 0);
        ~ScanLawDialog();

        void setAcceleratorVoltage( double, bool original = false );
        void setTDelay( double, bool original = false );
        void setL1( double, bool original = false );

        void setLinearLength( double );
        void setOrbitalLength( double );

        double acceleratorVoltage() const;
        double tDelay() const;
        double L1() const;

        void addPeak( uint32_t id, const QString& formula, double time, double matchedMass, int mode );
        bool commit();
        size_t peakCount() const;

        void setScanLaw( std::shared_ptr< admtcontrols::ScanLaw > );
        void setSpectrometerData( const boost::uuids::uuid& id, const QString&, std::shared_ptr< adcontrols::MassSpectrometer >  );

        void addObserver( const boost::uuids::uuid&, const QString& objtext, double va, double t0, bool checked = true );
        QVector< QString > checkedObservers() const;

    public slots:
        void handleCopyToClipboard();
        void handlePaste();
        void handleAddPeak();
    private:
        bool read( int row, adcontrols::MSPeaks& ) const;
        bool estimateAcceleratorVoltage( const adcontrols::MSPeaks& );
        bool estimateL1( const std::vector< std::string >&, const adcontrols::MSPeaks& );
        void updateMassError();
        void updateMasses();
        void updateObservers( double t0, double acclVolts );
        void handlePeakTableMenu( const QPoint& );

        class impl;
        std::unique_ptr< impl > impl_;
        friend class ScanLawDialog_archive;
    };

}
