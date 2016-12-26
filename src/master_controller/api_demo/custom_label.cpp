#include "custom_label.h"

void MyCustomLabel::mousePressEvent(QMouseEvent * e)
{
    QLabel::mousePressEvent(e);

    emit clicked(e->pos());
}
