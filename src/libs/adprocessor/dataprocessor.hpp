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

#pragma once

#include "adprocessor_global.hpp"
#include <adcontrols/datasubscriber.hpp>
#include <QObject>
#include <QString>
#include <memory>
#include <vector>

namespace adfs { class filesystem; class sqlite; }

namespace adcontrols {
    class datafile;
    class LCMSDataset;
    class ProcessMethod;
    class MassSpectrum;
    class MassSpectra;
    class Chromatogram;
    class MSAssignedMasses;
    class MSCalibrateMethod;
	class MSCalibrateResult;
    class CentroidMethod;
    class SpectrogramClusters;
    namespace lockmass { class mslock; }
}

namespace adprocessor {

    class ADPROCESSORSHARED_EXPORT dataprocessor : public QObject
                                               , public adcontrols::dataSubscriber {
        
        Q_OBJECT
        
    public:
        virtual ~dataprocessor();
        dataprocessor();

        // dataprocessor
        virtual void setModified( bool );

        virtual bool open( const QString&, QString& errmsg );
        
		const QString& filename() const;

        virtual adcontrols::datafile * file();
        
        virtual const adcontrols::LCMSDataset * rawdata();

        std::shared_ptr< adfs::sqlite > db() const;

        // implement adcontrols::dataSubscriber
        virtual bool subscribe( const adcontrols::LCMSDataset& ) override;
        virtual void notify( adcontrols::dataSubscriber::idError, const wchar_t * ) override;

    private:

    private slots:

    signals :
        void onNotify( const QString& );

    private:
        std::unique_ptr< adfs::filesystem > fs_;
        std::unique_ptr< adcontrols::datafile > file_;
        const adcontrols::LCMSDataset * rawdata_;
        bool modified_;
    };

} // mpxcontrols


