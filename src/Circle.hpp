#ifndef Circle_hpp
#define Circle_hpp

#include "Circle.hpp"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "Particle.hpp"

namespace ch {

using namespace ci;
using namespace ci::app;

class Circle : public Particle {
public:
    Circle(float size = 0.0f, const vec2& position = vec2{})
            : Particle{size, position} {}

    void update() override;
    void draw() const override;

    void setRadius(float radius) { bSize = radius; }
    void setCenter(vec2 center) { bPosition = center; }

    inline bool within(const Area& a) const { return a.contains(bPosition); }
};


void Circle::update() {}

void Circle::draw() const {
    gl::color(0.0f, 0.5f, 0.7f, 0.3);
    gl::drawSolidCircle(bPosition, bSize);
    gl::color(0.1f, 0.6f, 0.8f, 0.8);
    gl::drawStrokedCircle(bPosition, bSize);
}

}

#endif /* Circle_hpp */
