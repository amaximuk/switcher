#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include "settings.h"

namespace Ui {
class settings_dialog;
}

class settings_dialog : public QDialog
{
    Q_OBJECT

private:
    tray_settings tray_settings_;
    switcher_settings switcher_settings_;

public:
    explicit settings_dialog(QWidget *parent = nullptr);
    ~settings_dialog();

public:
    void set_settings(tray_settings ts, switcher_settings ss);
    void get_settings(tray_settings& ts, switcher_settings& ss);

private:
    Ui::settings_dialog *ui;
};

#endif // SETTINGS_DIALOG_H
