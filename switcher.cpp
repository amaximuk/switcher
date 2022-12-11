#include <QMutexLocker>
#include <QDebug>

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

bool switcher::switch_to_fastlab_internel()
{
    qDebug() << "switch_to_fastlab_internel";
    for (int i = 0; i < DEBUG_SWITCH_TO_FASTLAB_SECONDS; i++)
    {
        qDebug() << "f" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
    return ok_;
}

bool switcher::switch_to_postwin_internel()
{
    qDebug() << "switch_to_postwin_internel";
    for (int i = 0; i < DEBUG_SWITCH_TO_POSTWIN_SECONDS; i++)
    {
        qDebug() << "p" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
    return ok_;
}

bool switcher::update_internel()
{
    qDebug() << "update_internel";
    for (int i = 0; i < DEBUG_UPDATE_SECONDS; i++)
    {
        qDebug() << "u" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
    return ok_;
}

bool switcher::cancel_internel()
{
    qDebug() << "cancel_internel";
    for (int i = 0; i < DEBUG_CANCEL_SECONDS; i++)
    {
        qDebug() << "c" << i;
        //if (thread_exit_requested_)
        //    break;
        QThread::sleep(1);
    }
    return ok_;
}

void switcher::thread_finished()
{
    qDebug() << "thread_finished, result = " << future_.result();
    bool result = future_.result();

    if (!result)
    {
        {
            // current_process_mutex_ locked
            QMutexLocker locker(&current_process_mutex_);

            qDebug() << "!result";
            current_process_ = process::IDLE;
            thread_exit_requested_ = false;
        }

        // Emit state changed
        emit on_state_changed(state::ERROR_);
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
            emit on_state_changed(state::FASTLAB);
            break;
        case process::SWITCHING_TO_POSTWIN:
            emit on_state_changed(state::POSTWIN);
            break;
        case process::UPDATING:
            emit on_state_changed(ok2_);
            break;
        case process::CANCELING:
            emit on_state_changed(state::UNKNOWN);
            break;
        default:
            break;
        }
    }
}
