// Circle.hpp
// Callum Howard, 2017

#ifndef Circle_hpp
#define Circle_hpp

#include "Circle.hpp"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "Particle.hpp"
#include "chGlobals.hpp"                // Tick

namespace ch {

using namespace ci;
using namespace ci::app;

class Circle : public Particle {
public:
    enum CType {
        FOOD,
        CORPSE
    };

    Circle(Tick currentTick, float size = 0.0f,
            const vec2& position = vec2{}, const CType t = FOOD)
        : Particle{size, position, currentTick}, mType{t}, mActive{true} {
        switch (mType) {
        case FOOD:
            mEnergy = 25.0f;
            mFill = ColorA{0.0f, 0.5f, 0.7f, 0.3f};
            mOutline = ColorA{0.1f, 0.6f, 0.8f, 0.8f};
            break;
        case CORPSE:
            mEnergy = 50.0f;
            mFill = ColorA{0.7f, 0.2f, 0.3f, 0.1f};
            mOutline = ColorA{0.8f, 0.3f, 0.4f, 0.5f};
            break;
        }
    }

    void update() override;
    void draw() const override;

    void setRadius(float radius) { bSize = radius; }
    void setCenter(vec2 center) { bPosition = center; }

    bool within(const Area& a) const { return a.contains(bPosition); }

    CType getType() { return mType; }
    constexpr float getEnergy() const { return mEnergy; }
    void setEnergy(float energy) { mEnergy = energy; }
    bool isActive() const { return mActive; }
    void setActive(bool b) { mActive = b; }

private:
    bool mActive;
    CType mType;
    Color mFill, mOutline;
    float mEnergy;
};


void Circle::update() {
    //TODO add wander
}

void Circle::draw() const {
    if (not mActive) { return; }
    gl::color(mFill);
    gl::drawSolidCircle(bPosition, bSize);
    gl::color(mOutline);
    gl::drawStrokedCircle(bPosition, bSize, 1.0f);
}

}

#endif /* Circle_hpp */
