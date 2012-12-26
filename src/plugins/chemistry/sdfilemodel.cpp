/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "sdfilemodel.hpp"
#include "chemfile.hpp"
#include "svgitem.hpp"
#include <adchem/mol.hpp>
#include <adchem/conversion.hpp>

#if defined _MSC_VER
#  pragma warning(disable:4100)
#endif
#include <openbabel/mol.h>
#include <openbabel/parsmart.h>
#if defined _MSC_VER
#  pragma warning(default:4100)
#endif

#include <adchem/mol.hpp>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <iomanip>
#include <qstring.h>
#include <qwidget.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <QGraphicsItem>
#include <QGraphicsSvgItem>
#include <QtSvg/qsvgwidget.h>
#include <qprogressdialog.h>
#include <qdebug.h>
#include <set>
#include <cmath>
#include <algorithm>

using namespace chemistry;

SDFileModel::SDFileModel( QObject *parent ) : QAbstractTableModel( parent )
{
    QHash< int, QByteArray > roles;
    roles[ 0 ] = "MOL";
    roles[ 1 ] = "Formula";
    roles[ 2 ] = "Exact mass";
    
    excludes_.push_back( "PartialCharges" );
    excludes_.push_back( "Formula" );
    excludes_.push_back( "MOL Chiral Flag" );
    excludes_.push_back( "Mol Weight" );
    excludes_.push_back( "TOLERANCE (Yes  Exampt)" );
    
    excludes_.push_back( "CdId" );
}

int
SDFileModel::rowCount( const QModelIndex& parent ) const
{
    (void)parent;
    return data_.size();
}

int
SDFileModel::columnCount( const QModelIndex& parent ) const
{
    (void)parent;
    if ( ! data_.empty() )
	return adchem::Mol::attributes( 
	    static_cast< const OpenBabel::OBMol &>(data_[0]), excludes_ ).size() + 3;
    return 3;
}

QVariant
SDFileModel::data( const QModelIndex& index, int role ) const
{
    using OpenBabel::OBMol;
    
    if ( ! index.isValid() )
	return QVariant();
    
    const OpenBabel::OBMol& mol = data_[ index.row() ];
    const size_t nfixed = 3;
    
    if ( role == Qt::DisplayRole ) {
	if ( index.column() == 0 ) {
	    SvgItem svg;
	    toSvg( svg, mol );
	    return QVariant::fromValue( svg );
	} else if ( index.column() == 1 ) {
	    return QString::fromStdString( const_cast<OBMol&>(mol).GetFormula() );
	} else if ( index.column() == 2 ) {
	    return QVariant( const_cast<OBMol&>(mol).GetExactMass() );
	} else {
	    std::vector< attribute_type > attrs = adchem::Mol::attributes( mol, excludes_ );
	    if ( index.column() - nfixed < attrs.size() )
		return QString::fromStdString( attrs[ index.column() - nfixed ].second );
	}
    }
    return QVariant();
}

QVariant
SDFileModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
	return QVariant();
    if ( orientation == Qt::Horizontal ) {
        switch( section ) {
	case 0: return tr( "MOL" );
	case 1: return tr( "Formula" );
	case 2: return tr( "m/z" );
	}
	
	if ( ! data_.empty() ) {
            int n = section - 3;
	    std::vector< attribute_type > attrs = adchem::Mol::attributes( data_[0], excludes_ );
            if ( attrs.size() > unsigned(n) )
		return QString::fromStdString( attrs[n].first );
	}
    }
    if ( orientation == Qt::Vertical ) {
	return section;
    }
    
    return QVariant();
}

Qt::ItemFlags
SDFileModel::flags( const QModelIndex& index ) const
{
    if ( ! index.isValid() )
	return Qt::ItemIsEnabled;
    return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;
}

bool 
SDFileModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    (void)value;
    
    if ( index.isValid() && role == Qt::EditRole ) {
	int row = index.row();
	(void)row;
	// set data here
	
	emit( dataChanged( index, index ) );
	
	return true;
    }
    return false;
}

bool 
SDFileModel::insertRows( int position, int rows, const QModelIndex& index )
{
    (void)index;
    beginInsertRows( QModelIndex(), position, position + rows - 1 );
    
    for (int row = 0; row < rows; ++row ) {
	// todo add data 
    }
    endInsertRows();
    return true;
}

bool 
SDFileModel::removeRows( int position, int rows, const QModelIndex& index )
{
    (void)index;
    beginRemoveRows( QModelIndex(), position, position + rows - 1 );
    
    for ( int row = 0; row < rows; ++row ) {
	// todo: remove data
    }
    endRemoveRows();
    return true;
}

void
SDFileModel::file( boost::shared_ptr< ChemFile >& file )
{
    using adchem::Mol;
    using adchem::Conversion;
    
    QProgressDialog progress( "Fetching data...", "Cancel", 0, 1000 );
    
    progress.setWindowModality( Qt::WindowModal );
    progress.setMaximum( file->fsize() );
    progress.show();
    
    std::set< std::string > set;
    beginResetModel();
    file_ = file;
    data_.clear();
    Mol mol; // OpenBabel::OBMol mol;
    while ( file->Read( static_cast< OpenBabel::OBMol& >(mol) ) && !progress.wasCanceled() ) {
	// take large molecule if not single molecule 
	std::vector< OpenBabel::OBMol > split = static_cast< OpenBabel::OBMol& >(mol).Separate();
	if ( split.size() >= 2 ) {
	    using OpenBabel::OBMol;
	    std::vector< OBMol >::iterator it
		= std::max_element( split.begin(), split.end()
				    , boost::bind( &Mol::GetExactMass, _1, true )
				    < boost::bind( &Mol::GetExactMass, _2, true ) );
	    mol = *it;
	}
    
	// duplicate check
	std::string smiles = Conversion::toSMILES( mol );
	if ( set.find( smiles ) == set.end() ) {
	    set.insert( smiles );
	    Mol::SetAttribute( mol, "SMILES", Conversion::toSMILES( mol ) );
	    data_.push_back( mol );
	    //-- trial code
	    OpenBabel::OBSmartsPattern sp;
	    if ( sp.Init( smiles ) ) {
		if ( sp.Match( mol ) ) {
		    std::vector< std::vector< int > > maplist = sp.GetUMapList();
		    BOOST_FOREACH( const std::vector< int >& matches, maplist ) {
			OpenBabel::OBMol& omol = static_cast<OpenBabel::OBMol&>(mol);
			OpenBabel::OBBond * b1 = omol.GetBond( matches[0] );
            (void)b1;
		    }
		}
	    }
	}
	progress.setValue( file->tellg() );
    }
  
    std::sort( data_.begin(), data_.end()
        , boost::bind( &Mol::getExactMass, _2, true ) < boost::bind( &Mol::getExactMass, _1, true ) );
    size_t cnt = 0;
    double prev = 0;
    BOOST_FOREACH( const Mol& mol, data_ ) {
	if ( prev == 0 ) {
	    prev = mol.getExactMass();
	    continue;
	}
	double d = std::fabs( prev - mol.getExactMass() );
	if ( d < 0.1 )
	    std::cout << ++cnt << ": " << std::fixed << std::setprecision(5)
		      << d << "(" << prev << " -- " << mol.getExactMass() << ")" << std::endl;
	prev = mol.getExactMass();
    }

    endResetModel();
}

bool
SDFileModel::toSvg( SvgItem& item, const OpenBabel::OBMol& mol )
{
    item.svg_ = adchem::Conversion::toSVG( mol ).c_str();
    return true;
}

const std::vector< adchem::Mol >&
SDFileModel::data() const
{
    return data_;
}

void
SDFileModel::data( const std::vector< adchem::Mol >& v)
{
    beginResetModel();
    data_.clear();
    data_ = v;
    endResetModel();
}
