// Ecosystem.hpp
// Callum Howard, 2017

#ifndef ECOSYSTEM_HPP
#define ECOSYSTEM_HPP

#include <cassert>
#include <vector>
#include <limits>                       // numeric_limits
#include <algorithm>                    // generate_n, sort
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/min_element.hpp>
#include <range/v3/algorithm/remove_if.hpp>
#include <range/v3/algorithm/sort.hpp>
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
    void addBarrier(const vec2& pos);
    void removeBarrier();
    bool isFocused() const;
    void setMode(Mode m);
    Mode getMode() const { return mMode; }

    Color mVehicleColor = Color{0.1f, 0.4f, 0.1f};

private:
    void updateVehicles();
    bool isOccluded(const Vehicle& v, const vec2& target);
    vec2 chooseSpawn() const;

    Mode mMode = PAN_VIEW;
    Tick mTickCount = 0;
    int mNumFood = 30;
    int mMaxNumFood = 30;
    int mNumVehicles = 50;
    int mMaxFoodSpawns = 10;
    Tick mFittestLifetime = 0;
    std::vector<Circle> mFood;
    std::vector<Vehicle> mVehicles;
    std::vector<Barrier> mBarriers;
    boost::circular_buffer<Circle> mCorpses;
    boost::circular_buffer<vec2> mFoodSpawns;

    //using SpatialStruct = sp::Grid2<Particle*>;
    using SpatialStruct = sp::KdTree2<Particle*>;

    SpatialStruct mParticleSpatialStruct;
};


void Ecosystem::setup() {
    mCorpses = boost::circular_buffer<Circle>{30};
    mFoodSpawns = boost::circular_buffer<vec2>{10};
    mBarriers = std::vector<Barrier>{};

    std::generate_n(std::back_inserter(mFood), mNumFood,
            [this]{ return Circle{0, 3.0f, makeRandPoint()}; });

    std::generate_n(std::back_inserter(mVehicles), mNumVehicles,
            [this]{ return Vehicle{0, makeRandPoint(), mVehicleColor}; });

    std::generate_n(std::back_inserter(mFoodSpawns), mMaxFoodSpawns, makeRandPoint);

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

    // find and replace oldest food to keep circulation going
    if (randFloat(0.0f, 1.0f) < 0.016f) {
        const auto found = min_element(mFood,
                [] (const Circle& lhs, const Circle& rhs) {
                    return lhs.getBirthTick() < rhs.getBirthTick();
                });
        *found = Circle{mTickCount, 3.0f, addNoise(chooseSpawn(), 180.0f)};
    }

    updateVehicles();

    for (auto& barrier : mBarriers) {
        barrier.setMode(mMode);
        barrier.update();
    }

    const auto to_erase = remove_if(mBarriers,
            [](const auto& barrier) { return not barrier.isActive(); });
    if (to_erase != std::end(mBarriers)) { mBarriers.erase(to_erase); }

    // update ecosystem tick count
    ++mTickCount;
    assert(mTickCount != std::numeric_limits<Tick>::max());
}

void Ecosystem::updateVehicles() {

    for (auto& vehicle : mVehicles) {
        if (vehicle.isDead()) {
            // check how long it survived and if it broke the record
            const auto lifetime = mTickCount - vehicle.getBirthTick();
            if (lifetime > mFittestLifetime) { mFittestLifetime = lifetime; }

            // place a corpse at its last position
            if (mTickCount - vehicle.getBirthTick() > 500) {
                mCorpses.push_back(
                        Circle{mTickCount, 5.0f, vehicle.getPosition(), Circle::CORPSE});
            }

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
            auto neighbors = mParticleSpatialStruct.rangeSearch(
                    vehicle.getPosition(), vehicle.getSightDist());

            // order by smallest distance first
            ranges::sort(neighbors,
                    [] (const auto& lhs, const auto& rhs) {
                        return lhs.second < rhs.second;
                    });

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
                if (not mFoodSpawns.empty()) {
                    *nearestFoodRef = Circle{mTickCount, 3.0f, addNoise(chooseSpawn(), 180.0f)};
                } else {
                    *nearestFoodRef = Circle{mTickCount, 3.0f, makeRandPoint()};
                }
                break;
            case Circle::CORPSE:
                nearestFoodRef->setActive(false);
                break;
            }
        }

        vehicle.arrive(nearestFoodRef->getPosition());
        vehicle.update(mBarriers);
    }
}

void Ecosystem::mouseDown(const vec2& mousePos) {
    switch (mMode) {
    case ADD_BARRIER:
        //TODO check to connect other barriers
        for (auto& barrier : mBarriers) { barrier.mouseDown(mousePos); }

        if (not any_of(mBarriers,
                [] (const Barrier& b) { return b.isFocused(); })) {
            addBarrier(mousePos);
        }
        break;
    case REMOVE_BARRIER:
        for (auto& barrier : mBarriers) { barrier.mouseDown(mousePos); }
        break;
    case ADD_FOOD:
        {
            // add to target locations around which food spawns
            mFoodSpawns.push_back(mousePos);
            // find oldest food
            const auto found = min_element(mFood,
                    [] (const Circle& lhs, const Circle& rhs) {
                        return lhs.getBirthTick() < rhs.getBirthTick();
                    });
            // add food at mouse, replace oldest food
            *found = Circle{mTickCount, 3.0f, mousePos};
            break;
        }
    default:
        break;
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

void Ecosystem::addBarrier(const vec2& pos) {
    mBarriers.push_back(Barrier{0, vec2{pos.x - 100, pos.y}, vec2{pos.x + 100, pos.y}});
}

void Ecosystem::removeBarrier() {
    if (mBarriers.empty()) { return; }
    mBarriers.pop_back();
}

void Ecosystem::setMode(Mode m) {
    mMode = m;
}

bool Ecosystem::isOccluded(const Vehicle& v, const vec2& target) {
    return ranges::any_of(mBarriers,
          [&v, &target] (const Barrier& b) {
              return b.hasCrossed(v.getPosition(), target);
          });

}

vec2 Ecosystem::chooseSpawn() const {
    return mFoodSpawns.at(biasRandInt(0, static_cast<int>(mFoodSpawns.size()), 1.5f));
}

bool Ecosystem::isFocused() const {
    return ranges::any_of(mBarriers, [] (const Barrier& b) { return b.isFocused(); });
}

void Ecosystem::draw() const {
    // draw food spawn areas
    gl::color(ColorA{0.1f, 0.1f, 0.6f, 0.3f});
    const auto offset = vec2{5.0f, 5.0f};
    for (const auto& spawn : mFoodSpawns) {
        cinder::gl::drawSolidRect(Rectf{spawn - offset, spawn + offset});
    }

    for (const auto& corpse : mCorpses) { corpse.draw(); }
    for (const auto& food : mFood) { food.draw(); }
    for (const auto& vehicle : mVehicles) { vehicle.draw(); }
    for (const auto& barrier : mBarriers) { barrier.draw(); }
}


} // namespace ch

#endif
