// Vehicle.cpp
// Callum Howard, 2017

#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <boost/circular_buffer.hpp>
#include <range/v3/view.hpp>
#include "chUtils.hpp"                  // limit, heading, setMagnitude, length
#include "chGlobals.hpp"                // Tick
#include "cinder/gl/gl.h"               // vec2
#include "cinder/Timeline.h"
#include "cinder/Rand.h"                // randFloat
#include "cinder/CinderMath.h"          // M_PI, lmap, constrain
#include "Particle.hpp"
#include "Barrier.hpp"

namespace ch {

using namespace  ci;

Color sGreen = Color{0.1f, 0.4f, 0.1f};
Color sBright = Color{0.4f, 0.9f, 0.4f};

class Vehicle : public Particle {
public:
    //Vehicle(Tick currentTick, float x, float y) : Vehicle{currentTick, vec2{x, y}} {}
    Vehicle(Tick currentTick, const vec2& point = vec2{}, const Color& c = sGreen) :
        Particle{6.0f, point, currentTick},
        mVelocity{0, 0},
        mAcceleration{0, 0},
        mMaxForce{0.8f},
        mMaxSpeed{40.0f},
        mMaxEnergy{100.0f},
        mHistorySkip{0},  // for spread length of tail
        mHistorySize{10} {
            mColor = c;
            mEnergy = randFloat(mMaxEnergy / 4.0f, mMaxEnergy / 2.0f);
            mHistory = boost::circular_buffer<vec2>{mHistorySize};
    }

    void update() override;  // updates the position of the vehicle
    void draw() const override;

    void arrive(const vec2& target);
    void eat(float energy);
    bool isDead() { return mEnergy <= 0.0f; }
    void setColor(Color c) { mColor = c; }

    // we could add mass here if we want A = F / M
    void applyForce(vec2 force) { mAcceleration += force / (bSize / 3.0f); }

private:
    void draw_tail() const;

    Anim<Color> mColor;
    vec2 mVelocity;
    vec2 mAcceleration;
    float mEnergy;
    float mMaxForce;
    float mMaxSpeed;
    float mMaxEnergy;
    size_t mHistorySkip;
    size_t mHistorySize;
    boost::circular_buffer<vec2> mHistory;
};


void Vehicle::update(std::vector<Barrier> barriers) {
    mVelocity += mAcceleration;  // update the velocity
    ch::limit(mVelocity, mMaxSpeed);
    if (mHistorySkip % 5 == 0) {
        mHistory.push_back(bPosition);
    }

    // do barrier collision detection
    for (const auto& barrier : barriers) {
        if (barrier.hasCrossed(bPosition, bPosition + mVelocity)) {
            return;
        }
    }

    bPosition += mVelocity;
    mAcceleration = vec2{0, 0};  // reset acceleration to 0 each cycle

    mEnergy -= 0.5f;  // as time passes
    mEnergy -= 0.1f * ch::length(mAcceleration) * bSize;  // F = M * A
}

void Vehicle::draw() const {
    draw_tail();

    // rotate in the direction of velocity
    const float theta = ch::heading(mVelocity) + M_PI / 2.0f;

    gl::ScopedModelMatrix modelMatrix;
    gl::translate(bPosition);
    gl::rotate(theta);

    const float vitality = lmap(mEnergy, 0.0f, mMaxEnergy, 0.3f, 1.0f);

    gl::color(ColorA{mColor.value(), vitality});
    gl::drawSolidCircle(vec2{}, bSize * vitality * 2.0f);
}

void Vehicle::eat(float energy) {
    mEnergy = constrain(mEnergy + energy, 0.0f, mMaxEnergy);
    mColor = sBright;
    timeline().apply(&mColor, sGreen, 1.0f, EaseOutAtan());
}

// calculates a steering force towards a target
void Vehicle::arrive(const vec2& target) {
    vec2 desired = target - bPosition;
    const float d = ch::length(desired);

    // scale within arbitrary damping within 100 pixels so that it arrives
    const float proximity = 100.0f;
    if (d < proximity) {
        const float m = lmap(d, 0.0f, proximity, 0.0f, mMaxSpeed);
        ch::setMagnitude(desired, m);
    } else {
        ch::setMagnitude(desired, mMaxSpeed);
    }

    vec2 steer = desired - mVelocity;
    ch::limit(steer, mMaxForce);
    applyForce(steer);
}

void Vehicle::draw_tail() const {
    const float vitality = lmap(mEnergy, 0.0f, mMaxEnergy, 0.4f, 1.0f);
    const float decayIncrement =
            lmap(1.f, 0.f, static_cast<float>(mHistorySize), 0.f, 1.f);
    float decay = decayIncrement;
    for (const auto& pos : mHistory) {
        const auto factor = 2.0f;
        gl::color(factor * sGreen[0], factor * sGreen[1], factor * sGreen[2],
                decay * 0.5f * vitality);
        gl::drawSolidCircle(pos, bSize * decay * vitality * 2.0f);
        decay += decayIncrement;
    }
}

}

#endif
