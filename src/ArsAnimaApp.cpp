// ArriveApp.cpp
// Callum Howard, 2017

#include <cmath>                    // round
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
//#include "cinder/params/Params.h"

#include "chGlobals.hpp"
#include "UserInterface.hpp"
#include "Background.hpp"
#include "Ecosystem.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class ArsAnimaApp : public App {
public:
    void setup() override;
    static void prepareSettings(ArsAnimaApp::Settings *settings);
    void mouseDown(MouseEvent event) override;
    void mouseUp(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseWheel(MouseEvent event) override;
    void mouseMove(MouseEvent event) override;
    void touchesBegan(TouchEvent event) override;
    void touchesMoved(TouchEvent event) override;
    void touchesEnded(TouchEvent event) override;
    void keyDown(KeyEvent event) override;
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
    int mFramerate;
    int mFittestLifetime;
    //params::InterfaceGl mParams;

    vec2 mDebugPoint;

    ch::Background mBackground;
    ch::Ecosystem mEcosystem;
    ch::UserInterface mUI;
};


void ArsAnimaApp::setup() {
    // viewport parameters setup
    mOffset = vec2{};
    mCursor = vec2{getWindowCenter()};
    mZoom = 1.0f;
    mZoomMax = 5.0f;
    mZoomMin = 1.0f;
    mZoomAmount = 0.2f;

    // global texture assets
    ch::gGlow = gl::Texture::create(loadImage(loadAsset("particle.png")));

    mEcosystem.setup();
    mUI = ch::UserInterface{};
    const auto windowWidth = static_cast<float>(getWindowWidth());
    const auto buttonWidth = 200;
    const auto buttonHeight = 200;
    const auto buttonSpacing = 20;
    const auto containHeight = buttonHeight + buttonSpacing;
    const auto numRectangles = 3;

    auto rectangles = std::vector<Rectf>{};
    for (auto i = 0; i < numRectangles; ++i) {
        rectangles.emplace_back(
                                windowWidth - buttonWidth, i * containHeight,
                                windowWidth, (i * containHeight) + buttonHeight);
    }

    mUI.addButton(rectangles.at(0),
                  std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::ADD_BARRIER));
    mUI.addButton(rectangles.at(1),
                  std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::REMOVE_BARRIER));
    mUI.addButton(rectangles.at(2),
                  std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::ADD_FOOD));

    mBackground.setup(getWindowWidth(), getWindowHeight());

    // debug gui setup
    //const auto windowCaption = "Parameters";
    //mParams = params::InterfaceGl{windowCaption, ivec2{100, 200}};
    //mParams.addParam("Framerate", &mFramerate, true);
    //mParams.addParam("Fittest Lifetime", &mFittestLifetime, true);
    //mParams.addParam("Vehicle Color", &ch::sGreen, false);
    //mParams.addParam("Vehicle Bright Color", &ch::sBright, false);

}

void ArsAnimaApp::prepareSettings(ArsAnimaApp::Settings *settings) {
    //settings->setWindowSize(1340, 800);
    settings->setFullScreen();
    //settings->setHighDensityDisplayEnabled();
}

void ArsAnimaApp::mouseDown(MouseEvent event) {
    const vec2 pos = event.getPos();

    mUI.mouseDown(pos);
    if (mUI.isFocused()) { return; }

    mEcosystem.mouseDown(pos + mOffset);
    if (mEcosystem.isFocused()) { return; }

    mLastPos = pos;
}

void ArsAnimaApp::mouseUp(MouseEvent event) {
    const vec2 pos = event.getPos();
    mEcosystem.mouseUp(pos + mOffset);
    //mEcosystem.mouseUp(pos);
    mUI.mouseUp(pos);
}

void ArsAnimaApp::mouseDrag(MouseEvent event) {
    // do nothing if draging in UI
    if (mUI.isFocused()) { return; }

    const vec2 pos = event.getPos();
    mDebugPoint = pos + mOffset;

    mEcosystem.mouseDrag(pos + mOffset);
    //mEcosystem.mouseDrag(pos);
    if (mEcosystem.isFocused()) { return; }

    mOffset += (mLastPos - pos) / mZoom;
    mLastPos = pos;
}

void ArsAnimaApp::mouseWheel(MouseEvent event) {
    zoomChange(event.getWheelIncrement());
}

void ArsAnimaApp::mouseMove(MouseEvent event) {
    const vec2 pos = event.getPos();

    mEcosystem.mouseMove(pos);
    if (mEcosystem.isFocused()) { return; }

    mUI.mouseMove(pos);
    if (mUI.isFocused()) { return; }

    mCursor = event.getPos();
}

void ArsAnimaApp::touchesBegan(TouchEvent event) {
    if (event.getTouches().size() == 2) {
        const vec2 pos0 = event.getTouches().at(0).getPos();
        const vec2 pos1 = event.getTouches().at(1).getPos();
        const vec2 pos = (pos0 + pos1) / 2.0f;
        mLastPos = pos;

    } else {
        const vec2 pos = event.getTouches().at(0).getPos();
        mUI.mouseDown(pos);
        if (mUI.isFocused()) { return; }

        mEcosystem.mouseDown(pos + mOffset);
        if (mEcosystem.isFocused()) { return; }
    }
}

void ArsAnimaApp::touchesMoved(TouchEvent event) {
    // do nothing if draging in UI
    if (mUI.isFocused()) { return; }

    if (event.getTouches().size() == 1) {
        const vec2 pos = event.getTouches().at(0).getPos();
        mDebugPoint = pos + mOffset;

        mEcosystem.mouseDrag(pos + mOffset);
        //mEcosystem.mouseDrag(pos);
        if (mEcosystem.isFocused()) { return; }

    } else if (event.getTouches().size() == 2) {

        const vec2 pos0 = event.getTouches().at(0).getPos();
        const vec2 pos1 = event.getTouches().at(1).getPos();
        const vec2 pos = (pos0 + pos1) / 2.0f;

        mOffset += (mLastPos - pos) / mZoom;
        mLastPos = pos;
    }
}

void ArsAnimaApp::touchesEnded(TouchEvent event) {
    if (event.getTouches().size() == 1) {
        const vec2 pos = event.getTouches().at(0).getPos();
        mEcosystem.mouseUp(pos + mOffset);
        //mEcosystem.mouseUp(pos);
        mUI.mouseUp(pos);
    }
}

void ArsAnimaApp::keyDown(KeyEvent event) {
    mEcosystem.keyDown(event);
}

void ArsAnimaApp::update() {
    mEcosystem.update();
    mUI.update();
    mBackground.update(mOffset);

    mFittestLifetime = static_cast<int>(mEcosystem.getFittestLifetime());
}

void ArsAnimaApp::draw() {
    //hideCursor();
    gl::clear(Color{0.0f, 0.05f, 0.1f});

    gl::color(0.2f, 0.2f, 0.5f, 0.5f);
    mBackground.draw();

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

        mEcosystem.draw(mOffset);
        //gl::drawSolidCircle(mDebugPoint, 10.0f);
    }

    mUI.draw();  // UI not affected by world transforms
    
    mFramerate = std::round(getAverageFps());
    std::cout << mFramerate << '\n';
    //mParams.draw();
}

CINDER_APP(ArsAnimaApp, RendererGl, &ArsAnimaApp::prepareSettings)
