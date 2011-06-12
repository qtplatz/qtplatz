// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include <adplugin/lifecycle.hpp>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class MassSpectrum;
    class MSReferences;
    class MSCalibrateResult;
}

class QStandardItemModel;

namespace qtwidgets {

    class MSCalibSummaryDelegate;

    class MSCalibSummaryWidget : public QTableView, public adplugin::LifeCycle { 
        Q_OBJECT
    public:
        ~MSCalibSummaryWidget();
        explicit MSCalibSummaryWidget(QWidget *parent = 0);

        // adplugin::LifeCycle
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
        virtual void OnUpdate( boost::any& );
        virtual void OnFinalClose();
        // <--

    signals:

    public slots:
        // void setData( const adcontrols::MSReferences& );
        void setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        void getLifeCycle( adplugin::LifeCycle*& );

    private:
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< MSCalibSummaryDelegate > pDelegate_;
        boost::scoped_ptr< adcontrols::MSReferences > pReferences_;
        boost::scoped_ptr< adcontrols::MassSpectrum > pCalibrantSpectrum_;
    };

}


