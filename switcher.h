#ifndef SWITCHER_H
#define SWITCHER_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

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
    QScopedPointer<QMutex> locker_;
    QAtomicInt thread_exit_requested_;
    QFuture<bool> future_;
    QFutureWatcher<void> future_watcher_;
    process current_process_;

    QAtomicInt cancelled_;

public:
    switcher();

signals:
    void on_state_changed(state st);
    void on_switched_to_fastlab();
    void on_switched_to_postwin();
    void on_updated_state();
    void on_updated();
    void on_canceled();
    void on_error();

public:
    void switch_to_fastlab_async();
    void switch_to_postwin_async();
    void update_state_async();
    void cancel();

private:
    bool switch_to_fastlab_internel();
    bool switch_to_postwin_internel();
    bool update_state_internel();
    void thread_finished();

};

#endif // SWITCHER_H
