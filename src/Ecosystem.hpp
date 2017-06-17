// Ecosystem.hpp
// Callum Howard, 2017

#ifndef ECOSYSTEM_HPP
#define ECOSYSTEM_HPP

#include <vector>
#include <range/v3/algorithm.hpp>
#include "cinder/gl/gl.h"
#include "chUtils.hpp"
#include "Circle.hpp"
#include "Vehicle.hpp"

namespace ch {

using namespace ci;
using namespace ranges;

class Ecosystem {
public:
    void setup();
    void update();
    void draw(const vec2& arrivePoint);

private:
    int mNumFood = 50;
    int mNumVehicles = 50;
    std::vector<Circle> mFood;
    std::vector<Vehicle> mVehicles;
};

void Ecosystem::setup() {
    mVehicles = std::vector<Vehicle>(mNumVehicles);
    generate(mVehicles, []{ return Vehicle{makeRandPoint()}; });
}

void Ecosystem::update() {}

void Ecosystem::draw(const vec2& arrivePoint) {
    for (auto& vehicle : mVehicles) {
        vehicle.arrive(arrivePoint);
        vehicle.update();
        vehicle.draw();
    }
}

}

#endif
