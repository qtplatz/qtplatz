// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TRACE_H
#define TRACE_H

#include <vector>
struct ISADPTrace;


namespace adil {
  namespace ui {

    class Annotations;
    class Fractions;
    class Markers;
    class Peaks;
    class Ranges;
    class Font;
    class ColorIndecies;
    class Baselines;
	class FilledRanges;

    class Trace {
    public:
      ~Trace();
      Trace( ISADPTrace * pi = 0 );
      Trace( const Trace& );
    public:

      Annotations annotations() const;
      Fractions fractions() const;
      Markers markers() const;
      Peaks peaks() const;
      Ranges ranges() const;

      bool visible() const;
      void visible(bool newValue);
      bool selected() const;
      void selected(bool newValue);
      long YAxis() const;
      void YAxis(long newValue);
      double offsetX() const;
      void offsetX(double newValue);
      double offsetY() const;
      void offsetY(double newValue);
      Font font() const;
      long traceStyle() const;
      void traceStyle(long newValue);
      long lineStyle() const;
      void lineStyle(long newValue);
      long annotationStyle() const;
      void annotationStyle(long newValue);
      bool autoAnnotation() const;
      void autoAnnotation(bool newValue);
      short colorIndex() const;
      void colorIndex(short newValue);
	  void setColorIndices( const std::vector<short>& Indices);
	  std::vector<short> colorIndices() const;
      double x(long Index) const;
      void x(long Index, double newValue);
      double y(long Index) const;
      void y(long Index, double newValue);
      long pointCount() const;
      void clear();
      Baselines baselines() const;
      long stripChartMode() const;
      void stripChartMode(long newValue);
      void initializeStripChart(long nPoints, double xStart, double xEnd);
      void addStripChartPoint(double x, double y, short ColorIndex);
      short lineWidth() const;
      void lineWidth(short newValue);
      long userData() const;
      void userData(long newValue);
      void setColorIndicesDirect(long nPts, short * pIndices);
      FilledRanges filledRanges() const;
      long orientation() const;
      void orientation(long newValue);
      double autoAnnotationThreshold() const;
      void autoAnnotationThreshold(double newValue);
	  //void setXY(VARIANT& XArray, VARIANT& YArray);
      void setXYDirect(long nPts, double * pX, double * pY);
      void setXYFloatDirect(long nPts, double * pX, float * pY);
      void setXYPointers(long nPts, double * pX, double * pY);
      void setXYFloatPointers(long nPts, double * pX, float * pY);
	  //void setXYChromSpec(LPDISPATCH piSpecOrChrom);
      void clone( Trace& );
      
    private:
      ISADPTrace * pi_;
    };
  }
}

#endif // TRACE_H
