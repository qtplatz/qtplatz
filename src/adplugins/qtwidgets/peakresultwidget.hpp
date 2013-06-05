/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#pragma once

#include <QTableView>
#if ! defined Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif
#include <adplugin/lifecycle.hpp>

namespace adcontrols {
    class Peaks;
    class Peak;
    class Baselines;
    class Chromatogram;
    class PeakResult;
}

class QStandardItemModel;

namespace qtwidgets {

    class PeakResultWidget : public QTableView, public adplugin::LifeCycle {
        Q_OBJECT
     public:
        ~PeakResultWidget();
        explicit PeakResultWidget(QWidget *parent = 0);

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

        
    signals:
            
    public slots:
        void setData( const adcontrols::Peaks& );
		void setData( const adcontrols::PeakResult& );
        void getLifeCycle( adplugin::LifeCycle*& );

    private:
        void add( const adcontrols::Peak& );
	
    private:
        boost::scoped_ptr< QStandardItemModel > pModel_;
    };
}

