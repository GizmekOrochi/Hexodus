#include "../../include/TUI-Rendering/Scene3D.hpp"
#include "../../include/util/Logger.hpp"

using namespace Geometry;

Scene3D::Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending) : Scene(origin, ending), camera_() {}

void Scene3D::addTriangle(const Triangle& triangle) {
    ObjectList.push_back(triangle);
}

void Scene3D::addTriangles(const std::vector<Triangle>& triangles) {
    ObjectList.insert(ObjectList.end(), triangles.begin(), triangles.end());
}

void Scene3D::clearTriangles() {
    int count = ObjectList.size();
    ObjectList.clear();
}

Camera& Scene3D::getCamera() {
    return camera_;
}

const Camera& Scene3D::getCamera() const {
    return camera_;
}

bool Scene3D::inPlan(const Vector& input, Vector& output) {
    Logger::log("inPlan() called");
    float d = 30.f;
    float Zfar = 2000.f;
    if (input.getZ() <= 0.f || input.getZ() > Zfar) return false;

    Vector projectedPoint{
        input.getX() * d / input.getZ(),
        input.getY() * d / input.getZ(),
        d,
        input.getW()
    };

    output = projectedPoint;
    return true;
}

Vector Scene3D::worldToCamera(const Vector& position) const {
    Logger::log("worldToCamera() called");
    Vector rel{position - camera_.getPosition()};

    float yaw = camera_.getYaw();
    float pitch = camera_.getPitch();

    float cy{std::cos(-yaw)};
    float sy{std::sin(-yaw)};
    float x1 = cy * rel.getX() + sy * rel.getZ();
    float y1 = rel.getY();
    float z1 = -sy * rel.getX() + cy * rel.getZ();

    float cp{std::cos(-pitch)};
    float sp{std::sin(-pitch)};
    float x2{x1};
    float y2{cp * y1 - sp * z1};
    float z2{sp * y1 + cp * z1};

    Logger::log("       Projected point=(" + std::to_string(x2) + "," + std::to_string(y2) + "," + std::to_string(z2) + ")");


    return Vector{x2, y2, z2, 1.f};
}

bool Scene3D::inScene(const Vector& input, Vector& output) {
    Logger::log("inScene() called");
    int SceneSizeX = Ending()[0] - Origin()[0];
    int SceneSizeY = Ending()[1] - Origin()[1];

    float halfX = SceneSizeX * 0.5f;
    float halfY = SceneSizeY * 0.5f;

    Vector cam = worldToCamera(input);
    Vector projected;

    if (!inPlan(cam, projected)) return false;

    if (projected.getX() < -halfX || projected.getX() > halfX) return false;
    if (projected.getY() < -halfY || projected.getY() > halfY) return false;

    output = projected;

    return true;
}

// project the triangles and remove the tiangles not in the scene
std::vector<Geometry::Triangle> Scene3D::projectTriangles() {
    std::vector<Triangle> res;
    res.reserve(ObjectList.size());

    for (const auto& triangle : ObjectList) {
        std::array<Vector, 3> projectedVerticals{};
        if(!inScene(triangle.getA(), projectedVerticals[0])) continue;
        if(!inScene(triangle.getB(), projectedVerticals[1])) continue;
        if(!inScene(triangle.getC(), projectedVerticals[2])) continue;
        res.push_back(Triangle{
            projectedVerticals[0], projectedVerticals[1], projectedVerticals[2],
            triangle.getColorA(), triangle.getColorB(), triangle.getColorC(),
            triangle.getGradiantA(), triangle.getGradiantB(), triangle.getGradiantC()
        });
    }
    return res;
}

std::vector<TUI::Pixel> Scene3D::convertScene(int outputHeight, int outputLength) {
    Logger::log("convertScene() called");
    Logger::log("output: " + std::to_string(outputLength) + "x" + std::to_string(outputHeight));

    int SceneOrigin_X = (this->Origin()[0] * outputLength) / 100;
    int SceneOrigin_Y = (this->Origin()[1] * outputHeight) / 100;
    
    int SceneEnding_X = (this->Ending()[0] * outputLength) / 100;
    int SceneEnding_Y = (this->Ending()[1] * outputHeight) / 100;

    int maxXBuffer = SceneEnding_X - SceneOrigin_X;
    int maxYBuffer = SceneEnding_Y - SceneOrigin_Y;

    Logger::log(
        "Scene bounds px: origin=(" +
        std::to_string(SceneOrigin_X) + "," +
        std::to_string(SceneOrigin_Y) + ") size=(" +
        std::to_string(maxXBuffer) + "," +
        std::to_string(maxYBuffer) + ")"
    );

    std::vector<TUI::Pixel> SceneBuffer{};
    std::vector<Triangle> TriangleList{projectTriangles()};

    Logger::log("Projected triangles count: " + std::to_string(TriangleList.size()));

    if (!TriangleList.empty()) {
        const auto& t = TriangleList[0];
        Logger::log(
            "Triangle[0] A=(" +
            std::to_string(t.getA().getX()) + "," +
            std::to_string(t.getA().getY()) + "," +
            std::to_string(t.getA().getZ()) + ")"
        );
        Logger::log(
            "Triangle[0] B=(" +
            std::to_string(t.getB().getX()) + "," +
            std::to_string(t.getB().getY()) + "," +
            std::to_string(t.getB().getZ()) + ")"
        );
        Logger::log(
            "Triangle[0] C=(" +
            std::to_string(t.getC().getX()) + "," +
            std::to_string(t.getC().getY()) + "," +
            std::to_string(t.getC().getZ()) + ")"
        );
    }

    // Pre-calculate pixel to world coordinates transformation
    float pixelToWorldX = 100.0f / outputLength;
    float pixelToWorldY = 100.0f / outputHeight;
    int debugX = maxXBuffer / 2;
    int debugY = maxYBuffer / 2;

    for (int y = 0; y < maxYBuffer; y++) {
        for (int x = 0; x < maxXBuffer; x++) {
            float Ztriangle{MAXFLOAT};
            TUI::Pixel currentBestColor;
            bool foundElement = false;

            float sceneX = SceneOrigin_X + x;
            float sceneY = SceneOrigin_Y + y;

            float halfW = maxXBuffer * 0.5f;
            float halfH = maxYBuffer * 0.5f;

            float pixelCenterX = (x + 0.5f) - halfW;
            float pixelCenterY = halfH - (y + 0.5f);

            if (x == debugX && y == debugY) {
                Logger::log(
                    "Debug pixel center scene=(" +
                    std::to_string(pixelCenterX) + "," +
                    std::to_string(pixelCenterY) + ")"
                );
            }

            for (const auto& triangle : TriangleList) {
                Vector A = triangle.getA();
                Vector B = triangle.getB();
                Vector C = triangle.getC();
                
                Vector a{A.getX(), A.getY(), 0.f, 0.f};
                Vector b{B.getX(), B.getY(), 0.f, 0.f};
                Vector c{C.getX(), C.getY(), 0.f, 0.f};

                Vector p{pixelCenterX, pixelCenterY, 0.f, 0.f};
                
                Vector v0 = c - a;
                Vector v1 = b - a;
                Vector v2 = p - a;

                float dot00 = v0.Dot(v0);
                float dot01 = v0.Dot(v1);
                float dot02 = v0.Dot(v2);
                float dot11 = v1.Dot(v1);
                float dot12 = v1.Dot(v2);

                float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
                float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

                if (x == debugX && y == debugY) {
                    Logger::log(
                        "u=" + std::to_string(u) +
                        " v=" + std::to_string(v) +
                        " u+v=" + std::to_string(u + v)
                    );
                }

                if ((u >= 0) && (v >= 0) && (u + v <= 1.0f)) {
                    float zValue = A.getZ() * (1 - u - v) + B.getZ() * u + C.getZ() * v;

                    if (zValue < Ztriangle) {
                        if (x == debugX && y == debugY) {
                            Logger::log(
                                "Pixel HIT triangle, z=" + std::to_string(zValue)
                            );
                        }

                        Ztriangle = zValue;
                        foundElement = true;
                        
                        TUI::Pixel colorA = triangle.getColorA();
                        TUI::Pixel colorB = triangle.getColorB();
                        TUI::Pixel colorC = triangle.getColorC();

                        uint8_t gradA = triangle.getGradiantA();
                        uint8_t gradB = triangle.getGradiantB();
                        uint8_t gradC = triangle.getGradiantC();

                        float r = (colorA.r * (1 - u - v) + colorB.r * u + colorC.r * v) / 255.0f;
                        float g = (colorA.g * (1 - u - v) + colorB.g * u + colorC.g * v) / 255.0f;
                        float b = (colorA.b * (1 - u - v) + colorB.b * u + colorC.b * v) / 255.0f;

                        float gradInterp = (gradA * (1 - u - v) + gradB * u + gradC * v) / 255.0f;
                        
                        //r *= (1.0f - gradInterp * 0.5f);
                        //g *= (1.0f - gradInterp * 0.5f);
                        //b *= (1.0f - gradInterp * 0.5f);

                        //r = std::max(0.0f, std::min(1.0f, r));
                        //g = std::max(0.0f, std::min(1.0f, g));
                        //b = std::max(0.0f, std::min(1.0f, b));
                        
                        currentBestColor.r = static_cast<uint8_t>(r * 255);
                        currentBestColor.g = static_cast<uint8_t>(g * 255);
                        currentBestColor.b = static_cast<uint8_t>(b * 255);
                    }
                }
            }
            
            if (foundElement) {
                SceneBuffer.push_back(currentBestColor);
            } else {
                SceneBuffer.push_back(this->Color());
            }
        }
    }
    Logger::log("convertScene() finished frame");

    return SceneBuffer;
}