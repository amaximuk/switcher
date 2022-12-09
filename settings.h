#pragma once
#include "QString"

struct tray_settings
{
	bool instance_name_is_set;
    QString instance_name;
};

//using switcher_settings = tray_settings;
struct switcher_settings
{
    QString host;
	QString login;
	QString password;
	quint32 normal_update_interval_sec;
	quint32 error_update_interval_sec;
};
