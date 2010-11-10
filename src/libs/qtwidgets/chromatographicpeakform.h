// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef CHROMATOGRAPHICPEAKFORM_H
#define CHROMATOGRAPHICPEAKFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace Ui {
    class ChromatographicPeakForm;
}

namespace qtwidgets {

    class ChromatographicPeakForm : public QWidget
                                  , public adplugin::LifeCycle {

        Q_OBJECT
      
    public:
        explicit ChromatographicPeakForm(QWidget *parent = 0);
        ~ChromatographicPeakForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--
    
    private:
        Ui::ChromatographicPeakForm *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
    };

}

#endif // CHROMATOGRAPHICPEAKFORM_H
