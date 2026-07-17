#include "../../include/TUI-Rendering/Scene.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

TUI::Pixel alphaBlend(const TUI::Pixel& source, const TUI::Pixel& destination) {
    const float alpha{static_cast<float>(source.a) / 255.0f};
    const float inverseAlpha{1.0f - alpha};

    return TUI::Pixel{
        static_cast<uint8_t>(static_cast<float>(source.r) * alpha + static_cast<float>(destination.r) * inverseAlpha),
        static_cast<uint8_t>(static_cast<float>(source.g) * alpha + static_cast<float>(destination.g) * inverseAlpha),
        static_cast<uint8_t>( static_cast<float>(source.b) * alpha + static_cast<float>(destination.b) * inverseAlpha),
        255
    };
}

} // namespace

Scene::Scene(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending)
    : origin_{origin},
      ending_{ending},
      backgroundColor_{0, 0, 0, 255},
      triangles_{},
      uiElements_{},
      camera_{},
      backfaceCulling_{false} {}

void Scene::setBackground(TUI::Pixel color) {
    backgroundColor_ = color;
}

TUI::Pixel Scene::Color() const {
    return backgroundColor_;
}

std::array<uint8_t, 2> Scene::Origin() const {
    return origin_;
}

std::array<uint8_t, 2> Scene::Ending() const {
    return ending_;
}

void Scene::Origin(const std::array<uint8_t, 2>& origin) {
    origin_ = origin;
}

void Scene::Ending(const std::array<uint8_t, 2>& ending) {
    ending_ = ending;
}

std::size_t Scene::addTriangle(const Geometry::Triangle& triangleValue) {
    triangles_.push_back(triangleValue);
    return triangles_.size() - 1;
}

Geometry::Triangle& Scene::triangle(std::size_t index) {
    return triangles_.at(index);
}

const Geometry::Triangle& Scene::triangle(std::size_t index) const {
    return triangles_.at(index);
}

void Scene::clearTriangles() {
    triangles_.clear();
}

std::size_t Scene::addUIElement(std::unique_ptr<UI::UIElement> element) {
    if (!element) {
        throw std::invalid_argument("Scene::addUIElement(): element is null");
    }

    uiElements_.push_back(std::move(element));
    return uiElements_.size() - 1;
}

std::size_t Scene::add(std::unique_ptr<UI::UIElement> element) {
    return addUIElement(std::move(element));
}

UI::UIElement& Scene::uiElement(std::size_t index) {
    return *uiElements_.at(index);
}

const UI::UIElement& Scene::uiElement(std::size_t index) const {
    return *uiElements_.at(index);
}

void Scene::clearUIElements() {
    uiElements_.clear();
}

Rendering::Camera& Scene::getCamera() {
    return camera_;
}

const Rendering::Camera& Scene::getCamera() const {
    return camera_;
}

void Scene::setBackfaceCulling(bool enabled) {
    backfaceCulling_ = enabled;
}

void Scene::toggleBackfaceCulling() {
    backfaceCulling_ = !backfaceCulling_;
}

bool Scene::backfaceCullingEnabled() const {
    return backfaceCulling_;
}

Geometry::Vector Scene::worldToCamera(const Geometry::Vector& position) const {
    const Geometry::Vector relative = position - camera_.getPosition();

    const float yaw{camera_.getYaw()};
    const float pitch{camera_.getPitch()};

    const float cosYaw{std::cos(-yaw)};
    const float sinYaw{std::sin(-yaw)};

    const float xAfterYaw{cosYaw * relative.getX() + sinYaw * relative.getZ()};
    const float yAfterYaw{relative.getY()};
    const float zAfterYaw{-sinYaw * relative.getX() + cosYaw * relative.getZ()};

    const float cosPitch{std::cos(-pitch)};
    const float sinPitch{std::sin(-pitch)};

    const float xAfterPitch{xAfterYaw};
    const float yAfterPitch{cosPitch * yAfterYaw - sinPitch * zAfterYaw};
    const float zAfterPitch{sinPitch * yAfterYaw + cosPitch * zAfterYaw};

    return Geometry::Vector{xAfterPitch, yAfterPitch, zAfterPitch, 1.0f};
}

bool Scene::projectPoint(const Geometry::Vector& input, Geometry::Vector& output) const {
    constexpr float nearPlane{1.0f};
    constexpr float farPlane{2000.0f};
    constexpr float focalLength{30.0f};

    if (input.getZ() < nearPlane || input.getZ() > farPlane) {
        return false;
    }

    output = Geometry::Vector{
        input.getX() * focalLength / input.getZ(),
        input.getY() * focalLength / input.getZ(),
        input.getZ(), input.getW()
    };

    return true;
}

ClippingPoint Scene::intersectNearPlane(const ClippingPoint& first, const ClippingPoint& second) const {
    constexpr float nearPlane{1.0f};

    const float zDistance = second.pos.getZ() - first.pos.getZ();
    if (std::abs(zDistance) < 1e-6f) {
        return first;
    }

    const float t{(nearPlane - first.pos.getZ()) / zDistance};

    Geometry::Vector position{
        first.pos.getX() + t * (second.pos.getX() - first.pos.getX()),
        first.pos.getY() + t * (second.pos.getY() - first.pos.getY()),
        nearPlane,
        1.0f
    };

    const auto interpolateChannel = [t](uint8_t a, uint8_t b) {
        return static_cast<uint8_t>(static_cast<float>(a) + t * (static_cast<float>(b) - static_cast<float>(a)));
    };

    TUI::Pixel color{
        interpolateChannel(first.color.r, second.color.r),
        interpolateChannel(first.color.g, second.color.g),
        interpolateChannel(first.color.b, second.color.b),
        interpolateChannel(first.color.a, second.color.a)
    };

    return ClippingPoint{position, color};
}

std::vector<Geometry::Triangle> Scene::clipTriangle(const Geometry::Triangle& triangleValue) const {
    constexpr float nearPlane{1.0f};

    ClippingPoint pointA{worldToCamera(triangleValue.getA()), triangleValue.getColorA()};
    ClippingPoint pointB{worldToCamera(triangleValue.getB()), triangleValue.getColorB()};
    ClippingPoint pointC{worldToCamera(triangleValue.getC()), triangleValue.getColorC()};

    std::vector<ClippingPoint> inside;
    std::vector<ClippingPoint> outside;

    const auto classify = [&](const ClippingPoint& point) {
        if (point.pos.getZ() >= nearPlane) {
            inside.push_back(point);
        } else {
            outside.push_back(point);
        }
    };

    classify(pointA);
    classify(pointB);
    classify(pointC);

    if (inside.empty()) {
        return {};
    }

    if (inside.size() == 3) {
        return {Geometry::Triangle{inside[0].pos, inside[1].pos, inside[2].pos, inside[0].color, inside[1].color, inside[2].color}};
    }

    if (inside.size() == 1) {
        const ClippingPoint intersection1{intersectNearPlane(inside[0], outside[0])};
        const ClippingPoint intersection2{intersectNearPlane(inside[0], outside[1])};

        return {Geometry::Triangle{inside[0].pos, intersection1.pos, intersection2.pos, inside[0].color, intersection1.color, intersection2.color}};
    }

    const ClippingPoint intersection1{intersectNearPlane(inside[0], outside[0])};
    const ClippingPoint intersection2{intersectNearPlane(inside[1], outside[0])};

    return {
        Geometry::Triangle{inside[0].pos, inside[1].pos, intersection1.pos, inside[0].color, inside[1].color, intersection1.color},
        Geometry::Triangle{inside[1].pos, intersection2.pos, intersection1.pos, inside[1].color, intersection2.color, intersection1.color}
    };
}

std::vector<Geometry::Triangle> Scene::projectTriangles() const {
    std::vector<Geometry::Triangle> projectedTriangles;

    for (const Geometry::Triangle& triangleValue : triangles_) {
        const std::vector<Geometry::Triangle> clippedTriangles{clipTriangle(triangleValue)};

        for (const Geometry::Triangle& clippedTriangle : clippedTriangles) {
            Geometry::Vector projectedA;
            Geometry::Vector projectedB;
            Geometry::Vector projectedC;

            if (!projectPoint(clippedTriangle.getA(), projectedA) || !projectPoint(clippedTriangle.getB(), projectedB) || !projectPoint(clippedTriangle.getC(), projectedC)) {
                continue;
            }

            if (backfaceCulling_) {
                const float signedArea{(projectedB.getX() - projectedA.getX()) * (projectedC.getY() - projectedA.getY()) - (projectedB.getY() - projectedA.getY()) * (projectedC.getX() - projectedA.getX())};

                if (signedArea >= 0.0f) {
                    continue;
                }
            }

            projectedTriangles.emplace_back(projectedA, projectedB, projectedC, clippedTriangle.getColorA(), clippedTriangle.getColorB(), clippedTriangle.getColorC());
        }
    }

    return projectedTriangles;
}

void Scene::renderTriangles(std::vector<TUI::Pixel>& colorBuffer, std::vector<float>& depthBuffer, int width, int height) const {
    const std::vector<Geometry::Triangle> projectedTriangles{projectTriangles()};

    const float halfWidth{static_cast<float>(width) * 0.5f};
    const float halfHeight{static_cast<float>(height) * 0.5f};

    for (const Geometry::Triangle& triangleValue : projectedTriangles) {
        const Geometry::Vector a{triangleValue.getA()};
        const Geometry::Vector b{triangleValue.getB()};
        const Geometry::Vector c{triangleValue.getC()};

        //Projected coordinates -> framebuffer coordinates.
        const float ax{a.getX() + halfWidth};
        const float ay{halfHeight - a.getY()};

        const float bx{b.getX() + halfWidth};
        const float by{halfHeight - b.getY()};

        const float cx{c.getX() + halfWidth};
        const float cy{halfHeight - c.getY()};

        //Only visit pixels inside the triangle's rectangular area.
        const int minX{std::clamp(static_cast<int>(std::floor(std::min({ax, bx, cx}))), 0, width - 1)};
        const int maxX{std::clamp(static_cast<int>(std::ceil(std::max({ax, bx, cx})) ), 0, width - 1)};
        const int minY{std::clamp(static_cast<int>(std::floor(std::min({ay, by, cy}))), 0, height - 1)};
        const int maxY{std::clamp(static_cast<int>(std::ceil(std::max({ay, by, cy}))), 0, height - 1)};

        if (minX > maxX || minY > maxY) {
            continue;
        }

        //Barycentric denominator in framebuffer coordinates.
        const float denominator{(by - cy) * (ax - cx) + (cx - bx) * (ay - cy)};

        if (std::abs(denominator) < 1e-6f) {
            continue;
        }

        const float inverseDenominator{1.0f / denominator};

        const float inverseZA{1.0f / a.getZ()};
        const float inverseZB{1.0f / b.getZ()};
        const float inverseZC{1.0f / c.getZ()};

        const TUI::Pixel colorA{triangleValue.getColorA()};
        const TUI::Pixel colorB{triangleValue.getColorB()};
        const TUI::Pixel colorC{triangleValue.getColorC()};

        for (int y{minY}; y <= maxY; ++y) {
            const float pixelY{static_cast<float>(y) + 0.5f};

            std::size_t bufferIndex{static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(minX)};

            for (int x{minX}; x <= maxX; ++x, ++bufferIndex) {
                const float pixelX{static_cast<float>(x) + 0.5f};

                const float weightA{((by - cy) * (pixelX - cx) + (cx - bx) * (pixelY - cy)) * inverseDenominator};
                const float weightB{((cy - ay) * (pixelX - cx) + (ax - cx) * (pixelY - cy)) * inverseDenominator};
                const float weightC{1.0f - weightA - weightB};

                //Works for either triangle winding direction.
                constexpr float edgeTolerance{-1e-5f};

                if (weightA < edgeTolerance || weightB < edgeTolerance || weightC < edgeTolerance) {
                    continue;
                }

                const float interpolatedInverseZ{inverseZA * weightA + inverseZB * weightB + inverseZC * weightC};

                if (interpolatedInverseZ <= 0.0f) {
                    continue;
                }

                const float depth{1.0f / interpolatedInverseZ};

                if (depth >= depthBuffer[bufferIndex]) {
                    continue;
                }

                const float redOverZ{
                    static_cast<float>(colorA.r) * inverseZA * weightA +
                    static_cast<float>(colorB.r) * inverseZB * weightB +
                    static_cast<float>(colorC.r) * inverseZC * weightC
                };

                const float greenOverZ{
                    static_cast<float>(colorA.g) * inverseZA * weightA +
                    static_cast<float>(colorB.g) * inverseZB * weightB +
                    static_cast<float>(colorC.g) * inverseZC * weightC
                };

                const float blueOverZ{
                    static_cast<float>(colorA.b) * inverseZA * weightA +
                    static_cast<float>(colorB.b) * inverseZB * weightB +
                    static_cast<float>(colorC.b) * inverseZC * weightC
                };

                const float alphaOverZ{
                    static_cast<float>(colorA.a) * inverseZA * weightA +
                    static_cast<float>(colorB.a) * inverseZB * weightB +
                    static_cast<float>(colorC.a) * inverseZC * weightC
                };

                const TUI::Pixel sourceColor{
                    static_cast<uint8_t>(std::clamp(redOverZ * depth, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(greenOverZ * depth, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(blueOverZ * depth, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(alphaOverZ * depth, 0.0f, 255.0f))
                };

                colorBuffer[bufferIndex] = alphaBlend(sourceColor, colorBuffer[bufferIndex]);

                depthBuffer[bufferIndex] = depth;
            }
        }
    }
}

void Scene::renderUI(std::vector<TUI::Pixel>& colorBuffer, int width, int height) const {
    for (int y{}; y < height; ++y) {
        for (int x{}; x < width; ++x) {
            int bestImportance{std::numeric_limits<int>::max()};
            const UI::UIElement* bestElement{nullptr};

            for (const auto& element : uiElements_) {
                const std::array<int, 2> elementOrigin{element->Origin()};
                const std::array<int, 2> elementEnding{element->Ending()};

                const int elementOriginX{(elementOrigin[0] * width) / 1000};
                const int elementOriginY{(elementOrigin[1] * height) / 1000};
                const int elementEndingX{(elementEnding[0] * width) / 1000};
                const int elementEndingY{(elementEnding[1] * height) / 1000};

                if (x < elementOriginX || x > elementEndingX || y < elementOriginY || y > elementEndingY) {
                    continue;
                }

                if (element->Importance() >= bestImportance) {
                    continue;
                }

                bestImportance = element->Importance();
                bestElement = element.get();
            }

            if (bestElement == nullptr) {
                continue;
            }

            const std::size_t bufferIndex{static_cast<std::size_t>(y * width + x)};

            if (const auto* square{dynamic_cast<const UI::UISquare*>(bestElement)}) {
                if (square->Color().a == 0) {
                    continue;
                }

                colorBuffer[bufferIndex] = alphaBlend(square->Color(), colorBuffer[bufferIndex]);
            } else if (dynamic_cast<const UI::UIString*>(bestElement) != nullptr) {

                colorBuffer[bufferIndex] = alphaBlend(TUI::Pixel{255, 255, 255, 255}, colorBuffer[bufferIndex]);
            }
        }
    }
}

std::vector<TUI::Pixel> Scene::convertScene( int outputHeight, int outputLength) const {
    const int sceneOriginX{(static_cast<int>(origin_[0]) * outputLength) / 100};
    const int sceneOriginY{(static_cast<int>(origin_[1]) * outputHeight) / 100};
    const int sceneEndingX{(static_cast<int>(ending_[0]) * outputLength) / 100};
    const int sceneEndingY{(static_cast<int>(ending_[1]) * outputHeight) / 100};

    const int width{sceneEndingX - sceneOriginX};
    const int height{sceneEndingY - sceneOriginY};

    if (width <= 0 || height <= 0) {
        return {};
    }

    const std::size_t bufferSize{static_cast<std::size_t>(width) * static_cast<std::size_t>(height)};

    std::vector<TUI::Pixel> colorBuffer(bufferSize, backgroundColor_);
    std::vector<float> depthBuffer(bufferSize, std::numeric_limits<float>::max());

    renderTriangles(colorBuffer, depthBuffer, width, height);
    renderUI(colorBuffer, width, height);

    return colorBuffer;
}
