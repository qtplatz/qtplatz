/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datainterpreter.hpp"
#include <socfpga/data_accessor.hpp>

using namespace socfpgainterpreter;

DataInterpreter::~DataInterpreter()
{
}

DataInterpreter::DataInterpreter()
{
}

adcontrols::translate_state
DataInterpreter::translate( adcontrols::TraceAccessor&
                            , const char * data, size_t dsize
                            , const char * meta, size_t msize, unsigned long events ) const
{
    return adcontrols::translate_error;
}

adcontrols::translate_state
DataInterpreter::translate( std::vector< socfpga::dgmod::advalue >& data, const int8_t* xdata, size_t dsize ) const
{
    socfpga::dgmod::data_accessor::deserialize( data, reinterpret_cast< const char *>( xdata ), dsize );
    return adcontrols::translate_complete;
}
