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
        acceleration = vec2{0, 0};
        velocity = vec2{0, 0};
        position = vec2{x, y};
        r = 6.0f;
        maxspeed = 4.0f;
        maxforce = 0.1f;
    }

    // updates the position
    void update() {
        velocity += acceleration;   // update the velocity
        ch::limit(velocity, maxspeed);
        position += velocity;
        acceleration = vec2{0, 0};   // reset acceleration to 0 each cycle
    }

    void applyForce(vec2 force) {
        // we could add mass here if we want A = F / M
        acceleration += force;
    }

    // calculates a steering force towards a target
    void arrive(vec2 target) {
        vec2 desired = target - position;
        const float d = length(desired);
        std::cout << d << '\n';
        // scale within arbitrary damping within 100 pixels so that it arrives
        if (d < 100) {
            const float m = lmap(d, 0.0f, 100.0f, 0.0f, maxspeed);
            ch::setMagnitude(desired, m);
        } else {
            ch::setMagnitude(desired, maxspeed);
        }

        vec2 steer = desired - velocity;
        ch::limit(steer, maxforce);
        applyForce(steer);
    }

    void draw() {
        // rotate in the direction of velocity
        const float theta = ch::heading(velocity) + M_PI / 2.0f;

        gl::pushModelMatrix();
        gl::translate(position);
        gl::rotate(theta);

        PolyLine2f pl;
        pl.push_back(vec2{0.f, -r * 2.f});
        pl.push_back(vec2{-r, r * 2.f});
        pl.push_back(vec2{r, r * 2.f});
        gl::drawSolid(pl);

        gl::popModelMatrix();
    }

private:
    vec2 position;
    vec2 velocity;
    vec2 acceleration;
    float r;
    float maxforce;
    float maxspeed;
};

#endif
