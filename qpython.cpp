
#include "qpython.h"

#include <QtDeclarative>

int QPython::instances = 0;

void
QPython::registerQML()
{
    qmlRegisterType<QPython>("com.thpinfo.python", 1, 0, "Python");
}

