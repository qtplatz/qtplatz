#include "outputwindow.h"

#include <QtGui/QPlainTextEdit>
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>

using namespace DataAnalysis;
using namespace Internal;

OutputWindow::~OutputWindow()
{
}

OutputWindow::OutputWindow(QWidget *parent) : QWidget(parent)
					    , combindText_( new CombinedPane(this) )
{
  setWindowTitle( tr("Logging") );
  
  combindText_ = new CombinedPane( this );
  combindText_->setReadOnly( true );
  combindText_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  inputText_ = new InputPane( this );
  inputText_->setReadOnly( false );
  inputText_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


  QSplitter * splitter( new QSplitter(Qt::Horizontal, this) );
  splitter->addWidget( inputText_ );
  splitter->addWidget( combindText_ );
  splitter->setStretchFactor( 0, 1 );
  splitter->setStretchFactor( 1, 3 );

  QVBoxLayout * layout = new QVBoxLayout( this );
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->addWidget( splitter );
  setLayout( layout );
}

void
OutputWindow::showOutput(int channel, const QString& output )
{
}

void
OutputWindow::showInput( int channel, const QString& input )
{
}

void
OutputWindow::clearContents()
{
  combindText_->clear();
  inputText_->clear();
}

void
OutputWindow::setCursor( const QCursor& cursor )
{
  combindText_->setCursor( cursor );
  inputText_->setCursor( cursor );
}

// #include "outputwindow.moc"
