#ifndef TUIGRAPHICMANAGER_HPP
#define TUIGRAPHICMANAGER_HPP

#include <sys/ioctl.h>  // terminal size
#include <signal.h>     // signals (later)
#include <iostream>

namespace tui {

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
enum TUI16Color {
    black = 0,
    red = 1,
    green = 2,
    yellow = 3,
    blue = 4,
    magenta = 5,
    cyan = 6,
    white = 7,
};

namespace ActionTable {
    //Moving
    constexpr char* string_CursorHome{"\x1b[H"};
    constexpr char* string_MoveCursor{"\x1b[%d;%dH"};
    //Clearing
    constexpr char* string_Clear{"\x1b[2J"};
    constexpr char* string_ClearCusorToRight{"\x1b[0K"};
    constexpr char* string_ClearCusorToLeft{"\x1b[1K"};
    constexpr char* string_ClearCusorLine{"\x1b[2K"};
    //Cusor things
    constexpr char* string_HideCursor{"\x1b[?25l"};
    constexpr char* string_ShowCursor{"\x1b[?25h"};
    //Coloring
    constexpr char* string_ResetColors{"\x1b[0m"};
    constexpr char* string_16ColorFGMode{"ESC[3%dm"};
    constexpr char* string_16ColorBGMode{"ESC[9%dm"};
};

class TUIGraphicManager {
private:
    int TUIheight_{};
    int TUIWidth_{};

    void TUIAction(const char* string_action);

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

    void TUIDisplayChar(const char chr);
    void TUImoveCursor(int height, int width);

};

} //namespace

#endif // TUIGRAPHICMANAGER_HPP
