// Barrier.hpp
// Callum Howard, 2017

#ifndef BARRIER_HPP
#define BARRIER_HPP

#include "chGlobals.hpp"                // Tick
#include "chUtils.hpp"                  // midpoint
#include "Particle.hpp"

namespace ch {

class Barrier : public Particle {
public:
    Barrier(Tick currentTick, const vec2& first, const vec2& second);
    void update() override;
    void draw() const override;

    //bool hasCrossed(const vec2& oldPos, const vec2& newPos) const; //TODO

    // inner class
    class EndPoint : public Particle {
    public:
        EndPoint(Tick currentTick, float size = 0.0f, const vec2& position = vec2{})
        : Particle{size, position, currentTick}, mActive{true} {
            mFill = ColorA{0.5f, 0.5f, 0.0f, 0.3};
            mOutline = ColorA{0.6f, 0.6f, 0.1f, 0.8};
        }

        void update() override {};

        void draw() const override {
            if (not mActive) { return; }
            gl::color(mFill);
            gl::drawSolidCircle(bPosition, bSize);
            gl::color(mOutline);
            gl::drawStrokedCircle(bPosition, bSize, 1.0f);
        }

        vec2 getPosition() const { return bPosition; }

    private:
        bool mActive;
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

    gl::color(ColorA{0.6f, 0.6f, 0.1f, 0.8});

    const auto translation = mSecond.getPosition() - mFirst.getPosition();
    const auto perpendicular = normalize(vec2{translation.y, translation.x});

    const auto linePoints = std::vector<vec2>{
        (mFirst.getPosition() - perpendicular * bSize),
        (mFirst.getPosition() + perpendicular * bSize),
        (mSecond.getPosition() - perpendicular * bSize),
        (mSecond.getPosition() + perpendicular * bSize)};

    const auto line = Rectf{linePoints};

    gl::drawSolidRect(line);

    mFirst.draw();
    mSecond.draw();
}

} // namespace ch

#endif
