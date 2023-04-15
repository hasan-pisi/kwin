/*
    SPDX-FileCopyrightText: 2018 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2021 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include "clientbuffer.h"

#include "core/dmabufattributes.h"

#include <QHash>
#include <QSet>
#include <sys/types.h>

namespace KWin
{
class RenderBackend;
}

namespace KWaylandServer
{

class Display;
class LinuxDmaBufV1ClientBufferPrivate;
class LinuxDmaBufV1ClientBufferIntegrationPrivate;
class LinuxDmaBufV1FeedbackPrivate;

/**
 * The LinuxDmaBufV1ClientBuffer class represents a linux dma-buf client buffer.
 *
 * The LinuxDmaBufV1ClientBuffer can be used even after the underlying wl_buffer object
 * is destroyed by the client.
 */
class KWIN_EXPORT LinuxDmaBufV1ClientBuffer : public ClientBuffer
{
    Q_OBJECT

public:
    LinuxDmaBufV1ClientBuffer(KWin::DmaBufAttributes &&attrs, quint32 flags);
    ~LinuxDmaBufV1ClientBuffer() override;

    quint32 format() const;
    quint32 flags() const;
    const KWin::DmaBufAttributes &attributes() const;

    QSize size() const override;
    bool hasAlphaChannel() const override;
    Origin origin() const override;

    static LinuxDmaBufV1ClientBuffer *get(wl_resource *resource);

private:
    void initialize(wl_resource *resource);

    std::unique_ptr<LinuxDmaBufV1ClientBufferPrivate> d;
    friend class LinuxDmaBufParamsV1;
};

class KWIN_EXPORT LinuxDmaBufV1Feedback : public QObject
{
    Q_OBJECT
public:
    ~LinuxDmaBufV1Feedback() override;

    enum class TrancheFlag : uint32_t {
        Scanout = 1,
    };
    Q_DECLARE_FLAGS(TrancheFlags, TrancheFlag)

    struct Tranche
    {
        dev_t device;
        TrancheFlags flags;
        QHash<uint32_t, QVector<uint64_t>> formatTable;
    };
    /**
     * Sets the list of tranches for this feedback object, with lower indices
     * indicating a higher priority / a more optimal configuration.
     * The main device does not need to be included
     */
    void setTranches(const QVector<Tranche> &tranches);

private:
    LinuxDmaBufV1Feedback(LinuxDmaBufV1ClientBufferIntegrationPrivate *integration);
    friend class LinuxDmaBufV1ClientBufferIntegrationPrivate;
    friend class LinuxDmaBufV1FeedbackPrivate;
    std::unique_ptr<LinuxDmaBufV1FeedbackPrivate> d;
};

/**
 * The LinuxDmaBufV1ClientBufferIntegration class provides support for linux dma-buf buffers.
 */
class KWIN_EXPORT LinuxDmaBufV1ClientBufferIntegration : public QObject
{
    Q_OBJECT

public:
    explicit LinuxDmaBufV1ClientBufferIntegration(Display *display);
    ~LinuxDmaBufV1ClientBufferIntegration() override;

    KWin::RenderBackend *renderBackend() const;
    void setRenderBackend(KWin::RenderBackend *renderBackend);

    void setSupportedFormatsWithModifiers(const QVector<LinuxDmaBufV1Feedback::Tranche> &tranches);

private:
    friend class LinuxDmaBufV1ClientBufferIntegrationPrivate;
    std::unique_ptr<LinuxDmaBufV1ClientBufferIntegrationPrivate> d;
};

} // namespace KWaylandServer
