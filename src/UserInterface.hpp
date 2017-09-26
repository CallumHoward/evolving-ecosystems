// UserInterface.hpp
// Callum Howard, 2017

#ifndef USERINTERFACE_HPP
#define USERINTERFACE_HPP

#include <range/v3/algorithm/any_of.hpp>    // any_of
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
    bool isFocused();

private:
    std::vector<UIButton> mButtons;
};

UserInterface::UserInterface() {
    mButtons.emplace_back(Rectf{100, 100, 200, 200});
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
    return ranges::any_of(mButtons,
            [] (const UIButton& button) { return button.isFocused(); });
}


} // namespace ch

#endif
