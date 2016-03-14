/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "moltable.hpp"
#include "mspeakinfoitem.hpp"

namespace adcontrols {

    namespace mschromatogramextractor {

        template< hor_axis >
        class xChromatogram {
            xChromatogram( const xChromatogram& ) = delete;
            xChromatogram& operator = ( const xChromatogram& ) = delete;
        public:
            xChromatogram( const moltable::value_type& target
                           , double width
                           , uint32_t fcn
                           , uint32_t target_index ) : fcn_( fcn )
                                                     , target_index_( target_index )
                                                     , count_( 0 )
                                                     , target_( target )
                                                     , pchr_( std::make_shared< adcontrols::Chromatogram >() )  {
                pchr_->addDescription(
                    adcontrols::description( L"Create"
                                                 , ( boost::wformat( L"%s %.4f (W:%.4gmDa) #%d" )
                                                 % adportable::utf::to_wstring( target.formula() )
                                                 % target.mass()
                                                 % ( width * 1000 )
                                                 % fcn_
                                                 ).str() ) );
                
            }
            
            xChromatogram( const std::pair< int, adcontrols::MSPeakInfoItem >& range
                           , uint32_t idx ) : fcn_( std::get< 0 >( range ) )
                                            , target_index_( idx )
                                            , count_( 0 )
                                            , pchr_( std::make_shared< adcontrols::Chromatogram >() )  {
                
                    double mass = range.second.mass();
                    double width = range.second.widthHH();
                    pchr_->addDescription( adcontrols::description( L"Create"
                        , ( boost::wformat( L"m/z %.4lf (W:%.4gmDa) #%d" ) % mass % ( width * 1000 ) % fcn_ ).str() ) );

            }

            xChromatogram( const wchar_t * debug ) : fcn_( 0 ), target_index_( 0 ), pos_( 0 ), count_( 0 )
                                                      , pchr_( std::make_shared< adcontrols::Chromatogram >() ) {
                pchr_->addDescription( adcontrols::description( L"Create", debug ) );
            }

            void append( uint32_t pos, double time, double y ) {

                if ( count_++ == 0 && pos > 0 )
                    return; // ignore first data after chromatogram condition change

                (*pchr_) << std::make_pair( time, y );
                pos_ = pos;

            }

            uint32_t fcn_;
            uint32_t target_index_;
            uint32_t pos_; // last pos
            uint32_t count_; // data count
            moltable::value_type target_;
            std::shared_ptr< adcontrols::Chromatogram > pchr_;
        };
    }
}

