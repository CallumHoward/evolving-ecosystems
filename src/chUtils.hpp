//
//  chUtils.hpp
//  ParticleSystem
//
//  Created by Callum Howard on 12/6/17.
//
//

#ifndef chUtils_h
#define chUtils_h

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "Cinder/app/App.h"

namespace ch {
    using namespace ci;
    using namespace ci::app;

    // prototypes
    void setMagnitude(vec2& v, float len);
    vec2 safeNormalize(const vec2& v);
    void limit(vec2& v, float maxLength);

    template<typename T>
    inline bool between(T a, T lower, T upper) { return a >= lower and a <= upper; }

    inline float lengthSquared(const vec2& v) { return v.x * v.x + v.y * v.y; }
    inline float length(const vec2& v) { return glm::sqrt(lengthSquared(v)); }
    inline float distance(const vec2& a, const vec2& b) { return length(a - b); }
    inline float heading(const vec2& v) { return atan2(v.y, v.x); }

    vec2 safeNormalize(const vec2& v) {
        const auto s = lengthSquared(v);
        if (s <= 0) { return vec2{}; }
        auto invL = glm::inversesqrt(s);
        return v * invL;
    }

    void limit(vec2& v, float maxLength) {
        const auto ls = lengthSquared(v);
        if ((ls > maxLength * maxLength) and (ls > 0)) {
            float ratio = maxLength / glm::sqrt(ls);
            v *= ratio;
        }
    }

    void setMagnitude(vec2& v, float len) { v = safeNormalize(v) * len; }
    
    inline vec2 makeRandPoint() {
        return vec2{randFloat(getWindowWidth()), randFloat(getWindowHeight())};
    }

}

#endif /* chUtils_h */
