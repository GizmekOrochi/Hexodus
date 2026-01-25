#include "../../include/TUI-Rendering/Scene2D.hpp"
#include <iostream>

Scene2D::Scene2D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending) 
    : Scene{origin, ending}, SceneElements_{} {}

    
UIElement& Scene2D::Element(size_t index) {
    return *SceneElements_.at(index);
}

const UIElement& Scene2D::Element(size_t index) const {
    return *SceneElements_.at(index);
}

void Scene2D::add(std::unique_ptr<UIElement> element) {
    SceneElements_.push_back(std::move(element));
}

std::vector<TUI::Pixel> Scene2D::convertScene(int outputHeight, int outputLength) {

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
            
            for (const auto& element : SceneElements_) {
                // Skip elements with higher importance value than current best
                if (element->Importance() > currentBestImportance) {
                    continue;
                }
                
                std::array<int, 2> elementOrigin = element->Origin();
                std::array<int, 2> elementEnding = element->Ending();
                
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
                if (auto* square = dynamic_cast<UISquare*>(element.get())) {
                    //Igninor when alpha channel == 0
                    if (square->Color().a == 0) continue;

                    currentBestImportance = element->Importance();
                    currentBestColor = square->Color();
                    foundElement = true;
                }
                /* For UIString elements NOT WORKING FOR NOW /!\ */
                else if (auto* str = dynamic_cast<UIString*>(element.get())) {
                    // For now, we just put a black pixel because the lower level is not ready to handle other pixel that █ or " "
                    if (element->Importance() < currentBestImportance) {
                        currentBestImportance = element->Importance();
                        // Default text color (white with black background perhaps)
                        currentBestColor = TUI::Pixel{255, 255, 255, 255};
                        foundElement = true;
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
    
    return SceneBuffer;
}