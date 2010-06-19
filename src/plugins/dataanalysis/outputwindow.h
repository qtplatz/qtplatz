#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QWidget>

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
  }
}

#endif // OUTPUTWINDOW_H
