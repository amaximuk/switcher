#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include "settings.h"
#include "logging_categories.h"
#include "switcher.h"

class QMenu;
class QAction;
class QSystemTrayIcon;
class QMovie;


class tray : public QObject
{
    Q_OBJECT

private:
    enum class ACTION
    {
        NONE,
        SWITCH_TO_FASTLAB,
        SWITCH_TO_POSTWIN,
        UPDATE,
        QUIT
    };

private:
    run_settings run_settings_;
    QString ini_file_name_;

    QMutex tray_settings_mutex_;
    tray_settings tray_settings_;

    //switcher_settings switcher_settings_;

    QMenu *tray_icon_menu_;
    QAction* header_action_;
    QAction* fastlab_action_;
    QAction* postwin_action_;
    QAction* update_action_;
    QAction* settings_action_;
    QAction* quit_action_;

    QMovie* gif_update_;
    QMovie* gif_switch_;
    QMovie* gif_cancel_;
    QSystemTrayIcon *tray_icon_;

    QScopedPointer<switcher> switcher_;

    QMutex pending_action_mutex_;
    ACTION pending_action_;
    switcher::STATE state_;

    QMutex update_time_mutex_;
    quint64 update_time_;
    int timer_id_;

protected:
    void timerEvent(QTimerEvent* event) override;

public:
    explicit tray(run_settings rs, QObject *parent = nullptr);
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
    void switcher_state_changed(switcher::STATE state, QString host, QString message);
    void tray_icon_activated(QSystemTrayIcon::ActivationReason ar);

signals:

};

#endif // TRAY_H
