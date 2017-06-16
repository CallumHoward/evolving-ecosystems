// Vehicle.cpp
// Callum Howard, 2017

#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include "chUtils.hpp"
#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"

using namespace  ci;

class Vehicle {
public:
    Vehicle(float x, float y) {
        mAcceleration = vec2{0, 0};
        mVelocity = vec2{0, 0};
        mPosition = vec2{x, y};
        mR = 6.0f;
        mMaxSpeed = 4.0f;
        mMaxForce = 0.1f;
    }

    // updates the mPosition
    void update() {
        mVelocity += mAcceleration;   // update the velocity
        ch::limit(mVelocity, mMaxSpeed);
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

    void draw() {
        // rotate in the direction of velocity
        const float theta = ch::heading(mVelocity) + M_PI / 2.0f;

        gl::pushModelMatrix();
        gl::translate(mPosition);
        gl::rotate(theta);

        PolyLine2f pl;
        pl.push_back(vec2{0.f, -mR * 2.f});
        pl.push_back(vec2{-mR, mR * 2.f});
        pl.push_back(vec2{mR, mR * 2.f});
        gl::drawSolid(pl);

        gl::popModelMatrix();
    }

private:
    vec2 mPosition;
    vec2 mVelocity;
    vec2 mAcceleration;
    float mR;
    float mMaxForce;
    float mMaxSpeed;
};

#endif
