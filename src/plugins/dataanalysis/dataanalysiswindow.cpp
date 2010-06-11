/* Copyright (C) 2010 Toshinobu Hondo, Ph.D.
   Science Liaison Project
   */

#include "dataanalysiswindow.h"

#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtWebKit/QWebView>
#include <QPlainTextEdit>

using namespace DataAnalysis::Internal;

struct DataAnalysisWindowData {
	QWebView * webView_;
    QPlainTextEdit * textEdit_;
    bool modified_;
    QString path_;
    DataAnalysisWindowData() : webView_(0), textEdit_(0), modified_(false) {}
};

DataAnalysisWindow::~DataAnalysisWindow()
{
    delete d_;
}

DataAnalysisWindow::DataAnalysisWindow( QWidget * parent ) : QTabWidget(parent)
                                                           , d_(0)
{
  d_ = new DataAnalysisWindowData();

  if ( d_ ) {
      d_->webView_ = new QWebView;
      d_->textEdit_ = new QPlainTextEdit;

      addTab( d_->webView_, "Preview" );
      addTab( d_->textEdit_, "Source" );
      setTabPosition( QTabWidget::South );
      setTabShape( QTabWidget::Triangular );

      d_->textEdit_->setFont( QFont("Courier", 12) );

      connect(this, SIGNAL(currentChanged(int)), this, SLOT( slotCurrentTabChanged(int) ) );
      connect(d_->textEdit_, SIGNAL( textChanged() ), this, SLOT( slotContentModified() ) );
      connect(d_->webView_, SIGNAL( titleChanged(QString) ), this, SIGNAL(titleChanged(QString)) );
  }
  /*
  QBoxLayout * layout = new QVBoxLayout( this );
  layout->addWidget( new QTextEdit( tr("Focus me to activate my context!") ) );
  setWindowTitle( tr("Data Analysis") );
  */
}

void
DataAnalysisWindow::setContent(const QByteArray& ba, const QString& path)
{
  if ( path.isEmpty() )
    d_->webView_->setHtml(ba);
  else
    d_->webView_->setHtml( ba, "file:///" + path );
  d_->textEdit_->setPlainText( ba );
  d_->modified_ = false;
  d_->path_ = path;
}


QByteArray
DataAnalysisWindow::content() const
{
  QString htmlText = d_->textEdit_->toPlainText();
  return htmlText.toAscii();
}

QString
DataAnalysisWindow::title() const
{
    return d_->webView_->title();
}

void
DataAnalysisWindow::slotCurrentTabChanged(int tab)
{
  if ( tab == 0 && d_->modified_ )
    setContent( content(), d_->path_ );
}

void
DataAnalysisWindow::slotContentModified()
{
    d_->modified_ = true;
    emit contentModified();
}
