// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef PEAKIDTABLEFORM_H
#define PEAKIDTABLEFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace Ui {
    class PeakIDTableForm;
}

namespace qtwidgets {

    class PeakIDTableForm : public QWidget
                          , public adplugin::LifeCycle {

        Q_OBJECT
        
    public:
        explicit PeakIDTableForm(QWidget *parent = 0);
        ~PeakIDTableForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--
        
    private:
        Ui::PeakIDTableForm *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
    };
}

#endif // PEAKIDTABLEFORM_H
