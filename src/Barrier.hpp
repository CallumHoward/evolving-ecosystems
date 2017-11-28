// Barrier.hpp
// Callum Howard, 2017

#ifndef BARRIER_HPP
#define BARRIER_HPP

#include "cinder/app/App.h"             // MouseEvent
#include "chGlobals.hpp"                // Tick
#include "chUtils.hpp"                  // midpoint, intersects, intersectionPoint
#include "Particle.hpp"

namespace ch {

class Barrier : public Particle {
public:
    Barrier(Tick currentTick, const vec2& first, const vec2& second);
    void update() override;
    void draw() const override;

    void mouseDown(vec2 mousePos);
    void mouseUp(vec2 mousePos);
    void mouseDrag(vec2 mousePos);
    void mouseMove(vec2 mousePos);
    void setMode(ch::Mode mode);
    bool isFocused() const;
    bool isActive() const { return mActive; }

    bool hasCrossed(const vec2& oldPos, const vec2& newPos) const;
    vec2 intersectionPoint(const vec2& oldPos, const vec2& newPos) const;
    vec2 reflectNormal(const vec2& incident) const;

    // inner class
    class EndPoint : public Particle {
    public:
        EndPoint(Tick currentTick, float size = 0.0f, const vec2& position = vec2{}) :
                Particle{size, position, currentTick}, mActive{true}, mMouseOver{false} {
            mFill = ColorA{0.5f, 0.5f, 0.0f, 0.3f};
            mOutline = ColorA{0.6f, 0.6f, 0.1f, 0.8f};
        }

        void update() override {};

        void draw() const override {
            if (not mActive) { return; }
            const auto size = mMouseOver or mTouchMode ? 2.0f * bSize : bSize;
            gl::color(mFill);
            gl::drawSolidCircle(bPosition - mOffset, size);
            gl::color(mOutline);
            gl::drawStrokedCircle(bPosition - mOffset, size, 1.0f);
        }

        vec2 getPosition() const { return bPosition - mOffset; }
        void setPosition(const vec2& pos) { bPosition = pos; }

        void mouseDown(const vec2& mousePos) {
            if (not contains(mousePos, 3.0f)) { return; }
            mMouseOver = true;
            mLastPos = mousePos;
        }

        void mouseUp(const vec2& mousePos, const vec2& other) {
            if (not mMouseOver) { return; }
            mMouseOver = false;
            bPosition -= mOffset;
            mOffset = vec2{};
        }

        void mouseDrag(const vec2& mousePos) {
            if (not mMouseOver) { return; }
            mOffset += (mLastPos - mousePos);
            mLastPos = mousePos;
        }

        bool isFocused() const { return mMouseOver; }

        void setTouchMode(bool active = true) { mTouchMode = active; }

    private:
        bool mActive = true;
        bool mMouseOver = false;
        bool mTouchMode = false;  // touch input and mode is ADD_FOOD
        vec2 mOffset, mLastPos;
        Color mFill, mOutline;
    };

    class MidPoint : public Particle {
    public:
        MidPoint(Tick currentTick, float size = 0.0f, const vec2& position = vec2{}) :
                Particle{size, position, currentTick}, mActive{true}, mMouseOver{false} {
            mFill = ColorA{0.5f, 0.0f, 0.0f, 0.3f};
            mOutline = ColorA{0.6f, 0.1f, 0.1f, 0.8f};
        }

        void update() override {};

        void draw() const override {
            if (not mActive or not mVisible) { return; }
            const auto size = 2.0f * bSize;
            gl::color(mFill);
            gl::drawSolidCircle(bPosition - mOffset, size);
            gl::color(mOutline);
            gl::drawStrokedCircle(bPosition - mOffset, size, 1.0f);
            gl::color(ColorA{0.8f, 0.8f, 0.8f, 0.8f});
            const auto c = bPosition - mOffset;
            const auto offset = vec2{bSize, 4.0f};
            gl::drawSolidRect(Rectf{c - offset, c + offset});
        }

        vec2 getPosition() const { return bPosition - mOffset; }
        void setPosition(const vec2& pos) { bPosition = pos; }

        void mouseDown(const vec2& mousePos) {
            if (not contains(mousePos, 2.0f)) { return; }
            mMouseOver = true;
        }

        void mouseUp(const vec2& mousePos, const vec2& other) {
            if (not mMouseOver or not mVisible) { return; }
            mMouseOver = false;
            if (not contains(mousePos, 2.0f)) { return; }
            mActive = false;
        }

        void mouseDrag(const vec2& mousePos) {
            if (not mMouseOver) { return; }
        }

        bool isFocused() const { return mMouseOver; }

        bool shouldRemove() const { return not mActive; }

        void setVisible(bool visible = true) { mVisible = visible; };

    private:
        bool mActive = true;
        bool mVisible = false;
        bool mMouseOver = false;
        vec2 mOffset;
        Color mFill, mOutline;
    };

private:
    EndPoint mFirst;
    EndPoint mSecond;
    MidPoint mMidPoint;
    vec2 mOffset, mLastPos;
    bool mPlaced = false;
    bool mActive = true;
};

Barrier::Barrier(Tick currentTick, const vec2& first = vec2{}, const vec2& second = vec2{}) :
        Particle{5.0f, midpoint(first, second), currentTick},
        mFirst{currentTick, bSize * 2.0f, first},
        mSecond{currentTick, bSize * 2.0f, second},
        mMidPoint{currentTick, bSize * 2.0f, first + 0.5f * (second - first)} {
    if (second == vec2{}) { mSecond = mFirst; }
}

void Barrier::update() {
    mFirst.update();
    mSecond.update();
    mMidPoint.update();
    const auto mid = mFirst.getPosition() +
            midpoint(mFirst.getPosition(), mSecond.getPosition());
    mMidPoint.setPosition(mid);
}

void Barrier::draw() const {
    if (not mActive) { return; }
    gl::ScopedModelMatrix modelMatrix;
    gl::translate(bPosition - mOffset);

    const auto translation = mSecond.getPosition() - mFirst.getPosition();
    const auto perpendicular = safeNormalize(vec2{-translation.y, translation.x});

    const auto linePoints = std::vector<vec2>{
        (mFirst.getPosition() + perpendicular * bSize),
        (mFirst.getPosition() - perpendicular * bSize),
        (mSecond.getPosition() - perpendicular * bSize),
        (mSecond.getPosition() + perpendicular * bSize)};

    const auto line = PolyLine2f{linePoints, true};

    gl::color(ColorA{0.6f, 0.6f, 0.1f, 0.8f});

    gl::drawSolid(line);

    mFirst.draw();
    mSecond.draw();
    mMidPoint.draw();
}

void Barrier::mouseDown(vec2 mousePos) {
    mLastPos = mousePos;
    mousePos -= bPosition;
    mFirst.mouseDown(mousePos);
    if (mFirst.isFocused()) { return; }
    mSecond.mouseDown(mousePos);
    if (mSecond.isFocused()) { return; }
    mMidPoint.mouseDown(mousePos);
}

void Barrier::mouseUp(vec2 mousePos) {
    const auto dist = distance(mFirst.getPosition(), mSecond.getPosition());
    if (dist < bSize * 4.0f) { return; }
    mousePos -= bPosition;
    mFirst.mouseUp(mousePos, mSecond.getPosition());
    mSecond.mouseUp(mousePos, mFirst.getPosition());
    mMidPoint.mouseUp(mousePos, mMidPoint.getPosition());
    bPosition -= mOffset;
    mOffset = vec2{};
    mPlaced = true;
    if (mMidPoint.shouldRemove()) { mActive = false; }
}

void Barrier::mouseDrag(vec2 mousePos) {
    //if (mMidPoint.isFocused()) {
    //    mOffset += (mLastPos - mousePos);
    //    mLastPos = bPosition;
    //}
    mousePos -= bPosition;
    if (not mPlaced) { return; }
    mFirst.mouseDrag(mousePos);
    if (mFirst.isFocused()) { return; }
    mSecond.mouseDrag(mousePos);
    if (mSecond.isFocused()) { return; }
    //mMidPoint.mouseDrag(mousePos);
}

void Barrier::mouseMove(vec2 mousePos) {
    //mousePos -= bPosition;
    mFirst.mouseDrag(mousePos);
    mSecond.mouseDrag(mousePos);
}

bool Barrier::isFocused() const {
    return mFirst.isFocused() or mSecond.isFocused() or mMidPoint.isFocused();
}

void Barrier::setMode(ch::Mode mode) {
    switch (mode) {
    case ADD_BARRIER:
        mFirst.setTouchMode(true);
        mSecond.setTouchMode(true);
        mMidPoint.setVisible(false);
        break;
    case REMOVE_BARRIER:
        mFirst.setTouchMode(false);
        mSecond.setTouchMode(false);
        mMidPoint.setVisible(true);
        break;
    default:
        mFirst.setTouchMode(false);
        mSecond.setTouchMode(false);
        mMidPoint.setVisible(false);
    }
}

inline bool Barrier::hasCrossed(const vec2& oldPos, const vec2& newPos) const {
    return intersects(oldPos, newPos,
            bPosition + mFirst.getPosition(), bPosition + mSecond.getPosition());
}

inline vec2 Barrier::intersectionPoint(const vec2& oldPos, const vec2& newPos) const {
    return getIntersection(oldPos, newPos,
            bPosition + mFirst.getPosition(), bPosition + mSecond.getPosition());
}

inline vec2 Barrier::reflectNormal(const vec2& incident) const {
    const vec2 lineNormal = ch::normal(mFirst.getPosition(), mSecond.getPosition());
    return glm::reflect(incident, safeNormalize(lineNormal));
}

} // namespace ch

#endif
