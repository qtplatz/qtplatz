// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef MSCALIBRATIONFORM_H
#define MSCALIBRATIONFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;

namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class MSCalibrateMethod;
}

namespace Ui {
    class MSCalibrationForm;
}

namespace qtwidgets {

    class MSCalibrateDelegate;

    class MSCalibrationForm : public QWidget
                            , public adplugin::LifeCycle {

        Q_OBJECT

    public:
        explicit MSCalibrationForm(QWidget *parent = 0);
        ~MSCalibrationForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--

    private:
        Ui::MSCalibrationForm *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< adcontrols::MSCalibrateMethod > pMethod_;
        boost::scoped_ptr< MSCalibrateDelegate > pDelegate_;
    };

}

#endif // MSCALIBRATIONFORM_H
