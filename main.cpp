
#include <QApplication>
#include <QTextStream>
#include <QDeclarativeView>
#include <QUrl>

#include "qpython.h"
#include "qpython_test.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    qpython_test();

    QPython::registerQML();

    QDeclarativeView *view = new QDeclarativeView();
    view->setSource(QUrl("pyotherside.qml"));
    view->show();

    app.exec();

    return 0;
}

