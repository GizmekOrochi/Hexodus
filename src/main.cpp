#include <string>
#include "include/TUIGraphicManager.hpp"
#include <iostream>



int main() {
    TUI::TUIGraphicManager view{};
    TUI::Colorator colorator{};
    std::string helloworld{"Hello, world!"};
    view.TUIHideCursor();
    view.TUIClear();
    view.TUICursorHome();
    view.TUISetColor(TUI::RGBPanel{234, 0, 100}, TUI::RGBPanel{0, 241, 24});
    for(int i{}; i < static_cast<int>(helloworld.size()); i++){
        view.TUImoveCursor(1 , view.getWidth() - 12);
        view.TUIDisplayChar(helloworld[i]);
    }
    //view.TUIResetColors();
    view.TUIShowCursor();
    return 0;
}