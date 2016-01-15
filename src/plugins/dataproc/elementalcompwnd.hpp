// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#ifndef ELEMENTALCOMPWND_H
#define ELEMENTALCOMPWND_H

#include <QWidget>
#include <memory>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class MassSpectrum;
	class ProcessMethod;
    class MSSimulatorMethod;
    enum hor_axis;
}

namespace dataproc {

    class Dataprocessor;

    class ElementalCompWndImpl;
    
    class ElementalCompWnd : public QWidget {
        Q_OBJECT
    public:
        explicit ElementalCompWnd(QWidget *parent = 0);
        ~ElementalCompWnd();
        void init();
        void draw1( std::shared_ptr< adcontrols::MassSpectrum >& );
        // void draw2( std::shared_ptr< adcontrols::MassSpectrum >& );
        void simulate( const adcontrols::MSSimulatorMethod& );
        void estimateScanLaw( const QString&, std::shared_ptr< adcontrols::MassSpectrum >& );
      
    signals:
      
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleProcessed( Dataprocessor*, portfolio::Folium& );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handleAxisChanged( adcontrols::hor_axis );        
      
    private:
        ElementalCompWndImpl * pImpl_;

    private slots:
        void selectedOnProcessed( const QRectF& );
    };
    
}

#endif // ELEMENTALCOMPWND_H
