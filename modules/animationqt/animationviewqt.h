/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_ANIMATIONVIEWQT_H
#define IVW_ANIMATIONVIEWQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/animationcontrollerobserver.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsView>
#include <warn/pop>

namespace inviwo {

namespace animation {

class AnimationController;

/**
 * \class AnimationViewQt
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */

class IVW_MODULE_ANIMATIONQT_API AnimationViewQt : public QGraphicsView, public AnimationControllerObserver {
public:
    AnimationViewQt(AnimationController& controller);
    virtual ~AnimationViewQt() = default;

protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);

	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void drawForeground(QPainter* painter, const QRectF& rect) override;

	virtual void onStateChanged(AnimationController* controller, AnimationState oldState, AnimationState newState) override;
	virtual void onTimeChanged(AnimationController* controller, Time oldTime, Time newTime) override;

	void updateZoom();

	dvec2 zoomH_{ 1,1 };
	dvec2 zoomV_{ 1,1 };

	AnimationController& controller_;
};

}  // namespace

}  // namespace

#endif  // IVW_ANIMATIONVIEWQT_H
