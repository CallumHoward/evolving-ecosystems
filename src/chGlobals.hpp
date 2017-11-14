// chGlobals.hpp
// Callum Howard, 2017

#ifndef CHGLOBALS_HPP
#define CHGLOBALS_HPP

namespace ch {

    using Tick = unsigned long;  // big enough to last for more than 2 years

    enum Mode {
        ADD_FOOD,
        ADD_BARRIER,
        REMOVE_BARRIER
    };


} // namespace ch

#endif
