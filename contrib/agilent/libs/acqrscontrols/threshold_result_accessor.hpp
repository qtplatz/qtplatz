/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <adicontroller/datawriter.hpp>
#include <vector>

namespace acqrscontrols {

    class threshold_result_accessor : public adicontroller::SignalObserver::DataAccess {

    public:
        threshold_result_accessor();
        size_t ndata() const override;

        void rewind() override;
        bool next() override;
        uint64_t elapsed_time() const override;
        uint64_t epoch_time() const override;
        uint64_t pos() const override;
        uint32_t fcn() const override;
        uint32_t events() const override;
        size_t xdata( std::string& ) const override;
        size_t xmeta( std::string& ) const override;

        // local impl
        std::shared_ptr< const acqrscontrols::u5303a::threshold_result > data() { return *it_; }
        
        std::vector< std::shared_ptr< const acqrscontrols::u5303a::threshold_result > > list;

    private:
        std::vector< std::shared_ptr< const acqrscontrols::u5303a::threshold_result > >::iterator it_;
    };

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////

    // T := const acqrscontrols::ap240::threshold_result

    template< typename T >
    class threshold_result_accessor_ : public adicontroller::SignalObserver::DataAccess {

    public:
        threshold_result_accessor_() {}

        size_t ndata() const override { return list.size(); }

        void rewind() override { it_ = list.begin(); }
        bool next() override   { return ++it_ != list.end(); }
        uint64_t elapsed_time() const override;
        uint64_t epoch_time() const override;
        uint64_t pos() const override;
        uint32_t fcn() const override;
        uint32_t events() const override;
        size_t xdata( std::string& ) const override;
        size_t xmeta( std::string& ) const override;

        // local impl
        std::shared_ptr< const T > data() { return *it_; }
        
        std::vector< std::shared_ptr< const T > > list;

    private:
        typename std::vector< std::shared_ptr< const T > >::iterator it_;
    };

    template<> class threshold_result_accessor_< const acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > >;
    template<> class threshold_result_accessor_< const acqrscontrols::u5303a::threshold_result >;
}
