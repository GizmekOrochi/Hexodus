#include "../../include/TUI-Rendering/Scene3D.hpp"

using namespace Geometry;

Scene3D::Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending)
    : Scene(origin, ending), camera_() {}

void Scene3D::addTriangle(const Triangle& triangle) {
    ObjectList.push_back(triangle);
}

void Scene3D::addTriangles(const std::vector<Triangle>& triangles) {
    ObjectList.insert(ObjectList.end(), triangles.begin(), triangles.end());
}

void Scene3D::clearTriangles() {
    ObjectList.clear();
}

Camera& Scene3D::getCamera() {
    return camera_;
}

const Camera& Scene3D::getCamera() const {
    return camera_;
}

Vector Scene3D::worldToCamera(Vector& point) const {
    Vector cameraPos = camera_.getPosition();
    Vector cameraForward = camera_.forward();
    Vector cameraRight = camera_.right();
    Vector cameraUp = camera_.up();

    Vector translated = point - cameraPos;   
    Matrix viewMatrix = Matrix::Identity();

    std::array<float, 16> viewValues = {
        cameraRight.getX(), cameraUp.getX(), -cameraForward.getX(), 0.0f,
        cameraRight.getY(), cameraUp.getY(), -cameraForward.getY(), 0.0f,
        cameraRight.getZ(), cameraUp.getZ(), -cameraForward.getZ(), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    Vector rotated = translated.Transform(Matrix(viewValues));
    
    return rotated;
}

bool Scene3D::projectToScreen(const Vector& point) const {
    if (point.getZ() <= 0.1f) return false;
    return true;
}

std::vector<Triangle> Scene3D::applyPainterAlgorithm(std::vector<Triangle>& input) {
    std::vector<Triangle> visibleTriangles;
    Vector cameraPos = camera_.getPosition();
    
    // Back-face culling
    for (const auto& triangle : input) {
        Vector A = triangle.getA();
        Vector B = triangle.getB();
        Vector C = triangle.getC();

        Vector AB = B - A;
        Vector AC = C - A;
        Vector normal = AB.Cross(AC);
        Vector triangleCenter = (A + B + C) / 3.0f;
        Vector viewDir = triangleCenter - cameraPos;
        
        float dotProduct = normal.Dot(viewDir);
        
        if (dotProduct < 0) {
            visibleTriangles.push_back(triangle);
        }
    }
    
    // Sort triangles by distance from camera (farthest first)
    std::sort(visibleTriangles.begin(), visibleTriangles.end(),
        [this, &cameraPos](const Triangle& a, const Triangle& b) {
            // Calculate center of each triangle
            Vector centerA = (a.getA() + a.getB() + a.getC()) / 3.0f;
            Vector centerB = (b.getA() + b.getB() + b.getC()) / 3.0f;

            float distA = (centerA - cameraPos).lengthSq();
            float distB = (centerB - cameraPos).lengthSq();

            return distA > distB;
        });
    
    return visibleTriangles;
}

void Scene3D::drawLine(std::vector<TUI::Pixel>& pixels, float x1, float y1, float x2, float y2, TUI::Pixel color, int height, int width) const {
    int ix1 = static_cast<int>(x1);
    int iy1 = static_cast<int>(y1);
    int ix2 = static_cast<int>(x2);
    int iy2 = static_cast<int>(y2);
    
    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        if (ix1 >= 0 && ix1 < width && iy1 >= 0 && iy1 < height) {
            int index = iy1 * width + ix1;
            if (index >= 0 && index < static_cast<int>(pixels.size())) {
                pixels[index] = color;
            }
        }
        
        if (ix1 == ix2 && iy1 == iy2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            ix1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            iy1 += sy;
        }
    }
}

std::vector<TUI::Pixel> Scene3D::convertScene(int outputHeight, int outputLength) {
    std::vector<TUI::Pixel> pixels(outputHeight * outputLength, TUI::Pixel());
    
    if (ObjectList.empty()) {
        return pixels;
    }
    
    // Apply painter's algorithm to sort and cull triangles
    std::vector<Triangle> sortedTriangles = applyPainterAlgorithm(ObjectList);
    
    // Field of view and projection parameters
    float fov = 1.0472f; // 60 degrees in radians
    float aspectRatio = static_cast<float>(outputLength) / outputHeight;
    float nearPlane = 0.1f;

    for (const auto& triangle : sortedTriangles) {
        Vector A = triangle.getA();
        Vector B = triangle.getB();
        Vector C = triangle.getC();

        Vector camA = worldToCamera(A);
        Vector camB = worldToCamera(B);
        Vector camC = worldToCamera(C);
        
        if (!projectToScreen(camA) || !projectToScreen(camB) || !projectToScreen(camC)) {
            continue;
        }

        float screenXA = (camA.getX() / (camA.getZ() * tan(fov/2))) * aspectRatio * 0.5f * outputLength + outputLength * 0.5f;
        float screenYA = (-camA.getY() / (camA.getZ() * tan(fov/2))) * 0.5f * outputHeight + outputHeight * 0.5f;
        
        float screenXB = (camB.getX() / (camB.getZ() * tan(fov/2))) * aspectRatio * 0.5f * outputLength + outputLength * 0.5f;
        float screenYB = (-camB.getY() / (camB.getZ() * tan(fov/2))) * 0.5f * outputHeight + outputHeight * 0.5f;
        
        float screenXC = (camC.getX() / (camC.getZ() * tan(fov/2))) * aspectRatio * 0.5f * outputLength + outputLength * 0.5f;
        float screenYC = (-camC.getY() / (camC.getZ() * tan(fov/2))) * 0.5f * outputHeight + outputHeight * 0.5f;

        screenXA = std::max(0.0f, std::min(static_cast<float>(outputLength - 1), screenXA));
        screenYA = std::max(0.0f, std::min(static_cast<float>(outputHeight - 1), screenYA));
        screenXB = std::max(0.0f, std::min(static_cast<float>(outputLength - 1), screenXB));
        screenYB = std::max(0.0f, std::min(static_cast<float>(outputHeight - 1), screenYB));
        screenXC = std::max(0.0f, std::min(static_cast<float>(outputLength - 1), screenXC));
        screenYC = std::max(0.0f, std::min(static_cast<float>(outputHeight - 1), screenYC));

        TUI::Pixel lineColor = triangle.getColor();

        drawLine(pixels, screenXA, screenYA, screenXB, screenYB, lineColor, outputHeight, outputLength);
        drawLine(pixels, screenXB, screenYB, screenXC, screenYC, lineColor, outputHeight, outputLength);
        drawLine(pixels, screenXC, screenYC, screenXA, screenYA, lineColor, outputHeight, outputLength);
    }
    
    return pixels;
}