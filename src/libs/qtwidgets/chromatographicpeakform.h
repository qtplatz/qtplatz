// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef CHROMATOGRAPHICPEAKFORM_H
#define CHROMATOGRAPHICPEAKFORM_H

#include <QWidget>

namespace Ui {
    class ChromatographicPeakForm;
}

namespace qtwidgets {

  class ChromatographicPeakForm : public QWidget
  {
    Q_OBJECT
      
      public:
    explicit ChromatographicPeakForm(QWidget *parent = 0);
    ~ChromatographicPeakForm();
    
  private:
    Ui::ChromatographicPeakForm *ui;
  };

}

#endif // CHROMATOGRAPHICPEAKFORM_H
