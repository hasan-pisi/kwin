/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "drm_commit_thread.h"
#include "drm_atomic_commit.h"
#include "drm_gpu.h"

namespace KWin
{

DrmCommitThread::DrmCommitThread()
{
    m_thread.reset(QThread::create([this]() {
        while (true) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                return;
            }
            std::unique_lock lock(m_mutex);
            if (!m_commit) {
                m_commitPending.wait(lock);
            }
            if (m_commit) {
                const auto now = std::chrono::steady_clock::now().time_since_epoch();
                if (m_targetCommitTime > now) {
                    lock.unlock();
                    std::this_thread::sleep_for(m_targetCommitTime - now);
                    lock.lock();
                }
                // the other thread may replace the commit, but not erase it
                Q_ASSERT(m_commit);
                if (!m_commit->commit()) {
                    QMetaObject::invokeMethod(this, &DrmCommitThread::commitFailed, Qt::ConnectionType::QueuedConnection);
                }
                m_commit.reset();
            }
        }
    }));
    m_thread->start(QThread::TimeCriticalPriority);
}

DrmCommitThread::~DrmCommitThread()
{
    m_thread->requestInterruption();
    std::scoped_lock lock(m_mutex);
    m_commitPending.notify_all();
}

void DrmCommitThread::setCommit(std::unique_ptr<DrmAtomicCommit> &&commit, std::chrono::nanoseconds targetCommitTime)
{
    std::scoped_lock lock(m_mutex);
    m_commit = std::move(commit);
    m_targetCommitTime = targetCommitTime;
    m_commitPending.notify_all();
}

bool DrmCommitThread::replaceCommit(std::unique_ptr<DrmAtomicCommit> &&commit)
{
    std::scoped_lock lock(m_mutex);
    if (m_commit) {
        m_commit = std::move(commit);
        m_commitPending.notify_all();
        return true;
    } else {
        return false;
    }
}

}
