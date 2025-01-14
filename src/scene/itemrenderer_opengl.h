/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "libkwineffects/kwineffects.h"
#include "libkwineffects/kwinglutils.h"
#include "scene/itemrenderer.h"

namespace KWin
{

class KWIN_EXPORT ItemRendererOpenGL : public ItemRenderer
{
public:
    struct RenderNode
    {
        GLTexture *texture = nullptr;
        RenderGeometry geometry;
        QMatrix4x4 transformMatrix;
        int firstVertex = 0;
        int vertexCount = 0;
        qreal opacity = 1;
        bool hasAlpha = false;
        TextureCoordinateType coordinateType = UnnormalizedCoordinates;
        qreal scale = 1.0;
    };

    struct RenderContext
    {
        QVector<RenderNode> renderNodes;
        QStack<QMatrix4x4> transformStack;
        QStack<qreal> opacityStack;
        const QRegion clip;
        const bool hardwareClipping;
        const qreal renderTargetScale;
    };

    ItemRendererOpenGL();

    void beginFrame(const RenderTarget &renderTarget, const RenderViewport &viewport) override;
    void endFrame() override;

    void renderBackground(const RenderTarget &renderTarget, const RenderViewport &viewport, const QRegion &region) override;
    void renderItem(const RenderTarget &renderTarget, const RenderViewport &viewport, Item *item, int mask, const QRegion &region, const WindowPaintData &data) override;

    ImageItem *createImageItem(Scene *scene, Item *parent = nullptr) override;

private:
    QVector4D modulate(float opacity, float brightness) const;
    void setBlendEnabled(bool enabled);
    void createRenderNode(Item *item, RenderContext *context);

    bool m_blendingEnabled = false;
};

} // namespace KWin
