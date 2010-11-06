// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef MSLOCKFORM_H
#define MSLOCKFORM_H

#include <QWidget>

namespace Ui {
    class MSLockForm;
}

namespace qtwidgets {

  class MSLockForm : public QWidget
  {
    Q_OBJECT
      
      public:
    explicit MSLockForm(QWidget *parent = 0);
    ~MSLockForm();
    
  private:
    Ui::MSLockForm *ui;
  };

}

#endif // MSLOCKFORM_H
