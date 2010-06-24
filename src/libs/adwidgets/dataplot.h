// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QWidget>
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

struct ISADataplot;

namespace adil {

  namespace ui {

    class Titles;
    class Title;
    class Traces;
    class Trace;
    class Axis;
    class Colors;
    class PlotRegion;
	class Legend;

	namespace internal {
		namespace win32 {
			class DataplotImpl;
			typedef boost::scoped_ptr< DataplotImpl > DataplotImplPtr;
		}
		namespace qt {
			class DataplotImpl; // TBD
		}
	}

	class Dataplot : public QWidget, boost::noncopyable {
      Q_OBJECT
    public:
      ~Dataplot();
      explicit Dataplot(QWidget *parent = 0);

      bool createControl();

	  void autoSize(bool);
	  bool autoSize() const;

      unsigned long backColor() const;
      void backColor( unsigned long );

      unsigned long borderColor() const;
      void borderColor( unsigned long );

      long borderStyle() const;
      void borderStyle( long );

      long borderWidth() const;
      void borderWidth( long );

      void foreColor(unsigned long newValue);
      unsigned long foreColor() const;

      void enabled(bool newValue);
      bool enabled() const;
      void borderVisible(bool newValue);
      bool borderVisible() const;
      Titles titles() const;
	  PlotRegion plotRegion() const;
      Traces traces() const;
      Colors colors() const;
      Axis axisX() const;
      Axis axisY() const;
      Axis axisY1() const;
      Axis axisY2() const;
      bool visible() const;
      void visible(bool newValue);
      bool redrawEnabled() const;
      void redrawEnabled(bool newValue);
      long cursorStyle() const;
      void cursorStyle(long newValue);
      bool scaleMinimumY() const;
      void scaleMinimumY(bool newValue);
      bool showCursor() const;
      void showCursor(bool newValue);

      void zoomXY(double minimumX, double minimumY, double maximumX, double maximumY);
      void zoomXAutoscaleY(double minimumX, double maximumX);
      void zoomOut();
      void highlightColor(unsigned long newValue);
      unsigned long highlightColor() const;
      bool highlight() const;
      void highlight(bool newValue);
      long currentMouseXPixel() const;
      long currentMouseYPixel() const;
      bool contourLegendVisible() const;
      void contourLegendVisible(bool newValue);
      long plotRegionPadding() const;
      void plotRegionPadding(long newValue);
      bool useBitmaps() const;
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
      Legend legend() const;

    signals:
	  void NotifyMouseDown(double x, double y, short button );
	  void NotifyMouseUp( double x, double y, short Button );
	  void NotifyMouseMove( double x, double y, short Button );
	  void NotifyCharacter( long KeyCode );
	  void NotifyKeyDown( long KeyCode );
	  void NotifySetFocus( long hWnd );
	  void NotifyKillFocus( long hWnd );
	  void NotifyMouseDblClk(double x, double y, short button );
	  
	private slots:
	  friend internal::win32::DataplotImpl;
	  
	  virtual void OnMouseDown(double x, double y, short button );
	  virtual void OnMouseUp( double x, double y, short Button );
	  virtual void OnMouseMove( double x, double y, short Button );
	  virtual void OnCharacter( long KeyCode );
	  virtual void OnKeyDown( long KeyCode );
	  virtual void OnSetFocus( long hWnd );
	  virtual void OnKillFocus( long hWnd );
	  virtual void OnMouseDblClk(double x, double y, short button );
	  
    private:
	  internal::win32::DataplotImplPtr pImpl_;

    protected:
      virtual void resizeEvent( QResizeEvent * );
    };

  }
}

#endif // DATAPLOT_H
