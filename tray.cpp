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

tray::tray(QObject *parent) : QObject(parent)
{
    pending_action_ = action::NONE;
    state_ = switcher::state::UNKNOWN;

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

    switcher_.reset(new switcher());
    QObject::connect(switcher_.get(), &switcher::on_state_changed, this, &tray::switcher_state_changed);

//    gif = new QMovie(":/images/refresh.gif");
//    connect(gif, &QMovie::frameChanged, this, &tray::updateIcon);
//    gif->start();


    QObject::connect(fastlabAction, &QAction::triggered, this, &tray::fastlab);
    QObject::connect(postwinAction, &QAction::triggered, this, &tray::postwin);
    QObject::connect(updateAction, &QAction::triggered, this, &tray::update);
    QObject::connect(settingsAction, &QAction::triggered, this, &tray::settings);
    QObject::connect(quitAction, &QAction::triggered, this, &tray::quit);
//    QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, &tray::hide);

    timer_id_ = startTimer(1000);
    update_time_ = QDateTime::currentMSecsSinceEpoch();
}

tray::~tray()
{
    killTimer(timer_id_);
}

void tray::timerEvent(QTimerEvent* event)
{
    if (timer_id_ == event->timerId())
    {
        {
            // pending_action_mutex_ locked
            QMutexLocker locker(&pending_action_mutex_);

            const qint64 current_time_ = QDateTime::currentMSecsSinceEpoch();

            if ((state_ == switcher::state::ERROR_ && (current_time_ - update_time_ > UPDATE_IF_ERROR_MS)) ||
                (state_ == switcher::state::UNKNOWN && (current_time_ - update_time_ > UPDATE_IF_UNKNOWN_MS)) ||
                (state_ == switcher::state::FASTLAB && (current_time_ - update_time_ > UPDATE_IF_NORMAL_MS)) ||
                (state_ == switcher::state::POSTWIN && (current_time_ - update_time_ > UPDATE_IF_NORMAL_MS)))
            {
                qDebug() << "timerEvent, time elapced = " << (current_time_ - update_time_) / 1000.0;
                gif_switch_->start(); // change to cancelling

                update_time_ = current_time_;
                pending_action_ = action::UPDATE;
                switcher_->cancel_async();
            }
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

        gif_switch_->start(); // change to cancelling

        pending_action_ = action::SWITCH_TO_FASTLAB;
        switcher_->cancel_async();
    }
}

void tray::postwin()
{
    //fastlabAction->setEnabled(false);
    //postwinAction->setEnabled(false);
    //refreshAction->setEnabled(false);
    //gif_update_->stop();
    //gif_switch_->start();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Switch to postwin", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    bool ok = (reply != QMessageBox::Yes);
    switcher_->set_result(ok);

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_switch_->start(); // change to cancelling

        pending_action_ = action::SWITCH_TO_POSTWIN;
        switcher_->cancel_async();
    }

    //switcher_->switch_to_postwin_async();


//    QMessageBox::StandardButton reply;
//    reply = QMessageBox::question(nullptr, "Switch to Postwin", "Are you shure?", QMessageBox::Yes | QMessageBox::No);
//    if (reply == QMessageBox::Yes)
//    {
////        trayIcon->setIcon(QIcon(":/images/fastlab.png"));
//    }
//    else
//    {
//      qDebug() << "Yes was *not* clicked";
//    }
//    switcher_->switch_to_fastlab_async();
//    gif->stop();
//    trayIcon->setIcon(QIcon(":/images/postwin.png"));
}

void tray::update()
{
    //fastlabAction->setEnabled(false);
    //postwinAction->setEnabled(false);
    //refreshAction->setEnabled(false);
    //gif_switch_->stop();
    //gif_update_->start();

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

    //switcher_->update_state_async();

    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        gif_switch_->start(); // change to cancelling

        pending_action_ = action::UPDATE;
        switcher_->cancel_async();
    }
}

void tray::settings()
{
    settings_dialog* sd = new settings_dialog();
    if (QDialog::Accepted == sd->exec())
    {
        qDebug() << "settings accepted";
    }
    sd->deleteLater();
}

void tray::quit()
{
    //switcher_->cancel();
    //trayIcon->hide();
    //QCoreApplication::quit();


    {
        // pending_action_mutex_ locked
        QMutexLocker locker(&pending_action_mutex_);

        fastlabAction->setEnabled(false);
        postwinAction->setEnabled(false);
        updateAction->setEnabled(false);
        gif_switch_->stop();
        gif_update_->stop();

        gif_switch_->start(); // change to cancelling

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

void tray::switcher_state_changed(switcher::state st)
{
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
                gif_switch_->start();
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
            fastlabAction->setEnabled(true);
            postwinAction->setEnabled(true);
            updateAction->setEnabled(true);
            gif_switch_->stop();
            gif_update_->stop();

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

        update_time_ = QDateTime::currentMSecsSinceEpoch();
    }
}
