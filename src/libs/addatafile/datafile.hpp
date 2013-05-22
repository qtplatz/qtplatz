// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#ifndef DATAFILE_H
#define DATAFILE_H

#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <boost/smart_ptr.hpp>
#include <adfs/adfs.hpp>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class ProcessedDataset;
}

namespace portfolio { class Portfolio; }

namespace addatafile {

    class datafile : public adcontrols::datafile
                   , public adcontrols::LCMSDataset { 
    public:
        ~datafile();
        datafile();
        
        bool open( const std::wstring& filename, bool readonly = false );
        // bool open_qtms( const std::wstring& filename, bool readonly = false );

        //--------- implement adcontrols::datafile ----------------
        virtual void accept( adcontrols::dataSubscriber& );
        virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const;

        // create, modify and delete methods
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio&, const adcontrols::datafile& );
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio& );
        virtual bool update( const std::wstring&, boost::any& ) { return false; }

        virtual adcontrols::datafile::factory_type factory() { return 0; }

        // LCMSDataset
        virtual size_t getFunctionCount() const;
        virtual size_t getSpectrumCount( int fcn = 0 ) const;
        virtual size_t getChromatogramCount() const;
        virtual bool getTIC( int fcn, adcontrols::Chromatogram& ) const;
        virtual bool getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ) const;
		virtual size_t posFromTime( double ) const;

    private:
        bool loadContents( portfolio::Portfolio&, const std::wstring& query );
    private:
        bool mounted_;
        std::wstring filename_;
        adfs::portfolio dbf_;
        boost::scoped_ptr< adcontrols::ProcessedDataset > processedDataset_;
        //boost::any data_;
    };

}

#endif // DATAFILE_H
