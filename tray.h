#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include "settings.h"
#include "switcher.h"

class QMenu;
class QAction;
class QSystemTrayIcon;
class QMovie;


class tray : public QObject
{
    Q_OBJECT

private:
    enum class action
    {
        NONE,
        SWITCH_TO_FASTLAB,
        SWITCH_TO_POSTWIN,
        UPDATE,
        QUIT
    };

private:
    run_settings run_settings_;
    QMutex tray_settings_mutex_;
    tray_settings tray_settings_;
    switcher_settings switcher_settings_;

    QMenu *trayIconMenu;
    QAction* fastlabAction;
    QAction* postwinAction;
    QAction* updateAction;
    QAction* settingsAction;
    QAction* quitAction;

    QMovie* gif_update_;
    QMovie* gif_switch_;
    QMovie* gif_cancel_;
    QSystemTrayIcon *trayIcon;

    QScopedPointer<switcher> switcher_;

    action pending_action_;
    QMutex pending_action_mutex_;
    switcher::state state_;

    QMutex update_time_mutex_;
    quint64 update_time_;
    int timer_id_;

protected:
    void timerEvent(QTimerEvent* event) override;

public:
    explicit tray(run_settings ts, QObject *parent = nullptr);
    ~tray() override;

    void show();

    void fastlab();
    void postwin();
    void update();
    void settings();
    void quit();

private:
    void updateIconUpdate();
    void updateIconSwitch();
    void updateIconCancel();

private slots:
    void switcher_state_changed(switcher::state st);

signals:

};

#endif // TRAY_H
