#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "Vehicle.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class ArriveApp : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void mouseMove(MouseEvent event) override;
    void update() override;
    void draw() override;

    std::vector<Vehicle> mVehicles;
    vec2 mCursor;
};

void ArriveApp::setup() {
    mVehicles = vector<Vehicle>{};
    const int nVehicles = 10;
    for (int i = 0; i < nVehicles; ++i) {
        mVehicles.emplace_back(randFloat(getWindowWidth()), randFloat(getWindowHeight()));
    }
    mCursor = vec2{getWindowCenter()};
}

void ArriveApp::mouseDown(MouseEvent event) {}

void ArriveApp::mouseMove(MouseEvent event) { mCursor = event.getPos(); }

void ArriveApp::update() {}

void ArriveApp::draw() {
    gl::clear(Color{0, 0, 0});

    gl::color(0.f, 0.f, 0.5f, 0.5f);
    gl::drawSolidCircle(mCursor, 100.0f);

    gl::color(1.0f, 1.0f, 1.0f);
    for (auto& vehicle : mVehicles) {
        vehicle.arrive(mCursor);
        vehicle.update();
        vehicle.draw();
    }
}

CINDER_APP(ArriveApp, RendererGl)
