// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <memory>
#include <vector>
#include <pugixml.hpp>
#include <boost/json/fwd.hpp>

namespace mzml {

    class mzMLChromatogram;
    class mzMLSpectrum;

    class node_only {
    protected:
        pugi::xml_node node_;
        node_only( const pugi::xml_node& node );
    public:
        const pugi::xml_node& node() const;
        virtual std::string toString() const;
    };

    class fileDescription : public node_only {
    public:
        fileDescription( const pugi::xml_node& node );
        fileDescription& operator = ( const fileDescription& );
    };

    class softwareList : public node_only {
    public:
        softwareList( const pugi::xml_node& node );
        softwareList& operator = ( const softwareList& );
    };

    class instrumentConfigurationList : public node_only {
    public:
        instrumentConfigurationList( const pugi::xml_node& node );
        instrumentConfigurationList& operator = ( const instrumentConfigurationList& );
    };

    class dataProcessingList : public node_only {
    public:
        dataProcessingList( const pugi::xml_node& node );
        dataProcessingList& operator = ( const dataProcessingList& );
    };
    void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const fileDescription& );
    void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const softwareList& );
    void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const instrumentConfigurationList& );

    using spectra_t = std::vector< std::shared_ptr< mzml::mzMLSpectrum > >;
    using chromatograms_t = std::vector< std::shared_ptr< mzml::mzMLChromatogram > >;

    using data_t = std::variant< fileDescription
                                 , softwareList
                                 , instrumentConfigurationList
                                 , dataProcessingList
                                 , spectra_t
                                 , chromatograms_t >;

    class mzMLWalker {
    public:
        ~mzMLWalker();
        mzMLWalker();
        std::vector< data_t > operator()( const pugi::xml_node& root_node );
    };

}
