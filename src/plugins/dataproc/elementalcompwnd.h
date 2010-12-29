// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ELEMENTALCOMPWND_H
#define ELEMENTALCOMPWND_H

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
    
        class ElementalCompWndImpl;
    
        class ElementalCompWnd : public QWidget {
            Q_OBJECT
        public:
            explicit ElementalCompWnd(QWidget *parent = 0);
            void init();
            void draw1( boost::shared_ptr< adcontrols::MassSpectrum >& );
            void draw2( boost::shared_ptr< adcontrols::MassSpectrum >& );
      
        signals:
      
        public slots:
            void handleSessionAdded( Dataprocessor* );
            void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
      
        private:
            boost::scoped_ptr<ElementalCompWndImpl> pImpl_;
      
        };
    
        /////////
    }
}

#endif // ELEMENTALCOMPWND_H
