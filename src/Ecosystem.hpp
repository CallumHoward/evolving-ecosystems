// Ecosystem.hpp
// Callum Howard, 2017

#ifndef ECOSYSTEM_HPP
#define ECOSYSTEM_HPP

#include <vector>
#include <range/v3/algorithm.hpp>
#include "cinder/gl/gl.h"
#include "sp/Grid.h"
#include "chUtils.hpp"
#include "Circle.hpp"
#include "Vehicle.hpp"

namespace ch {

using namespace ci;
using namespace ranges;

class Ecosystem {
public:
    void setup();
    void update(const vec2& arrivePoint);
    void draw();

private:
    int mNumFood = 50;
    int mNumVehicles = 50;
    std::vector<Circle> mFood;
    std::vector<Vehicle> mVehicles;

    using SpatialStruct = sp::Grid2<Particle*>;

    SpatialStruct mParticleSpatialStruct;
};


void Ecosystem::setup() {
    mFood = std::vector<Circle>(mNumFood);
    generate(mFood, []{ return Circle{3.0f, makeRandPoint()}; });

    mVehicles = std::vector<Vehicle>(mNumVehicles);
    generate(mVehicles, []{ return Vehicle{makeRandPoint()}; });

    mParticleSpatialStruct = SpatialStruct{};
}

void Ecosystem::update(const vec2& arrivePoint) {
    // update the grid
    mParticleSpatialStruct.clear();
    for (auto& particle : mFood) {
        mParticleSpatialStruct.insert(particle.getPosition(), &particle);
    }
    //for (auto& particle : mVehicles) {
    //    mParticleSpatialStruct.insert(particle.getPosition(), &particle);
    //}

    for (auto& vehicle : mVehicles) {
        const auto nn = mParticleSpatialStruct.nearestNeighborSearch(vehicle.getPosition());
        vehicle.arrive(nn->getPosition());
        vehicle.update();
    }
}

void Ecosystem::draw() {
    for (auto& vehicle : mVehicles) { vehicle.draw(); }
    for (auto& food : mFood) { food.draw(); }
}

}

#endif
