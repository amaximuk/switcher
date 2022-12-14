#include <QIcon>
#include "settings_dialog.h"
#include "ui_settings_dialog.h"

settings_dialog::settings_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings_dialog)
{
    ui->setupUi(this);
    setFixedSize(QSize(300, 200));
    setWindowIcon(QIcon(":/images/settings.png"));
    setWindowTitle("Switcher Settings");
}

settings_dialog::~settings_dialog()
{
    delete ui;
}

void settings_dialog::set_settings(tray_settings ts, switcher_settings ss)
{
    tray_settings_ = ts;
    ui->spinBoxNormalInterval->setValue(tray_settings_.normal_update_interval_sec);
    ui->spinBoxErrorInterval->setValue(tray_settings_.error_update_interval_sec);
    switcher_settings_ = ss;
    ui->lineEditHost->setText(switcher_settings_.host);
    ui->lineEditLogin->setText(switcher_settings_.login);
    ui->lineEditKey->setText(switcher_settings_.key);
}

void settings_dialog::get_settings(tray_settings& ts, switcher_settings& ss)
{
    ts.normal_update_interval_sec = ui->spinBoxNormalInterval->value();
    ts.error_update_interval_sec = ui->spinBoxErrorInterval->value();
    ss.host = ui->lineEditHost->text();
    ss.login = ui->lineEditLogin->text();
    ss.key = ui->lineEditKey->text();
}
