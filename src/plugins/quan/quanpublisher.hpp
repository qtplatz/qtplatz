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

#ifndef QUANPUBLISHER_HPP
#define QUANPUBLISHER_HPP

#include <boost/uuid/uuid.hpp>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <ostream>

namespace pugi { class xml_document; class xml_node; }
namespace adcontrols { class MassSpectrum; class MSPeakInfo; }
namespace adfs { class stmt; }
namespace adwidgets { class ProgressInterface; }

namespace quan {

    class QuanDocument;
    class QuanConnection;
    class QuanPlotData;
    class ProgressHandler;

    class QuanPublisher {
    public:
        QuanPublisher( const QuanPublisher& ); //= delete;
        QuanPublisher();

        operator bool () const { return bProcessed_; }

        bool operator()( QuanConnection * );
        bool operator()( QuanConnection *, std::function<void(int)> progress, pugi::xml_document * article = 0 );

        const std::filesystem::path& filepath() const;
        bool save_file( const char * filepath ) const;
        bool save( std::ostream& ) const;

        struct resp_data {
            boost::uuids::uuid cmpId;
            int64_t respId;
            std::string name;
            int sampType;
            std::string formula;
            double mass;
            double mass_error;
            double intensity;
            double amount;
            std::string dataSource;
            std::string dataGuid;
            int fcn;
            size_t idx;
            int level;
            resp_data() : cmpId({}), respId(0), sampType(0), mass(0), mass_error(0), intensity(0), amount(0), fcn(0), idx(0), level(0) {}
            resp_data( const resp_data& t ) = delete;
        };

        struct calib_curve {
            boost::uuids::uuid uuid;     // cmpdId
            std::string formula;
            std::wstring description;
            size_t n;
            double min_x;
            double max_x;
            std::string date;
            std::vector< double > coeffs;
            std::map< int, double > std_amounts;
            std::vector< std::pair< int, int64_t > > respIds;
            std::vector< std::pair< double, double > > xy;
            calib_curve() : uuid({}), n(0), min_x(0), max_x(0) {}
            calib_curve( const calib_curve& t ) = delete;
        };

        const calib_curve * find_calib_curve( const boost::uuids::uuid& );

        bool appendTraceData( adwidgets::ProgressInterface& progress );

    private:
        bool bProcessed_;
        std::shared_ptr< QuanConnection > conn_;
        std::shared_ptr< pugi::xml_document > xmloutput_;
        std::filesystem::path filepath_;

        std::map< boost::uuids::uuid, std::shared_ptr< calib_curve > > calib_curves_; // cmpdId, curve
        std::map< int64_t, std::shared_ptr< resp_data > > resp_data_;

        bool appendSampleSequence( pugi::xml_node& );
        bool appendProcessMethod( pugi::xml_node& );
        bool appendQuanResponseUnk( pugi::xml_node& );
        bool appendQuanResponseStd( pugi::xml_node& );
        bool appendQuanCalib( pugi::xml_node& );
        bool appendCountingCalib( pugi::xml_node&, bool );
        bool appendQuanDataGuids( pugi::xml_node& );
        bool appendTraceData( pugi::xml_node dst, const pugi::xml_node& response, const std::string& refGuid, int idx, int fcn );
        bool appendPlot( pugi::xml_node& dst, const QuanPlotData&, size_t idx, int fcn, const std::string& );
        bool appendMSPeakInfo( pugi::xml_node& dst, const adcontrols::MSPeakInfo&, size_t idx, int fcn );
        bool prepare_document();
    };

}

#endif // QUANPUBLISHER_HPP
