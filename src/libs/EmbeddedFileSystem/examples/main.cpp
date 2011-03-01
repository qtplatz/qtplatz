//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JInputStream.h"
#include "JFileInputStream.h"
#include "JBufferedInputStream.h"

#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QThread>

#include <iostream>
#include "time.h"

#include "Efs.h"

#include <map>
#include "EfsStream.h"
//#include "UrlKey.h"

using namespace std;

const int opCount = 900000;

void printFile( int fd, Efs *afs )
{
	char buf[ 1024 ];
	QByteArray data;
	int readSize = 0;

	if ( -1 == afs->seekFile( fd, 0 ) )
	{
		std::cout << "file seek failed." << std::endl;
	}

	while ( !afs->atEnd( fd ) )
	{
		if ( !( readSize = afs->readFile( fd, buf, 1024 ) ) )
		{
			std::cout << "file read failed." << std::endl;
		}
		else
		{
			buf[ readSize ] = 0;
			data.append( buf );
		}
	}

	std::cout << data.data() << std::endl;
}

void writeFile( int fd, Efs *afs )
{
	const char *bigText = "Permission is hereby granted, free of charge, to any person obtaining"
					"a copy of this software and associated documentation files (the \"Software\"), "
					"to deal in the Software without restriction, including without limitation "
					" the rights to use, copy, modify, merge, publish, distribute, sublicense, "
					"and or sell copies of the Software, and to permit persons to whom the "
					"Software is furnished to do so, subject to the following conditions:"
					"The above copyright notice and this permission notice shall be included "
					"in all copies or substantial portions of the Software."
					"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
					"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
					"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
					"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
					"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING "
					"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE ...";
	int bigTextSize = strlen( bigText );

	for ( int i = 0; i < 77; i++ )
	{
		afs->writeFile( fd, bigText, bigTextSize );
	}
}

void writeFile( QFileInfo &fi, Efs *afs )
{
	if ( fi.isFile() )
	{
		QByteArray filename = fi.fileName().toAscii();
		std::cout << filename.data() << endl;
		
		int fd = afs->createFile( filename.data(), EfsReadWrite, EfsOpenAlways );

		JFileInputStream fis( fi.absoluteFilePath() );
		const int bufSize = 4096;
		char buf[ bufSize ];
		int retSize = 0;

		while ( !fis.atEnd() )
		{
			retSize = fis.read( buf, bufSize );
			afs->writeFile( fd, buf, retSize );
		}

		afs->closeFile( fd );
	}
}

void pour( const QString &path, Efs *afs )
{
	QDir dirPath( path );
	QFileInfoList ilist = dirPath.entryInfoList();

	//foreach( QFileInfo fi, ilist )
	for ( int i = 1; i < ilist.size(); i += 2 )
	{
		QFileInfo fi1 = ilist.at( i - 1 );
		QFileInfo fi2 = ilist.at( i );

		QByteArray filename1 = fi1.fileName().toAscii();
		std::cout << filename1.data() << endl;
		QByteArray filename2 = fi2.fileName().toAscii();
		std::cout << filename2.data() << endl;

		int fd1 = afs->createFile( filename1.data(), EfsReadWrite, EfsOpenAlways );
		int fd2 = afs->createFile( filename2.data(), EfsReadWrite, EfsOpenAlways );

		JFileInputStream fis1( fi1.absoluteFilePath() );
		JFileInputStream fis2( fi2.absoluteFilePath() );
		const int bufSize = 4096;
		char buf1[ bufSize ];
		char buf2[ bufSize ];
		int retSize1 = 0;
		int retSize2 = 0;

		while ( !fis1.atEnd() || !fis2.atEnd() )
		{
			if ( !fis1.atEnd() )
			{
				retSize1 = fis1.read( buf1, bufSize );
				afs->writeFile( fd1, buf1, retSize1 );
			}

			if ( !fis2.atEnd() )
			{
				retSize2 = fis2.read( buf2, bufSize );
				afs->writeFile( fd2, buf2, retSize2 );
			}
		}

		afs->closeFile( fd1 );
		afs->closeFile( fd2 );
	}
}

void parseAll( Efs *afs )
{
	EfsFilenameList retList;
	afs->listFiles( retList );

	EfsFilenameList::iterator iter = retList.begin();
	for ( ; iter != retList.end(); iter++ )
	{
		EfsStream astream( afs );
		if ( !astream.open( iter->data(), EfsReadAccess, EfsOpenExisting ) )
		{
			continue;
		}

		QString path = QString( "e:/out/" ) + iter->data();
		QFile fl( path );
		fl.open( QIODevice::WriteOnly );
		const int bufLen = 8192;
		char buf[ bufLen ];
		int retSize = 0;

		while ( !astream.atEnd() )
		{
			retSize = astream.read( buf, bufLen );
			fl.write( buf, retSize );
		}

		astream.close();
		fl.close();
	}
}

void extractToOut( Efs &afs, const QString &path )
{
	EfsFilenameList files;
	const int maxBufSize = 8192;
	char buf[ maxBufSize + 1 ];
	afs.listFiles( files );

	EfsFilenameList::iterator iter = files.begin();
	for ( ; iter != files.end(); iter++ )
	{
		int fd = afs.createFile( *iter, EfsReadAccess, EfsOpenExisting );
		QFile diskFile( path + "/out/" + *iter );
		diskFile.open( QIODevice::WriteOnly );

		while ( !afs.atEnd( fd ) )
		{
			int read = afs.readFile( fd, buf, maxBufSize );
			diskFile.write( buf, read );
		}
		afs.closeFile( fd );
		diskFile.close();
	}
}

void make2files( Efs &afs )
{
	int fd1 = afs.createFile( "first.datat", EfsWriteAccess, EfsOpenAlways );
	int fd2 = afs.createFile( "second.datat", EfsWriteAccess, EfsOpenAlways );

	const char *bigText = "Gaviri PocketSearch™ is Gaviri's search technology for portable drives. You install PocketSearch on a portable drive. After installation PocketSearch indexes all the content on the drive and any new files added are indexed on the fly. You may tell PocketSearch not to index any file you do not want to index by clicking on the PocketSearch icon in the Windows tray, selecting Preferences, Files and unchecking any folder you do not wish to index.";
	int bigTextSize = strlen( bigText );
	const int writeCount = 12058;
	//char buf[ 100 ];

	for ( int i = 0; i < writeCount; i++ )
	{
		afs.writeFile( fd1, bigText, bigTextSize );
		afs.writeFile( fd2, bigText, bigTextSize );
	}

	afs.closeFile( fd1 );
	afs.closeFile( fd2 );
}

void bigTest( Efs &afs )
{
	int fd1 = afs.createFile( "BigTestDat", EfsWriteAccess, EfsOpenAlways );
	//int fd2 = afs.createFile( "BigTestDat1", EfsWriteAccess, EfsOpenAlways );
	//int fd3 = afs.createFile( "BigTestDat2", EfsWriteAccess, EfsOpenAlways );

	//int bigTextSize = strlen( bigText );
	const int bufSize = 150;
	byte buf[ bufSize + 1 ];

	// Generate buf
	for ( int j = 0; j < bufSize; j++ )
	{
		buf[ j ] = '7';
	}

	// Write to file
	for ( int i = 0; i < opCount; i++ )
	{
		//gint64 size = afs.size();

		if ( !afs.writeFile( fd1, ( char* ) buf, bufSize ) )
		{
			std::cout << i << std::endl;
		}
		//afs.writeFile( fd2, ( char* ) buf, bufSize );
		//afs.writeFile( fd3, ( char* ) buf, bufSize );
		//std::cout << "completed: " << i << "\r";
	}

	afs.closeFile( fd1 );
	//afs.closeFile( fd2 );
	//afs.closeFile( fd3 );
}

void testBig( Efs &afs )
{
	int fd1 = afs.createFile( "BigTestDat", EfsReadAccess, EfsOpenAlways );
	//int fd2 = afs.createFile( "BigTestDat1", EfsReadAccess, EfsOpenAlways );
	//int fd3 = afs.createFile( "BigTestDat2", EfsReadAccess, EfsOpenAlways );

	const int bufSize = 15;
	byte buf[ bufSize + 1 ];

	// Write to file
	for ( int i = 0; i < 1; i++ )
	{
		int read = afs.readFile( fd1, ( char* ) buf, bufSize );

		if ( read != bufSize )
		{
			std::cout << "buggg " << std::endl;
		}

		for ( int j = 0; j < bufSize; j++ )
		{
			if ( buf[ j ] != '7' )
			{
				std::cout << "bug" << std::endl;
			}
		}

		//afs.readFile( fd2, ( char* ) buf, bufSize );
		//for ( int j = 0; j < bufSize; j++ )
		//{
		//	if ( buf[ j ] != j )
		//	{
		//		std::cout << "bug" << std::endl;
		//	}
		//}

		//afs.readFile( fd3, ( char* ) buf, bufSize );
		//for ( int j = 0; j < bufSize; j++ )
		//{
		//	if ( buf[ j ] != j )
		//	{
		//		std::cout << "bug" << std::endl;
		//	}
		//}
	}

	afs.closeFile( fd1 );
	//afs.closeFile( fd2 );
	//afs.closeFile( fd3 );
}

bool ask( int )
{
	cout << "asking..." << endl;
	return true;
}

class test
{
public:

	typedef bool ( test::*tfunc )( int bytes );

	bool ask( int )
	{
		cout << "asking..." << endl;
		return true;
	}

	void c()
	{
		tfunc f = &test::ask;
		( this->*f )( 3 );
	}

};

class EfsWrite : public QThread
{
public:

	EfsWrite( const QByteArray &filename, Efs &afs ) :
		efs_( afs )
	{
		fn_ = filename;
	}

	void run()
	{
		int fd1 = efs_.createFile( fn_.data(), EfsReadWrite, EfsOpenAlways );

		const int bufSize = 77;
		char buf[ bufSize + 1 ];

		// Write to file
		for ( int i = 0; i < opCount; i++ )
		{
			itoa( i, buf, 10 );
			int len = strlen( buf );
			efs_.writeFile( fd1, buf, len );
		}

		efs_.closeFile( fd1 );
		std::cout << "Write thread has exited." << "\n";
	}

	QByteArray fn_;
	Efs &efs_;
};

class EfsRead : public QThread
{
public:

	EfsRead( const QByteArray &filename, Efs &afs ) :
		efs_( afs )
	{
		fn_ = filename;
	}

	void run()
	{
		int fd1 = efs_.createFile( fn_.data(), EfsReadWrite, EfsOpenExisting );

		const int bufSize = 77;
		char buf[ bufSize + 1 ];

		for ( int i = 0; i < opCount; i++ )
		{
			itoa( i, buf, 10 );
			int len = strlen( buf );
			efs_.readFile( fd1, buf, len );
			buf[ len ] = 0;

			if ( atoi( buf ) != i )
			{
				std::cout << "ERROR!" << std::endl;
			}
		}

		efs_.closeFile( fd1 );
		std::cout << "Read thread has exited." << "\n";
	}

	QByteArray fn_;
	Efs &efs_;
};

int main()
{
	QString path = "e:/";
	//time_t startTime = time( 0 );

	//Main< 3, int, int, Cont > m;
	//m.print();

	Efs afs;
	if ( !afs.mount( ( path + "/afs.filesystem" ).utf16() ) )
	{
		std::cout << "mount failed." << std::endl;
	}

	std::cout << "Big test started ..." << std::endl;
	bigTest( afs );

	//afs.debugWrite();

	//std::cout << time( 0 ) - startTime << std::endl;
	//cin.get();

	//EfsRead ar1( "test1", afs );
	//ar1.start( QThread::NormalPriority );

	//EfsRead ar2( "test2", afs );
	//ar2.start( QThread::NormalPriority );

	//EfsRead ar3( "test3", afs );
	//ar3.start( QThread::NormalPriority );

	afs.umount();

	std::cout << "Ok." << std::endl;
	cin.get();

	return 0;
}
