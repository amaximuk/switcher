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
    settings settings_;

public:
    explicit settings_dialog(QWidget *parent = nullptr);
    ~settings_dialog();

public:
    void set_settings(settings s);
    settings get_settings();

private:
    Ui::settings_dialog *ui;
};

#endif // SETTINGS_DIALOG_H
