// UserInterface.hpp
// Callum Howard, 2017

#ifndef USERINTERFACE_HPP
#define USERINTERFACE_HPP

#include <algorithm>
#include <vector>
#include "cinder/gl/gl.h"                   // vec2, Rectf
#include "UIButton.hpp"


namespace ch {

using namespace ci;

class UserInterface {
public:
    UserInterface();
    void update();
    void draw() const;
    void mouseMove(const vec2& pos);
    void mouseDown(const vec2& pos);
    void mouseUp(const vec2& pos);
    void addButton(const Rectf& r, const std::function<void()>& callback);
    bool isFocused();

private:
    std::vector<UIButton> mButtons;
};

UserInterface::UserInterface() {}

void UserInterface::addButton(const Rectf& r, const std::function<void()>& callback) {
    mButtons.emplace_back(r, callback);
}

void UserInterface::update() {
    for (auto& button : mButtons) { button.update(); }
}

void UserInterface::draw() const {
    for (const auto& button : mButtons) { button.draw(); }
}

void UserInterface::mouseMove(const vec2& pos) {
    for (auto& button : mButtons) { button.mouseMove(pos); }
}

void UserInterface::mouseDown(const vec2& pos) {
    for (auto& button : mButtons) { button.mouseDown(pos); }
}

void UserInterface::mouseUp(const vec2& pos) {
    for (auto& button : mButtons) { button.mouseUp(pos); }
}

bool UserInterface::isFocused() {
    return any_of(mButtons.cbegin(), mButtons.cend(),
            [] (const UIButton& button) { return button.isFocused(); });
}


} // namespace ch

#endif
