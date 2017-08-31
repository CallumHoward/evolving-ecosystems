// Particle.hpp
// Callum Howard, 2017

#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "cinder/gl/gl.h"
#include "chGlobals.hpp"    // Tick

namespace ch {

using namespace ci;

class Particle {
public:
    Particle(float size, const vec2 &position, Tick currentTick)
            : bSize{size}, bPosition{position}, bBirthTick{currentTick} {}

    virtual void update() = 0;
    virtual void draw() const = 0;

    vec2 getPosition() const { return bPosition; }
    float getSize() const { return bSize; }

protected:
    vec2 bPosition;
    float bSize;
    Tick bBirthTick;
};

}

#endif
