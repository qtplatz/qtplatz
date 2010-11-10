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

namespace Ui {
    class IsotopeForm;
}

namespace qtwidgets {

  class IsotopeForm : public QWidget  {
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
  };
    
}

#endif // ISOTOPEFORM_H
