// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include <map>
#include <string>
#include <vector>
#include <cstdint>

namespace adio {

    class ioEvent {
    public:
        ioEvent() {
        }

        ioEvent( const std::string& id
                 , const std::string& action ) : id_( id )
                                               , action_( action ) {
        }
        
        ioEvent( const ioEvent& t ) : id_( t.id_ )
                                    , action_( t.action_ ) {
        }
        
        const std::string& id() const { return id_; }
        void setId( const std::string& id ) { id_ = id; }

        const std::string& action() const { return action_; }
        void setAction( const std::string& a ) { action_ = a; }

    private:
        std::string id_;
        std::string action_; // 'disable', 'open/load' 'close/inject', 'pulse'
    };

    class ioEventSequence {
    public:
        ioEventSequence();
        ioEventSequence( const ioEventSequence& );
        typedef std::pair< double, std::array< ioEvent, 4 > > row_type;

        double methodTime() const;
        void setMethodTime( double );

        static bool read_json( std::istream&, ioEventSequence& );
        static bool write_json( std::ostream&, const ioEventSequence&, bool = true );
            
        inline std::vector< row_type >& sequence() { return sequence_; }
        inline const std::vector< row_type >& sequence() const { return sequence_; }

    private:
        double methodTime_;
        std::vector< row_type > sequence_;
    };
}

