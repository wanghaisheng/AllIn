#include "qtdemo.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>

extern QString font_family;
extern int font_size;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtDemo w;
    QFont f(font_family, font_size);
    a.setFont(f);
    w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint);
    if (!w.Init()) {
        perror("fails to init\n");
        return -1;
    }

    w.show();
    w.AutoOpen();
    return a.exec();
}
