// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
