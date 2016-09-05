/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <AcqirisImport.h> 
#include <AcqirisD1Import.h>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

class waveform {
    waveform( const waveform& ) = delete;
    waveform& operator = ( const waveform& ) = delete;
public:
    waveform();
    inline size_t size() const {
        return dataDesc_.returnedSamplesPerSeg;
    }
    inline int16_t operator [] ( size_t idx ) const {
        return data_[ idx + dataDesc_.indexFirstPoint ];
    }
    inline std::vector< int16_t >::const_iterator begin() const {
        return data_.begin() + dataDesc_.indexFirstPoint;
    }
    inline std::vector< int16_t >::const_iterator end() const {
        return data_.begin() + dataDesc_.indexFirstPoint + size();
    }
    inline double time( size_t idx ) const {
        return delayTime_ + ( idx * dataDesc_.sampTime );
    }
    double delayTime_;
    AqDataDescriptor dataDesc_;
    AqSegmentDescriptor segDesc_;
    std::vector< int16_t > data_;
    double toVolts( int16_t ) const;
};
