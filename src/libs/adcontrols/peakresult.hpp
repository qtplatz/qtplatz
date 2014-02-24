/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef PEAKRESULT_HPP
#define PEAKRESULT_HPP

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class Peaks;
	class Baselines;

    class ADCONTROLSSHARED_EXPORT PeakResult {
	public:
		~PeakResult();
		PeakResult();
		PeakResult( const PeakResult& );
		PeakResult( const Baselines&, const Peaks& );
		static const wchar_t * dataClass() { return L"PeakResult"; }

		const Baselines& baselines() const;
		Baselines& baselines();

		const Peaks& peaks() const;
		Peaks& peaks();

	private:
#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif
		boost::scoped_ptr< Baselines > baselines_;
		boost::scoped_ptr< Peaks > peaks_;

        friend class boost::serialization::access;
		template<class Archive>
        void serialize(Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(baselines_);
            ar & BOOST_SERIALIZATION_NVP(peaks_);
        }
	public:
		static bool archive( std::ostream&, const PeakResult& );
		static bool restore( std::istream&, PeakResult& );
	};

	typedef std::shared_ptr<PeakResult> PeakResultPtr;
}

#endif // PEAKRESULT_HPP
