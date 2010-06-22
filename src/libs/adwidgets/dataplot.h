// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QWidget>

namespace adil {

  namespace ui {

    class Titles;
    class Title;
    class Traces;
    class Trace;
    class Axis;
    class Colors;

    class Dataplot : public QWidget {
      Q_OBJECT
    public:
      ~Dataplot();
      explicit Dataplot(QWidget *parent = 0);

      bool createControl();

      bool autoSize(bool);
      void autoSize() const;

      unsigned long backColor() const;
      void backColor( unsigned long );

      unsigned long borderColor() const;
      void borderColor( unsigned long );

      long borderStyle() const;
      void borderStyle( long );

      long borderWidth() const;
      void borderWidth( long );

      void foreColor(unsigned long newValue);
      unsigned long foreColor();

      void enabled(bool newValue);
      bool enabled();
      void borderVisible(bool newValue);
      bool borderVisible();
      Titles titles();
      //LPDISPATCH plotRegion();
      Traces traces();
      Colors colors();
      Axis axisX();
      Axis axisY();
      Axis axisY1();
      Axis axisY2();
      bool visible();
      void bisible(bool newValue);
      bool redrawEnabled();
      void redrawEnabled(bool newValue);
      long cursorStyle();
      void cursorStyle(long newValue);
      bool scaleMinimumY();
      void scaleMinimumY(bool newValue);
      bool showCursor();
      void showCursor(bool newValue);

      void zoomXY(double minimumX, double minimumY, double maximumX, double maximumY);
      void zoomXAutoscaleY(double minimumX, double maximumX);
      void zoomOut();
      void highlightColor(unsigned long newValue);
      unsigned long highlightColor();
      bool highlight();
      void highlight(bool newValue);
      long currentMouseXPixel();
      long currentMouseYPixel();
      bool contourLegendVisible();
      void contourLegendVisible(bool newValue);
      long plotRegionPadding();
      void plotRegionPadding(long newValue);
      bool useBitmaps();
      void useBitmaps(bool newValue);
      long worldToPixelX(double x);
      long worldToPixelY(double y);
      long worldToPixelY2(double y);
      double pixelToWorldX(long x);
      double pixelToWorldY(long y);
      double pixelToWorldY2(long y);
      void setMousePosition(double x, double y);
      void setCursorPosition(double x, double y);
      void moveCursorPosition(double x, double y);
      // LPDISPATCH Legend();

      signals:
      
	public slots:
	
    private:
		struct DataplotImpl * pImpl_;
    protected:
		virtual void resizeEvent( QResizeEvent * );
    };

  }
}

#endif // DATAPLOT_H
