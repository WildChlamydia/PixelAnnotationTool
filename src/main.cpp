#include <QApplication>
#include <QFile>
#include "main_window.h"
#include <QtDebug>
#include <QTranslator>
#ifdef Q_OS_WIN
#include <QTextCodec>
#define _HAS_ITERATOR_DEBUGGING 0
#define _ITERATOR_DEBUG_LEVEL 0
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP1251"));
#endif

    QApplication app(argc, argv);

	MainWindow win;
    win.setWindowTitle("PixelAnnotationTool " + VERSION);
    win.setWindowIcon(QIcon("://icon.png"));

	win.show();

    return app.exec();
}
