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

#include "datasequencewidget.hpp"
#include "datasequencetree.hpp"
#include "quandocument.hpp"
#include "paneldata.hpp"
#include <utils/styledbar.h>
#include <adcontrols/datafile.hpp>
#include <adportable/profile.hpp>
#include <adportable/date_string.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>

using namespace quan;

DataSequenceWidget::~DataSequenceWidget()
{
}

DataSequenceWidget::DataSequenceWidget(QWidget *parent) : QWidget(parent)
                                                        , layout_( new QGridLayout )
                                                        , dataSequenceTree_( new DataSequenceTree )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    const int row = layout_->rowCount();
    layout_->addWidget( dataSelectionBar(), row, 0 );
    layout_->addWidget( dataSequenceTree_.get(), row + 1, 0 );
}

QWidget *
DataSequenceWidget::dataSelectionBar()
{
    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        // [DATA OPEN]|[SAVE][...line edit...][EXEC]

        auto label = new QLabel;
        // label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Open data files" );
        toolBarLayout->addWidget( label );

        auto button = new QToolButton;
        button->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
        button->setToolTip( tr("Open data file...") );
        toolBarLayout->addWidget( button );

        toolBarLayout->addWidget( new Utils::StyledSeparator );

        auto label2 = new QLabel;
        label2->setText( "Data save in:" );
        toolBarLayout->addWidget( label2 );

        auto toolButton = new QToolButton;
        toolButton->setIcon( QIcon( ":/quan/images/filesave.png" ) );
        toolButton->setToolTip( tr("Set result file name...") );
        toolBarLayout->addWidget( toolButton );

        auto edit = new QLineEdit;
        toolBarLayout->addWidget( edit );
        do { // insert default result file
            boost::filesystem::path path( adportable::profile::user_data_dir< wchar_t >() );
            path /= L"data";
            path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
            boost::filesystem::path name = path += "_quan.adfs";
            int i = 1;
            while ( boost::filesystem::exists( name ) )
                name = (boost::wformat( L"%s_%d_quan.adfs" ) % path.wstring() % i++).str();
            edit->setText( QString::fromStdWString( path.wstring() ) );
        } while(0);

        auto execButton = new QToolButton;
        execButton->setIcon( QIcon( ":/quan/images/run.png" ) );
        execButton->setToolTip( tr("Run quantitation process") );
        toolBarLayout->addWidget( execButton );

        auto stopButton = new QToolButton;
        stopButton->setIcon( QIcon( ":/quan/images/stop_small.png" ) );
        stopButton->setToolTip( tr("Stop quantitation process") );
        stopButton->setEnabled( false );
        toolBarLayout->addWidget( stopButton );

        connect( button, &QToolButton::clicked, this, [this] ( bool ){
                boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() );
                dir /= L"data";

                QFileDialog dlg( 0, tr("Open data file(s)"), QString::fromStdWString( dir.wstring() ) );
                dlg.setNameFilter( tr("Data Files(*.adfs *.csv *.txt *.spc)") );
                dlg.setFileMode( QFileDialog::ExistingFiles );
                
                if ( dlg.exec() == QDialog::Accepted ) {
                    auto result = dlg.selectedFiles();
                    dataSequenceTree_->setData( result );
                }
            } );

        connect( toolButton, &QToolButton::clicked, this, [this] ( bool ){
                QString dstfile;
                if ( auto edit = findChild< QLineEdit *>() )
                    dstfile = edit->text();
                if ( dstfile.isEmpty() ) {
                    boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() );
                    dstfile = QString::fromStdWString( (dir / L"data").wstring() );
                }
                
                QString name = QFileDialog::getSaveFileName( this, "Data save in", dstfile, tr( "Quan result (*.adfs)" ) );
                if ( !name.isEmpty() ) 
                    if ( auto edit = findChild< QLineEdit *>() ) 
                        edit->setText( name );
            } );

        connect( execButton, &QToolButton::clicked, this, [this] (){ execute(); } );
        
        return toolBar;
    }
    return 0;
}

void
DataSequenceWidget::execute()
{
    QMessageBox::information(this, "Execute", "Execute pressed" );
}
