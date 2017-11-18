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

    UIButton(const Rectf& rectf, const Color& c,
            gl::TextureRef icon, const std::function<void()>& callback);
    void update();
    void draw() const;
    void mouseMove(const vec2& pos);
    void mouseDown(const vec2& pos);
    void mouseUp(const vec2& pos);

    void setColor(Color c) { mColor = c; }
    void reset() { if (mState == DOWN) { mState = UP; } }
    bool isFocused(const vec2& pos) const;

private:
    State mState;
    const Rectf mRectf;
    Color mColor = Color{0.6f, 0.6f, 0.1f};
    gl::TextureRef mTex;
    std::function<void()> mCallback;
};


UIButton::UIButton(const Rectf& rectf, const Color& c,
        gl::TextureRef icon, const std::function<void()>& callback) :
        mRectf{rectf}, mColor{c}, mTex{icon}, mState{UP}, mCallback{callback} {}

void UIButton::update() {

}

void UIButton::draw() const {

    switch (mState) {
        case UP:
            gl::color(Color{0.8f, 0.8f, 0.8f});
            gl::drawStrokedRect(mRectf, 3.0f);
            gl::draw(mTex, mRectf);
            break;
        case HOVER:
            gl::color(mColor);
            gl::drawStrokedRect(mRectf, 3.0f);
            gl::draw(mTex, mRectf);
            break;
        case DOWN:
            gl::color(mColor);
            gl::drawSolidRect(mRectf);
            gl::color(Color::black());
            gl::draw(mTex, mRectf);
            break;
    }

}

void UIButton::mouseMove(const vec2& pos) {
    if (mRectf.contains(pos) and mState != DOWN) { mState = HOVER; }
    if (not mRectf.contains(pos) and mState != DOWN) { mState = UP; }
}

void UIButton::mouseDown(const vec2& pos) {
    if (mRectf.contains(pos) and mState != DOWN) {
        mCallback();
        mState = DOWN;
    } else if (mRectf.contains(pos) and mState == DOWN) {
        mState = UP;
        // TODO set to default mode
    }
}

void UIButton::mouseUp(const vec2& pos) {
    //if (mState == DOWN) { mState = UP; }
}

bool UIButton::isFocused(const vec2& pos) const {
    return mRectf.contains(pos);
}


} // namespace ch

#endif
