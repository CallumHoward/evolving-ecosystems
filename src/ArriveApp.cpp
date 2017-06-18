#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "Ecosystem.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class ArriveApp : public App {
public:
    void setup() override;
    static void prepareSettings(ArriveApp::Settings *settings);
    void mouseDown(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseWheel(MouseEvent event) override;
    void mouseMove(MouseEvent event) override;
    void update() override;
    void draw() override;

private:
    void zoomChange(float amount) {
        mZoom = constrain(mZoom + amount, mZoomMin, mZoomMax);
    };

    // viewport parameters
    vec2 mCursor;
    vec2 mLastPos;
    vec2 mOffset;
    float mZoom;
    float mZoomMax;
    float mZoomMin;
    float mZoomAmount;

    // debug gui
    float mFramerate;
    params::InterfaceGl mParams;

    ch::Ecosystem mEcosystem;
};


void ArriveApp::setup() {
    // viewport parameters setup
    mOffset = vec2{};
    mCursor = vec2{getWindowCenter()};
    mZoom = 1.0f;
    mZoomMax = 5.0f;
    mZoomMin = 1.0f;
    mZoomAmount = 0.2f;

    // debug gui setup
    const auto windowCaption = "Parameters";
    mParams = params::InterfaceGl(windowCaption, ivec2{100, 200});
    mParams.addParam("framerate", &mFramerate, "");

    mEcosystem.setup();
}

void ArriveApp::prepareSettings(ArriveApp::Settings *settings) {
    //settings->setWindowSize(1340, 800);
    settings->setFullScreen();
    settings->setHighDensityDisplayEnabled();
}

void ArriveApp::mouseDown(MouseEvent event) { mLastPos = event.getPos(); }

void ArriveApp::mouseDrag(MouseEvent event) {
    const vec2 pos = event.getPos();
    mOffset += (mLastPos - pos) / mZoom;
    mLastPos = pos;
}

void ArriveApp::mouseWheel(MouseEvent event) {
    zoomChange(event.getWheelIncrement());
}

void ArriveApp::mouseMove(MouseEvent event) { mCursor = event.getPos(); }

void ArriveApp::update() { mEcosystem.update(); }

void ArriveApp::draw() {
    gl::clear(Color{0.0f, 0.05f, 0.1f});

    // draw cursor (don't translate)
    gl::color(0.f, 0.f, 0.5f, 0.2f);
    //gl::drawSolidCircle(mCursor, 100.0f);

    {
        gl::ScopedModelMatrix modelMatrix;

        // move origin to the center of the window for zooming
        gl::translate(+getWindowCenter());
        gl::scale(mZoom, mZoom);
        gl::translate(-getWindowCenter());

        // apply translational offset
        gl::translate(-mOffset);

        mEcosystem.draw();
    }

    mFramerate = getAverageFps();
    mParams.draw();
}

CINDER_APP(ArriveApp, RendererGl, &ArriveApp::prepareSettings)
