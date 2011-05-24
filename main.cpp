
#include <QtCore>
#include <QtDeclarative>

#include "qpython.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QPython::registerQML();

    /*QTextStream out(stderr);
    QPython py;
    py.importModule("pyotherside");
    out << py.evaluate("pyotherside.demo()");*/

    QDeclarativeView *view = new QDeclarativeView();
    view->setSource(QUrl("pyotherside.qml"));
    view->show();

    app.exec();

    return 0;
}

