#include <QtWidgets/QApplication>
#include <QDirModel>
#include <QDeclarativeContext>
#include "qmlapplicationviewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QmlApplicationViewer viewer;

    QDirModel model;
    viewer.rootContext()->setContextProperty( "dirModel", &model);
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/qdirmodel/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
