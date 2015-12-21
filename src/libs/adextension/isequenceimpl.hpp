/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <adextension/isequence.hpp>
#include <vector>
#include <memory>

namespace adextension {

    class iSequenceImpl : public adextension::iSequence {
        QString configname_;

    public:
        iSequenceImpl() {}
        iSequenceImpl( const QString& configname ) : configname_( configname ) {}
        
        size_t size() const override { return v_.size(); }

        const_reference operator [] ( size_t idx ) const override {
            if ( idx >= v_.size() )
                throw std::runtime_error( "subscript out-of-range" );
            return * ( v_[ idx ].get() );
        }

        const QString& configuration() const override { return configname_; }
        
        iSequenceImpl& operator << ( std::shared_ptr< const adextension::iEditorFactory > ptr ) {
            v_.push_back( ptr );
            return *this;
        }
        
    private:
        std::vector< std::shared_ptr< const adextension::iEditorFactory > > v_;
    };

}

