/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "jcb2009_helper.hpp"
#include "generator_property.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportfolio/folium.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <iterator>

using namespace adprocessor;
using namespace adprocessor::jcb2009_helper;

void
printer::print( const portfolio::Folium& folium )
{
    ADDEBUG() << "==========================================================";
    if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
        ADDEBUG() << folium.name() << "\tpeaks.size: " << chro->peaks().size();

        auto jv = adportable::json_helper::parse( chro->generatorProperty() );
        if ( jv.is_object() ) {
            if ( auto gen = adportable::json_helper::if_contains( jv, "generator.extract_by_peak_info" ) ) {
                if ( auto mv = adportable::json_helper::if_contains( *gen, "pkinfo.mass" ) )
                    ADDEBUG() << "-------- extract_by_peak_info: mass = " << mv->as_double();
            } else if ( auto gen = adportable::json_helper::if_contains( jv, "generator.extract_by_mols" ) ) {
                if ( auto mv = adportable::json_helper::if_contains( *gen, "moltable.mass" ) )
                    ADDEBUG() << "--------- extract_by_mols: moltable.mass :" << mv->as_double();
                if ( auto formula = adportable::json_helper::if_contains( *gen, "moltable.formula" ) )
                    ADDEBUG() << "--------- extract_by_mols: moltable.formula :" << formula->as_string();
            } else if ( auto gen = adportable::json_helper::if_contains( jv, "generator.extract_by_axis_range" ) ) {
                ADDEBUG() << "extract_by_axis_range: " << *gen;
            }
        }
    }
    ADDEBUG() << "==========================================================";
}

adcontrols::Peaks
find_peaks::get( const portfolio::Folium& folium )
{
    if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
        return chro->peaks();
    }
    return {};
}

std::tuple< double, double, double >
find_peaks::tR( const adcontrols::Peak& pk )
{
    auto [front,back] = pk.retentionTime().boundary();
    return { pk.peakTime(), front, back };
}

/////////////////////////

namespace adprocessor {
    namespace jcb2009_helper {

        class annotator::impl {
        public:
            portfolio::Folium folium_;
            adcontrols::MSFinder msFinder_;
            double mass_;
            std::optional< std::string > formula_;

        public:
            impl( const portfolio::Folium& folium
                  , const adcontrols::ProcessMethod& m ) : folium_( folium )
                                                         , mass_( 0 ) {

                if ( auto tm = m.find< adcontrols::TargetingMethod >() ) {
                    auto t = tm->tolerance( tm->toleranceMethod() );
                    auto a = tm->findAlgorithm();
                    auto m = tm->toleranceMethod();
                    msFinder_ = { t, a, m };
                }

                if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                    adprocessor::generator_property gprop( *chro );
                    mass_ = gprop.mass();
                    formula_ = gprop.formula();
                }
            }
        };

        annotator::~annotator()
        {
            delete impl_;
        }

        annotator::annotator( const portfolio::Folium& folium
                              , const adcontrols::ProcessMethod& m )
            : impl_( new impl( folium, m ) )
        {
        }

        std::optional< adcontrols::annotation >
        annotator::operator()( const adcontrols::MassSpectrum& centroid
                               , const std::tuple<double,double,double>& tR )
        {
            typedef adcontrols::MassSpectrum T;
            int fcn(0);
            if ( impl_->mass_ > 0 ) {
                for ( auto& ms: adcontrols::segment_wrapper< const T >( centroid ) ) {
                    auto idx = (impl_->msFinder_)( ms, impl_->mass_ );
                    if ( idx != adcontrols::MSFinder::npos ) {

                        auto text = (boost::format("%.3f@%.1fs") % ms.mass(idx) % std::get<0>(tR)).str();
                        return adcontrols::annotation( text, ms.mass( idx ), ms.intensity( idx ), static_cast< int >(idx) );

                    } else {
                        ADDEBUG() << "### annotator -- mass " << impl_->mass_ << "@" << tR << " s (W " << impl_->msFinder_.width() << ") not found.";
                    }
                }
            }
            return {};
        }

        void
        annotator::operator()( std::shared_ptr< adcontrols::MSPeakInfo > pInfo )
        {
            typedef adcontrols::MSPeakInfo T;
            if ( impl_->mass_ > 0 ) {
                for ( auto& info: adcontrols::segment_wrapper< adcontrols::MSPeakInfo >( *pInfo ) ) {
                    auto it = (impl_->msFinder_)( info, impl_->mass_ );
                    if ( it != info.end() ) {
                        // ADDEBUG() << "### found info ###" << it->toJson(); //std::distance( info.begin(), it );
                        it->annotation( impl_->folium_.name() );
                    }
                }
            }
        }

        //////////////////////////////////////////
    }
}
