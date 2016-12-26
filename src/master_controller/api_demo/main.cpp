#include "qtdemo.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtDemo w;
    w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint);
    if (!w.Init()) {
        perror("fails to init\n");
        return -1;
    }

    w.show();
    return a.exec();
}
