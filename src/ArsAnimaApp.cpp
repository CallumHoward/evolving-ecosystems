// ArriveApp.cpp
// Callum Howard, 2017

#include <cmath>                    // round
#include <functional>               // bind, placeholder
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
//#include "cinder/params/Params.h"

#include "chGlobals.hpp"
#include "UserInterface.hpp"
#include "CommsManager.hpp"
#include "Background.hpp"
#include "Ecosystem.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class ArsAnimaApp : public App {
public:

    struct WindowData {
        bool isPrimary = true;
        bool isFlipped = false;
        bool renderUI = true;
        vec2 viewOffset = vec2{};
    };

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
    ch::CommsManager mCommsManager;
    ch::UserInterface mUI;

    std::unordered_map<int, int> mMidiSeq;
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

    // set up primary control window
    getWindow()->setUserData(new WindowData);
    WindowData *dataPrimary = getWindow()->getUserData<WindowData>();
    dataPrimary->isPrimary = false;
    dataPrimary->renderUI = false;

    // set up secondary display window
    const auto displays = Display::getDisplays();
    if (displays.size() > 1) {

        const auto windowFormat = Window::Format()
                .display(*displays.cbegin())
                .size(1920, 1080)
                //.fullScreen()
                .resizable(false);

        app::WindowRef newWindow = createWindow(windowFormat);
        newWindow->setUserData(new WindowData);

        WindowData *dataSecondary = newWindow->getUserData<WindowData>();
        dataSecondary->isFlipped = ch::gFlippedDisplay;  // change in chGlobals
        dataSecondary->isPrimary = true;
        dataSecondary->renderUI = true;
    }

    // set up virtual world
    mEcosystem.setup();
    mBackground.setup(getWindowWidth(), getWindowHeight());

    // set up comms
    //auto puffFunc = std::bind(&ch::Ecosystem::puffVehicles, &mEcosystem,
    //        std::placeholders::_1);

    //mCommsManager.addListener("/midi/midi_fighter_twister/0/8/control_change",
    //        puffFunc);

    mCommsManager.setup("/midi/midi_fighter_twister/0/8/control_change",
            [this] (int index, int value) {
                mEcosystem.puffVehicles(1);
                //auto puffFunc = std::bind(&ch::Ecosystem::puffVehicles,
                //        &mEcosystem, std::placeholders::_1);
                //puffFunc(1);
            });

    // set up user interface
    mUI = ch::UserInterface{std::bind(&ch::Ecosystem::getMode, &mEcosystem)};
    const auto windowWidth = static_cast<float>(getWindowWidth());
    const auto windowHeight = static_cast<float>(getWindowHeight());
    const auto buttonWidth = 120;
    const auto buttonHeight = 120;
    const auto buttonSpacing = 20;
    const auto containHeight = buttonHeight + buttonSpacing;
    const auto containWidth = windowWidth - buttonSpacing;
    const auto numRectangles = 6;

    auto rectangles = std::vector<Rectf>{};
    for (auto i = 0; i < numRectangles; ++i) {
        rectangles.emplace_back(
                containWidth - buttonWidth, i * containHeight + buttonSpacing,
                containWidth, (i * containHeight + buttonSpacing) + buttonHeight);
    }

    gl::TextureRef buttonNav =
            gl::Texture::create(loadImage(loadAsset("button_nav.png")));
    gl::TextureRef buttonBarrier =
            gl::Texture::create(loadImage(loadAsset("button_barrier.png")));
    gl::TextureRef buttonRemove =
            gl::Texture::create(loadImage(loadAsset("button_remove.png")));
    gl::TextureRef buttonFood =
            gl::Texture::create(loadImage(loadAsset("button_food.png")));
    gl::TextureRef buttonHome =
            gl::Texture::create(loadImage(loadAsset("button_home.png")));
    gl::TextureRef buttonInfo =
            gl::Texture::create(loadImage(loadAsset("button_sonify.png")));

    mUI.addPanel(Rectf{windowWidth - buttonWidth - 2 * buttonSpacing, 0,
            windowWidth, windowHeight});
    mUI.addButton(rectangles.at(0), Color{0.1f, 0.2f, 0.5f}, buttonNav,
            std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::PAN_VIEW));
    mUI.addButton(rectangles.at(1), Color{0.5f, 0.5f, 0.0f}, buttonBarrier,
            std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::ADD_BARRIER));
    mUI.addButton(rectangles.at(2), Color{0.5f, 0.0f, 0.0f}, buttonRemove,
            std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::REMOVE_BARRIER));
    mUI.addButton(rectangles.at(3), Color{0.0f, 0.5f, 0.7f}, buttonFood,
            std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::ADD_FOOD));
    mUI.addButton(rectangles.at(4), Color{0.1f, 0.2f, 0.5f}, buttonHome,
            std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::GO_HOME));
    mUI.addButton(rectangles.at(5), Color{0.0f, 0.5f, 0.2f}, buttonInfo,
            std::bind(&ch::Ecosystem::setMode, &mEcosystem, ch::SONIFY));

    // set up debug gui
    //const auto windowCaption = "Parameters";
    //mParams = params::InterfaceGl{windowCaption, ivec2{100, 200}};
    //mParams.addParam("Framerate", &mFramerate, true);
    //mParams.addParam("Fittest Lifetime", &mFittestLifetime, true);
    //mParams.addParam("Vehicle Color", &ch::sGreen, false);
    //mParams.addParam("Vehicle Bright Color", &ch::sBright, false);

}

void ArsAnimaApp::prepareSettings(ArsAnimaApp::Settings *settings) {
    //const auto displays = Display::getDisplays();
    //settings->setDisplay(displays.at(1));
    settings->setWindowSize(1920, 1080);
    settings->setResizable(false);
    settings->setFullScreen();
    //settings->setHighDensityDisplayEnabled();
    //settings->setMultiTouchEnabled(true);
}

void ArsAnimaApp::mouseDown(MouseEvent event) {
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };
    const vec2 pos = event.getPos();

    mUI.mouseDown(pos);
    if (mUI.isFocused(pos)) { return; }

    mEcosystem.mouseDown(pos + mOffset);
    if (mEcosystem.isFocused()) { return; }

    if (mEcosystem.getMode() == ch::PAN_VIEW) {
        mLastPos = pos;
    }
}

void ArsAnimaApp::mouseUp(MouseEvent event) {
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };
    const vec2 pos = event.getPos();
    mEcosystem.mouseUp(pos + mOffset);
    //mEcosystem.mouseUp(pos);
    mUI.mouseUp(pos);
}

void ArsAnimaApp::mouseDrag(MouseEvent event) {
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };

    const vec2 pos = event.getPos();

    // do nothing if draging in UI
    if (mUI.isFocused(pos)) { return; }

    mDebugPoint = pos + mOffset;

    mEcosystem.mouseDrag(pos + mOffset);
    //mEcosystem.mouseDrag(pos);
    if (mEcosystem.isFocused()) { return; }

    if (mEcosystem.getMode() == ch::PAN_VIEW) {
        mOffset += (mLastPos - pos) / mZoom;
        mLastPos = pos;
    }
}

void ArsAnimaApp::mouseWheel(MouseEvent event) {
    //zoomChange(event.getWheelIncrement());
}

void ArsAnimaApp::mouseMove(MouseEvent event) {
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };
    const vec2 pos = event.getPos();

    mEcosystem.mouseMove(pos);
    if (mEcosystem.isFocused()) { return; }

    mUI.mouseMove(pos);
    if (mUI.isFocused(pos)) { return; }

    mCursor = event.getPos();
}

void ArsAnimaApp::touchesBegan(TouchEvent event) {
    if (not ch::gTouchEnabled) { return; }
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };
    if (event.getTouches().size() == 2) {
        const vec2 pos0 = event.getTouches().at(0).getPos();
        const vec2 pos1 = event.getTouches().at(1).getPos();
        const vec2 pos = (pos0 + pos1) / 2.0f;
        mLastPos = pos;

    } else {
        const vec2 pos = event.getTouches().at(0).getPos();
        mUI.mouseDown(pos);
        if (mUI.isFocused(pos)) { return; }

        mEcosystem.mouseDown(pos + mOffset);
        if (mEcosystem.isFocused()) { return; }
    }
}

void ArsAnimaApp::touchesMoved(TouchEvent event) {
    if (not ch::gTouchEnabled) { return; }
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };

    const vec2 pos = event.getTouches().at(0).getPos();

    // do nothing if draging in UI
    if (mUI.isFocused(pos)) { return; }

    if (event.getTouches().size() == 1) {
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
    if (not ch::gTouchEnabled) { return; }
    if (not getWindow()->getUserData<WindowData>()->isPrimary) { return; };
    if (event.getTouches().size() == 1) {
        const vec2 pos = event.getTouches().at(0).getPos();
        mEcosystem.mouseUp(pos + mOffset);
        //mEcosystem.mouseUp(pos);
        mUI.mouseUp(pos);
    }
}

void ArsAnimaApp::keyDown(KeyEvent event) {
    //mEcosystem.keyDown(event);
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE: quit(); break;
        case KeyEvent::KEY_SPACE: mEcosystem.setMode(ch::PAN_VIEW); break;
        case KeyEvent::KEY_f: setFullScreen(not isFullScreen()); break;
    }
}

void ArsAnimaApp::update() {
    mEcosystem.update();
    mUI.update();

    if (mEcosystem.getMode() == ch::GO_HOME) {
        mOffset = vec2{};
        mEcosystem.setMode(ch::PAN_VIEW);

    } else if (mEcosystem.getMode() == ch::SONIFY) {
        mCommsManager.generateEvent();
        mEcosystem.setMode(ch::PAN_VIEW);
    }

    mFittestLifetime = static_cast<int>(mEcosystem.getFittestLifetime());
}

void ArsAnimaApp::draw() {
    getWindow()->getRenderer()->makeCurrentContext();

    hideCursor();
    gl::clear(Color{0.0f, 0.05f, 0.1f});

    gl::ScopedModelMatrix displayModelMatrix;
    if (getWindow()->getUserData<WindowData>()->isPrimary) {

        if (getWindow()->getUserData<WindowData>()->isFlipped) {
            // flip screen to hide cables
            gl::rotate(M_PI);
            gl::translate(-vec2{getWindowWidth(), getWindowHeight()});
        }

        mBackground.update(mOffset);

    } else {
        mBackground.update(getWindow()->getUserData<WindowData>()->viewOffset);
    }

    gl::color(0.2f, 0.2f, 0.5f, 0.5f);
    mBackground.draw();

    // draw cursor (don't translate)
    //gl::color(0.f, 0.f, 0.5f, 0.2f);
    //gl::drawSolidCircle(mCursor, 100.0f);

    {
        gl::ScopedModelMatrix worldModelMatrix;

        // move origin to the center of the window for zooming
        gl::translate(+getWindowCenter());
        //gl::scale(mZoom, mZoom);
        gl::translate(-getWindowCenter());

        // apply translational offset
        if (getWindow()->getUserData<WindowData>()->isPrimary) {
            getWindow()->getUserData<WindowData>()->viewOffset = mOffset;
        }

        const auto offset = getWindow()->getUserData<WindowData>()->viewOffset;
        gl::translate(-offset);

        mEcosystem.draw(getWindow()->getUserData<WindowData>()->viewOffset,
            getWindow()->getUserData<WindowData>()->isPrimary);

        //gl::drawSolidCircle(mDebugPoint, 10.0f);
        //gl::color(Color::white());
        //gl::drawSolidCircle(vec2{200, 200}, 10.0f);
    }

    if (getWindow()->getUserData<WindowData>()->renderUI) {
        mUI.draw();  // UI not affected by world transforms
    }

    mFramerate = std::round(getAverageFps());
    //CI_LOG_I(mFramerate);
    //mParams.draw();
}

CINDER_APP(ArsAnimaApp, RendererGl, &ArsAnimaApp::prepareSettings)
