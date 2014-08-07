// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#ifndef DATAPROC_HPP
#define DATAPROC_HPP

#pragma once

#include "dataproc_global.hpp"
#include <extensionsystem/iplugin.h>
#include <memory>

namespace adcontrols {
	class ProcessMethod;
	class MassSpectrum;
}

namespace dataproc {

    class Document;
    class SessionManager;
    class EditorFactory;
    class MainWindow;
    class Mode;

    class DataprocManager;
    class ActionManager;
    class DataprocessorFactory;
    class iSequenceImpl;
    class iSnapshotHandlerImpl;
	class iPeptideHandlerImpl;

    class DataprocPlugin : public ExtensionSystem::IPlugin {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "com.ms-cheminfo.QtPlatzPlugin" FILE "Dataproc.json")

    public:
        DataprocPlugin();
        ~DataprocPlugin();

        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();

        //---
        inline static DataprocPlugin * instance() { return instance_; }
        inline static MainWindow * mainWindow() { return instance_ ? instance_->mainWindow_.get() : 0; }

        void applyMethod( const adcontrols::ProcessMethod& );
        void onSelectTimeRangeOnChromatogram( double x1, double x2 );
        void onSelectSpectrum( double minutes, size_t pos, int fcn );
        DataprocessorFactory * dataprocessorFactory() { return dataprocFactory_.get(); }
        dataproc::ActionManager * actionManager() { return pActionManager_.get(); }
        void handleFileCreated( const QString& filename );

    private:
        enum { idActSpectrogram, nActions };

        static DataprocPlugin * instance_;

        std::unique_ptr< MainWindow > mainWindow_;
        std::unique_ptr< Mode > mode_;

        std::unique_ptr< SessionManager > pSessionManager_;
        std::unique_ptr< ActionManager > pActionManager_;
        std::unique_ptr< DataprocessorFactory > dataprocFactory_;
        std::unique_ptr< iSequenceImpl > iSequence_;
        std::unique_ptr< iSnapshotHandlerImpl > iSnapshotHandler_;
        std::unique_ptr< iPeptideHandlerImpl > iPeptideHandler_;

        //static DataprocPlugin * instance_;
        // static bool install_dataprovider( const adportable::Configuration&, const std::wstring& );
        // static bool install_isequence( const adportable::Configuration&, const std::wstring&, iSequenceImpl& );
        static void delete_editorfactories( std::vector< EditorFactory * >& );
        
        bool connect_isnapshothandler_signals();
        void disconnect_isnapshothandler_signals();

    signals:
        void onApplyMethod( const adcontrols::ProcessMethod& );
            
    private slots:
        // void triggerAction();
        void handle_portfolio_created( const QString token );
        void handle_folium_added( const QString, const QString, const QString );
    };
    
} // namespace Dataproc

#endif // DATAPROC_HPP

