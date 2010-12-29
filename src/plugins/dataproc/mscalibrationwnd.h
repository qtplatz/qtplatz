// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MSCALIBRATIONWND_H
#define MSCALIBRATIONWND_H

#include <QWidget>
#include <boost/smart_ptr.hpp>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class MassSpectrum;
}

namespace dataproc {

    class Dataprocessor;

    namespace internal {

        class MSCalibrationWndImpl;

        class MSCalibrationWnd : public QWidget {
            Q_OBJECT
        public:
            explicit MSCalibrationWnd(QWidget *parent = 0);
            void init();
      
        signals:
      
        public slots:
            void handleSessionAdded( Dataprocessor* );
            void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );

        private:
            boost::shared_ptr<MSCalibrationWndImpl> pImpl_;
      
        };

    }
}

#endif // MSCALIBRATIONWND_H
