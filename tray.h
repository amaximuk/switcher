#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include "switcher.h"

class QMenu;
class QAction;
class QSystemTrayIcon;
class QMovie;


class tray : public QObject
{
    Q_OBJECT

    QMenu *trayIconMenu;
    QAction* fastlabAction;
    QAction* postwinAction;
    QAction* refreshAction;
    QAction* quitAction;

//    QAction *minimizeAction;
//    QAction *maximizeAction;
//    QAction *restoreAction;
//    QAction *xxxAction;
//    QAction *quitAction;
    QMovie* gif_update_;
    QMovie* gif_switch_;
    QSystemTrayIcon *trayIcon;

    QScopedPointer<switcher> switcher_;

public:
    explicit tray(QObject *parent = nullptr);

    void show();

    void fastlab();
    void postwin();
    void refresh();
    void quit();

private:
    void updateIconUpdate();
    void updateIconSwitch();

private slots:
    void switcher_state_changed(switcher::state st);

signals:

};

#endif // TRAY_H
