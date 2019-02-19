/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "protein.hpp"
#include <algorithm>
#include <cctype>

using namespace adprot;

protein::protein()
{
}

protein::protein( const protein& t ) : name_( t.name_ )
                                     , sequence_( t.sequence_ )
                                     , gene_( t.gene_ )
                                     , organism_( t.organism_ )
                                     , url_( t.url_ )
{
}

protein::protein( const std::string& name
                  , const std::string& sequence ) : name_( name )
                                                  , sequence_( sequence )
{
#if defined __linux__
    sequence_.erase( std::remove_if( sequence_.begin(), sequence_.end(), [](const char c){return std::isspace(c);}), sequence_.end() );
#else
    sequence_.erase( std::remove_if( sequence_.begin(), sequence_.end(), std::isspace ), sequence_.end() );
#endif
}

protein::protein( const std::string& name
                  , const std::string& gene
                  , const std::string& organism
                  , const std::string& url
                  , const std::string& sequence ) : name_( name )
                                                  , sequence_( sequence )
                                                  , gene_( gene )
                                                  , organism_( organism )
                                                  , url_( url )
{
    sequence_.erase( std::remove_if( sequence_.begin(), sequence_.end(), [](const char c){return std::isspace(c);}), sequence_.end() );
}

const std::string&
protein::name() const
{
    return name_;
}

void
protein::set_name( const std::string& var )
{
    name_ = var;
}


const std::string&
protein::sequence() const
{
    return sequence_;
}

void
protein::set_sequence( const std::string& var )
{
    sequence_ = var;
}

const std::string&
protein::gene() const
{
    return gene_;
}

void
protein::set_gene( const std::string& t )
{
    gene_ = t;
}

const std::string&
protein::url() const
{
    return url_;
}

void
protein::set_url( const std::string& t )
{
    url_ = t;
}

const std::string&
protein::organism() const
{
    return organism_;
}

void
protein::set_organism( const std::string& t )
{
    organism_ = t;
}
