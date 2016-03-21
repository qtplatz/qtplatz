// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#ifndef CHROMATOGRAMWND_H
#define CHROMATOGRAMWND_H

#include <QWidget>
#include <memory>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
	class PeakResult;
    class ProcessMethod;
}

namespace dataproc {

    class Dataprocessor;

    class ChromatogramWndImpl;
    
    class ChromatogramWnd : public QWidget {
        Q_OBJECT
    public:
        explicit ChromatogramWnd( QWidget *parent = 0 );
        ~ChromatogramWnd();
        
        void init();
        void draw1( std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw2( std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw( std::shared_ptr< adcontrols::Chromatogram >& );
        void draw( std::shared_ptr< adcontrols::PeakResult >& );
        
    signals:
                                                         
    public slots :
        void handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool );
        void handleSessionAdded( Dataprocessor* );
        void handleProcessed( Dataprocessor*, portfolio::Folium& );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handlePrintCurrentView( const QString& );
        
    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };
}

#endif // CHROMATOGRAMWND_H
