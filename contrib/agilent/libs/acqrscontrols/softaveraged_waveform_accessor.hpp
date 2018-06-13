/**************************************************************************
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

#include <adacquire/datawriter.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <vector>

namespace acqrscontrols {

    class softaveraged_waveform_accessor : public adacquire::SignalObserver::DataAccess {
        std::vector< std::shared_ptr< const acqrscontrols::u5303a::waveform > >::iterator it_;
        softaveraged_waveform_accessor( const softaveraged_waveform_accessor& ) = delete;

    public:
        softaveraged_waveform_accessor();
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

        std::vector< std::shared_ptr< const acqrscontrols::u5303a::waveform > > avgd;
    };

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////

    // T := const acqrscontrols::ap240::waveform
    template< typename waveform_type >
    class softaveraged_waveform_accessor_ : public adacquire::SignalObserver::DataAccess {

        typename std::vector< std::shared_ptr< const waveform_type > >::iterator it_;
        softaveraged_waveform_accessor_( const softaveraged_waveform_accessor_& ) = delete;

    public:
        softaveraged_waveform_accessor_() {}

        size_t ndata() const override { return avgd.size(); }

        void rewind() override {
            it_ = avgd.begin();
        }
        bool next() override {
            return ++it_ != avgd.end();
        }
        uint64_t elapsed_time() const override {
            return uint64_t( (*it_)->meta_.initialXTimeSeconds * 1.0e9 );
        }
        uint64_t epoch_time() const override {
            return (*it_)->timeSinceEpoch_;
        }
        uint64_t pos() const override {
            return (*it_)->serialnumber_;
        }
        uint32_t fcn() const override {
            return ( *it_ )->method_.protocolIndex();
        }
        uint32_t events() const override {
            return (*it_)->wellKnownEvents_;
        }
        size_t xdata( std::string& ar ) const override {
            return (*it_)->serialize_xdata( ar );
        }
        size_t xmeta( std::string& ar ) const override {
            return (*it_)->serialize_xmeta( ar );    
        }

        std::vector< std::shared_ptr< const waveform_type > > avgd;
    };
    
}

