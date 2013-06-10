/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#ifndef CENTROIDMETHODMODEL_HPP
#define CENTROIDMETHODMODEL_HPP

#include <QObject>

#include <QVariant>
#include <QStandardItemModel>
// #include <QDeclarativeItem>
#if ! defined Q_MOC_RUN
#include <adcontrols/centroidmethod.hpp>
#endif
#include <boost/noncopyable.hpp>

namespace qtwidgets {

    class CentroidMethodModel : public QObject {
        Q_OBJECT
        Q_ENUMS( ScanType )
        Q_ENUMS( AreaHeight )
        Q_PROPERTY( ScanType scanType READ scanType WRITE scanType NOTIFY scanTypeChanged )
        Q_PROPERTY( AreaHeight areaHeight READ areaHeight WRITE areaHeight NOTIFY areaHeightChanged )
        Q_PROPERTY( double baseline_width READ baseline_width WRITE baseline_width NOTIFY valueChanged )
        Q_PROPERTY( double peak_centroid_fraction READ peak_centroid_fraction WRITE peak_centroid_fraction NOTIFY valueChanged )
        Q_PROPERTY( double peakwidth_propo_in_ppm READ peakwidth_propo_in_ppm WRITE peakwidth_propo_in_ppm NOTIFY valueChanged )
        Q_PROPERTY( double peakwidth_tof_in_da READ peakwidth_tof_in_da WRITE peakwidth_tof_in_da NOTIFY valueChanged )
        Q_PROPERTY( double peakwidth_tof_at_mz READ peakwidth_tof_at_mz WRITE peakwidth_tof_at_mz NOTIFY valueChanged )
        Q_PROPERTY( double peakwidth_const_in_da READ peakwidth_const_in_da WRITE peakwidth_const_in_da NOTIFY valueChanged )
    public:
        explicit CentroidMethodModel( QObject * parent = 0 );
        
        enum ScanType {
            ScanTypeTof = adcontrols::CentroidMethod::ePeakWidthTOF
            , ScanTypeProportional = adcontrols::CentroidMethod::ePeakWidthProportional
            , ScanTypeConstant = adcontrols::CentroidMethod::ePeakWidthConstant
        };
        
        enum AreaHeight {
            Area, Height
        };
        
        //------------------------
        
        double baseline_width() const;
        void baseline_width(double);
        
        double peak_centroid_fraction() const;
        void peak_centroid_fraction(double);
        
        double peakwidth_tof_in_da() const;
        void peakwidth_tof_in_da( double );
        
        double peakwidth_tof_at_mz() const;
        void peakwidth_tof_at_mz( double );
        
        double peakwidth_propo_in_ppm() const;
        void peakwidth_propo_in_ppm( double );
        
        double peakwidth_const_in_da() const;
        void peakwidth_const_in_da( double );
        
        ScanType scanType() const;
        void scanType( ScanType );
        
        AreaHeight areaHeight() const;
        void areaHeight( AreaHeight );

        inline const adcontrols::CentroidMethod& method() const { return method_; }
        
    signals:
        void scanTypeChanged();
        void areaHeightChanged();
        void valueChanged();
                           
    public slots:
        
    private:
        adcontrols::CentroidMethod method_;
    };

}

#endif // CENTROIDMETHODMODEL_HPP
