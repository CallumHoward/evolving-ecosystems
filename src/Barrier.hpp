// Barrier.hpp
// Callum Howard, 2017

#ifndef BARRIER_HPP
#define BARRIER_HPP

#include "cinder/app/App.h"             // MouseEvent
#include "chGlobals.hpp"                // Tick
#include "chUtils.hpp"                  // midpoint
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
    bool isFocused() const;

    //bool hasCrossed(const vec2& oldPos, const vec2& newPos) const; //TODO

    // inner class
    class EndPoint : public Particle {
    public:
        EndPoint(Tick currentTick, float size = 0.0f, const vec2& position = vec2{}) :
                Particle{size, position, currentTick}, mActive{true}, mMouseOver{false} {
            mFill = ColorA{0.5f, 0.5f, 0.0f, 0.3};
            mOutline = ColorA{0.6f, 0.6f, 0.1f, 0.8};
        }

        void update() override {};

        void draw() const override {
            if (not mActive) { return; }
            const auto size = mMouseOver ? 2.0f * bSize : bSize;
            gl::color(mFill);
            gl::drawSolidCircle(bPosition - mOffset, size);
            gl::color(mOutline);
            gl::drawStrokedCircle(bPosition - mOffset, size, 1.0f);
        }

        vec2 getPosition() const { return bPosition - mOffset; }

        void mouseDown(const vec2& mousePos) {
            if (not contains(mousePos)) { return; }
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

    private:
        bool mActive;
        bool mMouseOver;
        vec2 mOffset, mLastPos;
        Color mFill, mOutline;
    };

private:
    EndPoint mFirst;
    EndPoint mSecond;
};

Barrier::Barrier(Tick currentTick, const vec2& first = vec2{}, const vec2& second = vec2{}) :
    Particle{5.0f, midpoint(first, second), currentTick},
    mFirst{currentTick, bSize * 2.0f, first},
    mSecond{currentTick, bSize * 2.0f, second} {
        if (second == vec2{}) { mSecond = mFirst; }
}

void Barrier::update() {
    mFirst.update();
    mSecond.update();
}

void Barrier::draw() const {
    gl::ScopedModelMatrix modelMatrix;
    gl::translate(bPosition);

    const auto translation = mSecond.getPosition() - mFirst.getPosition();
    const auto perpendicular = normalize(vec2{-translation.y, translation.x});

    const auto linePoints = std::vector<vec2>{
        (mFirst.getPosition() + perpendicular * bSize),
        (mFirst.getPosition() - perpendicular * bSize),
        (mSecond.getPosition() - perpendicular * bSize),
        (mSecond.getPosition() + perpendicular * bSize)};

    const auto line = PolyLine2f{linePoints, true};

    gl::color(ColorA{0.6f, 0.6f, 0.1f, 0.8});

    gl::drawSolid(line);

    mFirst.draw();
    mSecond.draw();
}

void Barrier::mouseDown(vec2 mousePos) {
    mousePos -= bPosition;
    mFirst.mouseDown(mousePos);
    mSecond.mouseDown(mousePos);
}

void Barrier::mouseUp(vec2 mousePos) {
    mousePos -= bPosition;
    mFirst.mouseUp(mousePos, mSecond.getPosition());
    mSecond.mouseUp(mousePos, mFirst.getPosition());
}

void Barrier::mouseDrag(vec2 mousePos) {
    mousePos -= bPosition;
    mFirst.mouseDrag(mousePos);
    mSecond.mouseDrag(mousePos);
}

bool Barrier::isFocused() const {
    return mFirst.isFocused() or mSecond.isFocused();
}

} // namespace ch

#endif
