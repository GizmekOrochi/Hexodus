#include "../src/include/TUIRenderer.hpp"
#include <unistd.h>
int main() {
    try {
        TUI::TUIRenderer renderer;
        renderer.activate();renderer.drawBuffer();
        char c; if (::read(STDIN_FILENO,&c,1)!=1) return 2;
        renderer.drawBuffer(); // Resize injected by the parent.
        // Destructor restores the terminal even without an explicit restore().
    } catch (...) { return 1; }
}
