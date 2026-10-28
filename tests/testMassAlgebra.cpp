#include "tester.hpp"
#include "../src/include/TUI-Rendering/Geometry/object/Matrix.hpp"
#include "../src/include/TUI-Rendering/Geometry/object/Vector.hpp"
#include <numbers>

using Geometry::Matrix;
using Geometry::Vector;
namespace {
void checkVector(const Vector& v, float x, float y, float z, float w) {
    CHECK_NEAR(v.getX(), x, 1e-5); CHECK_NEAR(v.getY(), y, 1e-5);
    CHECK_NEAR(v.getZ(), z, 1e-5); CHECK_NEAR(v.getW(), w, 1e-5);
}
template<class F> void checkOutOfRange(F action) {
    bool thrown = false;
    try { action(); } catch (const std::out_of_range&) { thrown = true; }
    CHECK(thrown); // Other exception types propagate to the framework.
}
Matrix sequence() { return Matrix({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}); }
}
TEST(Vector, ConstructorsAndAssignment) {
    checkVector(Vector(),0,0,0,1);
    const Vector direction(1,2,3,true), point(4,5,6,false), homogeneous(7,8,9,0.5f);
    checkVector(direction,1,2,3,0); checkVector(point,4,5,6,1);
    checkVector(homogeneous,7,8,9,0.5f);
    Vector copy(point); checkVector(copy,4,5,6,1);
    copy = direction; checkVector(copy,1,2,3,0);
    copy = copy; checkVector(copy,1,2,3,0);
}
TEST(Vector, AdditionAndSubtraction) {
    const Vector p(1,2,3,false), q(4,5,6,false), a(1,0,0,true), b(0,1,0,true);
    checkVector(a+b,1,1,0,0); checkVector(a-b,1,-1,0,0);
    checkVector(p+a,2,2,3,1); checkVector(p-a,0,2,3,1);
    // Point - point yields a direction (the old demo's comment was incorrect).
    checkVector(p-q,-3,-3,-3,0);
    checkOutOfRange([&]{ (void)(p+q); });
    checkOutOfRange([&]{ (void)(a-p); });
    // Preserve Hexodus' current asymmetric addition contract.
    checkOutOfRange([&]{ (void)(a+p); });
}
TEST(Vector, ScalarsAndCompoundOperations) {
    const Vector a(2,4,6,true), p(2,4,6,false);
    checkVector(a*2.5f,5,10,15,0); checkVector(a/2,1,2,3,0);
    checkVector(p*2,4,8,12,1);
    Vector v(1,2,3,true); v+=Vector(0.5f,0.5f,0.5f,true);
    checkVector(v,1.5f,2.5f,3.5f,0);
    v-=Vector(0.5f,0.5f,0.5f,true); v*=2; v/=2;
    checkVector(v,1,2,3,0);
}
TEST(Vector, NormDotAndCross) {
    const Vector a(3,4,0,true), b(1,2,3,true), z(0,0,1,true);
    CHECK_NEAR(a.length(),5,1e-6); CHECK_NEAR(a.Norm(),5,1e-6);
    CHECK_NEAR(a.lengthSq(),25,1e-6); CHECK_NEAR(a.Dot(b),11,1e-6);
    checkVector(b.Cross(z),2,-1,0,0); checkVector(z.Cross(b),-2,1,0,0);
    CHECK_NEAR(b.Dot(b.Cross(z)),0,1e-6);
}
TEST(Vector, HomogeneousCoordinatesAndDivisionErrors) {
    checkVector(Vector(2,4,6,2.0f).getRealPosition(),1,2,3,1);
    const Vector v(1,2,3,true);
    checkOutOfRange([&]{ (void)v.getRealPosition(); });
    checkOutOfRange([&]{ (void)(v/0.0f); });
}
TEST(Matrix, ConstructorsIdentityAndAssignment) {
    const Matrix identity;
    for(std::size_t i=0;i<16;++i) CHECK_EQ(identity[i],i%5==0?1.0f:0.0f);
    CHECK(identity==Matrix::Identity());
    const auto m=sequence();
    for(std::size_t i=0;i<16;++i) CHECK_EQ(m[i],float(i+1));
    Matrix copy; copy=m; CHECK(copy==m); copy=copy; CHECK(copy==m);
    CHECK(m!=identity); CHECK(!(m!=m));
}
TEST(Matrix, ArithmeticAndCompoundOperations) {
    const auto a=sequence(); const Matrix b({16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1});
    const auto sum=a+b, diff=a-b, scaled=a*2, divided=a/2;
    for(std::size_t i=0;i<16;++i) {
        CHECK_EQ(sum[i],17.0f); CHECK_EQ(diff[i],2.0f*float(i)-15.0f);
        CHECK_EQ(scaled[i],2.0f*float(i+1)); CHECK_EQ(divided[i],float(i+1)/2);
    }
    Matrix m; m+=a; m-=a; CHECK(m==Matrix::Identity());
    m=a; m*=2; CHECK(m==scaled); m/=2; CHECK(m==a);
}
TEST(Matrix, MultiplicationKnownResult) {
    const auto a=sequence(); const Matrix b({16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1});
    const Matrix expected({80,70,60,50,240,214,188,162,400,358,316,274,560,502,444,386});
    CHECK(a*b==expected); CHECK(a*Matrix::Identity()==a); CHECK(Matrix::Identity()*a==a);
    CHECK(a*b!=b*a);
}
TEST(Matrix, TransposeAndIndexing) {
    auto m=sequence(); auto t=m.transpose();
    CHECK_EQ(t[1],5.0f); CHECK_EQ(t[4],2.0f); CHECK(t.transpose()==m);
    m[5]=100; CHECK_EQ(m[5],100.0f);
    checkOutOfRange([&]{ (void)m[16]; });
    const Matrix c; checkOutOfRange([&]{ (void)c[16]; });
    checkOutOfRange([&]{ (void)(c/0.0f); });
}
TEST(Transform, TranslationAndScale) {
    const Vector p(1,2,3,false), d(1,2,3,true);
    const auto t=Matrix::createTranslation(1,2,3), s=Matrix::createScale(2,3,4);
    checkVector(p.Transform(t),2,4,6,1); checkVector(p*t,2,4,6,1);
    checkVector(d.Transform(t),1,2,3,0); checkVector(d.Transform(s),2,6,12,0);
    checkVector(p.Translate(d),2,4,6,1); checkVector(p.Transform(d),2,4,6,1);
}
TEST(Transform, QuarterTurnsCurrentConvention) {
    // Hexodus uses row-vector multiplication; positive angles follow these signs.
    const float angle=std::numbers::pi_v<float>/2;
    checkVector(Vector(0,1,0,true).Transform(Matrix::createRotationX(angle)),0,0,-1,0);
    checkVector(Vector(0,0,1,true).Transform(Matrix::createRotationY(angle)),-1,0,0,0);
    checkVector(Vector(1,0,0,true).Transform(Matrix::createRotationZ(angle)),0,-1,0,0);
    checkVector(Vector(0,1,0,true).Rotate(Vector(angle,0,0,true)),0,0,-1,0);
}
TEST(Transform, CompositionOrder) {
    const auto scale=Matrix::createScale(2,3,4), translation=Matrix::createTranslation(1,2,3);
    const Vector p(1,2,3,false);
    checkVector(p.Transform(scale*translation),3,8,15,1);
    checkVector(p.Transform(translation*scale),4,12,24,1);
    checkVector(p.Transform(scale).Transform(translation),3,8,15,1);
}
