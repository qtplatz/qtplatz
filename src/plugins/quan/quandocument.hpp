/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef QUANDOCUMENT_HPP
#define QUANDOCUMENT_HPP

#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <functional>

namespace adcontrols { class QuanMethod; class QuanCompounds; }

namespace quan {

    class PanelData;

    enum idDataChanged {
        idQuanMethod
        , idQuanCompounds
    };

    class QuanDocument {
        ~QuanDocument();
        QuanDocument();
        static QuanDocument * instance_;
        static std::mutex mutex_;
    public:
        static QuanDocument * instance();

        PanelData * addPanel( int idx, int subIdx, std::shared_ptr< PanelData >& );
        PanelData * findPanel( int idx, int subIdx, int pos );
        const adcontrols::QuanMethod& quanMethod();
        void quanMethod( const adcontrols::QuanMethod & );
        const adcontrols::QuanCompounds& quanCompounds();
        void quanCompounds( const adcontrols::QuanCompounds& );
        void register_dataChanged( std::function< void( int ) > );

    private:
        typedef std::vector< std::shared_ptr< PanelData > > page_type;
        typedef std::map< int, page_type > chapter_type;
        std::map< int, chapter_type > book_;

        std::shared_ptr< adcontrols::QuanMethod > quanMethod_;
        std::shared_ptr< adcontrols::QuanCompounds > quanCompounds_;

        std::vector< std::function< void( int ) > > clients_;
    };
}

#endif // QUANDOCUMENT_HPP
