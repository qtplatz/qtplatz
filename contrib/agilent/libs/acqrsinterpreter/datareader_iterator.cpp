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

#include "datareader_iterator.hpp"
#include "datareader.hpp"

using namespace acqrsinterpreter;

DataReader_index::~DataReader_index()
{
}

DataReader_index::DataReader_index( const DataReader& t ) : reader_( t )
{
}

int64_t
DataReader_index::pos() const
{
    return 0;
}

int64_t
DataReader_index::elapsed_time() const
{
    return 0;
}

double
DataReader_index::time_since_inject() const
{
    return 0;
}

void
DataReader_index::operator ++()
{
}

bool
DataReader_index::operator == ( adcontrols::DataReader_index& t ) const
{
    return false;
}

bool
DataReader_index::operator != ( adcontrols::DataReader_index& t ) const
{
    return false;
}

//datareader.obj : error LNK2019: unresolved external symbol
//"__declspec(dllimport) public:
//__cdecl
// adcontrols::DataReader_iterator<class adcontrols::DataReader_index>::DataReader_iterator<class adcontrols::DataReader_index>(
// class std::unique_ptr<class adcontrols::DataReader_index,struct std::default_delete<class adcontrols::DataReader_index> > &&)


// "public: virtual class adcontrols::DataReader_iterator<class adcontrols::DataReader_index> const __cdecl acqrsinterpreter::DataReader::begin(void)const "

// (?begin@DataReader@acqrsinterpreter@@UEBA?BV?$DataReader_iterator@VDataReader_index@adcontrols@@@adcontrols@@XZ)
// 1>C:\Users\Toshi\src\build-x86_64\qtplatz.release_vc14\lib\qtplatz\plugins\MS-Cheminformatics\acqrsinterpreterd.dll : fatal error LNK1120: 1 unresolved externals
