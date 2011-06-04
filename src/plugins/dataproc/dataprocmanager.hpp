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

#pragma once

#include <QObject>
#include <boost/smart_ptr.hpp>

class QMainWindow;

namespace adportable {
    class Configuration;
}

namespace adcontrols {
    class datafile;
    class ProcessMethod;
}

namespace portfolio {
    class Folium;
}

namespace dataproc {
    
    class Dataprocessor;
    
    namespace internal {
        
        class DataprocManagerImpl;
        
        class DataprocManager : public QObject {
            Q_OBJECT
            public:
            explicit DataprocManager(QObject *parent = 0);
	    ~DataprocManager();
            
            QMainWindow * mainWindow() const;
            void init( const adportable::Configuration&, const std::wstring& apppath );
            void setSimpleDockWidgetArrangement();
            void OnInitialUpdate();
            void OnFinalClose();
            
            void getProcessMethod( adcontrols::ProcessMethod& );
            
        signals:
            void signalUpdateFile( adcontrols::datafile * );
            void signalGetProcessMethod( adcontrols::ProcessMethod& );
                                                                     
        public slots:
            void handleSessionAdded( Dataprocessor * );
            void handleSelectionChanged( Dataprocessor *, portfolio::Folium& );
                                                      
        private slots:
            void handleApplyMethod();
            
        private:
            DataprocManagerImpl * pImpl_;
        };

    }
}

