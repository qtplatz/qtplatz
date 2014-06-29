// This is a -*- C++ -*- header.
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

#pragma once

#include <QString>
#include <QFileDialog>

namespace qtwrapper {

    class QFileDialog : public ::QFileDialog {
    public:

        static QString getSaveFileName( QWidget * parent
                                        , const QString& caption
                                        , const QString& dir
                                        , const QString& name
                                        , const QString& filter
                                        , QString * selectedFilter = 0
                                        , Options options = 0 ) {
            // create a qt dialog
            ::QFileDialog dialog( parent, caption, dir, filter );
            dialog.setFileMode( AnyFile );
            dialog.selectFile( name );
            dialog.setOptions( options );
            dialog.setAcceptMode(AcceptSave);
            if (selectedFilter && !selectedFilter->isEmpty())
                dialog.selectNameFilter(*selectedFilter);
            if (dialog.exec() == QDialog::Accepted) {
                if (selectedFilter)
                    *selectedFilter = dialog.selectedNameFilter();
                return dialog.selectedFiles().value(0);
            }

            return QString();
        }
    };
}

