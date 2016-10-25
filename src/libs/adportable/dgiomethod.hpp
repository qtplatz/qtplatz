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

#include <string>
#include <vector>
#include <cstdint>

namespace adportable {
    namespace dg {

        enum tAction     { ioHigh, ioLow, ioPulse };

        class configuration;

        class tEvent {
        public:
            uint32_t pid_;  // corresponding to gpio pin#
            double elapsed_time_;
            tAction action_;
        };
        
        class iEvent {
        public:
            uint32_t pid_;  // corresponding to gpio pin#
            tAction action_;
        };

        class method {
        public:
            method();
            method( const method& );
            
            static bool read_json( std::istream&, method& );
            static bool write_json( std::ostream&, const method& );
            
            void setConfig( const configuration& );

            inline std::vector< iEvent >& prepare() { return prepare_; }
            inline const std::vector< iEvent >& prepare() const { return prepare_; }            

            inline std::vector< tEvent >& table() { return table_; }
            inline const std::vector< tEvent >& table() const { return table_; }

            inline const std::string& title() const { return title_; }
            inline std::string& title() { return title_; }

            inline const uint32_t& id() const { return id_; };
            inline uint32_t& id() { return id_; };
            
        private:
            uint32_t id_;
            std::string title_;
            std::vector< iEvent > prepare_;
            std::vector< tEvent > table_;
        };
        
    }
}
