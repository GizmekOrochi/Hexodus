#ifndef TUIGRAPHICMANAGER_HPP
#define TUIGRAPHICMANAGER_HPP

#pragma once
#include <sys/ioctl.h>  // terminal size
#include <signal.h>     // signals (later)
#include <iostream>
#include <string_view>
#include <format>

#include "TUI-Coloring/Colorator.hpp"
#include "TUIStruct.hpp"

namespace TUI {

/*
Move cursor to Cursor       \x1b[H	            Move cursor to (1,1)
Move cursor to (x, y)	    \x1b[%d;%dH	        Move cursor to in x, y

Clear entire screen	        \x1b[2J	            Erase all cells ( NOT THE COLORS )
Clear to end of line	    \x1b[K	            Erase from cursor -> EOL for 0, SOL -> cursor for 1 and all the line for 2

Hide cursor	                \x1b[?25l	        Set Cursor invisible
Show cursor	                \x1b[?25h	        Set Cursor visible

Reset colors	            \x1b[0m	Reset       colors & styles

Color Mod 16 FG             "ESC[3Xm"           put the color to the forground with X bing between 0 to 7 see the 16 color enum
Color Mod 16 FG             "ESC[9Xm"           put the color to the background with X bing between 0 to 7 see the 16 color enum
*/

class TUIGraphicManager {
private:
    int TUIheight_{};
    int TUIWidth_{};
    Colorator colorator_{};

    void TUIAction(const std::string_view& string_action);

public:
    int getHeigth() { return TUIheight_; }
    int getWidth() { return TUIWidth_; }
    
    //Cursor moving
    TUIGraphicManager();

    void TUICursorHome();
    //Clearing
    void TUIClear();
    void TUIClearCusorToRight();
    void TUIClearCusorToLeft();
    void TUIClearCursorLine();
    //cursor thingd
    void TUIHideCursor();
    void TUIShowCursor();
    //Coloring
    void TUIResetColors();
    void TUISetColor(RGBPanel colorFront, RGBPanel colorBack);

    void TUIDisplayChar(const char chr);
    void TUImoveCursor(int height, int width);


};

} //namespace

#endif // TUIGRAPHICMANAGER_HPP
