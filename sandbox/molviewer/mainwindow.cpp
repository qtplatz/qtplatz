/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QDragEnterEvent>
#include <QUrl>
#include <QDebug>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops( true );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			std::wstring file = urlList.at(i).toLocalFile().toStdWString();
			boost::filesystem::path path( file ); 
			if ( path.extension() == L".mol" ) {
				event->acceptProposedAction();
				return;
			}
		}
	}
}

void
MainWindow::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			std::wstring file = urlList.at(i).toLocalFile().toStdWString();
			boost::filesystem::path path( file ); 
			if ( path.extension() == L".mol" ) {
                molfile_open( path );
			}
		}
	}
}

class atom {
public:
	atom() {
		memset( this, 0, sizeof( atom ));
	}
	double x;
	double y;
	double z;
	char symbol[10];
    int mass_difference;
    int charge; // charge
    int atom_stereo_parity; // atom stereo parity
    int hydrogen_count; // hydrogen count + 1
    int stereo_care_box;  // stereo care box
    int valence;
    int H0; // H0 designator
    int atom_atom_mapping_number; // atom-atom mapping number
    int inversion_retention_flag; // inversion/retention flag
	int exact_change_flag; // exact change flag
};

class bond {
public:
	int first_atom_number;
	int second_atom_number;
    int bond_type;  // 1 single, 2 double, 3 triple, 4 aromatic, 5 single or double, 6 single or aromatic 7 double or aroamtic, 8 any
    int bond_stereo;
    int bond_topology;
    int reacting_center_status;
};

void
MainWindow::molfile_open( const boost::filesystem::path& path )
{
	boost::filesystem::ifstream inf( path );

	std::vector< std::string > header;
	std::string line;
	for ( int i = 0; i < 3; ++i ) {
		std::getline( inf, line );
		header.push_back( line );
	}

	typedef boost::char_separator< char > separator;
	typedef boost::tokenizer< separator > tokenizer;
 
	// count block
	int natoms = 0, nbounds = 0;
    int chiral = 0;
    int nprops = 0;
	std::string version;
	separator sep( ", \t", "", boost::drop_empty_tokens );
	if ( std::getline( inf, line ) ) {
		tokenizer tokens( line, sep );
		tokenizer::iterator it = tokens.begin();
		if ( it != tokens.end() )
			natoms = atoi( it->c_str() );
        if ( ++it != tokens.end() )
			nbounds = atoi( it->c_str() );
		if ( ++it != tokens.end() ) // number of atom lists
			;
		if ( ++it != tokens.end() ) // fff obsolete
			;
		if ( ++it != tokens.end() ) // ccc chiral flag
			chiral = atoi( it->c_str() );
		++it; // obsolete
		++it; // obsolete
		++it; // obsolete
		++it; // obsolete
		if ( ++it != tokens.end() ) // mmm number of lines of additional properties
			nprops = atoi( it->c_str() );
        if ( ++it != tokens.end() )
			version = *it;
	}

	// atoms block
	std::vector< atom > atoms;
	for ( int i = 0; i < natoms; ++i ) {
        atom a;
		if ( std::getline( inf, line ) ) {
			tokenizer tokens( line, sep );
			tokenizer::iterator it = tokens.begin();
            if ( it != tokens.end() )
				a.x = atof( it->c_str() );
            if ( ++it != tokens.end() )
				a.y = atof( it->c_str() );
            if ( ++it != tokens.end() )
				a.z = atof( it->c_str() );
            if ( ++it != tokens.end() )
				strcpy( a.symbol, it->c_str() );
			if ( ++it != tokens.end() )
				a.mass_difference = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.charge = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.atom_stereo_parity = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.hydrogen_count = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.stereo_care_box = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.valence = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.H0 = atoi( it->c_str() );
			++it;
			++it;
			if ( ++it != tokens.end() )
				a.atom_atom_mapping_number = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.inversion_retention_flag = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.exact_change_flag = atoi( it->c_str() );
		}
		atoms.push_back( a );
	}

	std::vector< bond > bonds;
	for ( int i = 0; i < nbounds; ++i ) {
		bond b;
		if ( std::getline( inf, line ) ) {
			tokenizer tokens( line, sep );
			tokenizer::iterator it = tokens.begin();
            if ( it != tokens.end() )
				b.first_atom_number = atoi( it->c_str() );
            if ( ++it != tokens.end() )
				b.second_atom_number = atoi( it->c_str() );
            if ( ++it != tokens.end() )
				b.bond_type = atoi( it->c_str() );
            if ( ++it != tokens.end() )
				b.bond_stereo = atoi( it->c_str() );
			++it;
			if ( ++it != tokens.end() )
				b.bond_topology = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				b.reacting_center_status = atoi( it->c_str() );
		}
		bonds.push_back( b );
	}

}