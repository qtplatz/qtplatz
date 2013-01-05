// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/shared_ptr.hpp>

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
        explicit ChromatogramWnd( const std::wstring&, QWidget *parent = 0);
        ~ChromatogramWnd();
        
        void init( const std::wstring& );
        void draw1( boost::shared_ptr< adcontrols::MassSpectrum >& );
        void draw2( boost::shared_ptr< adcontrols::MassSpectrum >& );
        void draw( boost::shared_ptr< adcontrols::Chromatogram >& );
        void draw( boost::shared_ptr< adcontrols::PeakResult >& );
        
    signals:
        void fireSetData( const adcontrols::PeakResult& );
                                                         
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void onApplyMethod( const adcontrols::ProcessMethod& );
        
    private:
        boost::shared_ptr<ChromatogramWndImpl> pImpl_;
        
    };
}

#endif // CHROMATOGRAMWND_H
