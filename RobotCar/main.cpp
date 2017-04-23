#include "robotcar.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RobotCar w;
    w.show();

    return a.exec();
}
