#include <QMutexLocker>
#include <QDebug>

#include "switcher.h"

#define DEBUG_SWITCH_TO_FASTLAB_SECONDS 3
#define DEBUG_SWITCH_TO_POSTWIN_SECONDS 3
#define DEBUG_UPDATE_SECONDS 3

switcher::switcher()
{
    current_process_ = process::IDLE;
    thread_exit_requested_ = false;
    ok_ = true;
    ok2_ = switcher::state::ERROR_;


    connect(&future_watcher_, &QFutureWatcher<void>::finished, this,  &switcher::thread_finished);
}

void switcher::switch_to_fastlab_async()
{
    QMutexLocker locker(&access_mutex_);

    qDebug() << "switch_to_fastlab_async";
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

void switcher::switch_to_postwin_async()
{
    QMutexLocker locker(&access_mutex_);

    qDebug() << "switch_to_postwin_async";
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

void switcher::update_state_async()
{
    QMutexLocker locker(&access_mutex_);

    qDebug() << "update_state_async";
    if (current_process_ == process::IDLE)
    {
        qDebug() << "was IDLE, UPDATING";
        current_process_ = process::UPDATING;
        future_ = QtConcurrent::run(this, &switcher::update_state_internel);
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

void switcher::cancel()
{
    QMutexLocker locker(&access_mutex_);

    qDebug() << "cancel";
//    if (current_process_ == process::IDLE)
//    {
//        // все равно вызвать для общности и emit
//        qDebug() << "nothing to do, already IDLE";
//    }
//    else
    {
        thread_exit_requested_ = true;
        future_.waitForFinished();
    }
    emit on_canceled();
    current_process_ = process::IDLE;
}

void switcher::set_result(bool ok)
{
    ok_ = ok;
}

void switcher::set_refresh_result(switcher::state ok2)
{
    ok2_ = ok2;
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

bool switcher::update_state_internel()
{
    qDebug() << "update_state_internel";
    for (int i = 0; i < DEBUG_UPDATE_SECONDS; i++)
    {
        qDebug() << "u" << i;
        if (thread_exit_requested_)
            break;
        QThread::sleep(1);
    }
    return ok_;
}

void switcher::thread_finished()
{
    qDebug() << "thread_finished";
    bool result = future_.result();
    qDebug() << result;

    if (!result)
    {
        qDebug() << "!result";
        emit on_error();
        emit on_state_changed(state::ERROR_);
    }
    else
    {
        if (current_process_ == process::IDLE)
        {
            qDebug() << "IDLE";
            //emit on_switched_to_fastlab();
        }
        else if (current_process_ == process::SWITCHING_TO_FASTLAB)
        {
            qDebug() << "SWITCHING_TO_FASTLAB";
            emit on_switched_to_fastlab();
            emit on_state_changed(state::FASTLAB);
        }
        else if (current_process_ == process::SWITCHING_TO_POSTWIN)
        {
            qDebug() << "SWITCHING_TO_POSTWIN";
            emit on_switched_to_postwin();
            emit on_state_changed(state::POSTWIN);
        }
        else if (current_process_ == process::UPDATING)
        {
            qDebug() << "UPDATING";
            emit on_updated();
            emit on_state_changed(ok2_); //!!!!!!!!!! FASTLAB
        }
        else if (current_process_ == process::CANCELING)
        {
            qDebug() << "CANCELING";
            //emit on_canceled();
            emit on_state_changed(state::UNKNOWN); //!!!!!!!!!! FASTLAB
        }
        else
        {
            // error
        }
    }
    current_process_ = process::IDLE;
    thread_exit_requested_ = false;
}
