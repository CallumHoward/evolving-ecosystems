// Ecosystem.hpp
// Callum Howard, 2017

#ifndef ECOSYSTEM_HPP
#define ECOSYSTEM_HPP

#include <cassert>
#include <vector>
#include <limits>                       // numeric_limits
#include <range/v3/algorithm.hpp>       // generate
#include <boost/circular_buffer.hpp>
#include "cinder/gl/gl.h"
#include "sp/Grid.h"
#include "sp/KdTree.h"
#include "chUtils.hpp"                  // makeRandPoint
#include "chGlobals.hpp"                // Tick
#include "Circle.hpp"
#include "Vehicle.hpp"

namespace ch {

using namespace ci;
using namespace ranges;

class Ecosystem {
public:
    void setup();
    void update();
    void draw();

    Color mVehicleColor = Color{0.1f, 0.4f, 0.1f};

private:
    Tick mTickCount = 0;
    int mNumFood = 30;
    int mMaxNumFood = 30;
    int mNumVehicles = 50;
    std::vector<Circle> mFood;
    std::vector<Vehicle> mVehicles;
    boost::circular_buffer<Circle> mCorpses;

    //using SpatialStruct = sp::Grid2<Particle*>;
    using SpatialStruct = sp::KdTree2<Particle*>;

    SpatialStruct mParticleSpatialStruct;
};


void Ecosystem::setup() {
    mFood = std::vector<Circle>(mNumFood);
    generate(mFood, [this]{ return Circle{0, 3.0f, makeRandPoint()}; });

    mVehicles = std::vector<Vehicle>(mNumVehicles);
    generate(mVehicles, [this]{ return Vehicle{0, makeRandPoint(), mVehicleColor}; });

    mCorpses = boost::circular_buffer<Circle>{30};
    mParticleSpatialStruct = SpatialStruct{};
}

void Ecosystem::update() {
    // update the grid
    mParticleSpatialStruct.clear();
    for (auto& particle : mFood) {
        mParticleSpatialStruct.insert(particle.getPosition(), &particle);
    }
    for (auto& particle : mCorpses) {
        if (not particle.isActive()) { continue; }
        mParticleSpatialStruct.insert(particle.getPosition(), &particle);
    }

    for (auto& vehicle : mVehicles) {
        if (vehicle.isDead()) {
            mCorpses.push_back(Circle{mTickCount, 5.0f, vehicle.getPosition(), Circle::CORPSE});
            vehicle = Vehicle{mTickCount, makeRandPoint(), mVehicleColor};
            continue;
        }

        float distanceSquared;
        auto nn = mParticleSpatialStruct.nearestNeighborSearch(
                vehicle.getPosition(), &distanceSquared);
        Circle* nearestFoodRef = static_cast<Circle *>(nn->getData());

        const auto size = vehicle.getSize();
        if (distanceSquared < size * size) {
            vehicle.eat(nearestFoodRef->getEnergy());
            switch (nearestFoodRef->getType()) {
            case Circle::FOOD:
                *nearestFoodRef = Circle{mTickCount, 3.0f, makeRandPoint()};
                break;
            case Circle::CORPSE:
                nearestFoodRef->setActive(false);
                break;
            }
        }

        vehicle.arrive(nearestFoodRef->getPosition());
        vehicle.update();

        // update ecosystem tick count
        ++mTickCount;
        assert(mTickCount != std::numeric_limits<Tick>::max);
    }
}

void Ecosystem::draw() {
    for (auto& corpse : mCorpses) { corpse.draw(); }
    for (auto& food : mFood) { food.draw(); }
    for (auto& vehicle : mVehicles) { vehicle.draw(); }
}

}

#endif
