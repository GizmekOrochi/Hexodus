#include "tester.hpp"
#include "../src/include/TUIRenderer.hpp"
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
    static auto project(Scene& s) { s.getCamera().setPosition({0,0,0,1.0f}); return s.projectTriangles(16,16); }
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

TEST(Encoding, CentroidsImproveGradient) {
    const auto cell=TUI::RendererTestAccess::encode({Pixel{0,0,0,255},Pixel{10,10,10,255},Pixel{100,100,100,255},Pixel{110,110,110,255}});
    CHECK_EQ(cell.foreground.R_,5); CHECK_EQ(cell.background.R_,105);
    CHECK_EQ(cell.glyph,TUI::RendererTestAccess::glyph(3));
}
TEST(Encoding, AlphaDoesNotChangeEncoding) {
    CHECK(TUI::RendererTestAccess::encode({Pixel{10,20,30,0},Pixel{40,50,60,10},Pixel{70,80,90,100},Pixel{100,110,120,255}})==
          TUI::RendererTestAccess::encode({Pixel{10,20,30,255},Pixel{40,50,60,255},Pixel{70,80,90,255},Pixel{100,110,120,255}}));
}
TEST(Encoding, RoundedCentroid) {
    const auto cell=TUI::RendererTestAccess::encode({Pixel{0,0,0,255},Pixel{1,1,1,255},Pixel{254,254,254,255},Pixel{255,255,255,255}});
    CHECK_EQ(cell.foreground.R_,1); CHECK_EQ(cell.background.R_,255);
}
TEST(Transparency, LayerOrderAndOpaqueDepth) {
    for(bool reverse:{false,true}) {
        Scene s({0,0},{100,100});
        const auto near=triangle(2,{255,0,0,128}), far=triangle(3,{0,255,0,128});
        s.addTriangle(reverse?near:far);s.addTriangle(reverse?far:near);
        s.addTriangle(triangle(4,{0,0,255,255}));
        Frame f;SceneTestAccess::render(s,f.color,f.depth);
        CHECK_NEAR(f.color[136].r,128,1);CHECK_NEAR(f.color[136].g,63,1);
        CHECK_NEAR(f.color[136].b,63,1);CHECK_NEAR(f.depth[136],4,0.001);
    }
}
TEST(Transparency, BehindOpaqueIsHidden) {
    Scene s({0,0},{100,100});s.addTriangle(triangle(4,{255,0,0,128}));s.addTriangle(triangle(2,{0,255,0,255}));
    Frame f;SceneTestAccess::render(s,f.color,f.depth);CHECK_EQ(f.color[136].g,255);CHECK_EQ(f.color[136].r,0);
}
TEST(Coverage, SharedDiagonalBlendsExactlyOnce) {
    for(bool reverse:{false,true}) {
        Scene s({0,0},{100,100});const Pixel p{255,0,0,128};
        Geometry::Vector a{-20,-20,2,1.0f},b{20,-20,2,1.0f},c{20,20,2,1.0f},d{-20,20,2,1.0f};
        s.addTriangle(reverse?Geometry::Triangle{c,b,a,p,p,p}:Geometry::Triangle{a,b,c,p,p,p});
        s.addTriangle(reverse?Geometry::Triangle{d,c,a,p,p,p}:Geometry::Triangle{a,c,d,p,p,p});
        Frame f;SceneTestAccess::render(s,f.color,f.depth);
        for(const auto pixel:f.color) CHECK_EQ(pixel.r,128);
        for(float depth:f.depth)CHECK(std::isinf(depth));
    }
}
TEST(Clipping, AllProjectedVerticesWithinFrustum) {
    Scene s({0,0},{100,100});const Pixel p{255,255,255,255};
    s.addTriangle({{-10000,-10000,0.5f,1.0f},{10000,-10000,3000,1.0f},{0,10000,1000,1.0f},p,p,p});
    const auto triangles=SceneTestAccess::project(s);CHECK(!triangles.empty());
    for(const auto& t:triangles)for(const auto& v:{t.getA(),t.getB(),t.getC()}) {
        CHECK(v.getZ()>=0.999f);CHECK(v.getZ()<=2000.001f);
        CHECK(std::abs(v.getX())<=8.001f);CHECK(std::abs(v.getY())<=8.001f);
    }
}
TEST(Projection, ConfigurableAspectAndValidation) {
    Scene s({0,0},{100,100});s.addTriangle(triangle(100,{255,0,0,255}));
    auto before=SceneTestAccess::project(s);s.setPixelAspectRatio(1);auto after=SceneTestAccess::project(s);
    CHECK_NEAR(after[0].getA().getX()*2,before[0].getA().getX(),0.001);
    for(float value:{0.0f,-1.0f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        bool threw=false;try{s.setPixelAspectRatio(value);}catch(const std::invalid_argument&){threw=true;}CHECK(threw);
    }
}
TEST(Color, Palette256CubeAndGray) {
    Environment env("xterm-256color",nullptr);
    TUI::Colorator c;c.SetRGBColor({255,0,0},{128,128,128});
    CHECK_EQ(c.GetColorString(),std::string("\x1b[38;5;196m\x1b[48;5;244m"));
}
TEST(Color, LinuxIgnoresInheritedTruecolor) {
    Environment env("linux","truecolor");
    TUI::Colorator c;c.SetRGBColor({255,0,0},{0,0,255});
    CHECK_EQ(c.GetColorString(),std::string("\x1b[91m\x1b[104m"));
}
