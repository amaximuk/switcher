#include <QMutexLocker>
#include <QDebug>
#include <QProcess>

#include "switcher.h"

#define DEBUG_SWITCH_TO_FASTLAB_SECONDS 9
#define DEBUG_SWITCH_TO_POSTWIN_SECONDS 9
#define DEBUG_UPDATE_SECONDS 9
#define DEBUG_CANCEL_SECONDS 3

namespace definitions
{
    constexpr char* postwin_process_name = "PostWin";
    constexpr char* postwin_required_file_name = "/home/root/PostWin/bin/PostWin";
    constexpr char* postwin_service_name = "PostWin.service";
    constexpr char* postwin_service_path = "/home/root/PostWin/PostWin.service";

    constexpr char* fastlab_process_name = "starter";
    constexpr char* fastlab_required_file_name = "/home/root/fastlab/starter";
    constexpr char* fastlab_service_name = "starter.service";
    constexpr char* fastlab_service_path = "/home/root/fastlab/firmware_sigma/services/starter.service";

    constexpr int max_retry_count = 6;
    constexpr int retry_timeout_sec = 10;
}

switcher::switcher()
{
    current_process_ = process::IDLE;
    thread_exit_requested_ = false;
    ok_ = true;
    ok2_ = switcher::state::UNKNOWN;

    connect(&future_watcher_, &QFutureWatcher<void>::finished, this,  &switcher::thread_finished);
}

void switcher::switch_to_fastlab_async()
{
    QMutexLocker locker(&access_mutex_);
    qDebug() << "switch_to_fastlab_async";

    {
        // current_process_mutex_ locked
        QMutexLocker locker(&current_process_mutex_);

        if (current_process_ == process::IDLE)
        {
            qDebug() << "was IDLE, SWITCHING_TO_FASTLAB";
            current_process_ = process::SWITCHING_TO_FASTLAB;
            future_ = QtConcurrent::run(this, &switcher::switch_to_fastlab_internel);
            future_watcher_.setFuture(future_);
        }
        else if (current_process_ == process::SWITCHING_TO_FASTLAB)
        {
            qDebug() << "nothing to do, already SWITCHING_TO_FASTLAB";
        }
        else if (current_process_ == process::CANCELING)
        {
            qDebug() << "nothing to do, already CANCELING";
        }
        else
        {
            qDebug() << "something wrong, CANCELING";
            current_process_ = process::CANCELING;
            thread_exit_requested_ = true;
        }
    }
}

void switcher::switch_to_postwin_async()
{
    QMutexLocker locker(&access_mutex_);
    qDebug() << "switch_to_postwin_async";

    {
        // current_process_mutex_ locked
        QMutexLocker locker(&current_process_mutex_);

        if (current_process_ == process::IDLE)
        {
            qDebug() << "was IDLE, SWITCHING_TO_POSTWIN";
            current_process_ = process::SWITCHING_TO_POSTWIN;
            future_ = QtConcurrent::run(this, &switcher::switch_to_postwin_internel);
            future_watcher_.setFuture(future_);
        }
        else if (current_process_ == process::SWITCHING_TO_POSTWIN)
        {
            qDebug() << "nothing to do, already SWITCHING_TO_POSTWIN";
        }
        else if (current_process_ == process::CANCELING)
        {
            qDebug() << "nothing to do, already CANCELING";
        }
        else
        {
            qDebug() << "something wrong, CANCELING";
            current_process_ = process::CANCELING;
            thread_exit_requested_ = true;
        }
    }
}

void switcher::update_async()
{
    QMutexLocker locker(&access_mutex_);
    qDebug() << "update_state_async";

    {
        // current_process_mutex_ locked
        QMutexLocker locker(&current_process_mutex_);

        if (current_process_ == process::IDLE)
        {
            qDebug() << "was IDLE, UPDATING";
            current_process_ = process::UPDATING;
            future_ = QtConcurrent::run(this, &switcher::update_internel);
            future_watcher_.setFuture(future_);
        }
        else if (current_process_ == process::UPDATING)
        {
            qDebug() << "nothing to do, already UPDATING";
        }
        else if (current_process_ == process::CANCELING)
        {
            qDebug() << "nothing to do, already CANCELING";
        }
        else
        {
            qDebug() << "something wrong, CANCELING";
            current_process_ = process::CANCELING;
            thread_exit_requested_ = true;
        }
    }
}

void switcher::cancel_async()
{
    QMutexLocker locker(&access_mutex_);
    qDebug() << "cancel_async";

    {
        // current_process_mutex_ locked
        QMutexLocker locker(&current_process_mutex_);

        if (current_process_ == process::IDLE)
        {
            qDebug() << "create dummy thread, already IDLE";
            current_process_ = process::CANCELING;
            future_ = QtConcurrent::run(this, &switcher::cancel_internel);
            future_watcher_.setFuture(future_);
        }
        else
        {
            qDebug() << "some process is running, CANCELING";
            current_process_ = process::CANCELING;
            thread_exit_requested_ = true;
        }
    }
}

void switcher::set_result(bool ok)
{
    ok_ = ok;
}

void switcher::set_refresh_result(switcher::state ok2)
{
    ok2_ = ok2;
}

void switcher::apply_settings(switcher_settings ss)
{
    QMutexLocker locker(&access_mutex_);

    {
        // switcher_settings_mutex_ locked
        QMutexLocker locker(&switcher_settings_mutex_);
        switcher_settings_ = ss;
    }
}

//bool run_ssh_command_no_wait(const switcher_settings ss, const QString command, const bool ignore_command_error, QString& output)
//{
//    QString key_path = QDir::home().filePath(".ssh/" + ss.key);
//    QString user_host = QString("%1@%2").arg(ss.login, ss.host);
//    QString real_command(command);
//    if (ignore_command_error)
//        real_command += " || exit 0";
//
//    QProcess qp;
//    qp.start("ssh", QStringList() << "-i" << key_path << "-o" << "BatchMode=yes" << "-o" << "StrictHostKeyChecking=no" << user_host << real_command);
//    //qp.start("ssh", QStringList() << "-i" << key_path << "-o" << "BatchMode=yes" << "-o" << "StrictHostKeyChecking=no" <<
//    //    "-o" << "ServerAliveInterval=5" << "-o" << "ServerAliveCountMax=1" << user_host << real_command);
//    bool finished = qp.waitForFinished(5000);
//    if (!finished)
//    {
//        return true;
//    }
//    else
//    {
//        int exit_code = qp.exitCode();
//        if (exit_code == 255)
//        {
//            output = "SSH connection failed (255)";
//            qDebug() << output;
//            return false;
//        }
//        else if (exit_code != 0)
//        {
//            output = "SSH subcommand error: " + qp.readAllStandardError();
//            qDebug() << output;
//            return false;
//        }
//        else
//        {
//            output = qp.readAllStandardOutput();
//            qDebug() << output;
//            return true;
//        }
//    }
//}

bool run_ssh_command(const switcher_settings ss, const QString command, const bool ignore_command_error, const int timeout_msec, const bool ignore_timeout_error, QString& output)
{
    QString key_path = QDir::home().filePath(".ssh/" + ss.key);
    QString user_host = QString("%1@%2").arg(ss.login, ss.host);
    QString real_command(command);
    if (ignore_command_error)
        real_command += " || exit 0";

    QProcess qp;
    qp.start("ssh", QStringList() << "-i" << key_path << "-o" << "BatchMode=yes" << "-o" << "StrictHostKeyChecking=no" << user_host << real_command);
    bool finished = qp.waitForFinished(timeout_msec);
    if (!finished)
    {
        if (ignore_timeout_error)
        {
            output = "";
            return true;
        }
        else
        {
            output = "SSH process timeout";
            qDebug() << output;
            return false;
        }
        qp.terminate();
    }
    else
    {
        int exit_code = qp.exitCode();
        if (exit_code == 255)
        {
            output = "SSH connection failed (255)";
            qDebug() << output;
            return false;
        }
        else if (exit_code != 0)
        {
            output = "SSH subcommand error: " + qp.readAllStandardError();
            qDebug() << output;
            return false;
        }
        else
        {
            output = qp.readAllStandardOutput();
            if (output.length() > 0)
                qDebug() << output;
            return true;
        }
    }
}

bool check_process(const switcher_settings ss, const QString process_name, bool& found, QString& output)
{
    QString command = QString("ps -C %1").arg(process_name);
    if (!run_ssh_command(ss, command, true, 10000, false, output))
    {
        found = false;
        return false;
    }

    QString s15 = process_name.length() > 15 ? process_name.left(15) : process_name;
    if (output.contains(s15))
    {
        found = true;
        output = "";
    }
    else
    {
        found = false;
        output = "";
    }

    return true;
}

bool start_service(const switcher_settings ss, const QString service_name, QString& output)
{
    QString command = QString("systemctl start %1").arg(service_name);
    if (!run_ssh_command(ss, command, false, 10000, false, output))
        return false;

    output = "";
    return true;
}

bool stop_service(const switcher_settings ss, const QString service_name, QString& output)
{
    QString command = QString("systemctl stop %1").arg(service_name);
    if (!run_ssh_command(ss, command, true, 10000, false, output))
        return false;

    output = "";
    return true;
}

bool enable_service(const switcher_settings ss, const QString service_name, QString& output)
{
    QString command = QString("systemctl enable %1").arg(service_name);
    if (!run_ssh_command(ss, command, false, 10000, false, output))
        return false;

    output = "";
    return true;
}

bool disable_service(const switcher_settings ss, const QString service_name, QString& output)
{
    QString command = QString("systemctl disable %1").arg(service_name);
    if (!run_ssh_command(ss, command, true, 10000, false, output))
        return false;

    output = "";
    return true;
}

bool sync(const switcher_settings ss, QString& output)
{
    QString command = QString("sync");
    if (!run_ssh_command(ss, command, true, 10000, false, output))
        return false;

    output = "";
    return true;
}

bool reboot(const switcher_settings ss, QString& output)
{
    QString command = QString("KSUtility -b b");
    if (!run_ssh_command(ss, command, true, 5000, true, output))
        return false;

    output = "";
    return true;
}

#define CANCEL_POINT if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

switcher::thread_result switcher::switch_to_fastlab_internel()
{
    qDebug() << "switch_to_fastlab_internel";
    thread_result tr{};

    switcher_settings ss{};

    {
        // switcher_settings_mutex_ locked
        QMutexLocker locker(&switcher_settings_mutex_);
        ss = switcher_settings_;
    }

    tr.host = ss.host;
    tr.state = state::ERROR_;

    QString output;
    if (!stop_service(ss, definitions::postwin_service_name, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!disable_service(ss, definitions::postwin_service_name, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!enable_service(ss, definitions::fastlab_service_path, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!sync(ss, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!reboot(ss, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    int retry_count = 0;
    bool found_fastlab;
    while (retry_count < definitions::max_retry_count)
    {
        if (!check_process(ss, definitions::fastlab_process_name, found_fastlab, output))
        {
            std::this_thread::sleep_for(std::chrono::seconds(definitions::retry_timeout_sec));
            retry_count++;
        }
        else
        {
            break;
        }

        if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }
    }
    if (retry_count >= definitions::max_retry_count)
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    tr.state = state::FASTLAB;

    //for (int i = 0; i < DEBUG_SWITCH_TO_FASTLAB_SECONDS; i++)
    //{
    //    qDebug() << "f" << i;
    //    if (thread_exit_requested_)
    //        break;
    //    QThread::sleep(1);
    //}
    return tr;
}

switcher::thread_result switcher::switch_to_postwin_internel()
{
    qDebug() << "switch_to_postwin_internel";
    thread_result tr{};

    switcher_settings ss{};

    {
        // switcher_settings_mutex_ locked
        QMutexLocker locker(&switcher_settings_mutex_);
        ss = switcher_settings_;
    }

    tr.host = ss.host;
    tr.state = state::ERROR_;

    QString output;
    if (!stop_service(ss, definitions::fastlab_service_name, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!disable_service(ss, definitions::fastlab_service_name, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!enable_service(ss, definitions::postwin_service_path, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!sync(ss, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    if (!reboot(ss, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    int retry_count = 0;
    bool found_fastlab;
    while (retry_count < definitions::max_retry_count)
    {
        if (!check_process(ss, definitions::postwin_process_name, found_fastlab, output))
        {
            std::this_thread::sleep_for(std::chrono::seconds(definitions::retry_timeout_sec));
            retry_count++;
        }
        else
        {
            break;
        }

        if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }
    }
    if (retry_count >= definitions::max_retry_count)
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    tr.state = state::POSTWIN;

    //for (int i = 0; i < DEBUG_SWITCH_TO_POSTWIN_SECONDS; i++)
    //{
    //    qDebug() << "p" << i;
    //    if (thread_exit_requested_)
    //        break;
    //    QThread::sleep(1);
    //}
    return tr;
}

switcher::thread_result switcher::update_internel()
{
    qDebug() << "update_internel";
    thread_result tr{};

    switcher_settings ss{};

    {
        // switcher_settings_mutex_ locked
        QMutexLocker locker(&switcher_settings_mutex_);
        ss = switcher_settings_;
    }

    tr.host = ss.host;
    tr.state = state::ERROR_;

    QString output;
    bool found_fastlab;
    if (!check_process(ss, definitions::fastlab_process_name, found_fastlab, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (thread_exit_requested_) { tr.state = state::UNKNOWN; tr.error_message = "Cancelled"; return tr; }

    bool found_postwin;
    if (!check_process(ss, definitions::postwin_process_name, found_postwin, output))
    {
        tr.error = true;
        tr.error_message = output;
        return tr;
    }

    if (found_fastlab && !found_postwin)
    {
        tr.state = state::FASTLAB;
        return tr;
    }
    else if (!found_fastlab && found_postwin)
    {
        tr.state = state::POSTWIN;
        return tr;
    }
    else
    {
        tr.state = state::UNKNOWN;
        return tr;
    }

    //for (int i = 0; i < DEBUG_UPDATE_SECONDS; i++)
    //{
    //    qDebug() << "u" << i;
    //    if (thread_exit_requested_)
    //        break;
    //    QThread::sleep(1);
    //}
    return tr;
}

switcher::thread_result switcher::cancel_internel()
{
    qDebug() << "cancel_internel";
    thread_result tr{};

    switcher_settings ss{};

    {
        // switcher_settings_mutex_ locked
        QMutexLocker locker(&switcher_settings_mutex_);
        ss = switcher_settings_;
    }

    tr.host = ss.host;
    tr.state = state::ERROR_;


    tr.state = state::UNKNOWN;

    //for (int i = 0; i < DEBUG_CANCEL_SECONDS; i++)
    //{
    //    qDebug() << "c" << i;
    //    QThread::sleep(1);
    //}
    return tr;
}

void switcher::thread_finished()
{
    thread_result result = future_.result();
    qDebug() << "thread_finished, error = " << result.error;

    if (result.error)
    {
        {
            // current_process_mutex_ locked
            QMutexLocker locker(&current_process_mutex_);

            qDebug() << "thread finished with error";
            current_process_ = process::IDLE;
            thread_exit_requested_ = false;
        }

        // Emit state changed
        emit on_state_changed(state::ERROR_, result.host, result.error_message);
    }
    else
    {
        process ps = process::IDLE;
        {
            // current_process_mutex_ locked
            QMutexLocker locker(&current_process_mutex_);

            ps = current_process_;
            switch (ps)
            {
            case process::IDLE:
                break;
            case process::SWITCHING_TO_FASTLAB:
                qDebug() << "SWITCHING_TO_FASTLAB";
                current_process_ = process::IDLE;
                thread_exit_requested_ = false;
                break;
            case process::SWITCHING_TO_POSTWIN:
                qDebug() << "SWITCHING_TO_POSTWIN";
                current_process_ = process::IDLE;
                thread_exit_requested_ = false;
                break;
            case process::UPDATING:
                qDebug() << "UPDATING";
                current_process_ = process::IDLE;
                thread_exit_requested_ = false;
                break;
            case process::CANCELING:
                qDebug() << "CANCELING";
                current_process_ = process::IDLE;
                thread_exit_requested_ = false;
                break;
            default:
                break;
            }
        }

        // Emit state changed
        QString state;
        switch (result.state)
        {
        case state::UNKNOWN:
            state = "Unknown";
            break;
        case state::FASTLAB:
            state = "Fastlab";
            break;
        case state::POSTWIN:
            state = "Postwin";
            break;
        case state::ERROR_:
            state = "Error";
            break;
        default:
            state = "Default";
            break;
        }

        emit on_state_changed(result.state, result.host, state);
        //switch (ps)
        //{
        //case process::IDLE:
        //    break;
        //case process::SWITCHING_TO_FASTLAB:
        //    emit on_state_changed(state::FASTLAB, result.host, "Fastlab");
        //    break;
        //case process::SWITCHING_TO_POSTWIN:
        //    emit on_state_changed(state::POSTWIN, result.host, "Postwin");
        //    break;
        //case process::UPDATING:
        //    break;
        //case process::CANCELING:
        //    emit on_state_changed(state::UNKNOWN, result.host, "Unknown");
        //    break;
        //default:
        //    break;
        //}
    }
}
