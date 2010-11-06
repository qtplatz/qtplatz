// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ISOTOPEFORM_H
#define ISOTOPEFORM_H

#include <QWidget>

namespace Ui {
    class IsotopeForm;
}

namespace qtwidgets {

  class IsotopeForm : public QWidget  {
      Q_OBJECT
      
  public:
      explicit IsotopeForm(QWidget *parent = 0);
      ~IsotopeForm();
      
  private:
      Ui::IsotopeForm *ui;
  };
    
}

#endif // ISOTOPEFORM_H
