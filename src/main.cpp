#include <string>
#include "include/TUIGraphicManager.hpp"
#include <iostream>

int main() {
    tui::TUIGraphicManager view{};
    std::string helloworld{"Hello, world!"};
    view.TUIClear();
    for(int i{}; i < helloworld.size(); i++){
        view.TUImoveCursor(1 , int{view.getWidth()} - 12);
        view.TUIDisplayChar(helloworld[i]);
    }
    return 0;
}