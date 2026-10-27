#include "../tester.hpp"
#include "../../src/include/TUIRenderer.hpp"
#include <limits>
#include <optional>
#include <cstdlib>

namespace TUI {
// Narrow friend adapter: exercise pure encoding without opening a terminal.
struct RendererTestAccess {
    static auto encode(const std::array<Pixel,4>& p) {
        return TUIRenderer::encodeQuad(p[0],p[1],p[2],p[3]);
    }
    static auto glyph(unsigned m) { return TUIRenderer::glyphForMask(m); }
};
}
struct SceneTestAccess {
    static void render(Scene& s, std::vector<TUI::Pixel>& c, std::vector<float>& d) {
        s.getCamera().setPosition({0,0,0,1.0f});
        s.renderTriangles(c,d,16,16);
    }
};
using TUI::Pixel;
bool equal(TUI::RGBPanel a, Pixel b) { return a.R_==b.r && a.G_==b.g && a.B_==b.b; }
TEST(Encoding, BlueIsNotAlpha) {
    for (unsigned alpha : {0U, 127U, 255U}) {
        const Pixel p{12,34,56,static_cast<uint8_t>(alpha)};
        const auto cell=TUI::RendererTestAccess::encode({p,p,p,p});
        CHECK(equal(cell.background,p));
    }
}
TEST(Encoding, AllBinaryQuadrantsReconstructExactly) {
    const Pixel dark{0,0,0,255}, light{255,255,255,255};
    for (unsigned mask=0;mask<16;++mask) {
        std::array<Pixel,4> pixels;
        for(unsigned i=0;i<4;++i) pixels[i]=(mask&(1U<<i))?light:dark;
        auto cell=TUI::RendererTestAccess::encode(pixels);
        unsigned encoded=16;
        for(unsigned m=0;m<16;++m) if(cell.glyph==TUI::RendererTestAccess::glyph(m)) encoded=m;
        CHECK(encoded<16);
        for(unsigned i=0;i<4;++i) CHECK(equal((encoded&(1U<<i))?cell.foreground:cell.background,pixels[i]));
    }
}
TEST(Encoding, RepeatableMulticolorBlock) {
    const std::array<Pixel,4> p{{{255,0,0,255},{0,255,0,255},{0,0,255,255},{255,255,0,255}}};
    CHECK(TUI::RendererTestAccess::encode(p)==TUI::RendererTestAccess::encode(p));
}
struct Environment {
    std::optional<std::string> term, color;
    Environment(const char* t,const char* c) {
        if(auto p=std::getenv("TERM")) term=p;
        if(auto p=std::getenv("COLORTERM")) color=p;
        setenv("TERM",t,1); if(c) setenv("COLORTERM",c,1); else unsetenv("COLORTERM");
    }
    ~Environment(){ if(term)setenv("TERM",term->c_str(),1);else unsetenv("TERM"); if(color)setenv("COLORTERM",color->c_str(),1);else unsetenv("COLORTERM"); }
};
TEST(Color, BrightAnsiSequences) {
    Environment env("linux",nullptr);
    TUI::Colorator c;
    c.SetRGBColor({255,0,0},{0,0,255});
    CHECK_EQ(c.GetColorString(),std::string("\x1b[91m\x1b[104m"));
}
TEST(Color, NormalAnsiSequences) {
    Environment env("linux",nullptr);
    TUI::Colorator c; c.SetRGBColor({128,0,0},{0,128,0});
    CHECK_EQ(c.GetColorString(),std::string("\x1b[31m\x1b[42m"));
}
TEST(Color, TruecolorPreservesChannels) {
    Environment env("xterm-256color","truecolor");
    TUI::Colorator c; c.SetRGBColor({12,34,56},{78,90,123});
    CHECK_EQ(c.GetColorString(),std::string("\x1b[38;2;12;34;56m\x1b[48;2;78;90;123m"));
}
Geometry::Triangle triangle(float z,Pixel p) {
    return {{-10,-10,z,1.0f},{10,-10,z,1.0f},{0,10,z,1.0f},p,p,p};
}
struct Frame {
    std::vector<Pixel> color=std::vector<Pixel>(256,Pixel{0,0,0,255});
    std::vector<float> depth=std::vector<float>(256,std::numeric_limits<float>::infinity());
};
TEST(Depth, InvisibleDoesNotOcclude) {
    Scene s({0,0},{100,100});
    s.addTriangle(triangle(2,{255,0,0,0}));
    s.addTriangle(triangle(4,{0,255,0,255}));
    Frame f; SceneTestAccess::render(s,f.color,f.depth);
    CHECK_EQ(f.color[136].g,255); CHECK_NEAR(f.depth[136],4,0.001);
}
TEST(Depth, InvisibleLeavesDepthUntouched) {
    Scene s({0,0},{100,100});s.addTriangle(triangle(2,{255,0,0,0}));
    Frame f; SceneTestAccess::render(s,f.color,f.depth);
    for(float d:f.depth) CHECK(std::isinf(d));
}
TEST(Depth, OpaqueNearWinsInBothOrders) {
    for(bool reversed:{false,true}) {
        Scene s({0,0},{100,100});
        auto near=triangle(2,{255,0,0,255}), far=triangle(4,{0,255,0,255});
        s.addTriangle(reversed?far:near);s.addTriangle(reversed?near:far);
        Frame f;SceneTestAccess::render(s,f.color,f.depth);
        CHECK_EQ(f.color[136].r,255);CHECK_NEAR(f.depth[136],2,0.001);
    }
}
TEST(Clipping, BehindNearPlaneIsRejected) {
    Scene s({0,0},{100,100});s.addTriangle(triangle(0.5,{255,0,0,255}));
    Frame f;SceneTestAccess::render(s,f.color,f.depth);
    for(float d:f.depth)CHECK(std::isinf(d));
}
TEST(Clipping, CrossingNearPlaneIsFinite) {
    Scene s({0,0},{100,100});const Pixel p{255,255,255,255};
    s.addTriangle({{-2,-2,0.5,1.0f},{2,-2,3,1.0f},{0,2,3,1.0f},p,p,p});
    Frame f;SceneTestAccess::render(s,f.color,f.depth);
    int hits=0;for(float d:f.depth)if(std::isfinite(d)){CHECK(d>=1);++hits;}
    CHECK(hits>0);
}
