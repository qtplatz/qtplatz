/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "controlmethodhelper.hpp"
#include "method.hpp"
#include <adportable/debug.hpp>

using namespace adinterface;

ControlMethodHelper::ControlMethodHelper()
{
}

ControlMethodHelper::ControlMethodHelper( const ControlMethodHelper& t ) : method_(t.method_)
{
}

ControlMethodHelper::ControlMethodHelper( const ControlMethod::Method& m ) : method_(m)
{
}

const wchar_t *
ControlMethodHelper::subject() const 
{
    return method_.subject.in();
}

void
ControlMethodHelper::subject( const std::wstring& text )
{
    method_.subject = CORBA::wstring_dup( text.c_str() );
}

const wchar_t *
ControlMethodHelper::description() const 
{
    return method_.description.in();
}

void
ControlMethodHelper::description( const std::wstring& text )
{
    method_.description = CORBA::wstring_dup( text.c_str() );
}

unsigned int
ControlMethodHelper::findInstrument( const std::wstring& modelname, unsigned long unitnumber )
{
    for ( size_t i = 0; i < method_.iinfo.length(); ++i ) {
        if ( modelname == method_.iinfo[uint32_t(i)].modelname.in() && uint32_t(unitnumber) == uint32_t(method_.iinfo[uint32_t(i)].unit_number) )
            return static_cast<unsigned int>(i);
    }
    return (-1); // error
}

// static
unsigned int
ControlMethodHelper::findInstrument( const ControlMethod::Method& m, const std::wstring& modelname, unsigned long unitnumber )
{
    for ( CORBA::ULong i = 0; i < m.iinfo.length(); ++i ) {
        if ( modelname == m.iinfo[i].modelname.in() && unitnumber == m.iinfo[i].unit_number )
            return i;
    }
    return (-1); // error
}

::ControlMethod::InstInfo&
ControlMethodHelper::addInstrument( const std::wstring& modelname, unsigned long unitnumber )
{
    // adportable::debug() << "ControlMethodHelper::addInstrument(" << modelname << ", " << unitnumber << ")";
    unsigned int index;
    if ( ( index = findInstrument( modelname, unitnumber ) ) == unsigned(-1) ) {
        method_.iinfo.length( method_.iinfo.length() + 1 );
        ControlMethod::InstInfo& info = method_.iinfo[ method_.iinfo.length() - 1 ];
        info.index = method_.iinfo.length() - 1;
        return info;
    }
    return method_.iinfo[ index ];
}

::ControlMethod::MethodLine&
ControlMethodHelper::add( const std::wstring& modelname, unsigned long unitnumber )
{
    // adportable::debug() << "ControlMethodHelper::add(" << modelname << ", " << unitnumber << ")";

    method_.lines.length( method_.lines.length() + 1 );
    ::ControlMethod::MethodLine&  line = method_.lines[ method_.lines.length() - 1 ];

    line.modelname = CORBA::wstring_dup( modelname.c_str() );
    line.index = findInstrument( modelname, unitnumber );
    line.unitnumber = unitnumber;
    line.isInitialCondition = true;

    return line;
}


// static
ControlMethod::MethodLine *
ControlMethodHelper::findFirst( ControlMethod::Method& method, const std::wstring& modelname, unsigned long unitnumber )
{
    size_t nlines = method.lines.length();
    for ( size_t i = 0; i < nlines; ++i ) {
        ControlMethod::MethodLine& line = method.lines[ int(i) ];
        if ( modelname == line.modelname.in() && unitnumber == line.unitnumber )
            return &line;
    }
    return 0;
}

// static
const ControlMethod::MethodLine *
ControlMethodHelper::findFirst( const ControlMethod::Method& method, const std::wstring& modelname, unsigned long unitnumber )
{
    size_t nlines = method.lines.length();
    for ( CORBA::ULong i = 0; i < nlines; ++i ) {
        const ControlMethod::MethodLine& line = method.lines[ i ];
        if ( modelname == line.modelname.in() && unitnumber == line.unitnumber )
            return &line;
    }
    return 0;
}

// static
const ControlMethod::MethodLine *
ControlMethodHelper::findNext( const ControlMethod::Method& method, const ControlMethod::MethodLine * line )
{
    if ( line ) {
        CORBA::ULong nlines = method.lines.length();
        if ( line >= &method.lines[0] && line < &method.lines[ nlines ] ) {
            for ( CORBA::ULong i = uint32_t( line - &method.lines[0] ); i < nlines; ++i ) {
                if ( line->modelname.in() == method.lines[i].modelname.in()
                    && line->unitnumber == method.lines[i].unitnumber )
                    return &method.lines[i];
            }
        }
    }
    return 0;
}

ControlMethod::MethodLine *
ControlMethodHelper::findNext( ControlMethod::Method& method, const ControlMethod::MethodLine * line )
{
    if ( line ) {
        CORBA::ULong nlines = method.lines.length();
        if ( line >= &method.lines[0] && line < &method.lines[ nlines ] ) {
            for ( uint32_t i = uint32_t( line - &method.lines[0] ); i < nlines; ++i ) {
                if ( line->modelname.in() == method.lines[i].modelname.in()
                    && line->unitnumber == method.lines[i].unitnumber )
                    return &method.lines[i];
            }
        }
    }
    return 0;
}

// static
::ControlMethod::MethodLine&
ControlMethodHelper::add( ControlMethod::Method& m, const std::wstring& modelname, unsigned long unitnumber )
{
    m.lines.length( m.lines.length() + 1 );
    ::ControlMethod::MethodLine&  line = m.lines[ m.lines.length() - 1 ];

    line.modelname = CORBA::wstring_dup( modelname.c_str() );
    line.index = findInstrument( m, modelname, unitnumber );
    line.unitnumber = unitnumber;
    line.isInitialCondition = true;

    return line;
}

// static
bool
ControlMethodHelper::append( ControlMethod::Method& method, const ControlMethod::MethodLine& line
                           , const std::wstring& modelname, unsigned long unitnumber )
{
    CORBA::ULong n = method.lines.length();
    method.lines.length( n + 1 );
    method.lines[ n ] = line;
    method.lines[ n ].unitnumber = unitnumber;
    method.lines[ n ].modelname = CORBA::wstring_dup( modelname.c_str() );

    return true;
}


/////////////////
unsigned long
ControlMethodLine::index() const
{
    return line_.index;
}

const wchar_t *
ControlMethodLine::modelname() const
{
    return line_.modelname.in();
}

void
ControlMethodLine::modelname( const std::wstring& modelname )
{
    line_.modelname = CORBA::wstring_dup( modelname.c_str() );
}

unsigned long
ControlMethodLine::unitnumber() const
{
    return line_.unitnumber;
}

void
ControlMethodLine::unitnumber( unsigned long unitnumber )
{
    line_.unitnumber = unitnumber;
}

bool
ControlMethodLine::isInitialCondition() const
{
    return line_.isInitialCondition;
}

void
ControlMethodLine::isInitialCondition( bool d )
{
    line_.isInitialCondition = d;
}

//////////////

ControlMethodInstInfo::ControlMethodInstInfo( ControlMethod::InstInfo& t ) : info_( t )
{
}

unsigned long
ControlMethodInstInfo::index() const
{
    return info_.index;
}

unsigned long
ControlMethodInstInfo::unit_number() const // 0..n
{
    return info_.unit_number;
}

::ControlMethod::eDeviceCategory
ControlMethodInstInfo::category() const
{
    return info_.category;
}

const wchar_t *
ControlMethodInstInfo::modelname() const
{
    return info_.modelname.in();
}

const wchar_t *
ControlMethodInstInfo::serial_number() const
{
    return info_.serial_number.in();
}

const wchar_t *
ControlMethodInstInfo::description() const
{
    return info_.description.in();
}

// static
bool
ControlMethodHelper::copy( Method& dst, const ControlMethod::Method& src )
{
    dst.iinfo.clear();
    dst.lines.clear();
    dst.subject = src.subject.in();
    dst.description = src.subject.in();
    for ( CORBA::ULong i = 0; i < src.iinfo.length(); ++i ) {
        const ControlMethod::InstInfo& s = src.iinfo[ i ];
        InstInfo info;
        info.index = s.index;
        info.unit_number = s.unit_number;
        info.category = static_cast< eDeviceCategory >( s.category );
        info.modelname = s.modelname;
        info.serial_number = s.serial_number;
        info.description = s.description;
        dst.iinfo.push_back ( info );
    }

	for ( CORBA::ULong i = 0; i < src.lines.length(); ++i ) {
        const ControlMethod::MethodLine& s = src.lines[ i ];
        dst.lines.push_back( Method::Line() );
        Method::Line& line = dst.lines.back();

        line.modelname = s.modelname.in();
        line.index = s.index;
        line.unitnumber = s.unitnumber;
        line.isInitialCondition = s.isInitialCondition;
        line.time.sec_ = s.time.sec_;
        line.time.usec_ = s.time.usec_;
        line.funcid = s.funcid;
        line.xdata.resize( s.xdata.length() );
        std::copy( s.xdata.get_buffer(), s.xdata.get_buffer() + s.xdata.length(), line.xdata.begin() );
    }
    return true;
}

//static 
bool
ControlMethodHelper::copy( ControlMethod::Method& dst, const Method& src )
{
    dst.iinfo.length( static_cast< CORBA::ULong >(src.iinfo.size()) );
    dst.lines.length( static_cast< CORBA::ULong >(src.lines.size()) );

	dst.subject = CORBA::wstring_dup( src.subject.c_str() );
	dst.description = CORBA::wstring_dup( src.subject.c_str() );

    for ( CORBA::ULong i = 0; i < src.iinfo.size(); ++i ) {
        const InstInfo& s = src.iinfo[ i ];
        ControlMethod::InstInfo& info = dst.iinfo[ i ];
        info.index = s.index;
        info.unit_number = s.unit_number;
        info.category = static_cast< ControlMethod::eDeviceCategory >( s.category );
        info.modelname = CORBA::wstring_dup( s.modelname.c_str() );
        info.serial_number = CORBA::wstring_dup( s.serial_number.c_str() );
        info.description = CORBA::wstring_dup( s.description.c_str() );
    }

    for ( CORBA::ULong i = 0; i < src.lines.size(); ++i ) {
        const Method::Line& s = src.lines[ i ];
        ControlMethod::MethodLine& d = dst.lines[ i ];

        d.modelname = CORBA::wstring_dup( s.modelname.c_str() );
        d.index = s.index;
        d.unitnumber = s.unitnumber;
        d.isInitialCondition = s.isInitialCondition;
        d.time.sec_ = s.time.sec_;
        d.time.usec_ = s.time.usec_;
        d.funcid = s.funcid;
        d.xdata.length( static_cast<CORBA::ULong>(s.xdata.size()) );
        std::copy( s.xdata.begin(), s.xdata.end(), d.xdata.get_buffer() );
    }
    return true;
}

