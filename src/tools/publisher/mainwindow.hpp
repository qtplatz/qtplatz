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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

namespace adpublisher { class document; }
class QSettings;
class QString;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void onInitialUpdate( std::shared_ptr< QSettings >& );
    static void addRecentFiles( QSettings&, const QString& group, const QString& pfx, const QString& value, const QString& key = "File"  );

private:
    Ui::MainWindow *ui;
    std::shared_ptr< adpublisher::document > doc_;
    std::shared_ptr< QSettings > settings_;
    std::string xmlpath_;
    std::string xslpath_;
    QString processed_;

    void populateStylesheets( const QString& );

    void addRecentFiles( const QString& group, const QString& pfx, const QString& value, const QString& key = "File"  );
    void getRecentFiles( const QString& group, const QString& pfx, std::vector<QString>& list, const QString& key = "File" ) const;
    QString recentFile( const QString& group, const QString& pfx, const QString& key = "File" ) const;

    static void getRecentFiles( QSettings&, const QString& group, const QString& pfx, std::vector<QString>& list, const QString& key = "File" );

private slots:
    void handleOpenFile();
    void handleApplyStylesheet();
    void handleSaveProcessedAs();
    void handleStylesheetChanged( const QString& );
};

#endif // MAINWINDOW_HPP
