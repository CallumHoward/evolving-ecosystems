// Particle.hpp
// Callum Howard, 2017

#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "cinder/gl/gl.h"

namespace ch {

using namespace ci;

class Particle {
public:
    Particle(float size, const vec2 &position)
            : bSize{size}, bPosition{position} {}

    virtual void update() = 0;
    virtual void draw() const = 0;

    vec2 getPosition() const { return bPosition; }
    float getSize() const { return bSize; }

protected:
    bool operator==(const Particle& p) const {
        return bPosition == p.bPosition and bSize == p.bSize;
    }

    bool operator!=(const Particle& p) const { return not (*this == p); }

    vec2 bPosition;
    float bSize;
};

}

#endif
