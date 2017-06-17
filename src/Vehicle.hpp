// Vehicle.cpp
// Callum Howard, 2017

#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include "chUtils.hpp"
#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include <boost/circular_buffer.hpp>
#include <range/v3/view.hpp>

using namespace  ci;

class Vehicle {
public:
    Vehicle(float x, float y) : Vehicle{vec2{x, y}} {}
    Vehicle(const vec2& point = vec2{}) {
        mAcceleration = vec2{0, 0};
        mVelocity = vec2{0, 0};
        mPosition = point;
        mR = 6.0f;
        mMaxSpeed = 40.0f;
        mMaxForce = 0.8f;
        mHistorySkip = 0;  // for spread length of tail
        mHistorySize = 10;
        mHistory = boost::circular_buffer<vec2>(mHistorySize);
    }

    // updates the mPosition
    void update() {
        mVelocity += mAcceleration;   // update the velocity
        ch::limit(mVelocity, mMaxSpeed);
        if (mHistorySkip % 5 == 0) {
            mHistory.push_back(mPosition);
        }
        mPosition += mVelocity;
        mAcceleration = vec2{0, 0};   // reset acceleration to 0 each cycle
    }

    void applyForce(vec2 force) {
        // we could add mass here if we want A = F / M
        mAcceleration += force;
    }

    // calculates a steering force towards a target
    void arrive(vec2 target) {
        vec2 desired = target - mPosition;
        const float d = length(desired);

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

    void draw() const {

        draw_tail();

        // rotate in the direction of velocity
        const float theta = ch::heading(mVelocity) + M_PI / 2.0f;

        gl::pushModelMatrix();
        gl::translate(mPosition);
        gl::rotate(theta);

        gl::color(0.2f, 0.8f, 0.2f, 1.f);
        gl::drawSolidCircle(vec2{}, mR);

        gl::popModelMatrix();
    }

private:

    void draw_tail() const {
        // draw tail
        const float decayIncrement =
                lmap(1.f, 0.f, static_cast<float>(mHistorySize), 0.f, 1.f);
        float decay = decayIncrement;
        for (const auto& pos : mHistory) {
            gl::color(0.2f, 0.8f, 0.2f, decay * 0.5f);
            gl::drawSolidCircle(pos, mR * decay);
            decay += decayIncrement;
        }
    }

    vec2 mPosition;
    vec2 mVelocity;
    vec2 mAcceleration;
    boost::circular_buffer<vec2> mHistory;
    size_t mHistorySize;
    size_t mHistorySkip;
    float mR;
    float mMaxForce;
    float mMaxSpeed;
};

#endif
