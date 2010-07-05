// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {
	namespace internal {

		class Element {
		public:
			Element();
			Element( const std::wstring& symbol, const std::wstring& name, int atomicNumber, int valence );
			Element( const Element& );
      
			class Isotope {
			public:
				Isotope( double mass = 0, double abund = 0 ) : mass_(mass), abundance_(abund) {}
				double mass_;
				double abundance_;
			private:
				friend class boost::serialization::access;
				template<class Archive>
				void serialize(Archive& ar, const unsigned int version) {
					if ( version >= 0 ) {
						ar & BOOST_SERIALIZATION_NVP(mass_);
						ar & BOOST_SERIALIZATION_NVP(abundance_);
					}
				}
			};
      
			const std::wstring& symbol() const;
			const std::wstring& name() const;
			int atomicNumber() const;
			int valence() const;
			size_t isotopeCount() const;
			const Isotope& operator [] ( int idx ) const;
			void addIsotope( const Isotope& );

		private:
			std::wstring symbol_;
			std::wstring name_;
			int atomicNumber_;
			int valence_;
			std::vector< Isotope > isotopes_;

			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive& ar, const unsigned int version) {
				if ( version >= 0 ) {
					ar & BOOST_SERIALIZATION_NVP(name_);
					ar & BOOST_SERIALIZATION_NVP(symbol_);
					ar & BOOST_SERIALIZATION_NVP(atomicNumber_);
					ar & BOOST_SERIALIZATION_NVP(valence_);
					ar & BOOST_SERIALIZATION_NVP(isotopes_);
				}
			}
		};

		class SuperAtom {
		public:
			SuperAtom();
			SuperAtom( const std::wstring& name, const std::wstring& alias_, const std::wstring& formula_, int valence );
			SuperAtom( const SuperAtom& );

			std::wstring name_;
			std::wstring alias_;
			std::wstring formula_;
			int valence_;
		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive& ar, const unsigned int version) {
				if ( version >= 0 ) {
					ar & BOOST_SERIALIZATION_NVP(name_);
					ar & BOOST_SERIALIZATION_NVP(alias_);
					ar & BOOST_SERIALIZATION_NVP(formula_);
					ar & BOOST_SERIALIZATION_NVP(valence_);
				}
			}
		};
	}
}

BOOST_CLASS_VERSION(adcontrols::internal::Element, 1)
BOOST_CLASS_VERSION(adcontrols::internal::Element::Isotope, 1)
BOOST_CLASS_VERSION(adcontrols::internal::SuperAtom, 1)

#endif // ELEMENT_H
