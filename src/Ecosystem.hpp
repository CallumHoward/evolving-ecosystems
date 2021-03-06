// Ecosystem.hpp
// Callum Howard, 2017

#ifndef ECOSYSTEM_HPP
#define ECOSYSTEM_HPP

#include <cassert>
#include <vector>
#include <limits>                       // numeric_limits
#include <algorithm>                    // generate_n, sort, any_of, find_if, min_element, remove_if
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"             // MouseEvent, getWindowWidth, getWindowHeight
#include "sp/Grid.h"
#include "sp/KdTree.h"
#include "chUtils.hpp"                  // makeRandPoint, distance
#include "chGlobals.hpp"                // Tick
#include "Circle.hpp"
#include "Vehicle.hpp"
#include "Barrier.hpp"

namespace ch {

using namespace ci;

class Ecosystem {
public:
    void setup();
    void update();
    void draw(const vec2& offset = vec2{}, bool isPrimaryWindow = true) const;
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
    Tick getFittestLifetime() const { return mFittestLifetime; }
    void puffVehicles(int midiChannel);

    Color mVehicleColor = Color{0.1f, 0.4f, 0.1f};

private:
    void updateVehicles();
    bool isOccluded(const Vehicle& v, const vec2& target);
    vec2 chooseSpawn() const;

    Mode mMode = PAN_VIEW;
    Tick mTickCount = 0;
    Tick mFittestLifetime = 0;

    int mNumFood = 60;
    int mMaxNumFood = 60;
    int mNumVehicles = 50;
    int mMaxFoodSpawns = 10;

    std::vector<Circle> mFood;
    std::vector<Vehicle> mVehicles;
    std::vector<Barrier> mBarriers;
    boost::circular_buffer<Circle> mCorpses;
    boost::circular_buffer<vec2> mFoodSpawns;

    //using SpatialStruct = sp::Grid2<Particle*>;
    using SpatialStruct = sp::KdTree2<Particle*>;
    SpatialStruct mParticleSpatialStruct;

    gl::BatchRef mBatchPrimary;
    gl::BatchRef mBatchSecondary;
    gl::VboMeshRef mMesh;
    gl::GlslProgRef mShader;

    gl::FboRef mFoodSpawnsFbo;
    gl::FboRef mFoodSpawnsFboSecondary;
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

    mParticleSpatialStruct = SpatialStruct{};

    // create a default shader with color and texture support
    mShader = gl::context()->getStockShader(gl::ShaderDef().color());
    // create ball mesh ( much faster than using gl::drawSolidCircle() )
    mMesh = Vehicle::createMesh();
    // combine mesh and shader into batch for much better performance
    mBatchPrimary = gl::Batch::create(mMesh, mShader);
    mBatchSecondary = gl::Batch::create(mMesh, mShader);

    mFoodSpawnsFbo = gl::Fbo::create(getWindowWidth(), getWindowHeight());
    mFoodSpawnsFboSecondary = gl::Fbo::create(getWindowWidth(), getWindowHeight());
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
        const auto found = min_element(mFood.begin(), mFood.end(),
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

    const auto to_erase = remove_if(mBarriers.begin(), mBarriers.end(),
            [](const auto& barrier) { return not barrier.isActive(); });
    if (to_erase != std::end(mBarriers)) { mBarriers.erase(to_erase); }

    // update ecosystem tick count
    ++mTickCount;
    assert(mTickCount != std::numeric_limits<Tick>::max());
}

void Ecosystem::updateVehicles() {

    Vehicle* reproReady = nullptr;

    for (auto& vehicle : mVehicles) {
        if (vehicle.readyToReproduce() and (reproReady == nullptr or
                vehicle.getBirthTick() < reproReady->getBirthTick())) {
            reproReady = &vehicle;

        } else if (vehicle.isDead()) {
            // check how long it survived and if it broke the record
            const auto lifetime = mTickCount - vehicle.getBirthTick();
            if (lifetime > mFittestLifetime) { mFittestLifetime = lifetime; }

            // place a corpse at its last position
            if (mTickCount - vehicle.getBirthTick() > 300 or vehicle.getIsChild()) {
                mCorpses.push_back(
                        Circle{mTickCount, 5.0f, vehicle.getPosition(), Circle::CORPSE});
            }

            // spawn a new vehicle in its place
            if (reproReady != nullptr) {
                vehicle = Vehicle{mTickCount, reproReady->getPosition(), mVehicleColor};
                vehicle.setIsChild();  // they will have corpse
                reproReady->setIsChild();

                // inherit color
                vehicle.setColor(reproReady->getColor());

                // randomly mutate color
                if (randFloat(0, 1) < 0.4f and
                        mTickCount - reproReady->getBirthTick() > 250) {
                    vehicle.setColor(vehicle.getColor() + randVec3() * vec3{0.7, 0.8, 0.5});
                }

                // split energy evenly between parent and child (mitosis)
                vehicle.setEnergy(reproReady->getEnergy() * 0.75f);
                reproReady->setEnergy(reproReady->getEnergy() * 0.75f);

                reproReady = nullptr;  // no longer ready to reproduce

            } else {  // make new child at initial spawn area
                vehicle = Vehicle{mTickCount, makeRandPoint(), mVehicleColor};
            }

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
            sort(neighbors.begin(), neighbors.end(),
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
                if (not mFoodSpawns.empty() and randBool()) {
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

        if (distance(vehicle.getPosition(), nearestFoodRef->getPosition()) < 400.0f) {
            vehicle.arrive(nearestFoodRef->getPosition());
        } else {
            vehicle.arrive(vehicle.getPosition() +
                    400.0f * randVec2());
        }
        vehicle.update(mBarriers);
    }
}

void Ecosystem::puffVehicles(int midiChannel) {
    for (auto& vehicle : mVehicles) { vehicle.puff(midiChannel); }
}

void Ecosystem::mouseDown(const vec2& mousePos) {
    switch (mMode) {
    case ADD_BARRIER:
        //TODO check to connect other barriers
        for (auto& barrier : mBarriers) { barrier.mouseDown(mousePos); }

        if (not any_of(mBarriers.cbegin(), mBarriers.cend(),
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
            const auto found = min_element(mFood.begin(), mFood.end(),
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

void Ecosystem::keyDown(KeyEvent event) { }

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
    return any_of(mBarriers.cbegin(), mBarriers.cend(),
          [&v, &target] (const Barrier& b) {
              return b.hasCrossed(v.getPosition(), target);
          });
}

vec2 Ecosystem::chooseSpawn() const {
    return mFoodSpawns.at(biasRandInt(0, static_cast<int>(mFoodSpawns.size()), 1.5f));
}

bool Ecosystem::isFocused() const {
    return any_of(mBarriers.cbegin(), mBarriers.cend(),
            [] (const Barrier& b) { return b.isFocused(); });
}

void Ecosystem::draw(const vec2& offset, bool isPrimaryWindow) const {

    if (isPrimaryWindow) {
        gl::ScopedModelMatrix modelMatrix;

        if (gFlippedDisplay) {
            gl::translate(vec2{getWindowWidth(), getWindowHeight()});
            gl::rotate(M_PI);
            gl::translate(-2.0f * offset);
        }

        gl::ScopedFramebuffer fbo{mFoodSpawnsFbo};
        gl::clear(ColorA{0, 0, 0, 0});

        // draw food spawn areas
        const auto drawOffset = 500.0f * vec2{1.0f, 1.0f};
        gl::color(Color::white());

        for (const auto& spawn : mFoodSpawns) {
            gl::draw(gGlow, Rectf{spawn - drawOffset, spawn + drawOffset});
        }

    }

    if (isPrimaryWindow) {
        gl::ScopedModelMatrix modelMatrix;
        gl::translate(offset);

        if (gFlippedDisplay) {
            gl::rotate(M_PI);
            gl::translate(-vec2{getWindowWidth(), getWindowHeight()});
        }

        gl::ScopedFramebuffer fbo{mFoodSpawnsFboSecondary};
        gl::clear(ColorA{0, 0, 0, 0});

        // draw food spawn areas
        const auto drawOffset = 500.0f * vec2{1.0f, 1.0f};
        gl::color(Color::white());

        for (const auto& spawn : mFoodSpawns) {
            gl::draw(gGlow, Rectf{spawn - drawOffset, spawn + drawOffset});
        }
    }

    const auto viewport = Rectf{offset, vec2{getWindowWidth(), getWindowHeight()} + offset};
    gl::color(ColorA{0.1f, 0.2f, 0.5f, 0.3f});

    if (isPrimaryWindow) {
        gl::draw(mFoodSpawnsFbo->getColorTexture(), viewport);
    } else {
        gl::draw(mFoodSpawnsFboSecondary->getColorTexture(), viewport);
    }



    for (const auto& corpse : mCorpses) { corpse.draw(); }
    for (const auto& food : mFood) { food.draw(); }

    {
        gl::ScopedGlslProg shader(mShader);

        if (isPrimaryWindow) {
            for (const auto& vehicle : mVehicles) { vehicle.draw(mBatchPrimary); }
        } else {
            for (const auto& vehicle : mVehicles) { vehicle.draw(mBatchSecondary); }
        }
    }

    for (const auto& barrier : mBarriers) { barrier.draw(); }
}


} // namespace ch

#endif
