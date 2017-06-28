/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANREPORTWIDGET_HPP
#define QUANREPORTWIDGET_HPP

#include <adwidgets/progressinterface.hpp>
#include <QWidget>
#include <memory>

class QGridLayout;
class QVBoxLayout;
class QMenu;
class QTextBrowser;
class QToolBar;

namespace adpublisher { class docEditor; }

namespace quan {

    class QuanQueryForm;
    class QuanResultTable;
    class QuanDocument;
    class ProgressHandler;

    class QuanReportWidget : public QWidget  {
        Q_OBJECT
    public:
        ~QuanReportWidget();
        explicit QuanReportWidget(QWidget *parent = 0);
        void onInitialUpdate( QuanDocument * );

    private:
        QVBoxLayout * layout_;

        std::unique_ptr< QTextBrowser > docBrowser_;
 
        void importDocTemplate();
        void exportDocTemplate();
        void setupFileActions( QMenu *, QToolBar * );

        QString currentStylesheet() const;

        std::pair< QString, QString> publishTask( const QString& xsl, adwidgets::ProgressInterface );

    signals:

    public slots :

    private slots:
        void filePublish();
        void fileDebug();
        void filePrintPdf();
        void handleConnectionChanged();
    };

}

#endif // QUANREPORTWIDGET_HPP
