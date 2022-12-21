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
    enum class STATE
    {
        UNKNOWN,
        FASTLAB,
        POSTWIN,
        ERROR_
    };

private:
    enum class PROCESS
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
        STATE state;

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
    PROCESS current_process_;

    QAtomicInt cancelled_;

    QMutex switcher_settings_mutex_;
    switcher_settings switcher_settings_;

public:
    switcher();

signals:
    void on_state_changed(STATE state, QString host, QString message);

public:
    void switch_to_fastlab_async();
    void switch_to_postwin_async();
    void update_async();
    void cancel_async();
    void apply_settings(switcher_settings ss);

private:
    thread_result switch_to_fastlab_internel();
    thread_result switch_to_postwin_internel();
    thread_result update_internel();
    thread_result cancel_internel();
    void thread_finished();
};

#endif // SWITCHER_H
