#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QWidget>
#include <QtGui/QPlainTextEdit>

class QPlainTextEdit;
class QLineEdit;

namespace DataAnalysis {
  namespace Internal {
    
    class OutputWindow : public QWidget  {
      Q_OBJECT
	public:
      ~OutputWindow();
      explicit OutputWindow(QWidget *parent = 0);
      
    signals:
      void showPage();
      void statusMessageRequested( const QString& msg, int );

      public slots:
      void clearContents();
      void showOutput( int channel, const QString& output );
      void showInput( int channel, const QString& input );
      void setCursor( const QCursor& );
      
    private:
      QPlainTextEdit* combindText_;
      QPlainTextEdit* inputText_;
      QLineEdit* commandEdit_;

    };

    class DebuggerPane : public QPlainTextEdit {
      Q_OBJECT
    public:
      virtual ~DebuggerPane() {}
      DebuggerPane( QWidget * parent ) : QPlainTextEdit( parent ) {
      }
	virtual void addContextActions( QMenu * ) {}
	void contextMenuEvent(QContextMenuEvent *e) {
	}
    };
    
    class CombinedPane : public DebuggerPane {
      Q_OBJECT
    public:
      virtual ~CombinedPane() {}
      CombinedPane( QWidget * parent ) : DebuggerPane( parent ) { }
    };
    
    class InputPane : public DebuggerPane {
      Q_OBJECT
    public:
      virtual ~InputPane() {}
      InputPane( QWidget * parent ) : DebuggerPane( parent ) { }
    };

  }
}

#endif // OUTPUTWINDOW_H
