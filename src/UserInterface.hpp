// UserInterface.hpp
// Callum Howard, 2017

#ifndef USERINTERFACE_HPP
#define USERINTERFACE_HPP

#include <algorithm>
#include <vector>
#include "cinder/gl/gl.h"                   // vec2, Rectf
#include "chGlobals.hpp"                    // mode
#include "UIButton.hpp"


namespace ch {

using namespace ci;

class UserInterface {
public:
    UserInterface() {};
    UserInterface(const std::function<ch::Mode()>& getMode);
    void update();
    void draw() const;
    void mouseMove(const vec2& pos);
    void mouseDown(const vec2& pos);
    void mouseUp(const vec2& pos);
    void addButton(const Rectf& r, const Color& c,
            gl::TextureRef icon, const std::function<void()>& callback);
    void addPanel(const Rectf& r);
    bool isFocused(const vec2& pos);

private:
    std::vector<UIButton> mButtons;
    Rectf mPanel;
    std::function<ch::Mode()> mGetMode;
};

UserInterface::UserInterface(const std::function<ch::Mode()>& getMode) :
        mGetMode{getMode} {}

void UserInterface::addButton(const Rectf& r, const Color& c,
        gl::TextureRef icon, const std::function<void()>& callback) {
    mButtons.emplace_back(r, c, icon, callback);
}

void UserInterface::addPanel(const Rectf& r) {
    mPanel = r;
}

void UserInterface::update() {
    for (auto& button : mButtons) { button.update(); }
    if (mGetMode() == ch::PAN_VIEW) {
        for (auto& button : mButtons) { button.reset(); }
        mButtons.begin()->set();
    }
}

void UserInterface::draw() const {
    gl::color(Color::black());
    cinder::gl::drawSolidRect(mPanel);

    for (const auto& button : mButtons) { button.draw(); }
}

void UserInterface::mouseMove(const vec2& pos) {
    for (auto& button : mButtons) { button.mouseMove(pos); }
}

void UserInterface::mouseDown(const vec2& pos) {
    if (any_of(mButtons.cbegin(), mButtons.cend(),
            [&pos] (const UIButton& button) { return button.isFocused(pos); })) {
        for (auto& button : mButtons) {
            if (not button.isFocused(pos)) { button.reset(); }
        }
    }

    for (auto& button : mButtons) { button.mouseDown(pos); }
}

void UserInterface::mouseUp(const vec2& pos) {
    for (auto& button : mButtons) { button.mouseUp(pos); }
}

bool UserInterface::isFocused(const vec2& pos) {
    return mPanel.contains(pos);
}


} // namespace ch

#endif
