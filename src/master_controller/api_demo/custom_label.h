#ifndef CUSTOM_LABEL_H_
#define CUSTOM_LABEL_H_

#include <QMouseEvent>
#include <qwidget.h>
#include <QLabel>

class MyCustomLabel : public QLabel
{
private:
    Q_OBJECT

public:
    MyCustomLabel(QWidget* wi) : QLabel(wi) {}

protected:
    void mousePressEvent(QMouseEvent * e);

signals:
    void clicked(const QPoint & pos);
};

#endif
