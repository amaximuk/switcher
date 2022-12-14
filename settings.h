#pragma once
#include "QString"

struct run_settings
{
	bool instance_name_is_set;
	QString instance_name;
};

struct tray_settings
{
	quint32 normal_update_interval_sec;
	quint32 error_update_interval_sec;
};

struct switcher_settings
{
    QString host;
	QString login;
    QString key;
};
