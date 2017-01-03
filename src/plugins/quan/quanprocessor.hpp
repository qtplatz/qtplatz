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

#ifndef QUANPROCESSOR_HPP
#define QUANPROCESSOR_HPP

#include <map>
#include <memory>
#include <vector>

namespace adcontrols { class QuanSequence; class QuanSample;  class ProcessMethod; class QuanCalibration; }
namespace adfs { class sqlite; }
namespace adwidgets { class Progress; }

namespace quan {

    class QuanProcessor : public std::enable_shared_from_this<QuanProcessor> {
        QuanProcessor& operator = (const QuanProcessor&) = delete;
    public:
        ~QuanProcessor();
        QuanProcessor();
        QuanProcessor( const QuanProcessor& );
        QuanProcessor( std::shared_ptr< adcontrols::QuanSequence >&
                       , std::shared_ptr< adcontrols::ProcessMethod >& );

        // combine per number of threads (for counting)
        QuanProcessor( std::shared_ptr< adcontrols::QuanSequence >
                       , std::shared_ptr< adcontrols::ProcessMethod >, size_t );

        adcontrols::QuanSequence * sequence();
        const adcontrols::QuanSequence * sequence() const;
        const std::shared_ptr< adcontrols::ProcessMethod >& procmethod() const;

        typedef std::map< std::wstring, std::vector< adcontrols::QuanSample > >::iterator iterator;
        typedef std::map< std::wstring, std::vector< adcontrols::QuanSample > >::const_iterator const_iterator;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        void complete( const adcontrols::QuanSample * );
        void doCalibration( adfs::sqlite& db );
        void doQuantification( adfs::sqlite& db );
        void doCountingCalibration( adfs::sqlite& db );
        void doCountingQuantification( adfs::sqlite& db );

    protected:
        std::shared_ptr< adcontrols::QuanSequence > sequence_;
        std::shared_ptr< adcontrols::ProcessMethod > procmethod_;
        std::map< std::wstring, std::vector< adcontrols::QuanSample > > que_;
        std::shared_ptr< adwidgets::Progress > progress_;
        int progress_total_;
        int progress_count_;
    };

}

#endif // QUANPROCESSOR_HPP
