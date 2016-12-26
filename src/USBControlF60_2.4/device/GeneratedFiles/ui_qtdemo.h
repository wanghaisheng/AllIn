/********************************************************************************
** Form generated from reading UI file 'qtdemo.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDEMO_H
#define UI_QTDEMO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtDemoClass
{
public:
    QWidget *centralWidget;
    QPushButton *btn_open_dev;
    QPushButton *btn_close_dev;
    QPushButton *pb_redetect_;
    QGroupBox *groupBox;
    QLabel *label;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *paper_cam_status_;
    QLabel *env_cam_status_;
    QLabel *safe_cam_status_;
    QPushButton *paper_cam_preview_;
    QPushButton *env_cam_preview_;
    QPushButton *safe_cam_preview_;
    QLabel *camera_preview_;
    QGroupBox *groupBox_2;
    QLabel *label_8;
    QLabel *slot1_;
    QLabel *slot2_;
    QLabel *label_11;
    QLabel *slot3_;
    QLabel *label_13;
    QLabel *slot4_;
    QLabel *label_15;
    QLabel *slot5_;
    QLabel *label_17;
    QLabel *slot6_;
    QLabel *label_19;
    QGroupBox *groupBox_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_20;
    QLabel *safe_door_status_;
    QLabel *paper_door_status_;
    QLabel *top_door_status_;
    QGroupBox *groupBox_4;
    QLabel *label_2;
    QLabel *dev_code_;
    QLabel *label_3;
    QLabel *dev_status_;
    QGroupBox *groupBox_5;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *door_alarm_status_;
    QLabel *vibration_alarm_status_;
    QGroupBox *groupBox_6;
    QPushButton *pb_error_des_;
    QLabel *err_code_;
    QPushButton *logo_img_;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtDemoClass)
    {
        if (QtDemoClass->objectName().isEmpty())
            QtDemoClass->setObjectName(QStringLiteral("QtDemoClass"));
        QtDemoClass->resize(964, 820);
        centralWidget = new QWidget(QtDemoClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        btn_open_dev = new QPushButton(centralWidget);
        btn_open_dev->setObjectName(QStringLiteral("btn_open_dev"));
        btn_open_dev->setGeometry(QRect(240, 0, 93, 28));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font.setPointSize(9);
        btn_open_dev->setFont(font);
        btn_close_dev = new QPushButton(centralWidget);
        btn_close_dev->setObjectName(QStringLiteral("btn_close_dev"));
        btn_close_dev->setGeometry(QRect(350, 0, 93, 28));
        btn_close_dev->setFont(font);
        pb_redetect_ = new QPushButton(centralWidget);
        pb_redetect_->setObjectName(QStringLiteral("pb_redetect_"));
        pb_redetect_->setGeometry(QRect(730, 520, 81, 61));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        pb_redetect_->setFont(font1);
        pb_redetect_->setAutoFillBackground(false);
        pb_redetect_->setStyleSheet(QStringLiteral("background-color: rgb(255, 0, 0);"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 390, 571, 361));
        groupBox->setFont(font1);
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 70, 91, 16));
        label->setFont(font1);
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(10, 170, 91, 16));
        label_6->setFont(font1);
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(10, 290, 101, 16));
        label_7->setFont(font1);
        paper_cam_status_ = new QLabel(groupBox);
        paper_cam_status_->setObjectName(QStringLiteral("paper_cam_status_"));
        paper_cam_status_->setGeometry(QRect(120, 70, 31, 16));
        QFont font2;
        font2.setBold(false);
        font2.setWeight(50);
        paper_cam_status_->setFont(font2);
        paper_cam_status_->setAutoFillBackground(true);
        env_cam_status_ = new QLabel(groupBox);
        env_cam_status_->setObjectName(QStringLiteral("env_cam_status_"));
        env_cam_status_->setGeometry(QRect(120, 170, 31, 16));
        env_cam_status_->setFont(font2);
        env_cam_status_->setAutoFillBackground(true);
        safe_cam_status_ = new QLabel(groupBox);
        safe_cam_status_->setObjectName(QStringLiteral("safe_cam_status_"));
        safe_cam_status_->setGeometry(QRect(120, 290, 31, 16));
        safe_cam_status_->setFont(font2);
        safe_cam_status_->setAutoFillBackground(true);
        paper_cam_preview_ = new QPushButton(groupBox);
        paper_cam_preview_->setObjectName(QStringLiteral("paper_cam_preview_"));
        paper_cam_preview_->setGeometry(QRect(170, 70, 51, 21));
        paper_cam_preview_->setFont(font2);
        env_cam_preview_ = new QPushButton(groupBox);
        env_cam_preview_->setObjectName(QStringLiteral("env_cam_preview_"));
        env_cam_preview_->setGeometry(QRect(170, 170, 51, 21));
        env_cam_preview_->setFont(font2);
        safe_cam_preview_ = new QPushButton(groupBox);
        safe_cam_preview_->setObjectName(QStringLiteral("safe_cam_preview_"));
        safe_cam_preview_->setGeometry(QRect(170, 290, 51, 21));
        safe_cam_preview_->setFont(font2);
        camera_preview_ = new QLabel(groupBox);
        camera_preview_->setObjectName(QStringLiteral("camera_preview_"));
        camera_preview_->setGeometry(QRect(230, 10, 341, 351));
        camera_preview_->setFrameShape(QFrame::WinPanel);
        camera_preview_->setFrameShadow(QFrame::Sunken);
        camera_preview_->setLineWidth(1);
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 220, 571, 121));
        groupBox_2->setFont(font1);
        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(0, 30, 53, 16));
        slot1_ = new QLabel(groupBox_2);
        slot1_->setObjectName(QStringLiteral("slot1_"));
        slot1_->setGeometry(QRect(60, 30, 91, 16));
        slot1_->setFont(font2);
        slot1_->setAutoFillBackground(true);
        slot2_ = new QLabel(groupBox_2);
        slot2_->setObjectName(QStringLiteral("slot2_"));
        slot2_->setGeometry(QRect(240, 30, 91, 16));
        slot2_->setFont(font2);
        slot2_->setAutoFillBackground(true);
        label_11 = new QLabel(groupBox_2);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setGeometry(QRect(180, 30, 53, 16));
        slot3_ = new QLabel(groupBox_2);
        slot3_->setObjectName(QStringLiteral("slot3_"));
        slot3_->setGeometry(QRect(430, 30, 91, 16));
        slot3_->setFont(font2);
        slot3_->setAutoFillBackground(true);
        label_13 = new QLabel(groupBox_2);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setGeometry(QRect(370, 30, 53, 16));
        slot4_ = new QLabel(groupBox_2);
        slot4_->setObjectName(QStringLiteral("slot4_"));
        slot4_->setGeometry(QRect(60, 80, 91, 16));
        slot4_->setFont(font2);
        slot4_->setAutoFillBackground(true);
        label_15 = new QLabel(groupBox_2);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setGeometry(QRect(0, 80, 53, 16));
        slot5_ = new QLabel(groupBox_2);
        slot5_->setObjectName(QStringLiteral("slot5_"));
        slot5_->setGeometry(QRect(240, 80, 91, 16));
        slot5_->setFont(font2);
        slot5_->setAutoFillBackground(true);
        label_17 = new QLabel(groupBox_2);
        label_17->setObjectName(QStringLiteral("label_17"));
        label_17->setGeometry(QRect(180, 80, 53, 16));
        slot6_ = new QLabel(groupBox_2);
        slot6_->setObjectName(QStringLiteral("slot6_"));
        slot6_->setGeometry(QRect(430, 80, 91, 16));
        slot6_->setFont(font2);
        slot6_->setAutoFillBackground(true);
        label_19 = new QLabel(groupBox_2);
        label_19->setObjectName(QStringLiteral("label_19"));
        label_19->setGeometry(QRect(370, 80, 53, 16));
        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(410, 40, 171, 141));
        groupBox_3->setFont(font1);
        label_4 = new QLabel(groupBox_3);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(10, 30, 91, 16));
        label_4->setFont(font1);
        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(10, 70, 91, 16));
        label_5->setFont(font1);
        label_20 = new QLabel(groupBox_3);
        label_20->setObjectName(QStringLiteral("label_20"));
        label_20->setGeometry(QRect(10, 110, 53, 16));
        label_20->setFont(font1);
        safe_door_status_ = new QLabel(groupBox_3);
        safe_door_status_->setObjectName(QStringLiteral("safe_door_status_"));
        safe_door_status_->setGeometry(QRect(100, 30, 41, 16));
        safe_door_status_->setFont(font2);
        safe_door_status_->setAutoFillBackground(true);
        paper_door_status_ = new QLabel(groupBox_3);
        paper_door_status_->setObjectName(QStringLiteral("paper_door_status_"));
        paper_door_status_->setGeometry(QRect(100, 70, 41, 16));
        paper_door_status_->setFont(font2);
        paper_door_status_->setAutoFillBackground(true);
        top_door_status_ = new QLabel(groupBox_3);
        top_door_status_->setObjectName(QStringLiteral("top_door_status_"));
        top_door_status_->setGeometry(QRect(100, 110, 41, 16));
        top_door_status_->setFont(font2);
        top_door_status_->setAutoFillBackground(true);
        groupBox_4 = new QGroupBox(centralWidget);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(10, 40, 381, 141));
        groupBox_4->setFont(font1);
        label_2 = new QLabel(groupBox_4);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 40, 91, 16));
        label_2->setFont(font1);
        dev_code_ = new QLabel(groupBox_4);
        dev_code_->setObjectName(QStringLiteral("dev_code_"));
        dev_code_->setGeometry(QRect(90, 40, 271, 16));
        dev_code_->setFont(font2);
        dev_code_->setAutoFillBackground(true);
        dev_code_->setWordWrap(true);
        label_3 = new QLabel(groupBox_4);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 90, 91, 16));
        label_3->setFont(font1);
        dev_status_ = new QLabel(groupBox_4);
        dev_status_->setObjectName(QStringLiteral("dev_status_"));
        dev_status_->setGeometry(QRect(90, 90, 271, 16));
        dev_status_->setFont(font2);
        dev_status_->setAutoFillBackground(true);
        groupBox_5 = new QGroupBox(centralWidget);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(670, 40, 251, 141));
        groupBox_5->setFont(font1);
        label_9 = new QLabel(groupBox_5);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(10, 40, 61, 16));
        label_10 = new QLabel(groupBox_5);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(10, 90, 71, 16));
        door_alarm_status_ = new QLabel(groupBox_5);
        door_alarm_status_->setObjectName(QStringLiteral("door_alarm_status_"));
        door_alarm_status_->setGeometry(QRect(100, 40, 41, 16));
        door_alarm_status_->setFont(font2);
        door_alarm_status_->setAutoFillBackground(true);
        vibration_alarm_status_ = new QLabel(groupBox_5);
        vibration_alarm_status_->setObjectName(QStringLiteral("vibration_alarm_status_"));
        vibration_alarm_status_->setGeometry(QRect(100, 90, 41, 16));
        vibration_alarm_status_->setFont(font2);
        vibration_alarm_status_->setAutoFillBackground(true);
        groupBox_6 = new QGroupBox(centralWidget);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        groupBox_6->setGeometry(QRect(670, 220, 251, 121));
        groupBox_6->setFont(font1);
        pb_error_des_ = new QPushButton(groupBox_6);
        pb_error_des_->setObjectName(QStringLiteral("pb_error_des_"));
        pb_error_des_->setGeometry(QRect(80, 70, 91, 31));
        pb_error_des_->setFont(font2);
        err_code_ = new QLabel(groupBox_6);
        err_code_->setObjectName(QStringLiteral("err_code_"));
        err_code_->setGeometry(QRect(60, 30, 141, 20));
        err_code_->setFont(font2);
        err_code_->setAutoFillBackground(true);
        logo_img_ = new QPushButton(centralWidget);
        logo_img_->setObjectName(QStringLiteral("logo_img_"));
        logo_img_->setEnabled(false);
        logo_img_->setGeometry(QRect(440, 80, 251, 251));
        logo_img_->setAutoFillBackground(false);
        logo_img_->setStyleSheet(QStringLiteral("border-image: url(:/QtDemo/abc.jpg);"));
        logo_img_->setFlat(false);
        QtDemoClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtDemoClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 964, 26));
        QtDemoClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtDemoClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        QtDemoClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtDemoClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtDemoClass->setStatusBar(statusBar);

        retranslateUi(QtDemoClass);

        QMetaObject::connectSlotsByName(QtDemoClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtDemoClass)
    {
        QtDemoClass->setWindowTitle(QApplication::translate("QtDemoClass", "QtDemo", 0));
        btn_open_dev->setText(QApplication::translate("QtDemoClass", "\346\211\223\345\274\200\350\256\276\345\244\207", 0));
        btn_close_dev->setText(QApplication::translate("QtDemoClass", "\345\205\263\351\227\255\350\256\276\345\244\207", 0));
        pb_redetect_->setText(QApplication::translate("QtDemoClass", "\351\207\215\346\226\260\346\243\200\346\265\213", 0));
        groupBox->setTitle(QApplication::translate("QtDemoClass", "\346\221\204\345\203\217\345\244\264", 0));
        label->setText(QApplication::translate("QtDemoClass", "\345\207\255\350\257\201\346\221\204\345\203\217\345\244\264\357\274\232", 0));
        label_6->setText(QApplication::translate("QtDemoClass", "\347\216\257\345\242\203\346\221\204\345\203\217\345\244\264\357\274\232", 0));
        label_7->setText(QApplication::translate("QtDemoClass", "\345\256\211\345\205\250\351\227\250\346\221\204\345\203\217\345\244\264\357\274\232", 0));
        paper_cam_status_->setText(QApplication::translate("QtDemoClass", "\347\246\273\347\272\277", 0));
        env_cam_status_->setText(QApplication::translate("QtDemoClass", "\347\246\273\347\272\277", 0));
        safe_cam_status_->setText(QApplication::translate("QtDemoClass", "\347\246\273\347\272\277", 0));
        paper_cam_preview_->setText(QApplication::translate("QtDemoClass", "\351\242\204\350\247\210", 0));
        env_cam_preview_->setText(QApplication::translate("QtDemoClass", "\351\242\204\350\247\210", 0));
        safe_cam_preview_->setText(QApplication::translate("QtDemoClass", "\351\242\204\350\247\210", 0));
        camera_preview_->setText(QString());
        groupBox_2->setTitle(QApplication::translate("QtDemoClass", "\345\215\260\347\253\240\345\210\227\350\241\250", 0));
        label_8->setText(QApplication::translate("QtDemoClass", "\345\215\241\346\247\2751:", 0));
        slot1_->setText(QString());
        slot2_->setText(QString());
        label_11->setText(QApplication::translate("QtDemoClass", "\345\215\241\346\247\2752:", 0));
        slot3_->setText(QString());
        label_13->setText(QApplication::translate("QtDemoClass", "\345\215\241\346\247\2753:", 0));
        slot4_->setText(QString());
        label_15->setText(QApplication::translate("QtDemoClass", "\345\215\241\346\247\2754:", 0));
        slot5_->setText(QString());
        label_17->setText(QApplication::translate("QtDemoClass", "\345\215\241\346\247\2755:", 0));
        slot6_->setText(QString());
        label_19->setText(QApplication::translate("QtDemoClass", "\345\215\241\346\247\2756:", 0));
        groupBox_3->setTitle(QApplication::translate("QtDemoClass", "\351\227\250\347\212\266\346\200\201", 0));
        label_4->setText(QApplication::translate("QtDemoClass", "\345\256\211\345\205\250\351\227\250\357\274\232", 0));
        label_5->setText(QApplication::translate("QtDemoClass", "\350\277\233\347\272\270\351\227\250\357\274\232", 0));
        label_20->setText(QApplication::translate("QtDemoClass", "\351\241\266\347\233\226\351\227\250\357\274\232", 0));
        safe_door_status_->setText(QString());
        paper_door_status_->setText(QString());
        top_door_status_->setText(QString());
        groupBox_4->setTitle(QApplication::translate("QtDemoClass", "\350\256\276\345\244\207", 0));
        label_2->setText(QApplication::translate("QtDemoClass", "\350\256\276\345\244\207\347\274\226\345\217\267\357\274\232", 0));
        dev_code_->setText(QString());
        label_3->setText(QApplication::translate("QtDemoClass", "\350\256\276\345\244\207\347\212\266\346\200\201\357\274\232", 0));
        dev_status_->setText(QString());
        groupBox_5->setTitle(QApplication::translate("QtDemoClass", "\346\212\245\350\255\246\345\231\250", 0));
        label_9->setText(QApplication::translate("QtDemoClass", "\351\227\250\346\212\245\350\255\246\357\274\232", 0));
        label_10->setText(QApplication::translate("QtDemoClass", "\346\214\257\345\212\250\346\212\245\350\255\246\357\274\232", 0));
        door_alarm_status_->setText(QString());
        vibration_alarm_status_->setText(QString());
        groupBox_6->setTitle(QApplication::translate("QtDemoClass", "\346\237\245\347\234\213\346\225\205\351\232\234", 0));
        pb_error_des_->setText(QApplication::translate("QtDemoClass", "\346\225\205\351\232\234\350\257\246\346\203\205", 0));
        err_code_->setText(QApplication::translate("QtDemoClass", "           \346\227\240\346\225\205\351\232\234", 0));
        logo_img_->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class QtDemoClass: public Ui_QtDemoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDEMO_H
