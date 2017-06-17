#ifndef Circle_hpp
#define Circle_hpp

#include "Circle.hpp"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

namespace ch {

using namespace ci;
using namespace ci::app;

class Circle {
public:
    Circle(float radius = 0.0f, const vec2 &center = vec2{})
            : radius_{radius}, center_{center} {}

    void draw() const;

    void setRadius(float radius) { radius_ = radius; }
    void setCenter(vec2 center) { center_ = center; }

    inline bool within(const Area& a) const { return a.contains(center_); }

private:
    float radius_;
    vec2 center_;
};


void Circle::draw() const {
    gl::color(0.0f, 0.5f, 0.7f, 0.3);
    gl::drawSolidCircle(center_, radius_);
    gl::color(0.1f, 0.6f, 0.8f, 0.8);
    gl::drawStrokedCircle(center_, radius_);
}

}

#endif /* Circle_hpp */
