#ifndef SWITCHER_H
#define SWITCHER_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include "settings.h"

class switcher : public QObject
{
    Q_OBJECT

public:
    enum class state
    {
        UNKNOWN,
        FASTLAB,
        POSTWIN,
        ERROR_
    };

private:
    enum class process
    {
        IDLE,
        SWITCHING_TO_FASTLAB,
        SWITCHING_TO_POSTWIN,
        UPDATING,
        CANCELING
    };

    struct thread_result
    {
        bool error;
        QString error_message;
        QString host;
        state state;

        thread_result& operator =(const thread_result& a)
        {
            error = a.error;
            error_message = a.error_message;
            host = a.host;
            state = a.state;
            return *this;
        }
    };

private:
    QMutex access_mutex_;
    QAtomicInt thread_exit_requested_;
    QFuture<thread_result> future_;
    QFutureWatcher<void> future_watcher_;

    QMutex current_process_mutex_;
    process current_process_;

    QAtomicInt cancelled_;

    bool ok_;
    switcher::state ok2_;

    QMutex switcher_settings_mutex_;
    switcher_settings switcher_settings_;

public:
    switcher();

signals:
    void on_state_changed(state st, QString host, QString message);

public:
    void switch_to_fastlab_async();
    void switch_to_postwin_async();
    void update_async();
    void cancel_async();
    void set_result(bool ok);
    void set_refresh_result(switcher::state ok2);
    void apply_settings(switcher_settings ss);

private:
    thread_result switch_to_fastlab_internel();
    thread_result switch_to_postwin_internel();
    thread_result update_internel();
    thread_result cancel_internel();
    void thread_finished();

};

#endif // SWITCHER_H
