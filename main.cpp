#include "MainWidget.h"
#include <QtWidgets/QApplication>

int Task::count = 0;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;
    w.show();
    return a.exec();
}
