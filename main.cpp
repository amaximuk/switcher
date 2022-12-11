#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMovie>
#include <QLoggingCategory>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QMessageBox>

#include "settings.h"
#include "tray.h"

// Умный указатель на файл логирования
QScopedPointer<QFile> logFile_;
QtMessageHandler default_message_handler_;

// Объявляение обработчика
void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(switcher);

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription("Application Switcher");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption instanceNameOption(QStringList() << "i" << "instance",
            "Set instance name to <instance-name>.",
            "instance-name");
    parser.addOption(instanceNameOption);
    parser.process(app);

    run_settings ts{};
    ts.instance_name_is_set = parser.isSet(instanceNameOption);
    if (ts.instance_name_is_set)
        ts.instance_name = parser.value(instanceNameOption);

    if (!QDir("log").exists())
        QDir().mkdir("log");

    QString instance_log = ts.instance_name_is_set ? QString("log/switcher_%1").arg(ts.instance_name): "log/switcher";
    QString file_name = QString("%1_%2.txt").arg(instance_log, QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    // Устанавливаем файл логирования
    logFile_.reset(new QFile(file_name));
    // Открываем файл логирования
    logFile_->open(QFile::ReadWrite | QFile::Unbuffered | QFile::Text);
    // Устанавливаем обработчик
    default_message_handler_ = qInstallMessageHandler(message_handler);

    tray main_tray(ts);

    return app.exec();
}

void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // Открываем поток записи в файл
    QTextStream out(logFile_.data());

    // Записываем дату записи
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

    switch (type)
    {
    case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }

    // Записываем в вывод категорию сообщения и само сообщение
    out << ": " << msg << Qt::endl;
    out.flush();

    default_message_handler_(type, context, msg);
}

#else

#include <QLabel>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString text("QSystemTrayIcon is not supported on this platform");

    QLabel *label = new QLabel(text);
    label->setWordWrap(true);

    label->show();
    qDebug() << text;

    app.exec();
}

#endif
