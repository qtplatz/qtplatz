/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef SDFILEVIEW_HPP
#define SDFILEVIEW_HPP

#include <QWidget>
#include <memory>

namespace Ui {
class SDFileView;
}

namespace OpenBabel { class OBMol; }
namespace adchem { class Mol; }

class QModelIndex;

namespace chemistry {

	class SDFileModel;
	class SDFileDelegate;
	class ChemFile;

	class SDFileView : public QWidget {
		Q_OBJECT
    
	public:
		explicit SDFileView(QWidget *parent = 0);
		~SDFileView();

		void file( std::shared_ptr<ChemFile>& );
		void setData( const std::vector< adchem::Mol >& );

    signals:
		void rawClicked( int, const SDFileModel* );

	private slots:
		void handleRawClicked( int );
    
	private:
		Ui::SDFileView *ui;
		SDFileModel * model_;
		SDFileDelegate * delegate_;
	};

}

#endif // SDFILEVIEW_HPP
