// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef TARGETINGFORM_H
#define TARGETINGFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class TargetingMethod;
}

namespace Ui {
    class TargetingForm;
}

namespace qtwidgets {

    class TargetingDelegate;

    class TargetingForm : public QWidget
                        , public adplugin::LifeCycle {

        Q_OBJECT
        
    public:
        explicit TargetingForm(QWidget *parent = 0);
        ~TargetingForm();
        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--

    private:
        Ui::TargetingForm *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< adcontrols::TargetingMethod > pMethod_;
        boost::scoped_ptr< TargetingDelegate > pDelegate_;
    };

}

#endif // TARGETINGFORM_H
