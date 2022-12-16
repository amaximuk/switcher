#include <QMutexLocker>
#include <QDebug>
#include <QProcess>

#include "switcher.h"

#define DEBUG_SWITCH_TO_FASTLAB_SECONDS 9
#define DEBUG_SWITCH_TO_POSTWIN_SECONDS 9
#define DEBUG_UPDATE_SECONDS 9
#define DEBUG_CANCEL_SECONDS 3

switcher::switcher()
{
    current_process_ = process::IDLE;
    thread_exit_requested_ = false;
    ok_ = true;
    ok2_ = switcher::state::UNKNOWN;

    // ssh-keygen -t rsa -q -f ~/.ssh/id_rsa -N ""
    // type c:\Users\User\.ssh\id_rsa.pub | ssh user@192.168.88.46 "cat >> .ssh/authorized_keys"
    // ssh-keyscan -H 192.168.88.46 >> ~/.ssh/known_hosts
    // ssh user@192.168.88.46 -o BatchMode=no -o StrictHostKeyChecking=no ls
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

    QProcess qp;
    //QStringList arguments({QString("%1@%2").arg(ss.login, ss.host), "-o BatchMode=yes", "-o StrictHostKeyChecking=no", "ls"});
    //QStringList arguments({QString("%1@%2").arg(ss.login, ss.host), "-i c:/Users/alexander/.ssh/astra112", "ls"});
    //qp.start("ssh", arguments);
    //qp.start("ssh -i c:/Users/alexander/.ssh/astra112 -o BatchMode=yes -o StrictHostKeyChecking=no user@172.18.10.137 ls");
    QString command = "ls";
    QString key_path = QDir::home().filePath(".ssh/" + ss.key);
    QString run_command = QString("ssh -i %1 -o BatchMode=yes -o StrictHostKeyChecking=no %2@%3 %4").arg(key_path, ss.login, ss.host, command);
    qp.start(run_command);
    bool finished = qp.waitForFinished();
    int exit_code = qp.exitCode();
    if (!finished || exit_code != 0)
    {
        tr.error = true;
        tr.error_message = "SSH error";
        last_error_ = "SSH error";
    }
    QString output(qp.readAllStandardOutput());
    qDebug() << output;
    //QString output1(qp.readAllStandardError());


    
    // ssh user@192.168.88.46 -o BatchMode=no -o StrictHostKeyChecking=no ls
    // ssh user@172.18.10.137 -o StrictHostKeyChecking=no ls
    // ssh -i c:/Users/alexander/.ssh/astra112 -o BatchMode=yes -o StrictHostKeyChecking=no user@172.18.10.137 ls
    //pingProcess.start(exec, params);
    //pingProcess.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    //QString output(pingProcess.readAllStandardOutput());
    //connect(&pingProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readData()));

    for (int i = 0; i < DEBUG_SWITCH_TO_FASTLAB_SECONDS; i++)
    {
        qDebug() << "f" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
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

    for (int i = 0; i < DEBUG_SWITCH_TO_POSTWIN_SECONDS; i++)
    {
        qDebug() << "p" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
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

    for (int i = 0; i < DEBUG_UPDATE_SECONDS; i++)
    {
        qDebug() << "u" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
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

    for (int i = 0; i < DEBUG_CANCEL_SECONDS; i++)
    {
        qDebug() << "c" << i;
        //if (thread_exit_requested_)
        //    break;
        QThread::sleep(1);
    }
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
        switch (ps)
        {
        case process::IDLE:
            break;
        case process::SWITCHING_TO_FASTLAB:
            emit on_state_changed(state::FASTLAB, result.host, "Fastlab");
            break;
        case process::SWITCHING_TO_POSTWIN:
            emit on_state_changed(state::POSTWIN, result.host, "Postwin");
            break;
        case process::UPDATING:
            emit on_state_changed(ok2_, result.host, "");
            break;
        case process::CANCELING:
            emit on_state_changed(state::UNKNOWN, result.host, "Unknown");
            break;
        default:
            break;
        }
    }
}
