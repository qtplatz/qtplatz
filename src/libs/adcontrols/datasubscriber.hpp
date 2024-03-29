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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATASUBSCRIBER_H
#define DATASUBSCRIBER_H

#include "adcontrols_global.h"
#include <string>
#include <memory>

namespace adfs { class file; class sqlite; }

namespace adcontrols {

    class dataPublisher;
    class AcquiredDataset;
    class LCMSDataset;
    class ProcessedDataset;

    class ADCONTROLSSHARED_EXPORT dataSubscriber {  // visitable
    public:
        virtual ~dataSubscriber();
        dataSubscriber();
        enum idError { idUndefinedSpectrometers };

        virtual bool subscribe( const LCMSDataset& ) { return false; }
        virtual bool subscribe( const ProcessedDataset& ) { return false; }
		virtual bool onFileAdded( const std::wstring& /* path */, adfs::file& ) { return false; }
        virtual void notify( idError, const std::string& json ) { }
        virtual std::shared_ptr< adfs::sqlite > db() const { return nullptr; }
    };

}

#endif // DATASUBSCRIBER_H
