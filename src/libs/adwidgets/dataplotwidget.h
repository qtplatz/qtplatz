// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPLOTWIDGET_H
#define DATAPLOTWIDGET_H

#include "dataplot.h"
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace adwidgets {
    namespace ui {

        namespace internal {
            class DataplotWidgetImpl;
        }

        class DataplotWidget : public Dataplot {
            Q_OBJECT

        public:
            virtual ~DataplotWidget();
            explicit DataplotWidget(QWidget *parent = 0);
            enum { eVK_SHIFT
                , eVK_CONTROL 
                , eVK_MENU 
            };

            void link( DataplotWidget * );
            void unlink( DataplotWidget * );

        private:
            bool init();

            // Dataplot class
            virtual void OnMouseDown( double x, double y, short button );
            virtual void OnMouseUp( double x, double y, short Button );
            virtual void OnMouseMove( double x, double y, short Button );
            virtual void OnCharacter( long KeyCode );
            virtual void OnKeyDown( long KeyCode );
            virtual void OnSetFocus( long hWnd );
            virtual void OnKillFocus( long hWnd );
            virtual void OnMouseDblClk( double x, double y, short button );

        public:
            const std::pair<double, double>& display_range_x() const;
            const std::pair<double, double>& display_range_y() const;
            void display_range_x( const std::pair<double, double>& );
            void display_range_y( const std::pair<double, double>& );
            void display_range_y2( const std::pair<double, double>& );

        signals:
            friend internal::DataplotWidgetImpl;
            void signalZoomXY( double x1, double y1, double x2, double y2 );
            void signalZoomXAutoscaleY( double x1, double x2 );

        public slots:
            virtual void handleZoomXY( double x1, double y1, double x2, double y2 );
            virtual void handleZoomXAutoscaleY( double x1, double x2 );

        protected slots:

        private:
            internal::DataplotWidgetImpl * pImpl_;
        };

    }
}

#endif // DATAPLOTWIDGET_H
