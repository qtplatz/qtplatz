// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ISOTOPEFORM_H
#define ISOTOPEFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;

namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class IsotopeMethod;
}

namespace Ui {
    class IsotopeForm;
}

namespace qtwidgets {

  class IsotopeDelegate;

  class IsotopeForm : public QWidget
                    , public adplugin::LifeCycle {

      Q_OBJECT
      
  public:
      explicit IsotopeForm(QWidget *parent = 0);
      ~IsotopeForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--
      
  private:
      Ui::IsotopeForm *ui;
      boost::scoped_ptr< QStandardItemModel > pModel_;
      boost::scoped_ptr< adportable::Configuration > pConfig_;
      boost::scoped_ptr< adcontrols::IsotopeMethod > pMethod_;
      boost::scoped_ptr< IsotopeDelegate > pDelegate_;
  };
    
}

#endif // ISOTOPEFORM_H
