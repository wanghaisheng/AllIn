/****************************************************************************
** Meta object code from reading C++ file 'qtdemo.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtdemo.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtdemo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QtDemo_t {
    QByteArrayData data[25];
    char stringdata[267];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_QtDemo_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_QtDemo_t qt_meta_stringdata_QtDemo = {
    {
QT_MOC_LITERAL(0, 0, 6),
QT_MOC_LITERAL(1, 7, 8),
QT_MOC_LITERAL(2, 16, 0),
QT_MOC_LITERAL(3, 17, 13),
QT_MOC_LITERAL(4, 31, 11),
QT_MOC_LITERAL(5, 43, 4),
QT_MOC_LITERAL(6, 48, 3),
QT_MOC_LITERAL(7, 52, 14),
QT_MOC_LITERAL(8, 67, 8),
QT_MOC_LITERAL(9, 76, 5),
QT_MOC_LITERAL(10, 82, 13),
QT_MOC_LITERAL(11, 96, 15),
QT_MOC_LITERAL(12, 112, 5),
QT_MOC_LITERAL(13, 118, 19),
QT_MOC_LITERAL(14, 138, 15),
QT_MOC_LITERAL(15, 154, 19),
QT_MOC_LITERAL(16, 174, 8),
QT_MOC_LITERAL(17, 183, 8),
QT_MOC_LITERAL(18, 192, 8),
QT_MOC_LITERAL(19, 201, 8),
QT_MOC_LITERAL(20, 210, 8),
QT_MOC_LITERAL(21, 219, 8),
QT_MOC_LITERAL(22, 228, 19),
QT_MOC_LITERAL(23, 248, 7),
QT_MOC_LITERAL(24, 256, 9)
    },
    "QtDemo\0explains\0\0ConnectStatus\0"
    "const char*\0path\0msg\0UpdateProgress\0"
    "progress\0total\0HandleConnect\0"
    "HandleTabChange\0index\0HandleErrCodeChange\0"
    "HandleSelectImg\0HandleCamListChange\0"
    "Stamper1\0Stamper2\0Stamper3\0Stamper4\0"
    "Stamper5\0Stamper6\0HandleCheckStampInk\0"
    "checked\0TimerDone\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDemo[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06,
       3,    2,   95,    2, 0x06,
       7,    2,  100,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
      10,    2,  105,    2, 0x08,
      11,    1,  110,    2, 0x08,
      13,    1,  113,    2, 0x08,
      14,    1,  116,    2, 0x08,
      15,    1,  119,    2, 0x08,
      16,    0,  122,    2, 0x08,
      17,    0,  123,    2, 0x08,
      18,    0,  124,    2, 0x08,
      19,    0,  125,    2, 0x08,
      20,    0,  126,    2, 0x08,
      21,    0,  127,    2, 0x08,
      22,    1,  128,    2, 0x08,
      24,    0,  131,    2, 0x08,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, QMetaType::UInt,    5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    8,    9,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4, QMetaType::UInt,    5,    6,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   23,
    QMetaType::Void,

       0        // eod
};

void QtDemo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDemo *_t = static_cast<QtDemo *>(_o);
        switch (_id) {
        case 0: _t->explains(); break;
        case 1: _t->ConnectStatus((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 2: _t->UpdateProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->HandleConnect((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 4: _t->HandleTabChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->HandleErrCodeChange((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->HandleSelectImg((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->HandleCamListChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->Stamper1(); break;
        case 9: _t->Stamper2(); break;
        case 10: _t->Stamper3(); break;
        case 11: _t->Stamper4(); break;
        case 12: _t->Stamper5(); break;
        case 13: _t->Stamper6(); break;
        case 14: _t->HandleCheckStampInk((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->TimerDone(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtDemo::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDemo::explains)) {
                *result = 0;
            }
        }
        {
            typedef void (QtDemo::*_t)(const char * , unsigned int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDemo::ConnectStatus)) {
                *result = 1;
            }
        }
        {
            typedef void (QtDemo::*_t)(int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDemo::UpdateProgress)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject QtDemo::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_QtDemo.data,
      qt_meta_data_QtDemo,  qt_static_metacall, 0, 0}
};


const QMetaObject *QtDemo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDemo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDemo.stringdata))
        return static_cast<void*>(const_cast< QtDemo*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int QtDemo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void QtDemo::explains()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QtDemo::ConnectStatus(const char * _t1, unsigned int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtDemo::UpdateProgress(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
