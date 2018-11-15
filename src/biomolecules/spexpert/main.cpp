#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include "waittasklist.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<WaitTaskListTraits::TaskItem>();

    QSettings settings(QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + ".ini",QSettings::IniFormat);
    settings.beginGroup("MainApp");
    QString language = settings.value("language").toString();
    settings.endGroup();

    QTranslator translator;
    if (language.isEmpty()) {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
//        QLocale::setDefault(QLocale(QLocale::Czech, QLocale::CzechRepublic));
        translator.load(QCoreApplication::applicationDirPath() + "/languages/spexpert_en");
        a.installTranslator(&translator);
    } else {
        QLocale::setDefault(QLocale(language));
        translator.load(QCoreApplication::applicationDirPath() + "/languages/spexpert_" + language);
        a.installTranslator(&translator);
    }
    qDebug() << "main.cpp: language:" << QLocale::languageToString(QLocale().language());

    MainWindow w;
    w.show();

    return a.exec();
}
