#include <string>
#include "include/TUIRenderer.hpp"
#include "include/TUI-Rendering/Scene3D.hpp"
#include "include/TUI-Rendering/Geometry/object/Triangle.hpp"
#include "include/TUI-Rendering/Geometry/object/Vector.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include <thread>

using namespace Geometry;
using namespace Rendering;

// Helper function to apply transformation to all triangles in a vector
std::vector<Triangle> transformTriangles(const std::vector<Triangle>& triangles, 
                                        float tx = 0, float ty = 0, float tz = 0,
                                        float rx = 0, float ry = 0, float rz = 0) {
    std::vector<Triangle> result;
    for (const auto& triangle : triangles) {
        Triangle transformed = triangle;
        if (rx != 0) transformed = transformed.rotateX(rx);
        if (ry != 0) transformed = transformed.rotateY(ry);
        if (rz != 0) transformed = transformed.rotateZ(rz);
        if (tx != 0 || ty != 0 || tz != 0) transformed = transformed.translate(tx, ty, tz);
        result.push_back(transformed);
    }
    return result;
}

// Function to create a cube with 12 triangles (2 per face)
std::vector<Triangle> createCube(float size = 1.0f) {
    std::vector<Triangle> triangles;
    
    // Define the 8 vertices of the cube
    float halfSize = size / 2.0f;
    
    Vector vertices[8] = {
        // Front face vertices
        Vector(-halfSize, -halfSize, -halfSize, 1.0f), // 0: front bottom left
        Vector( halfSize, -halfSize, -halfSize, 1.0f), // 1: front bottom right
        Vector( halfSize,  halfSize, -halfSize, 1.0f), // 2: front top right
        Vector(-halfSize,  halfSize, -halfSize, 1.0f), // 3: front top left
        
        // Back face vertices
        Vector(-halfSize, -halfSize,  halfSize, 1.0f), // 4: back bottom left
        Vector( halfSize, -halfSize,  halfSize, 1.0f), // 5: back bottom right
        Vector( halfSize,  halfSize,  halfSize, 1.0f), // 6: back top right
        Vector(-halfSize,  halfSize,  halfSize, 1.0f)  // 7: back top left
    };
    
    // Create triangles for each face with different colors
    
    // Front face (red)
    triangles.push_back(Triangle(vertices[0], vertices[1], vertices[2], TUI::Pixel{255, 0, 0, 255}));
    triangles.push_back(Triangle(vertices[2], vertices[3], vertices[0], TUI::Pixel{255, 0, 0, 255}));
    
    // Back face (green)
    triangles.push_back(Triangle(vertices[5], vertices[4], vertices[7], TUI::Pixel{0, 255, 0, 255}));
    triangles.push_back(Triangle(vertices[7], vertices[6], vertices[5], TUI::Pixel{0, 255, 0, 255}));
    
    // Top face (blue)
    triangles.push_back(Triangle(vertices[3], vertices[2], vertices[6], TUI::Pixel{0, 0, 255, 255}));
    triangles.push_back(Triangle(vertices[6], vertices[7], vertices[3], TUI::Pixel{0, 0, 255, 255}));
    
    // Bottom face (yellow)
    triangles.push_back(Triangle(vertices[4], vertices[5], vertices[1], TUI::Pixel{255, 255, 0, 255}));
    triangles.push_back(Triangle(vertices[1], vertices[0], vertices[4], TUI::Pixel{255, 255, 0, 255}));
    
    // Left face (cyan)
    triangles.push_back(Triangle(vertices[4], vertices[0], vertices[3], TUI::Pixel{0, 255, 255, 255}));
    triangles.push_back(Triangle(vertices[3], vertices[7], vertices[4], TUI::Pixel{0, 255, 255, 255}));
    
    // Right face (magenta)
    triangles.push_back(Triangle(vertices[1], vertices[5], vertices[6], TUI::Pixel{255, 0, 255, 255}));
    triangles.push_back(Triangle(vertices[6], vertices[2], vertices[1], TUI::Pixel{255, 0, 255, 255}));
    
    return triangles;
}

int main() {
    TUI::TUIRenderer renderer{};
    
    // Create a 3D scene
    Scene3D* scene3d = new Scene3D{{0, 0}, {100, 100}};
    
    std::cout << "Creating 3D scene with rotating cubes...\n";
    
    float rotationAngle = 0.0f;
    
    while (true) {
        rotationAngle += 0.02f;
        
        // Clear previous triangles
        scene3d->clearTriangles();
        
        // Create base cube
        std::vector<Triangle> cube = createCube(2.0f);
        
        // Create and add first rotating cube
        std::vector<Triangle> cube1 = transformTriangles(cube, 
            -3.0f, 0.0f, 0.0f,  // Translation
            rotationAngle * 0.5f, rotationAngle, 0.0f  // Rotation
        );
        scene3d->addTriangles(cube1);
        
        // Create and add second rotating cube
        std::vector<Triangle> cube2 = transformTriangles(cube,
            3.0f, 0.0f, 0.0f,  // Translation
            rotationAngle * 0.7f, -rotationAngle, 0.0f  // Rotation
        );
        scene3d->addTriangles(cube2);
        
        // Create and add a third cube in the middle
        std::vector<Triangle> cube3 = transformTriangles(cube,
            0.0f, 0.0f, 3.0f,  // Translation
            0.0f, rotationAngle * 0.3f, rotationAngle  // Rotation
        );
        scene3d->addTriangles(cube3);
        
        // Initialize and draw the scene
        renderer.InitScene(scene3d);
        renderer.drawBuffer();
        
        // Frame rate limiting
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}