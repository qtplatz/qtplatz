//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JTempFileStream.h"
#include <QFileInfo>
#include <QFSFileEngine>
#include <time.h>


//	======================================================
//	JTempFileStream::JTempFileStream

JTempFileStream::JTempFileStream()
	: autoDelete_( true )
{
	QString tempFilePath = QFSFileEngine::tempPath() + "/JSTemp" + tmpnam( 0 );
	tempFilePath.replace( '\\', '_' );
	
	open( tempFilePath );
}

//	======================================================
//	JTempFileStream::JTempFileStream

JTempFileStream::JTempFileStream( const QString& name )
	: autoDelete_( true )
{
	open( name );
}

//	======================================================
//	JTempFileStream::~JTempFileStream

JTempFileStream::~JTempFileStream()
{
	flush();
	cleanUp();
}

//	======================================================
//	JTempFileStream::setAutoDelete

void JTempFileStream::setAutoDelete( bool flag )
{
	autoDelete_ = flag;
}

void JTempFileStream::cleanUp()
{
	if ( file_.isOpen() )
	{
		file_.close();
	}

	if ( autoDelete_ && file_.exists() )
	{
		file_.remove();
	}
}

//	======================================================
//	JTempFileStream::open

bool JTempFileStream::open( const QString& fileName ) 
{
	if( fileName.isEmpty() )
	{
		return false;
	}
	else
	{
		QFileInfo fi( fileName );
		if ( fi.isDir() )
		{
			return false;
		}
	}

	cleanUp();

	file_.setFileName( fileName );
	return file_.open( QIODevice::ReadWrite );
}

//	======================================================
// generates unique file name in specified location

//inline const QString& generateUnqueFileName( const QString &filePath, QString &uniqueFileName )
//{
//	//srand( time( 0 ) );
//	unsigned int number = 0; 
//	uniqueFileName = filePath + QString::number( rand() + JetTickCount() );
//
//	while ( QFileInfo( uniqueFileName ).exists() )
//	{
//		number++;
//		uniqueFileName = filePath + QString::number( number );
//	}
//	
//	return uniqueFileName;
//}

