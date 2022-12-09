#include "settings_dialog.h"
#include "ui_settings_dialog.h"

settings_dialog::settings_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings_dialog)
{
    ui->setupUi(this);
    setFixedSize(QSize(300, 200));
}

settings_dialog::~settings_dialog()
{
    delete ui;
}

void settings_dialog::set_settings(switcher_settings ss)
{
    switcher_settings_ = ss;
    ui->lineEditHost->setText(switcher_settings_.host);
    ui->lineEditLogin->setText(switcher_settings_.login);
    ui->lineEditPassword->setText(switcher_settings_.password);
    ui->spinBoxNormalInterval->setValue(switcher_settings_.normal_update_interval_sec);
    ui->spinBoxErrorInterval->setValue(switcher_settings_.error_update_interval_sec);
}

switcher_settings settings_dialog::get_settings()
{
    switcher_settings_.host = ui->lineEditHost->text();
    switcher_settings_.login = ui->lineEditLogin->text();
    switcher_settings_.password = ui->lineEditPassword->text();
    switcher_settings_.normal_update_interval_sec = ui->spinBoxNormalInterval->value();
    switcher_settings_.error_update_interval_sec = ui->spinBoxErrorInterval->value();
    return switcher_settings_;
}
