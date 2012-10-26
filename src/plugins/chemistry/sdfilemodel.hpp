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

#ifndef SDFILEMODEL_HPP
#define SDFILEMODEL_HPP

#include <QObject>
#include <QAbstractTableModel>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

class QModelIndex;
class QByteArray;

namespace OpenBabel { class OBMol; }

namespace chemistry {

	class ChemFile;
	class SvgItem;

	class SDFileModel : public QAbstractTableModel {
		Q_OBJECT
	public:
		explicit SDFileModel(QObject *parent = 0);

		int rowCount( const QModelIndex& parent ) const;
		int columnCount( const QModelIndex& parent ) const;
		QVariant data( const QModelIndex& index, int role ) const;
		QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
		Qt::ItemFlags flags( const QModelIndex& index ) const;
		bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
		bool insertRows( int position, int rows, const QModelIndex& index = QModelIndex() );
		bool removeRows( int position, int rows, const QModelIndex& index = QModelIndex() );

		//----
		void file( boost::shared_ptr< ChemFile >& );
		typedef std::pair< std::string, std::string> attribute_type;
    signals:
    
	public slots:

	private:
		static bool toSvg( SvgItem&, const OpenBabel::OBMol& );
		boost::shared_ptr< ChemFile > file_;
		std::vector< OpenBabel::OBMol > data_;
		static std::vector< attribute_type > attributes( const OpenBabel::OBMol& );

	};

}

#endif // SDFILEMODEL_HPP
