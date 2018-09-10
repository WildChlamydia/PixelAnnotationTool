/**
 * Image Annotation Tool for image annotations with pixelwise masks
 *
 * Author: Rudra Poudel
 */
#include <QApplication>
#include <QFile>
#include "main_window.h"
#include <QtDebug>
#include <QTranslator>
#ifdef Q_OS_WIN
#include <QTextCodec>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP1251"));
#endif

    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("://rus.qm");
    app.installTranslator(&qtTranslator);

	MainWindow win;
    win.setWindowIcon(QIcon("://icon.png"));

	win.show();

    return app.exec();
}
