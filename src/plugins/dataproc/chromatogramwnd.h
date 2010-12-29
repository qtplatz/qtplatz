// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef CHROMATOGRAMWND_H
#define CHROMATOGRAMWND_H

#include <QWidget>
#include <boost/shared_ptr.hpp>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
}

namespace dataproc {

    class Dataprocessor;

    namespace internal {

        class ChromatogramWndImpl;

        class ChromatogramWnd : public QWidget {
            Q_OBJECT
        public:
            explicit ChromatogramWnd(QWidget *parent = 0);
            void init();
            void draw1( boost::shared_ptr< adcontrols::MassSpectrum >& );
            void draw2( boost::shared_ptr< adcontrols::MassSpectrum >& );
            void draw( boost::shared_ptr< adcontrols::Chromatogram >& );

        signals:
      
        public slots:
            void handleSessionAdded( Dataprocessor* );
            void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );

        private:
            boost::shared_ptr<ChromatogramWndImpl> pImpl_;

        };
    }
}
#endif // CHROMATOGRAMWND_H
