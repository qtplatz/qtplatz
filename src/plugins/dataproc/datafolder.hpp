/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
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

#include <string>
#include <memory>
#include <QString>
#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>
#include <adportfolio/folium.hpp>


namespace adcontrols { class MassSpectrum; class Chromatogram; class PeakResult; }
namespace adprocessor { class dataprocessor; }

namespace dataproc {

    class Dataprocessor;

    class datafolder {
        int idx_;
        std::string filename_;
        QString display_name_; // fileneme::folium.name
        portfolio::Folium folium_;
        // std::wstring idFolium_;
        // boost::uuids::uuid idfolium_;
        // std::wstring idCentroid_;

        std::weak_ptr< adcontrols::MassSpectrum > primary_;   // usually profile, TBD for histogram data
        std::weak_ptr< adcontrols::MassSpectrum > profiledHistogram_;
        std::weak_ptr< adcontrols::MassSpectrum > centroid_;  // centroid
        std::weak_ptr< adcontrols::Chromatogram > chromatogram_;

        std::shared_ptr< adcontrols::PeakResult > peakResult_;
        std::shared_ptr< adcontrols::MassSpectrum > overlaySpectrum_; // y-scale normalized
        std::shared_ptr< adcontrols::Chromatogram > overlayChromatogram_;
        std::shared_ptr< adcontrols::MassSpectrum > self_profiled_histogram_; // locally created
        bool isCounting_;
        bool isChecked_;
    public:
        datafolder();
        // datafolder( const std::wstring& filename, const portfolio::Folium& folium );
        datafolder( const Dataprocessor *, const portfolio::Folium& folium );
        datafolder( const datafolder& t );

        operator bool () const;

        int idx() const;
        void setIdx( int );

        const std::string& filename() const;
        QString display_name() const;
        portfolio::Folium& folium();
        const portfolio::Folium& folium() const;
        boost::uuids::uuid uuid() const;
        [[deprecated]] std::wstring idFolium() const { return folium_.id<wchar_t>(); }

        bool isCounting() const { return isCounting_; } // a.k.a. PKD data source
        bool isChecked() const { return isChecked_; }

        std::shared_ptr< const adcontrols::Chromatogram > overlayChromatogram() const { return overlayChromatogram_; }
        void setOverlayChromatogram( std::shared_ptr< adcontrols::Chromatogram >&& ptr ) { overlayChromatogram_ = std::move( ptr ); }

        boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool > > get_spectrum_for_overlay() const;
        boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */> > get_primary() const;
        boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */> > get_processed() const;
        std::shared_ptr< const adcontrols::MassSpectrum > get_primary_spectrum() const;
        std::shared_ptr< const adcontrols::MassSpectrum > get_profiled_histogram() const;

        std::shared_ptr< adcontrols::Chromatogram > get_chromatogram() const;
        std::shared_ptr< adcontrols::Chromatogram > get_overlayChromatogram() const;
        std::shared_ptr< adcontrols::PeakResult > get_peakResult() const;

        static QString make_display_name( const std::wstring& fullpath, const portfolio::Folium& );
        static QString make_display_name( Dataprocessor *, const portfolio::Folium& );

        template< typename container > static
        typename container::const_iterator find( const container& v, const boost::uuids::uuid& uuid ) {
            return std::find_if( v.begin(), v.end(), [&]( const auto& a ){ return a.id() == uuid; } );
        }
    private:
        class folium_visitor;
        class attachment_visitor;
    };
}
