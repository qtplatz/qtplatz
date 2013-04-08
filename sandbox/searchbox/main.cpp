#include <QtWidgets/QApplication>
#include "qmlapplicationviewer.h"
#include "appcontroller.hpp"
#include <QDeclarativeContext>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AppController controller;


    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    viewer.rootContext()->setContextProperty( "controller", &controller );

    viewer.setMainQmlFile(QLatin1String("qml/searchbox/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
