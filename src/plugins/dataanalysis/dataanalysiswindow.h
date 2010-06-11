/* Copyright (C) 2010 Toshinobu Hondo, Ph.D.
   Science Liaison Project
   */

#ifndef DATAANALYSISWINDOW_H
#define DATAANALYSISWINDOW_H

#include <QtGui/QWidget>
#include <QTabWidget>

QT_FORWARD_DECLARE_CLASS(QLabel);

struct DataAnalysisWindowData;

namespace DataAnalysis {
    namespace Internal {

		class DataAnalysisWindow : public QTabWidget {
			Q_OBJECT
		public:
			~DataAnalysisWindow();
			DataAnalysisWindow( QWidget * parent = 0 );

            void setContent( const QByteArray& ba, const QString& path = QString() );
            QByteArray content() const;
            QString title() const;
		protected slots:
            void slotCurrentTabChanged( int tab );
			void slotContentModified();

        signals:
            void contentModified();
            void titleChanged( const QString& );
		private:
                        DataAnalysisWindowData * d_;

		};
	}
}

#endif // DATAANALYSISWINDOW_H
