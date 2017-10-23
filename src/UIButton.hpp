// UIButton.hpp
// Callum Howard, 2017

#ifndef UIBUTTON_HPP
#define UIBUTTON_HPP

#include "cinder/gl/gl.h"   // vec2

namespace ch {

using namespace ci;

class UIButton {

public:

    enum State {
        HOVER,
        DOWN,
        UP
    };

    UIButton(const Rectf& rectf, const std::function<void()>& callback);
    void update();
    void draw() const;
    void mouseMove(const vec2& pos);
    void mouseDown(const vec2& pos);
    void mouseUp(const vec2& pos);
    bool isFocused() const;

private:
    State mState;
    const Rectf mRectf;
    std::function<void()> mCallback;
};


UIButton::UIButton(const Rectf& rectf, const std::function<void()>& callback) :
        mRectf{rectf}, mState{UP}, mCallback{callback} {}

void UIButton::update() {

}

void UIButton::draw() const {

    switch (mState) {
        case HOVER: gl::color(ColorA{0.8f, 0.8f, 0.8f, 0.8f}); break;
        case DOWN: gl::color(ColorA{0.5f, 0.5f, 0.5f, 0.8f}); break;
        case UP: gl::color(ColorA{0.6f, 0.6f, 0.1f, 0.8}); break;
    }

    gl::drawSolidRect(mRectf);
}

void UIButton::mouseMove(const vec2& pos) {
    mState = mRectf.contains(pos) ? HOVER : UP;
}

void UIButton::mouseDown(const vec2& pos) {
    if (mRectf.contains(pos) and mState != DOWN) {
        mCallback();
        mState = DOWN;
    }
}

void UIButton::mouseUp(const vec2& pos) {
    if (mState == DOWN) { mState = UP; }
}

bool UIButton::isFocused() const { return mState == HOVER or mState == DOWN; }


} // namespace ch

#endif
