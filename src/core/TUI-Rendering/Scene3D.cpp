#include "../../include/TUI-Rendering/Scene3D.hpp"
#include "../../include/util/Logger.hpp"

using namespace Geometry;

Scene3D::Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending) : Scene(origin, ending), camera_(), BackfaceCulling_(false) {}

Triangle& Scene3D::Element(size_t index) { return ObjectList[index]; }

const Triangle& Scene3D::Element(size_t index) const { return ObjectList[index]; }

size_t Scene3D::addTriangle(const Triangle& triangle) {
    ObjectList.push_back(triangle);
    return ObjectList.size() - 1;
}

void Scene3D::clearTriangles() {
    int count{ObjectList.size()};
    ObjectList.clear();
}

Camera& Scene3D::getCamera() {
    return camera_;
}

const Camera& Scene3D::getCamera() const {
    return camera_;
}

void Scene3D::toggleBackfaceCulling() {
    this->BackfaceCulling_ = !BackfaceCulling_;
}

bool Scene3D::inPlan(const Vector& input, Vector& output) {
    float Znear{1.f};
    float Zfar{2000.f};
    float FOV{70.f};
    float PI{3.1415f};

    float fovRadians{FOV * PI / 180.f};
    float f{30.f};  //1.f / tan(fovRadians / 2.f)

    if (input.getZ() < Znear || input.getZ() > Zfar) return false;

    Vector projectedPoint{
        input.getX() * f / input.getZ(),
        input.getY() * f / input.getZ(),
        input.getZ(),
        input.getW()

    };

    output = projectedPoint;
    return true;
}

Vector Scene3D::worldToCamera(const Vector& position) const {
    Vector rel{position - camera_.getPosition()};

    float yaw{camera_.getYaw()};
    float pitch{camera_.getPitch()};

    float cy{std::cos(-yaw)};
    float sy{std::sin(-yaw)};
    float x1{cy * rel.getX() + sy * rel.getZ()};
    float y1{rel.getY()};
    float z1{-sy * rel.getX() + cy * rel.getZ()};

    float cp{std::cos(-pitch)};
    float sp{std::sin(-pitch)};
    float x2{x1};
    float y2{cp * y1 - sp * z1};
    float z2{sp * y1 + cp * z1};

    return Vector{x2, y2, z2, 1.f};
}

bool Scene3D::inScene(const Vector& input, Vector& output) {
    int SceneSizeX{Ending()[0] - Origin()[0]};
    int SceneSizeY{Ending()[1] - Origin()[1]};

    float halfX{SceneSizeX * 0.5f};
    float halfY{SceneSizeY * 0.5f};

    bool inScene{true};

    Vector cam{worldToCamera(input)};
    Vector projected;

    if (!inPlan(cam, projected)) return false;

    if (projected.getX() < -halfX || projected.getX() > halfX) inScene = false;
    if (projected.getY() < -halfY || projected.getY() > halfY) inScene = false;

    output = projected;

    return inScene;
}

ClippingPoint Scene3D::intersectPlane(const ClippingPoint& p1, const ClippingPoint& p2) {
    float zNear{1.0f};
    float t{(zNear - p1.pos.getZ()) / (p2.pos.getZ() - p1.pos.getZ())};
    
    // Interpolate Position
    Vector newPos{
        p1.pos.getX() + t * (p2.pos.getX() - p1.pos.getX()),
        p1.pos.getY() + t * (p2.pos.getY() - p1.pos.getY()),
        zNear,
        1.0f
    };

    TUI::Pixel newColor;
    newColor.r = static_cast<uint8_t>(p1.color.r + t * (p2.color.r - p1.color.r));
    newColor.g = static_cast<uint8_t>(p1.color.g + t * (p2.color.g - p1.color.g));
    newColor.b = static_cast<uint8_t>(p1.color.b + t * (p2.color.b - p1.color.b));

    return {newPos, newColor};
}


std::vector<Triangle> Scene3D::clipping(const Triangle& tri) {
    float zNear{1.0f};
    
    // Create structs for vertices to keep colors attached to their positions
    ClippingPoint pA{worldToCamera(tri.getA()), tri.getColorA()};
    ClippingPoint pB{worldToCamera(tri.getB()), tri.getColorB()};
    ClippingPoint pC{worldToCamera(tri.getC()), tri.getColorC()};
    
    std::vector<ClippingPoint> inside, outside;
    auto classify = [&](ClippingPoint p) {
        if (p.pos.getZ() >= zNear) inside.push_back(p); 
        else outside.push_back(p);
    };

    classify(pA); classify(pB); classify(pC);

    if (inside.empty()) return {};

    if (inside.size() == 3) {
        return { Triangle(pA.pos, pB.pos, pC.pos, pA.color, pB.color, pC.color) };
    }

    if (inside.size() == 1) {
        // One vertex inside: create 1 new triangle
        ClippingPoint i1{intersectPlane(inside[0], outside[0])};
        ClippingPoint i2{intersectPlane(inside[0], outside[1])};
        
        return { Triangle(inside[0].pos, i1.pos, i2.pos, inside[0].color, i1.color, i2.color) };
    }

    if (inside.size() == 2) {
        ClippingPoint i1{intersectPlane(inside[0], outside[0])};
        ClippingPoint i2{intersectPlane(inside[1], outside[0])};

        return {
            Triangle(inside[0].pos, inside[1].pos, i1.pos, inside[0].color, inside[1].color, i1.color),
            Triangle(inside[1].pos, i2.pos, i1.pos, inside[1].color, i2.color, i1.color)
        };
    }

    return {};
}

std::vector<Triangle> Scene3D::projectTriangles() {
    std::vector<Triangle> res;
    
    for (const auto& tri : ObjectList) {
        std::vector<Triangle> clipped{clipping(tri)};

        for (auto& clippedTri : clipped) {
            Vector pA, pB, pC;
            if (inPlan(clippedTri.getA(), pA) && 
                inPlan(clippedTri.getB(), pB) && 
                inPlan(clippedTri.getC(), pC)) {
                
                res.push_back(Triangle(pA, pB, pC, 
                    clippedTri.getColorA(), clippedTri.getColorB(), clippedTri.getColorC()));
            }
        }
    }
    return res;
}

void Scene3D::sortTriangles(std::vector<Triangle>& triangles) {
    int n{static_cast<int>(triangles.size())};

    for (int i = 1; i < n; ++i) {
        Triangle key{triangles[i]};
        float keyZ{(key.getA().getZ() + key.getB().getZ() + key.getC().getZ()) / 3.f};
        int j{i - 1};

        while (j >= 0) {
            float currentZ{(triangles[j].getA().getZ() + triangles[j].getB().getZ() + triangles[j].getC().getZ()) / 3.f};   
            if (currentZ < keyZ) {
                triangles[j + 1] = triangles[j];
                j = j - 1;
            } else {
                break;
            }
        }
        triangles[j + 1] = key;
    }
}

std::vector<TUI::Pixel> Scene3D::convertScene(int outputHeight, int outputLength) {
    int SceneOrigin_X{(this->Origin()[0] * outputLength) / 100};
    int SceneOrigin_Y{(this->Origin()[1] * outputHeight) / 100};
    int SceneEnding_X{(this->Ending()[0] * outputLength) / 100};
    int SceneEnding_Y{(this->Ending()[1] * outputHeight) / 100};

    int maxXBuffer{SceneEnding_X - SceneOrigin_X};
    int maxYBuffer{SceneEnding_Y - SceneOrigin_Y};

    std::vector<TUI::Pixel> SceneBuffer{};
    std::vector<Triangle> TriangleList{projectTriangles()};
    sortTriangles(TriangleList);

    for (int y = 0; y < maxYBuffer; y++) {
        for (int x = 0; x < maxXBuffer; x++) {
            float minZ{std::numeric_limits<float>::max()};
            TUI::Pixel currentBestColor{this->Color()};
            
            bool foundElement = false;

            float halfW{maxXBuffer * 0.5f};
            float halfH{maxYBuffer * 0.5f};
            float pixelCenterX{(x + 0.5f) - halfW};
            float pixelCenterY{halfH - (y + 0.5f)};

            for (const auto& triangle : TriangleList) {
                Vector A{triangle.getA()};
                Vector B{triangle.getB()};
                Vector C{triangle.getC()};

                Vector v0{C.getX() - A.getX(), C.getY() - A.getY(), 0.f, 1.0f};
                Vector v1{B.getX() - A.getX(), B.getY() - A.getY(), 0.f, 1.0f};
                Vector v2{pixelCenterX - A.getX(), pixelCenterY - A.getY(), 0.f, 1.0f};

                float dot00{v0.Dot(v0)};
                float dot01{v0.Dot(v1)};
                float dot02{v0.Dot(v2)};
                float dot11{v1.Dot(v1)};
                float dot12{v1.Dot(v2)};

                float denom = (dot00 * dot11 - dot01 * dot01);
                if (std::abs(denom) < 1e-6) continue;

                float invDenom = 1.0f / denom;
                float u{(dot11 * dot02 - dot01 * dot12) * invDenom};
                float v{(dot00 * dot12 - dot01 * dot02) * invDenom};
                float w{1.0f - u - v};

                if (u >= 0 && v >= 0 && w >= 0) {
                    float invZA{1.0f / A.getZ()};
                    float invZB{1.0f / B.getZ()};
                    float invZC{1.0f / C.getZ()};

                    float interpolatedInvZ{invZA * w + invZB * v + invZC * u};
                    float zValue{1.0f / interpolatedInvZ};

                    if (zValue < minZ) {
                        minZ = zValue;
                        foundElement = true;

                        TUI::Pixel cA{triangle.getColorA()};
                        TUI::Pixel cB{triangle.getColorB()};
                        TUI::Pixel cC{triangle.getColorC()};

                        float rOverZ {(cA.r * invZA * w) + (cB.r * invZB * v) + (cC.r * invZC * u)};
                        float gOverZ{(cA.g * invZA * w) + (cB.g * invZB * v) + (cC.g * invZC * u)};
                        float bOverZ{(cA.b * invZA * w) + (cB.b * invZB * v) + (cC.b * invZC * u)};
                        float aOverZ{(cA.a * invZA * w) + (cB.a * invZB * v) + (cC.a * invZC * u)};
                        
                        float interpolatedAlphaPerc{(aOverZ * zValue) / 255.f};
                        // It's just a VERRRYYY and why to hard to read max(0, min(1, inter))
                        float min{(1.f < interpolatedAlphaPerc) ? 1.f : interpolatedAlphaPerc};
                        interpolatedAlphaPerc = (0.f > min) ? 0.f : min;

                        currentBestColor.r = static_cast<uint8_t>((rOverZ * zValue * interpolatedAlphaPerc) + static_cast<float>(currentBestColor.r) * (1 - interpolatedAlphaPerc));
                        currentBestColor.g = static_cast<uint8_t>((gOverZ * zValue * interpolatedAlphaPerc) + static_cast<float>(currentBestColor.g) * (1 - interpolatedAlphaPerc));
                        currentBestColor.b = static_cast<uint8_t>((bOverZ * zValue * interpolatedAlphaPerc) + static_cast<float>(currentBestColor.b) * (1 - interpolatedAlphaPerc));
                        currentBestColor.a = 255;
                    }
                }
            }
            SceneBuffer.push_back(foundElement ? currentBestColor : this->Color());
        }
    }
    return SceneBuffer;
}