#include "../../include/TUI-Rendering/Scene3D.hpp"
#include "../../include/util/Logger.hpp"

using namespace Geometry;

Scene3D::Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending) : Scene(origin, ending), camera_() {

}

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

std::vector<TUI::Pixel> Scene3D::convertScene(int outputHeight, int outputLength) {
/*
    int SceneOrigin_X = (this->Origin()[0] * outputLength) / 100;
    int SceneOrigin_Y = (this->Origin()[1] * outputHeight) / 100;
    
    int SceneEnding_X = (this->Ending()[0] * outputLength) / 100;
    int SceneEnding_Y = (this->Ending()[1] * outputHeight) / 100;
    
    // used for the loop to start from 0,0
    int maxXBuffer = SceneEnding_X - SceneOrigin_X;
    int maxYBuffer = SceneEnding_Y - SceneOrigin_Y;

    std::vector<TUI::Pixel> SceneBuffer{};
    
    for (int y = 0; y < maxYBuffer; y++) {
        for (int x = 0; x < maxXBuffer; x++) {
            // Find the element with highest importance (lowest importance value) at this pixel
            int currentBestImportance = INT_MAX;
            TUI::Pixel currentBestColor;
            bool foundElement = false;
            
            for (const auto& triangle : ObjectList) {
                // Skip elements with higher importance value than current best
                if (triangle->Importance() > currentBestImportance) {
                    continue;
                }
                
                std::array<int, 2> elementOrigin = triangle->Origin();
                std::array<int, 2> elementEnding = triangle->Ending();
                
                // Convert element coordinates to Pixel screen coordinates
                int elemOriginX = (elementOrigin[0] * maxXBuffer) / 1000;
                int elemOriginY = (elementOrigin[1] * maxYBuffer) / 1000;
                int elemEndingX = (elementEnding[0] * maxXBuffer) / 1000;
                int elemEndingY = (elementEnding[1] * maxYBuffer) / 1000;

                // if current pixel before first element pixel don't care
                if (x < elemOriginX || y < elemOriginY) continue;
                // if current pixel after last element pixel don't care
                if (x >  elemEndingX || y > elemEndingY) continue;

                // Try to cast to UISquare to get color
                if (auto* square = dynamic_cast<UISquare*>(triangle.get())) {
                    //Igninor when alpha channel == 0
                    if (square->Color().a == 0) continue;

                    currentBestImportance = triangle->Importance();
                    currentBestColor = square->Color();
                    foundElement = true;
                }
            if (foundElement) {
                SceneBuffer.push_back(currentBestColor);
            } else {
                SceneBuffer.push_back(this->Color());
            }
        }
    }
    
    return SceneBuffer;
}
*/
}