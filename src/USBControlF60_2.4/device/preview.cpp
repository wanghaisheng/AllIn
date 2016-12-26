#include "preview.h"

CameraPreview::CameraPreview(QWidget *parent, const QString& title) : QDialog(parent)
{
    ui.setupUi(this);
    setWindowTitle(title);
}
