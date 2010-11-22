// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef MSPROCESSINGWND_H
#define MSPROCESSINGWND_H

#include <QWidget>
#include <boost/smart_ptr.hpp>
#include <map>

namespace adwidgets {
    class MassSpectrum;
}

namespace dataproc {

    class Dataprocessor;

    namespace internal {

        class MSProcessingWndImpl;

        class MSProcessingWnd : public QWidget {
            Q_OBJECT
        public:
            explicit MSProcessingWnd(QWidget *parent = 0);

            void init();
      
        signals:
      
        public slots:
            void handleSessionAdded( Dataprocessor* );

        private:
            boost::shared_ptr<MSProcessingWndImpl> pImpl_;
            std::map< std::wstring, boost::shared_ptr<adwidgets::MassSpectrum> > spectra_;
        };

    }
}


#endif // MSPROCESSINGWND_H
