// chGlobals.hpp
// Callum Howard, 2017

#ifndef CHGLOBALS_HPP
#define CHGLOBALS_HPP

#include "cinder/gl/gl.h"
#include <iso646.h>

namespace ch {

using Tick = unsigned long;  // big enough to last for more than 2 years

enum Mode {
    PAN_VIEW,
    ADD_FOOD,
    ADD_BARRIER,
    REMOVE_BARRIER
};

cinder::gl::TextureRef gGlow;

} // namespace ch

#endif
