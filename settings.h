#pragma once
#include "QString"

struct settings
{
	QString ip_address;
	QString login;
	QString password;
	quint32 normal_update_interval;
	quint32 error_update_interval;
};
