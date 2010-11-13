// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef REPORTFORM_H
#define REPORTFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class ReportMethod;
}

namespace Ui {
    class ReportForm;
}

namespace qtwidgets {

    class ReportDelegate;

    class ReportForm : public QWidget
                     , public adplugin::LifeCycle {
        Q_OBJECT
        
    public:
        explicit ReportForm(QWidget *parent = 0);
        ~ReportForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--
        
    private:
        Ui::ReportForm *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< adcontrols::ReportMethod > pMethod_;
        boost::scoped_ptr< ReportDelegate > pDelegate_;
    };

}

#endif // REPORTFORM_H
