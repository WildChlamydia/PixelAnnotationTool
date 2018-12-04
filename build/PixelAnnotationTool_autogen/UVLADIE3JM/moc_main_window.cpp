/****************************************************************************
** Meta object code from reading C++ file 'main_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/main_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[19];
    char stringdata0[280];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 8), // "autosave"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 11), // "changeLabel"
QT_MOC_LITERAL(4, 33, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(5, 50, 11), // "changeColor"
QT_MOC_LITERAL(6, 62, 4), // "item"
QT_MOC_LITERAL(7, 67, 14), // "saveConfigFile"
QT_MOC_LITERAL(8, 82, 14), // "loadConfigFile"
QT_MOC_LITERAL(9, 97, 12), // "runWatershed"
QT_MOC_LITERAL(10, 110, 37), // "on_tree_widget_img_currentIte..."
QT_MOC_LITERAL(11, 148, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(12, 165, 26), // "on_actionOpenDir_triggered"
QT_MOC_LITERAL(13, 192, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(14, 217, 8), // "closeTab"
QT_MOC_LITERAL(15, 226, 5), // "index"
QT_MOC_LITERAL(16, 232, 15), // "closeCurrentTab"
QT_MOC_LITERAL(17, 248, 13), // "updateConnect"
QT_MOC_LITERAL(18, 262, 17) // "treeWidgetClicked"

    },
    "MainWindow\0autosave\0\0changeLabel\0"
    "QListWidgetItem*\0changeColor\0item\0"
    "saveConfigFile\0loadConfigFile\0"
    "runWatershed\0on_tree_widget_img_currentItemChanged\0"
    "QTreeWidgetItem*\0on_actionOpenDir_triggered\0"
    "on_actionAbout_triggered\0closeTab\0"
    "index\0closeCurrentTab\0updateConnect\0"
    "treeWidgetClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x0a /* Public */,
       3,    2,   80,    2, 0x0a /* Public */,
       5,    1,   85,    2, 0x0a /* Public */,
       7,    0,   88,    2, 0x0a /* Public */,
       8,    0,   89,    2, 0x0a /* Public */,
       9,    0,   90,    2, 0x0a /* Public */,
      10,    2,   91,    2, 0x0a /* Public */,
      12,    0,   96,    2, 0x0a /* Public */,
      13,    0,   97,    2, 0x0a /* Public */,
      14,    1,   98,    2, 0x0a /* Public */,
      16,    0,  101,    2, 0x0a /* Public */,
      17,    1,  102,    2, 0x0a /* Public */,
      18,    0,  105,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 4,    2,    2,
    QMetaType::Void, 0x80000000 | 4,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 11,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->autosave(); break;
        case 1: _t->changeLabel((*reinterpret_cast< QListWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QListWidgetItem*(*)>(_a[2]))); break;
        case 2: _t->changeColor((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 3: _t->saveConfigFile(); break;
        case 4: _t->loadConfigFile(); break;
        case 5: _t->runWatershed(); break;
        case 6: _t->on_tree_widget_img_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 7: _t->on_actionOpenDir_triggered(); break;
        case 8: _t->on_actionAbout_triggered(); break;
        case 9: _t->closeTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->closeCurrentTab(); break;
        case 11: _t->updateConnect((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->treeWidgetClicked(); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "Ui::MainWindow"))
        return static_cast< Ui::MainWindow*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
