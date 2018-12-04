/****************************************************************************
** Meta object code from reading C++ file 'image_canvas.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/image_canvas.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'image_canvas.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ImageCanvas_t {
    QByteArrayData data[11];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ImageCanvas_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ImageCanvas_t qt_meta_stringdata_ImageCanvas = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ImageCanvas"
QT_MOC_LITERAL(1, 12, 12), // "scaleChanged"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 12), // "alphaChanged"
QT_MOC_LITERAL(4, 39, 10), // "setSizePen"
QT_MOC_LITERAL(5, 50, 9), // "clearMask"
QT_MOC_LITERAL(6, 60, 8), // "saveMask"
QT_MOC_LITERAL(7, 69, 4), // "undo"
QT_MOC_LITERAL(8, 74, 4), // "redo"
QT_MOC_LITERAL(9, 79, 11), // "scaleCanvas"
QT_MOC_LITERAL(10, 91, 5) // "delta"

    },
    "ImageCanvas\0scaleChanged\0\0alphaChanged\0"
    "setSizePen\0clearMask\0saveMask\0undo\0"
    "redo\0scaleCanvas\0delta"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ImageCanvas[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x0a /* Public */,
       3,    1,   57,    2, 0x0a /* Public */,
       4,    1,   60,    2, 0x0a /* Public */,
       5,    0,   63,    2, 0x0a /* Public */,
       6,    0,   64,    2, 0x0a /* Public */,
       7,    0,   65,    2, 0x0a /* Public */,
       8,    0,   66,    2, 0x0a /* Public */,
       9,    1,   67,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   10,

       0        // eod
};

void ImageCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ImageCanvas *_t = static_cast<ImageCanvas *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->scaleChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->alphaChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->setSizePen((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->clearMask(); break;
        case 4: _t->saveMask(); break;
        case 5: _t->undo(); break;
        case 6: _t->redo(); break;
        case 7: _t->scaleCanvas((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ImageCanvas::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_ImageCanvas.data,
      qt_meta_data_ImageCanvas,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ImageCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImageCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ImageCanvas.stringdata0))
        return static_cast<void*>(const_cast< ImageCanvas*>(this));
    return QLabel::qt_metacast(_clname);
}

int ImageCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
