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

private:
    QMutex access_mutex_;
    QAtomicInt thread_exit_requested_;
    QFuture<bool> future_;
    QFutureWatcher<void> future_watcher_;
    process current_process_;

    QAtomicInt cancelled_;

    bool ok_;
    switcher::state ok2_;

    QString last_error_;

    QMutex switcher_settings_mutex_;
    switcher_settings switcher_settings_;

public:
    switcher();

signals:
    void on_state_changed(state st);

public:
    void switch_to_fastlab_async();
    void switch_to_postwin_async();
    void update_async();
    void cancel_async();
    void set_result(bool ok);
    void set_refresh_result(switcher::state ok2);
    void apply_settings(switcher_settings ss);

private:
    bool switch_to_fastlab_internel();
    bool switch_to_postwin_internel();
    bool update_internel();
    bool cancel_internel();
    void thread_finished();

};

#endif // SWITCHER_H
