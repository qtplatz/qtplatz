/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSPEAKVIEW_HPP
#define MSPEAKVIEW_HPP

#include "adwidgets_global.hpp"
#include <adplugin/lifecycle.hpp>
#include <QWidget>
#include <memory>

namespace adcontrols { class MSPeaks; }

namespace adwidgets {

    class MSPeakSummary;
    class TOFTable;

    class ADWIDGETSSHARED_EXPORT MSPeakWidget : public QWidget
                                              , public adplugin::LifeCycle {

        Q_OBJECT

    public:
        explicit MSPeakWidget(QWidget *parent = 0);
        ~MSPeakWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;
        void * query_interface_workaround( const char * ) override;

        void currentChanged( int mode );
        void currentChanged( const std::string& );

    signals:
        void onSetData( const QString&, const adcontrols::MSPeaks& );
        void onSetData( int mode, const adcontrols::MSPeaks& );
            
    public slots:
        void handle_add_mspeaks( const adcontrols::MSPeaks& );

	private:
        std::unique_ptr< MSPeakSummary > peakSummary_;
        std::unique_ptr< TOFTable > tofTable_;
        std::unique_ptr< adcontrols::MSPeaks > mspeaks_;
        std::vector< QWidget * > clients_;
    };

}

#endif // MSPEAKVIEW_HPP
