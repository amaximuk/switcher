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

tray::tray(run_settings rs, QObject *parent) : QObject(parent)
{
    run_settings_ = rs;

    pending_action_ = ACTION::NONE;
    state_ = switcher::STATE::UNKNOWN;

    ini_file_name_ = "switcher";
    if (run_settings_.instance_name_is_set)
        ini_file_name_ += "_" + run_settings_.instance_name;
    ini_file_name_ += ".ini";


    tray_settings ts{};
    switcher_settings ss{};
    {
        QSettings app_settings(ini_file_name_, QSettings::IniFormat);
        ts.normal_update_interval_sec = app_settings.value("tray/normal_update_interval_sec", "300").toInt();
        ts.error_update_interval_sec = app_settings.value("tray/error_update_interval_sec", "60").toInt();
        ss.host = app_settings.value("switcher/host", "127.0.0.1").toString();
        ss.login = app_settings.value("switcher/login", "user").toString();
        ss.key = app_settings.value("switcher/key", "id_rsa").toString();
    }
    tray_settings_ = ts;

    header_action_ = new QAction(ss.host);
    header_action_->setEnabled(false);
    fastlab_action_ = new QAction("Fastlab");
    fastlab_action_->setIcon(QIcon(":/images/fastlab.png"));
    postwin_action_ = new QAction("Postwin");
    postwin_action_->setIcon(QIcon(":/images/postwin.png"));
    update_action_ = new QAction("Update");
    update_action_->setIcon(QIcon(":/images/unknown.png"));
    settings_action_ = new QAction("Settings");
    QFont f = settings_action_->font();
    f.setBold(true);
    settings_action_->setFont(f);
    quit_action_ = new QAction("Quit");

    tray_icon_menu_ = new QMenu();
    tray_icon_menu_->addAction(header_action_);
    tray_icon_menu_->addAction(fastlab_action_);
    tray_icon_menu_->addAction(postwin_action_);
    tray_icon_menu_->addAction(update_action_);
    tray_icon_menu_->addSeparator();
    tray_icon_menu_->addAction(settings_action_);
    tray_icon_menu_->addSeparator();
    tray_icon_menu_->addAction(quit_action_);

    tray_icon_ = new QSystemTrayIcon();
    tray_icon_->setContextMenu(tray_icon_menu_);
    tray_icon_->setIcon(QIcon(":/images/unknown.png"));
    tray_icon_->setToolTip(QString("%1\n%2").arg(ss.host, "Unknown"));
    tray_icon_->show();

    
    
    connect(tray_icon_, &QSystemTrayIcon::activated, this, &tray::tray_icon_activated);
    
    
    gif_update_ = new QMovie(":/images/update.gif");
    connect(gif_update_, &QMovie::frameChanged, this, &tray::updateIconUpdate);

    gif_switch_ = new QMovie(":/images/switch.gif");
    connect(gif_switch_, &QMovie::frameChanged, this, &tray::updateIconSwitch);

    gif_cancel_ = new QMovie(":/images/cancel.gif");
    connect(gif_cancel_, &QMovie::frameChanged, this, &tray::updateIconCancel);

    switcher_.reset(new switcher());
    switcher_->apply_settings(ss);
    QObject::connect(switcher_.get(), &switcher::on_state_changed, this, &tray::switcher_state_changed);

    QObject::connect(fastlab_action_, &QAction::triggered, this, &tray::fastlab);
    QObject::connect(postwin_action_, &QAction::triggered, this, &tray::postwin);
    QObject::connect(update_action_, &QAction::triggered, this, &tray::update);
    QObject::connect(settings_action_, &QAction::triggered, this, &tray::settings);
    QObject::connect(quit_action_, &QAction::triggered, this, &tray::quit);

    timer_id_ = startTimer(1000);
    update_time_ = QDateTime::currentSecsSinceEpoch();

    gif_cancel_->start();
    pending_action_ = ACTION::UPDATE;
    switcher_->cancel_async();
}

tray::~tray()
{
    killTimer(timer_id_);
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

            if ((state_ == switcher::STATE::ERROR_ && (ct - ut > ts.error_update_interval_sec)) ||
                (state_ == switcher::STATE::UNKNOWN && (ct - ut > ts.normal_update_interval_sec)) ||
                (state_ == switcher::STATE::FASTLAB && (ct - ut > ts.normal_update_interval_sec)) ||
                (state_ == switcher::STATE::POSTWIN && (ct - ut > ts.normal_update_interval_sec)))
            {
                qDebug() << "timerEvent, time elapced = " << (ct - ut);
                gif_switch_->stop();
                gif_update_->stop();
                gif_cancel_->start();

                updated = true;
                pending_action_ = ACTION::UPDATE;
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
    tray_icon_->show();
}

void tray::fastlab()
{
    //QMessageBox::StandardButton reply;
    //reply = QMessageBox::question(nullptr, "Switch to fastlab", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    //bool ok = (reply != QMessageBox::Yes);
    //switcher_->set_result(ok);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_switch_->stop();
        gif_update_->stop();
        gif_cancel_->start();

        pending_action_ = ACTION::SWITCH_TO_FASTLAB;
        switcher_->cancel_async();
    }
}

void tray::postwin()
{
    //QMessageBox::StandardButton reply;
    //reply = QMessageBox::question(nullptr, "Switch to postwin", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    //bool ok = (reply != QMessageBox::Yes);
    //switcher_->set_result(ok);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_switch_->stop();
        gif_update_->stop();
        gif_cancel_->start();

        pending_action_ = ACTION::SWITCH_TO_POSTWIN;
        switcher_->cancel_async();
    }
}

void tray::update()
{
    //QMessageBox::StandardButton reply;
    //reply = QMessageBox::question(nullptr, "Refresh", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    //bool ok = (reply != QMessageBox::Yes);
    //switcher_->set_result(ok);

    //QMessageBox::StandardButton reply2;
    //reply2 = QMessageBox::question(nullptr, "Refresh", "Refresh to Yes=Fastlab, No=Postwin, Abort=Unknown?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
    //switcher::state ok2 = switcher::STATE::ERROR_;
    //if (reply2 == QMessageBox::Yes) ok2 = switcher::STATE::FASTLAB;
    //if (reply2 == QMessageBox::No) ok2 = switcher::STATE::POSTWIN;
    //if (reply2 == QMessageBox::Abort) ok2 = switcher::STATE::UNKNOWN;
    //switcher_->set_refresh_result(ok2);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_switch_->stop();
        gif_update_->stop();
        gif_cancel_->start();

        pending_action_ = ACTION::UPDATE;
        switcher_->cancel_async();
    }
}

void tray::settings()
{
    tray_settings ts{};
    switcher_settings ss{};
    {
        QSettings app_settings(ini_file_name_, QSettings::IniFormat);
        ts.normal_update_interval_sec = app_settings.value("tray/normal_update_interval_sec", "300").toInt();
        ts.error_update_interval_sec = app_settings.value("tray/error_update_interval_sec", "60").toInt();
        ss.host = app_settings.value("switcher/host", "127.0.0.1").toString();
        ss.login = app_settings.value("switcher/login", "user").toString();
        ss.key = app_settings.value("switcher/key", "id_rsa").toString();
    }

    settings_dialog* sd = new settings_dialog();
    sd->set_settings(ts, ss);
    if (QDialog::Accepted == sd->exec())
    {
        qDebug() << "settings accepted";
        
        sd->get_settings(ts, ss);
        header_action_->setText(ss.host);
        tray_icon_->setToolTip(ss.host);

        {
            QSettings app_settings(ini_file_name_, QSettings::IniFormat);
            app_settings.setValue("tray/normal_update_interval_sec", ts.normal_update_interval_sec);
            app_settings.setValue("tray/error_update_interval_sec", ts.error_update_interval_sec);
            app_settings.setValue("switcher/host", ss.host);
            app_settings.setValue("switcher/login", ss.login);
            app_settings.setValue("switcher/key", ss.key);
            app_settings.sync();
        }

        {
            // update_time_mutex_ locked
            QMutexLocker locker(&update_time_mutex_);
            update_time_ = QDateTime::currentSecsSinceEpoch();
        }

        {
            // tray_settings_mutex_ locked
            QMutexLocker locker(&tray_settings_mutex_);
            tray_settings_ = ts;
        }

        switcher_->apply_settings(ss);
        
        {
            // pending_action_mutex_ locked
            QMutexLocker locker(&pending_action_mutex_);

            gif_switch_->stop();
            gif_update_->stop();
            gif_cancel_->start();

            pending_action_ = ACTION::UPDATE;
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

        fastlab_action_->setEnabled(false);
        postwin_action_->setEnabled(false);
        update_action_->setEnabled(false);
        settings_action_->setEnabled(false);
        quit_action_->setEnabled(false);
        gif_switch_->stop();
        gif_update_->stop();
        gif_cancel_->start();

        pending_action_ = ACTION::QUIT;
        switcher_->cancel_async();
    }
}

void tray::updateIconUpdate()
{
    tray_icon_->setIcon(gif_update_->currentPixmap());
}

void tray::updateIconSwitch()
{
    tray_icon_->setIcon(gif_switch_->currentPixmap());
}

void tray::updateIconCancel()
{
    tray_icon_->setIcon(gif_cancel_->currentPixmap());
}

void tray::switcher_state_changed(switcher::STATE state, QString host, QString message)
{
    fastlab_action_->setEnabled(true);
    postwin_action_->setEnabled(true);
    update_action_->setEnabled(true);
    gif_switch_->stop();
    gif_update_->stop();
    gif_cancel_->stop();

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        if (state == switcher::STATE::ERROR_ && pending_action_ != tray::ACTION::QUIT)
            pending_action_ = ACTION::NONE;

        if (pending_action_ != ACTION::NONE)
        {
            switch (pending_action_)
            {
            case tray::ACTION::NONE:
                break;
            case tray::ACTION::SWITCH_TO_FASTLAB:
                gif_switch_->start();
                switcher_->switch_to_fastlab_async();
                break;
            case tray::ACTION::SWITCH_TO_POSTWIN:
                gif_switch_->start();
                switcher_->switch_to_postwin_async();
                break;
            case tray::ACTION::UPDATE:
                gif_update_->start();
                switcher_->update_async();
                break;
            case tray::ACTION::QUIT:
                tray_icon_->hide();
                QCoreApplication::quit();
                break;
            default:
                break;
            }

            pending_action_ = ACTION::NONE;
        }
        else
        {
            if (state == switcher::STATE::FASTLAB)
            {
                tray_icon_->setIcon(QIcon(":/images/fastlab.png"));
            }
            else if (state == switcher::STATE::POSTWIN)
            {
                tray_icon_->setIcon(QIcon(":/images/postwin.png"));
            }
            else if (state == switcher::STATE::UNKNOWN)
            {
                tray_icon_->setIcon(QIcon(":/images/unknown.png"));
            }
            else if (state == switcher::STATE::ERROR_)
            {
                tray_icon_->setIcon(QIcon(":/images/error.png"));
            }
            tray_icon_->setToolTip(QString("%1\n%2").arg(host, message));
            state_ = state;
        }
    }

    {
        // update_time_mutex_ locked
        QMutexLocker locker(&update_time_mutex_);
        update_time_ = QDateTime::currentSecsSinceEpoch();
    }
}

void tray::tray_icon_activated(QSystemTrayIcon::ActivationReason ar)
{
    if (ar == QSystemTrayIcon::DoubleClick)
    {
        settings();
    }
}
