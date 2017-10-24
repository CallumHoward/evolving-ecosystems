// Ecosystem.hpp
// Callum Howard, 2017

#ifndef ECOSYSTEM_HPP
#define ECOSYSTEM_HPP

#include <cassert>
#include <vector>
#include <limits>                       // numeric_limits
#include <algorithm>                    // generate
#include <range/v3/algorithm/any_of.hpp>
#include <boost/circular_buffer.hpp>
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"             // MouseEvent
#include "sp/Grid.h"
#include "sp/KdTree.h"
#include "chUtils.hpp"                  // makeRandPoint, distance
#include "chGlobals.hpp"                // Tick
#include "Circle.hpp"
#include "Vehicle.hpp"
#include "Barrier.hpp"

namespace ch {

using namespace ci;
using namespace ranges;

class Ecosystem {
public:
    void setup();
    void update();
    void draw() const;
    void mouseDown(const vec2& mousePos);
    void mouseUp(const vec2& mousePos);
    void mouseDrag(const vec2& mousePos);
    void mouseMove(const vec2& mousePos);
    void keyDown(KeyEvent event);
    void addBarrier();
    void removeBarrier();
    bool isFocused() const;
    Color mVehicleColor = Color{0.1f, 0.4f, 0.1f};

private:
    void updateVehicles();
    bool isOccluded(const Vehicle& v, const vec2& target);

    Tick mTickCount = 0;
    int mNumFood = 30;
    int mMaxNumFood = 30;
    int mNumVehicles = 5;
    Tick mFittestLifetime = 0;
    std::vector<Circle> mFood;
    std::vector<Vehicle> mVehicles;
    std::vector<Barrier> mBarriers;
    boost::circular_buffer<Circle> mCorpses;

    //using SpatialStruct = sp::Grid2<Particle*>;
    using SpatialStruct = sp::KdTree2<Particle*>;

    SpatialStruct mParticleSpatialStruct;
};


void Ecosystem::setup() {

    std::generate_n(std::back_inserter(mFood), mNumFood,
            [this]{ return Circle{0, 3.0f, makeRandPoint()}; });

    std::generate_n(std::back_inserter(mVehicles), mNumVehicles,
            [this]{ return Vehicle{0, makeRandPoint(), mVehicleColor}; });

    mCorpses = boost::circular_buffer<Circle>{30};

    mBarriers = std::vector<Barrier>{};

    // make a sample barrier
    //mBarriers.push_back(Barrier{0});

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

    updateVehicles();

    for (auto& barrier : mBarriers) {
        barrier.update();
    }
}

void Ecosystem::updateVehicles() {

    for (auto& vehicle : mVehicles) {
        if (vehicle.isDead()) {
            // check how long it survived and if it broke the record
            const auto lifetime = mTickCount - vehicle.getBirthTick();
            if (lifetime > mFittestLifetime) { mFittestLifetime = lifetime; }

            // place a corpse at its last position
            mCorpses.push_back(Circle{mTickCount, 5.0f, vehicle.getPosition(), Circle::CORPSE});

            // spawn a new vehicle in its place
            vehicle = Vehicle{mTickCount, makeRandPoint(), mVehicleColor};
            continue;
        }

        // find a target to seek

        // in case a nearest neighbor can't be found
        auto fallbackTarget = Circle{mTickCount, 0.0f, makeRandPoint()};
        fallbackTarget.setActive(false);
        fallbackTarget.setEnergy(0.0f);

        Circle* nearestFoodRef = &fallbackTarget;
        float distanceSquared = distance(vehicle.getPosition(), fallbackTarget.getPosition());

        // optimistically do quick look for nearest neighbor
        float optimisticDistanceSquared;
        auto nn = mParticleSpatialStruct.nearestNeighborSearch(
               vehicle.getPosition(), &optimisticDistanceSquared);
        Circle* optimisticNearestFoodRef = static_cast<Circle *>(nn->getData());

        // if it is within line of sight then optimistic is a good choice
        if (not isOccluded(vehicle, nn->getPosition())) {
            nearestFoodRef = optimisticNearestFoodRef;
            distanceSquared = optimisticDistanceSquared;

        } else {  // try and find another target
            const auto neighbors = mParticleSpatialStruct.rangeSearch(
                    vehicle.getPosition(), vehicle.getSightDist());

            for (const auto& neighbor : neighbors) {
                const auto node = neighbor.first;
                const auto distSq = neighbor.second;

                // if line of sight to neighbor is occluded, try another neighbor
                if (isOccluded(vehicle, node->getPosition())) { continue; }

                // a good target has been found, stop searching
                nearestFoodRef = static_cast<Circle*>(node->getData());
                distanceSquared = distSq;
                break;
            }
        }

        // carry out vehicle actions
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
        vehicle.update(mBarriers);

        // update ecosystem tick count
        ++mTickCount;
        assert(mTickCount != std::numeric_limits<Tick>::max());
    }
}

void Ecosystem::mouseDown(const vec2& mousePos) {
    for (auto& barrier : mBarriers) {
        barrier.mouseDown(mousePos);
    }
}

void Ecosystem::mouseUp(const vec2& mousePos) {
    for (auto& barrier : mBarriers) {
        barrier.mouseUp(mousePos);
    }
}

void Ecosystem::mouseDrag(const vec2& mousePos) {
    for (auto& barrier : mBarriers) {
        barrier.mouseDrag(mousePos);
    }
}

void Ecosystem::mouseMove(const vec2& mousePos) {
    for (auto& barrier : mBarriers) {
        barrier.mouseMove(mousePos);
    }
}

void Ecosystem::keyDown(KeyEvent event) {
    if (event.getChar() == cinder::app::KeyEvent::KEY_SPACE) {
        mBarriers.push_back(Barrier{0});
    }
}

void Ecosystem::addBarrier() {
    int randX = randInt(10, 1890);
    int randY = randInt(10, 1050);
    mBarriers.push_back(Barrier{0, vec2{randX, randY}, vec2{randX + 50, randY}});
}

void Ecosystem::removeBarrier() {
    if (mBarriers.empty()) { return; }
    mBarriers.pop_back();
}

bool Ecosystem::isOccluded(const Vehicle& v, const vec2& target) {
    return ranges::any_of(mBarriers,
          [&v, &target] (const Barrier& b) {
              return b.hasCrossed(v.getPosition(), target);
          });

}

bool Ecosystem::isFocused() const {
    return ranges::any_of(mBarriers, [] (const Barrier& b) { return b.isFocused(); });
}

void Ecosystem::draw() const {
    for (const auto& corpse : mCorpses) { corpse.draw(); }
    for (const auto& food : mFood) { food.draw(); }
    for (const auto& vehicle : mVehicles) { vehicle.draw(); }
    for (const auto& barrier : mBarriers) { barrier.draw(); }
}

}

#endif
