// chUtils.hpp
// Callum Howard, 2017

#ifndef CHUTILS_H
#define CHUTILS_H

#include <cmath>                // fabs
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
    bool doIntersect(const vec2& p1, const vec2& q1, const vec2& p2, const vec2& q2);

    inline vec2 midpoint(const vec2& first, const vec2& second) {
        return (second - first) * 0.5f;
    };

    template<typename T>
    inline bool between(T a, T lower, T upper) { return a >= lower and a <= upper; }

    inline float lengthSquared(const vec2& v) { return v.x * v.x + v.y * v.y; }
    inline float length(const vec2& v) { return glm::sqrt(lengthSquared(v)); }
    inline float distance(const vec2& a, const vec2& b) { return length(a - b); }
    inline float heading(const vec2& v) { return atan2(v.y, v.x); }

    vec2 hOrV(const vec2& v) {
        return (std::fabs(v.x) > std::fabs(v.y)) ? vec2{v.x, 0} : vec2{0, v.y};
    }

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
