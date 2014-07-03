/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <compiler/disable_dll_interface.h>
#include <memory>
#include <map>

namespace adcontrols {

    class MassSpectrum;
    class MSQPeak;

    class ADCONTROLSSHARED_EXPORT MSQPeaks : public std::enable_shared_from_this< MSQPeaks > {
    public:
        ~MSQPeaks();
        MSQPeaks();
        MSQPeaks( const MSQPeaks& );
        static const wchar_t * dataClass() { return L"adcontrols::MSQPeaks"; }

        typedef MSQPeak value_type;
        typedef std::vector< value_type >::iterator iterator_type;
        typedef std::vector< value_type >::const_iterator const_iterator_type;

        void clear();
        size_t size() const;
        iterator_type begin();
        iterator_type end();
        const_iterator_type begin() const;
        const_iterator_type end() const;
        bool erase( const std::wstring& profGuid );
        iterator_type erase( iterator_type );
        iterator_type erase( iterator_type first, iterator_type last );
        MSQPeaks& operator << ( const MSQPeak& );
        const MSQPeak& operator [] ( size_t idx ) const;
        iterator_type find( const std::wstring& dataGuid, int idx, int fcn );
        void setData( const MassSpectrum&, const std::wstring& dataGuid, const std::wstring& profileGuid, const std::wstring& dataSource );
        bool replace( const MassSpectrum&, const std::wstring& dataGuid, const std::wstring& profileGuid, int idx, int fcn );
        const std::wstring& parentGuid( const std::wstring& ) const;
        const std::wstring& dataSource( const std::wstring& ) const;
        
    private:
        std::vector< value_type > vec_;
        std::map< std::wstring, std::pair< std::wstring, std::wstring > > ident_; // dataGuid, <profGuid, dataSource>
        std::map< std::wstring, std::map< uint32_t, std::wstring > > prot_texts_; // dataGuid, <fcn, text>

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            (void)(version);
            ar & vec_
                & ident_
                & proto_texts_;
        }
        
    };

    typedef std::shared_ptr<MSQPeaks> MSQPeaksPtr;   

}

