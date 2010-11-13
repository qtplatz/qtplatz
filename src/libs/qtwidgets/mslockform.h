// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef MSLOCKFORM_H
#define MSLOCKFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace Ui {
    class MSLockForm;
}

namespace qtwidgets {

  class MSLockDelegate;

  class MSLockForm : public QWidget
                   , public adplugin::LifeCycle {
    Q_OBJECT
      
  public:
      explicit MSLockForm(QWidget *parent = 0);
      ~MSLockForm();

      // adplugin::LifeCycle
      void OnCreate( const adportable::Configuration& );
      void OnInitialUpdate();
      void OnFinalClose();
      //<--
    
  private:
      Ui::MSLockForm *ui;
      boost::scoped_ptr< QStandardItemModel > pModel_;
      boost::scoped_ptr< adportable::Configuration > pConfig_;
      boost::scoped_ptr< MSLockDelegate > pDelegate_;
  };

}

#endif // MSLOCKFORM_H
