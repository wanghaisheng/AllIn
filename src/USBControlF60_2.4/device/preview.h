#include <QtWidgets/QDialog>
#include "ui_preview.h"

class CameraPreview: public QDialog {
private:
    Q_OBJECT

public:
    CameraPreview(QWidget *parent, const QString& title);

private:
    Ui::MyDialog ui;
};