#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMovie>
#include <QtWidgets>
#include <QMessageBox>

#include "switcher.h"
#include "tray.h"

tray::tray(QObject *parent) : QObject(parent)
{
    fastlabAction = new QAction("&Fastlab");
    fastlabAction->setIcon(QIcon(":/images/fastlab.png"));
    postwinAction = new QAction("&Postwin");
    postwinAction->setIcon(QIcon(":/images/postwin.png"));
    refreshAction = new QAction("&Refresh");
    quitAction = new QAction("&Quit");

    trayIconMenu = new QMenu();
    auto act = trayIconMenu->addAction("127.0.0.1");
    act->setEnabled(false);

    trayIconMenu->addAction(fastlabAction);
    trayIconMenu->addAction(postwinAction);
    trayIconMenu->addAction(refreshAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon();
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/fastlab.png"));
    trayIcon->setToolTip("127.0.0.1");
    trayIcon->show();

    gif_update_ = new QMovie(":/images/update.gif");
    connect(gif_update_, &QMovie::frameChanged, this, &tray::updateIconUpdate);

    gif_switch_ = new QMovie(":/images/switch.gif");
    connect(gif_switch_, &QMovie::frameChanged, this, &tray::updateIconSwitch);

    switcher_.reset(new switcher());
    QObject::connect(switcher_.get(), &switcher::on_state_changed, this, &tray::switcher_state_changed);
    QObject::connect(switcher_.get(), &switcher::ask, this, &tray::ask);

//    gif = new QMovie(":/images/refresh.gif");
//    connect(gif, &QMovie::frameChanged, this, &tray::updateIcon);
//    gif->start();


    QObject::connect(fastlabAction, &QAction::triggered, this, &tray::fastlab);
    QObject::connect(postwinAction, &QAction::triggered, this, &tray::postwin);
    QObject::connect(refreshAction, &QAction::triggered, this, &tray::refresh);
    QObject::connect(quitAction, &QAction::triggered, this, &tray::quit);
//    QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, &tray::hide);


}

void tray::show()
{
    trayIcon->show();
}

void tray::fastlab()
{

    fastlabAction->setEnabled(false);
    postwinAction->setEnabled(false);
    refreshAction->setEnabled(false);
    gif_update_->stop();
    gif_switch_->start();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Switch to fastlab", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    bool ok = (reply != QMessageBox::Yes);
    switcher_->set_result(ok);

    switcher_->switch_to_fastlab_async();

//    QMessageBox::StandardButton reply;
//    reply = QMessageBox::question(nullptr, "Switch to Fastlab", "Are you shure?", QMessageBox::Yes | QMessageBox::No);
//    if (reply == QMessageBox::Yes)
//    {

////        trayIcon->setIcon(QIcon(":/images/fastlab.png"));
//    }
//    else
//    {
//      qDebug() << "Yes was *not* clicked";
//    }
}

void tray::postwin()
{
    fastlabAction->setEnabled(false);
    postwinAction->setEnabled(false);
    refreshAction->setEnabled(false);
    gif_update_->stop();
    gif_switch_->start();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Switch to postwin", "Generate error?", QMessageBox::Yes | QMessageBox::No);
    bool ok = (reply != QMessageBox::Yes);
    switcher_->set_result(ok);

    switcher_->switch_to_postwin_async();


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

void tray::refresh()
{
    fastlabAction->setEnabled(false);
    postwinAction->setEnabled(false);
    refreshAction->setEnabled(false);
    gif_switch_->stop();
    gif_update_->start();

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

    switcher_->update_state_async();
}

void tray::quit()
{
    switcher_->cancel();
    trayIcon->hide();
    QCoreApplication::quit();
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
    fastlabAction->setEnabled(true);
    postwinAction->setEnabled(true);
    refreshAction->setEnabled(true);
    gif_switch_->stop();
    gif_update_->stop();

    if (st == switcher::state::FASTLAB)
        trayIcon->setIcon(QIcon(":/images/fastlab.png"));
    else if (st == switcher::state::POSTWIN)
        trayIcon->setIcon(QIcon(":/images/postwin.png"));
    else if (st == switcher::state::UNKNOWN)
        trayIcon->setIcon(QIcon(":/images/unknown.png"));
    else
        trayIcon->setIcon(QIcon(":/images/error.png"));
}

void tray::ask(bool& answer)
{
}
