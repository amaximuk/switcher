/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMovie>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QMessageBox>

#include "settings.h"
#include "tray.h"

//QMovie* gif;
//QSystemTrayIcon *trayIcon;

//void xxx()
//{
//    qDebug() << "!!!!!!!!!!!!!!!!!!";
//}

//void updateIcon()
//{
//    trayIcon->setIcon(gif->currentPixmap());
//}

//void quit()
//{
//    qDebug() << "!!!!!!!!!!!!!!!!!!";
//}


int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(switcher);

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);





    QCommandLineParser parser;
    parser.setApplicationDescription("Application Switcher");
    parser.addHelpOption();
    parser.addVersionOption();
    // An option with a value
    QCommandLineOption instanceNameOption(QStringList() << "i" << "instance",
            "Set instance name to <instance-name>.",
            "instance-name");
    parser.addOption(instanceNameOption);
    parser.process(app);

    run_settings ts{};
    ts.instance_name_is_set = parser.isSet(instanceNameOption);
    if (ts.instance_name_is_set)
        ts.instance_name = parser.value(instanceNameOption);


    //tray_settings ts{};
    //QSettings app_settings(ini_file_name, QSettings::IniFormat);
    //ts.host = app_settings.value("host", "127.0.0.1").toString();
    //ts.login = app_settings.value("login", "user").toString();
    //ts.password = app_settings.value("password", "rfhfcbr").toString();
    //ts.normal_update_interval_sec = app_settings.value("normal_update_interval_sec", "300").toInt();
    //ts.error_update_interval_sec = app_settings.value("error_update_interval_sec", "60").toInt();


    //app_settings.setValue("host", ts.host);
    //app_settings.setValue("login", ts.login);
    //app_settings.setValue("password", ts.password);
    //app_settings.setValue("normal_update_interval_sec", ts.normal_update_interval_sec);
    //app_settings.setValue("error_update_interval_sec", ts.error_update_interval_sec);
    //app_settings.sync();



    tray main_tray(ts);


    //tray* aaa = new tray();
    //aaa->deleteLater();
    //aaa->settings();

//    tray* main_tray = new tray();
//    QObject::connect(&app, &QCoreApplication::aboutToQuit, main_tray, &tray::hide);

//    main_tray->show();





//    QMenu *trayIconMenu;
////    QAction *minimizeAction;
////    QAction *maximizeAction;
////    QAction *restoreAction;
//    QAction *xxxAction;
//    QAction *quitAction;

//    quitAction = new QAction("&Quit", &app);
//    xxxAction = new QAction("&xxx", &app);
//  //  QObject::connect(xxxAction, &QAction::triggered, xxx);
//    QObject::connect(quitAction, &QAction::triggered, &app, &QCoreApplication::quit);


//    trayIconMenu = new QMenu();
////    trayIconMenu->addAction(minimizeAction);
////    trayIconMenu->addAction(maximizeAction);
////    trayIconMenu->addAction(restoreAction);
////    trayIconMenu->addSeparator();
//    trayIconMenu->addAction(quitAction);
//    trayIconMenu->addAction(xxxAction);

//    trayIcon = new QSystemTrayIcon();
//    trayIcon->setContextMenu(trayIconMenu);
//    QIcon icon = QIcon(":/images/heart.png");
//    trayIcon->setIcon(icon);
//    trayIcon->show();

//    QObject::connect(&app, &QCoreApplication::aboutToQuit, trayIcon, &QSystemTrayIcon::hide);





//    gif = new QMovie(":/images/refresh.gif");
//    QObject::connect(gif, &QMovie::frameChanged, updateIcon);
//    gif->start();






//    Window window;
//    window.show();
    return app.exec();
}

#else

#include <QLabel>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString text("QSystemTrayIcon is not supported on this platform");

    QLabel *label = new QLabel(text);
    label->setWordWrap(true);

    label->show();
    qDebug() << text;

    app.exec();
}

#endif
