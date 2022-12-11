#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMovie>
#include <QtWidgets>
#include <QMessageBox>

#include "settings_dialog.h"
#include "switcher.h"
#include "tray.h"

//#define UPDATE_IF_ERROR_MS 10000
//#define UPDATE_IF_UNKNOWN_MS 60000
//#define UPDATE_IF_NORMAL_MS 300000
#define UPDATE_IF_ERROR_MS 10000
#define UPDATE_IF_UNKNOWN_MS 20000
#define UPDATE_IF_NORMAL_MS 30000

tray::tray(run_settings ts, QObject *parent) : QObject(parent)
{
    run_settings_ = ts;

    pending_action_ = action::NONE;
    state_ = switcher::state::UNKNOWN;

    QString ini_file_name = "switcher";
    if (run_settings_.instance_name_is_set)
        ini_file_name += "_" + run_settings_.instance_name;
    ini_file_name += ".ini";

    QSettings app_settings(ini_file_name, QSettings::IniFormat);

    tray_settings_ = {};
    tray_settings_.normal_update_interval_sec = app_settings.value("normal_update_interval_sec", "300").toInt();
    tray_settings_.error_update_interval_sec = app_settings.value("error_update_interval_sec", "60").toInt();

    switcher_settings_ = {};
    switcher_settings_.host = app_settings.value("host", "127.0.0.1").toString();
    switcher_settings_.login = app_settings.value("login", "user").toString();
    switcher_settings_.password = app_settings.value("password", "rfhfcbr").toString();

    fastlabAction = new QAction("&Fastlab");
    fastlabAction->setIcon(QIcon(":/images/fastlab.png"));
    postwinAction = new QAction("&Postwin");
    postwinAction->setIcon(QIcon(":/images/postwin.png"));
    updateAction = new QAction("&Update");
    settingsAction = new QAction("&Settings");
    quitAction = new QAction("&Quit");

    trayIconMenu = new QMenu();
    auto act = trayIconMenu->addAction("127.0.0.1");
    act->setEnabled(false);

    trayIconMenu->addAction(fastlabAction);
    trayIconMenu->addAction(postwinAction);
    trayIconMenu->addAction(updateAction);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon();
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/unknown.png"));
    trayIcon->setToolTip("127.0.0.1");
    trayIcon->show();

    gif_update_ = new QMovie(":/images/update.gif");
    connect(gif_update_, &QMovie::frameChanged, this, &tray::updateIconUpdate);

    gif_switch_ = new QMovie(":/images/switch.gif");
    connect(gif_switch_, &QMovie::frameChanged, this, &tray::updateIconSwitch);

    gif_cancel_ = new QMovie(":/images/cancel.gif");
    connect(gif_cancel_, &QMovie::frameChanged, this, &tray::updateIconCancel);

    switcher_.reset(new switcher());
    QObject::connect(switcher_.get(), &switcher::on_state_changed, this, &tray::switcher_state_changed);

    QObject::connect(fastlabAction, &QAction::triggered, this, &tray::fastlab);
    QObject::connect(postwinAction, &QAction::triggered, this, &tray::postwin);
    QObject::connect(updateAction, &QAction::triggered, this, &tray::update);
    QObject::connect(settingsAction, &QAction::triggered, this, &tray::settings);
    QObject::connect(quitAction, &QAction::triggered, this, &tray::quit);

    timer_id_ = startTimer(1000);
    update_time_ = QDateTime::currentSecsSinceEpoch();
}

tray::~tray()
{
    killTimer(timer_id_);

    QString ini_file_name = "switcher";
    if (run_settings_.instance_name_is_set)
        ini_file_name += "_" + run_settings_.instance_name;
    ini_file_name += ".ini";

    QSettings app_settings(ini_file_name, QSettings::IniFormat);
    app_settings.setValue("normal_update_interval_sec", tray_settings_.normal_update_interval_sec);
    app_settings.setValue("error_update_interval_sec", tray_settings_.error_update_interval_sec);
    app_settings.setValue("host", switcher_settings_.host);
    app_settings.setValue("login", switcher_settings_.login);
    app_settings.setValue("password", switcher_settings_.password);
    app_settings.sync();
}

void tray::timerEvent(QTimerEvent* event)
{
    if (timer_id_ == event->timerId())
    {
        tray_settings ts{};
        {
            // tray_settings_mutex_ locked
            QMutexLocker locker(&tray_settings_mutex_);
            ts = tray_settings_;
        }

        quint64 ut{};
        {
            // update_time_mutex_ locked
            QMutexLocker locker(&update_time_mutex_);
            ut = update_time_;
        }

        const qint64 ct = QDateTime::currentSecsSinceEpoch();
        bool updated = false;
        {
            // pending_action_mutex_ locked
            QMutexLocker locker(&pending_action_mutex_);

            if ((state_ == switcher::state::ERROR_ && (ct - ut > ts.error_update_interval_sec)) ||
                (state_ == switcher::state::UNKNOWN && (ct - ut > ts.normal_update_interval_sec)) ||
                (state_ == switcher::state::FASTLAB && (ct - ut > ts.normal_update_interval_sec)) ||
                (state_ == switcher::state::POSTWIN && (ct - ut > ts.normal_update_interval_sec)))
            {
                qDebug() << "timerEvent, time elapced = " << (ct - ut);
                gif_cancel_->start();

                updated = true;
                pending_action_ = action::UPDATE;
                switcher_->cancel_async();
            }
        }

        if (updated)
        {
            // update_time_mutex_ locked
            QMutexLocker locker(&update_time_mutex_);
            update_time_ = ct;
        }
    }
    QObject::timerEvent(event);
}

void tray::show()
{
    trayIcon->show();
}

void tray::fastlab()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Switch to fastlab", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    bool ok = (reply != QMessageBox::Yes);
    switcher_->set_result(ok);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_cancel_->start();

        pending_action_ = action::SWITCH_TO_FASTLAB;
        switcher_->cancel_async();
    }
}

void tray::postwin()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Switch to postwin", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    bool ok = (reply != QMessageBox::Yes);
    switcher_->set_result(ok);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_cancel_->start();

        pending_action_ = action::SWITCH_TO_POSTWIN;
        switcher_->cancel_async();
    }
}

void tray::update()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Refresh", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    bool ok = (reply != QMessageBox::Yes);
    switcher_->set_result(ok);

    QMessageBox::StandardButton reply2;
    reply2 = QMessageBox::question(nullptr, "Refresh", "Refresh to Yes=Fastlab, No=Postwin, Abort=Unknown?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
    switcher::state ok2 = switcher::state::ERROR_;
    if (reply2 == QMessageBox::Yes) ok2 = switcher::state::FASTLAB;
    if (reply2 == QMessageBox::No) ok2 = switcher::state::POSTWIN;
    if (reply2 == QMessageBox::Abort) ok2 = switcher::state::UNKNOWN;
    switcher_->set_refresh_result(ok2);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_cancel_->start();

        pending_action_ = action::UPDATE;
        switcher_->cancel_async();
    }
}

void tray::settings()
{
    tray_settings ts{};
    switcher_settings ss{};
    {
        // tray_settings_mutex_ locked
        QMutexLocker locker(&tray_settings_mutex_);
        ts = tray_settings_;
        ss = switcher_settings_;
    }

    settings_dialog* sd = new settings_dialog();
    sd->set_settings(ts, ss);
    if (QDialog::Accepted == sd->exec())
    {
        qDebug() << "settings accepted";
        {
            // update_time_mutex_ locked
            QMutexLocker locker(&update_time_mutex_);
            update_time_ = QDateTime::currentSecsSinceEpoch();
        }

        sd->get_settings(ts, ss);
        {
            // tray_settings_mutex_ locked
            QMutexLocker locker(&tray_settings_mutex_);
            tray_settings_ = ts;
            switcher_settings_ = ss;
        }

        switcher_->apply_settings(ss);
        
        {
            // pending_action_mutex_ locked
            QMutexLocker locker(&pending_action_mutex_);

            gif_cancel_->start();

            pending_action_ = action::UPDATE;
            switcher_->cancel_async();
        }
    }
    sd->deleteLater();
}

void tray::quit()
{
    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        fastlabAction->setEnabled(false);
        postwinAction->setEnabled(false);
        updateAction->setEnabled(false);
        settingsAction->setEnabled(false);
        quitAction->setEnabled(false);
        gif_switch_->stop();
        gif_update_->stop();
        gif_cancel_->stop();

        gif_cancel_->start();

        pending_action_ = action::QUIT;
        switcher_->cancel_async();
    }
}

void tray::updateIconUpdate()
{
    trayIcon->setIcon(gif_update_->currentPixmap());
}

void tray::updateIconSwitch()
{
    trayIcon->setIcon(gif_switch_->currentPixmap());
}

void tray::updateIconCancel()
{
    trayIcon->setIcon(gif_cancel_->currentPixmap());
}

void tray::switcher_state_changed(switcher::state st)
{
    fastlabAction->setEnabled(true);
    postwinAction->setEnabled(true);
    updateAction->setEnabled(true);
    gif_switch_->stop();
    gif_update_->stop();
    gif_cancel_->stop();

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        if (st == switcher::state::ERROR_)
            pending_action_ = action::NONE;

        if (pending_action_ != action::NONE)
        {
            switch (pending_action_)
            {
            case tray::action::NONE:
                break;
            case tray::action::SWITCH_TO_FASTLAB:
                gif_switch_->start();
                switcher_->switch_to_fastlab_async();
                break;
            case tray::action::SWITCH_TO_POSTWIN:
                gif_switch_->start();
                switcher_->switch_to_postwin_async();
                break;
            case tray::action::UPDATE:
                gif_update_->start();
                switcher_->update_async();
                break;
            case tray::action::QUIT:
                trayIcon->hide();
                QCoreApplication::quit();
                break;
            default:
                break;
            }

            pending_action_ = action::NONE;
        }
        else
        {
            if (st == switcher::state::FASTLAB)
                trayIcon->setIcon(QIcon(":/images/fastlab.png"));
            else if (st == switcher::state::POSTWIN)
                trayIcon->setIcon(QIcon(":/images/postwin.png"));
            else if (st == switcher::state::UNKNOWN)
                trayIcon->setIcon(QIcon(":/images/unknown.png"));
            else if (st == switcher::state::ERROR_)
                trayIcon->setIcon(QIcon(":/images/error.png"));

            state_ = st;
        }
    }

    {
        // update_time_mutex_ locked
        QMutexLocker locker(&update_time_mutex_);
        update_time_ = QDateTime::currentSecsSinceEpoch();
    }
}
