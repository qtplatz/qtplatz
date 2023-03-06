/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "ionreactionmethod.hpp"
#include "serializer.hpp"
#include "moltable.hpp"
#include "constants.hpp"
#include "chemicalformula.hpp"
#include <adportable/float.hpp>
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <array>

namespace adcontrols {

    // reference,
    // http://fiehnlab.ucdavis.edu/staff/kind/Metabolomics/MS-Adduct-Calculator/

    typedef std::pair< int32_t, int32_t > chargeRange_t;

    class IonReactionMethod::impl {
    public:
        impl() : chargeRanges_( { 1, 1 }, { 1, 1 } )
               , polarity_( adcontrols::polarity_positive )
               , i8n_{ "ESI" }
               , pos_adducts_{ { true, "+[H]+" }
                               , {  true, "-H +[H]+"   } // -e
                               , {  true, "-H2O +[H]+" } // -OH-
                               , { false, "+[Na]+"    }
                               , { false, "+[NH4]+"   }
                               , { false, "+[K]+"     }
                               , { false, "+[CH3CN H]+" }
                               , { false, "+[CH3CN Na]+" }
                               , { false, "+[CH3OH H]+" }
                               , { false, "+[(CH3)2SO H]+" }        // DMSO
                               , { false, "+[C3H8O H]+" }           // IPA
                               , { false, "+[C3H8O Na]+" }
            }
               , neg_adducts_{ { true, "-[H]+"      }
                               , {  true, "+H -[H]+" }
                               , {  true, "+[OH]-"   }
                               , { false, "+[Cl]-"   }
                               , { false, "-[H2OH]+" }
                               , { false, "-[H2Na]+" }
                               , { false, "-[H2K]+"  }
                               , { false, "+[COO]-"  }
            } {
        }

        ion_polarity polarity_;
        std::string i8n_;
        std::string description_;
        std::tuple< chargeRange_t, chargeRange_t > chargeRanges_;
        std::vector< std::pair< bool, std::string > > pos_adducts_; // if start with '-' means lose instead of add
        std::vector< std::pair< bool, std::string > > neg_adducts_;

        //----------
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            // ADDEBUG() << "----- serialize -------";
            ar & BOOST_SERIALIZATION_NVP( polarity_ );
            ar & BOOST_SERIALIZATION_NVP( i8n_ );
            ar & BOOST_SERIALIZATION_NVP( description_ );
            ar & BOOST_SERIALIZATION_NVP( std::get< 0 >( chargeRanges_ ) ); // pos
            ar & BOOST_SERIALIZATION_NVP( std::get< 1 >( chargeRanges_ ) ); // neg
            ar & BOOST_SERIALIZATION_NVP( pos_adducts_ );
            ar & BOOST_SERIALIZATION_NVP( neg_adducts_ );
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    IonReactionMethod::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    IonReactionMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version <= 4 )
            impl_->serialize( ar, version );
        else
            ar & boost::serialization::make_nvp("impl", *impl_);
    }

    ///////// XML archive ////////
    template<> void
    IonReactionMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    IonReactionMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        if ( version <= 4 )
            impl_->serialize( ar, version );
        else
            ar & boost::serialization::make_nvp("impl", *impl_);
    }
}

BOOST_CLASS_VERSION( adcontrols::IonReactionMethod::impl, 1 )

using namespace adcontrols;

IonReactionMethod::IonReactionMethod() : impl_( new impl() )
{
}

IonReactionMethod::IonReactionMethod( const IonReactionMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

IonReactionMethod&
IonReactionMethod::operator = ( const IonReactionMethod& rhs )
{
    if ( impl_ != rhs.impl_ ) {
        delete impl_;
        impl_ = new impl( *rhs.impl_ );
    }
	return *this;
}

std::vector< std::pair< bool, std::string > >&
IonReactionMethod::adducts( ion_polarity polarity )
{
    return polarity == polarity_positive ? impl_->pos_adducts_ : impl_->neg_adducts_;
}

const std::vector< std::pair< bool, std::string > >&
IonReactionMethod::adducts( ion_polarity polarity ) const
{
    return polarity == polarity_positive ? impl_->pos_adducts_ : impl_->neg_adducts_;
}

std::pair< uint32_t, uint32_t >
IonReactionMethod::chargeState( ion_polarity pol ) const
{
    return pol == polarity_positive ? std::get<0>( impl_->chargeRanges_ ) : std::get<1>( impl_->chargeRanges_ );
}

void
IonReactionMethod::chargeState( std::pair< uint32_t, uint32_t >&& t, ion_polarity pol )
{
    switch( pol ) {
    case polarity_positive:
        std::get<0>( impl_->chargeRanges_ ) = std::move( t );
        break;
    case polarity_negative:
        std::get<1>( impl_->chargeRanges_ ) = std::move( t );
        break;
    }
}

ion_polarity
IonReactionMethod::polarity() const
{
    return impl_->polarity_;
}

void
IonReactionMethod::set_polarity( ion_polarity t ) const
{
    impl_->polarity_ = t;
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const IonReactionMethod& t )
    {
        jv = boost::json::object{
            { "polarity", static_cast< uint32_t >( t.impl_->polarity_ ) }
            , { "i8n",           t.impl_->i8n_ } // ESI, APCI, MALDI, MVCI,
            , { "description",   t.impl_->description_ }
            , { "polarity_positive"
                , {{ "chargeRange", std::get< 0 >( t.impl_->chargeRanges_ ) }
                   ,{ "pos_adducts", t.impl_->pos_adducts_ }}
            }
            , { "polarity_negative"
                , {{ "chargeRange", std::get< 1 >( t.impl_->chargeRanges_ ) }
                   ,{ "neg_adducts", t.impl_->neg_adducts_ }}
            }
        };
    }


    IonReactionMethod
    tag_invoke( boost::json::value_to_tag< IonReactionMethod >&, const boost::json::value& jv )
    {
        IonReactionMethod t;
        using namespace adportable::json;

        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            extract( obj, reinterpret_cast< uint32_t& >(t.impl_->polarity_), "polarity" );
            extract( obj, t.impl_->i8n_,         "i8n" );
            extract( obj, t.impl_->description_, "description" );

            boost::json::object tpos, tneg;
            extract( obj, tpos, "polarity_positive" );
            extract( obj, tneg, "polarity_negative" );


            extract( tpos, std::get< 0 >(t.impl_->chargeRanges_), "chargeRange" );
            extract( tpos, t.impl_->pos_adducts_, "pos_adducts" );

            extract( tneg, std::get< 1 >(t.impl_->chargeRanges_), "chargeRange" );
            extract( tneg, t.impl_->neg_adducts_, "neg_adducts" );
        }
        // ADDEBUG() << "---------------------------------------\n"
        //           << boost::json::value_from( t );
        return t;
    }
}
