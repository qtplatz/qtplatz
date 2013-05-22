/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "datafile.hpp"
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
// #include <xmlparser/pugixml.hpp>
#include <QtXml/QXmlDefaultHandler>

using namespace mzxml;

namespace mzxml {

	class XmlHandler : public QXmlDefaultHandler {
	public:
		bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes);
		bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
		bool characters(const QString &str);
		bool fatalError(const QXmlParseException &exception);
		QString errorString() const;
	};

}

datafile::datafile()
{
}

//virtual
void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    // AcquireDataset <LCMSDataset>
	sub.subscribe( *this );

    // subscribe processed dataset
	if ( processedDataset_ ) 
		sub.subscribe( *processedDataset_ );
}

// virtual
boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
	boost::any any;
	return any;
}

//virtual
adcontrols::datafile::factory_type
datafile::factory()
{ 
	return 0;
}

//virtual
size_t
datafile::getFunctionCount() const
{
	return 1;
}

//virtual
size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
	return 0;
}

//virtual
size_t
datafile::getChromatogramCount() const
{
	return 0;
}

//virtual
bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& ) const
{
	return false;
}

//virtual
bool
datafile::getSpectrum( int /* fcn*/, int /*idx*/, adcontrols::MassSpectrum& ) const
{
	return false;
}

/////////////////////////

bool
datafile::_open( const std::wstring& filename, bool )
{
	filename_ = filename;
	portfolio::Portfolio portfolio;
	portfolio.create_with_fullpath( filename_ );
	portfolio::Folder spectra = portfolio.addFolder( L"Spectra" );
/*
	boost::filesystem::directory_iterator pos( dw.pdata() );
	boost::filesystem::directory_iterator last;

	for ( ; pos != last; ++pos ) {
		boost::filesystem::path p( *pos );
		if ( boost::filesystem::is_directory( p ) ) {
			boost::filesystem::path rdfile( p / L"1r" );
			if ( boost::filesystem::is_regular_file( rdfile ) ) {
				std::wstring title;
				if ( boost::filesystem::is_regular_file( p / L"title" ) ) {
					boost::filesystem::wifstream inf( p / L"title" );
                    inf >> title;
				}
				if ( title.empty() )
					title = L"Spectrum " + p.leaf().wstring();
				portfolio::Folium folium = spectra.addFolium( title );
				folium.setAttribute( L"dataType", L"MassSpectrum" );
				folium.id( rdfile.wstring() );
			}
		}
	}
*/
	processedDataset_.reset( new adcontrols::ProcessedDataset );
	processedDataset_->xml( portfolio.xml() );
    return true;
}

//static
bool
datafile::is_valid_datafile( const std::wstring& filename )
{
	boost::filesystem::path path ( filename );
	bool result = path.extension() == L".mzxml"
		          || path.extension() == L".mzXML"
				  || path.extension() == L".MZXML"
				  ;

	//boost::filesystem::ifstream rdfile( path, std::ios_base::binary );
	//result = rdfile;
	//pugi::xml_document doc;
	//result = doc.load( rdfile );

    XmlHandler handler;
    QXmlSimpleReader reader;
	reader.setContentHandler( &handler );
    reader.setErrorHandler( & handler );
	QFile file( path.string().c_str() );
	if ( ! file.open( QFile::ReadOnly | QFile::Text ) )
		return false;
	QXmlInputSource xmlInputSource( &file );
	if ( reader.parse( &xmlInputSource, true ) ) {

	}
	return result;
}

bool
XmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
	return true;
}

bool
XmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	return true;
}

bool
XmlHandler::characters(const QString &str)
{
	return true;
}

bool
XmlHandler::fatalError(const QXmlParseException &exception)
{
	return true;
}

QString
XmlHandler::errorString() const
{
	return "Error";
}
