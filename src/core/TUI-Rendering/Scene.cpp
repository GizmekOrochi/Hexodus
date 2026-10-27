#include "../../include/TUI-Rendering/Scene.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {

bool samePixel(const TUI::Pixel& lhs, const TUI::Pixel& rhs) noexcept {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

TUI::Pixel alphaBlend(const TUI::Pixel& source, const TUI::Pixel& destination) {
    if (source.a == 0U) {
        return destination;
    }

    if (source.a == 255U) {
        return source;
    }

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

bool Scene::projectPoint(const Geometry::Vector& input, Geometry::Vector& output, int framebufferWidth, int framebufferHeight) const {
    constexpr float nearPlane{1.0f};
    constexpr float farPlane{2000.0f};

    constexpr float pi{3.14159265358979323846f};
    constexpr float verticalFovDegrees{70.0f};
    constexpr float verticalFovRadians{verticalFovDegrees * pi / 180.0f};

    constexpr float terminalPixelRatio{0.5f};

    if (framebufferWidth <= 0 || framebufferHeight <= 0 ||
        input.getZ() < nearPlane || input.getZ() > farPlane) {
        return false;
    }

    const float verticalLimit{std::tan(verticalFovRadians * 0.5f)};
    const float framebufferAspect{static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)};
    const float physicalAspect{framebufferAspect * terminalPixelRatio};
    const float horizontalLimit{verticalLimit * physicalAspect};

    const float focalX{static_cast<float>(framebufferWidth) / (2.0f * horizontalLimit)};
    const float focalY{static_cast<float>(framebufferHeight) / (2.0f * verticalLimit)};
    const float inverseZ{1.0f / input.getZ()};

    output = Geometry::Vector{
        input.getX() * focalX * inverseZ,
        input.getY() * focalY * inverseZ,
        input.getZ(),
        input.getW()
    };

    return true;
}

ClippingPoint Scene::intersectNearPlane(const ClippingPoint& first, const ClippingPoint& second) const {
    constexpr float nearPlane{1.0f};
    constexpr float epsilon{1e-6f};

    const float deltaZ{second.pos.getZ() - first.pos.getZ()};

    if (std::abs(deltaZ) <= epsilon) {
        return ClippingPoint{Geometry::Vector{first.pos.getX(), first.pos.getY(), nearPlane, 1.0f}, first.color};
    }

    const float t{std::clamp((nearPlane - first.pos.getZ()) / deltaZ, 0.0f, 1.0f)};

    const Geometry::Vector position{
        first.pos.getX() + (second.pos.getX() - first.pos.getX()) * t,
        first.pos.getY() + (second.pos.getY() - first.pos.getY()) * t,
        nearPlane,
        1.0f
    };

    const auto interpolateChannel = [t](uint8_t firstChannel, uint8_t secondChannel) {
        const float value{static_cast<float>(firstChannel) + (static_cast<float>(secondChannel) - static_cast<float>(firstChannel)) * t};

        return static_cast<uint8_t>(std::clamp(value, 0.0f, 255.0f));
    };

    return ClippingPoint{
        position,
        TUI::Pixel{
            interpolateChannel(first.color.r, second.color.r),
            interpolateChannel(first.color.g, second.color.g),
            interpolateChannel(first.color.b, second.color.b),
            interpolateChannel(first.color.a, second.color.a)}
    };
}

std::vector<Geometry::Triangle> Scene::clipTriangle(const Geometry::Triangle& triangleValue) const {
    constexpr float nearPlane{1.0f};

    const std::array<ClippingPoint, 3> input{
        ClippingPoint{worldToCamera(triangleValue.getA()), triangleValue.getColorA()},
        ClippingPoint{worldToCamera(triangleValue.getB()), triangleValue.getColorB()},
        ClippingPoint{worldToCamera(triangleValue.getC()), triangleValue.getColorC()}
    };

    std::array<ClippingPoint, 4> clipped{};
    std::size_t clippedCount{0};

    const auto isInside = [](const ClippingPoint& point) {
        return point.pos.getZ() >= nearPlane;
    };

    for (std::size_t index{0}; index < input.size(); ++index) {
        const ClippingPoint& current{input[index]};
        const ClippingPoint& previous{
            input[(index + input.size() - 1U) % input.size()]};

        const bool currentInside{isInside(current)};
        const bool previousInside{isInside(previous)};

        if (!previousInside && currentInside) {
            clipped[clippedCount++] = intersectNearPlane(previous, current);
            clipped[clippedCount++] = current;} else if (previousInside && currentInside) { 
            clipped[clippedCount++] = current;} else if (previousInside && !currentInside) {
            clipped[clippedCount++] = intersectNearPlane(previous, current);}
    }

    if (clippedCount < 3U) {
        return {};
    }

    if (clippedCount == 3U) {
        return {Geometry::Triangle{clipped[0].pos, clipped[1].pos, clipped[2].pos, clipped[0].color, clipped[1].color, clipped[2].color}};
    }

    return {
        Geometry::Triangle{clipped[0].pos, clipped[1].pos, clipped[2].pos, clipped[0].color, clipped[1].color, clipped[2].color},
        Geometry::Triangle{clipped[0].pos,clipped[2].pos, clipped[3].pos, clipped[0].color, clipped[2].color, clipped[3].color}
    };
}

std::vector<Geometry::Triangle> Scene::projectTriangles(int framebufferWidth, int framebufferHeight) const {
    std::vector<Geometry::Triangle> projectedTriangles;

    if (framebufferWidth <= 0 || framebufferHeight <= 0) {
        return projectedTriangles;
    }

    projectedTriangles.reserve(triangles_.size());

    constexpr float nearPlane{1.0f};
    constexpr float farPlane{2000.0f};

    constexpr float pi{3.14159265358979323846f};
    constexpr float verticalFovDegrees{70.0f};
    constexpr float verticalFovRadians{verticalFovDegrees * pi / 180.0f};
    constexpr float normalEpsilonSquared{1e-12f};

    const Geometry::Vector cameraPosition{camera_.getPosition()};

    const float yaw{camera_.getYaw()};
    const float pitch{camera_.getPitch()};

    const float cosYaw{std::cos(-yaw)};
    const float sinYaw{std::sin(-yaw)};
    const float cosPitch{std::cos(-pitch)};
    const float sinPitch{std::sin(-pitch)};

    constexpr float terminalPixelRatio{0.5f};

    const float verticalLimit{std::tan(verticalFovRadians * 0.5f)};

    const float framebufferAspect{ static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)};
    const float physicalAspect{ framebufferAspect * terminalPixelRatio};
    const float horizontalLimit{verticalLimit * physicalAspect};

    const float focalX{static_cast<float>(framebufferWidth) / (2.0f * horizontalLimit)};
    const float focalY{static_cast<float>(framebufferHeight) / (2.0f * verticalLimit)};

    const auto transformToCamera = [&](const Geometry::Vector& worldPosition) {
        const Geometry::Vector relative{worldPosition - cameraPosition};

        const float xAfterYaw{cosYaw * relative.getX() + sinYaw * relative.getZ()};
        const float yAfterYaw{relative.getY()};
        const float zAfterYaw{-sinYaw * relative.getX() + cosYaw * relative.getZ()};

        return Geometry::Vector{xAfterYaw, cosPitch * yAfterYaw - sinPitch * zAfterYaw, sinPitch * yAfterYaw + cosPitch * zAfterYaw, 1.0f};
    };

    const auto interpolateChannel = [](std::uint8_t first, std::uint8_t second, float t) {
        const float value{static_cast<float>(first) + (static_cast<float>(second) - static_cast<float>(first)) * t};

        return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 255.0f));
    };

    const auto intersectNear = [&](const ClippingPoint& first, const ClippingPoint& second) {
        const float deltaZ{second.pos.getZ() - first.pos.getZ()};

        float t{0.0f};

        if (std::abs(deltaZ) > 1e-6f) {
            t = std::clamp((nearPlane - first.pos.getZ()) / deltaZ, 0.0f, 1.0f);
        }

        return ClippingPoint{
            Geometry::Vector{
                first.pos.getX() + (second.pos.getX() - first.pos.getX()) * t,
                first.pos.getY() + (second.pos.getY() - first.pos.getY()) * t,
                nearPlane,
                1.0f},
            TUI::Pixel{
                interpolateChannel(first.color.r, second.color.r, t),
                interpolateChannel(first.color.g, second.color.g, t),
                interpolateChannel(first.color.b, second.color.b, t),
                interpolateChannel(first.color.a, second.color.a, t)}};
    };

const auto project = [&](const Geometry::Vector& cameraPoint) {
    const float inverseZ{1.0f / cameraPoint.getZ()};

    return Geometry::Vector{
        cameraPoint.getX() * focalX * inverseZ,
        cameraPoint.getY() * focalY * inverseZ,
        cameraPoint.getZ(),
        1.0f
    };
};

    for (const Geometry::Triangle& triangleValue : triangles_) {
        const std::array<ClippingPoint, 3> input{
            ClippingPoint{transformToCamera(triangleValue.getA()), triangleValue.getColorA()},
            ClippingPoint{transformToCamera(triangleValue.getB()), triangleValue.getColorB()},
            ClippingPoint{transformToCamera(triangleValue.getC()), triangleValue.getColorC()}};

        if (input[0].pos.getZ() > farPlane && input[1].pos.getZ() > farPlane && input[2].pos.getZ() > farPlane) {
            continue;
        }

        std::array<ClippingPoint, 4> clipped{};
        std::size_t clippedCount{0};

        for (std::size_t index{0}; index < input.size(); ++index) {
            const ClippingPoint& current{input[index]};
            const ClippingPoint& previous{input[(index + input.size() - 1U) % input.size()]};

            const bool currentInside{current.pos.getZ() >= nearPlane};
            const bool previousInside{previous.pos.getZ() >= nearPlane};

            if (!previousInside && currentInside) {
                clipped[clippedCount++] = intersectNear(previous, current);
                clipped[clippedCount++] = current;
            } else if (previousInside && currentInside) {
                clipped[clippedCount++] = current;
            } else if (previousInside && !currentInside) {
                clipped[clippedCount++] = intersectNear(previous, current);
            }
        }

        if (clippedCount < 3U) {
            continue;
        }

        for (std::size_t triangleIndex{1}; triangleIndex + 1U < clippedCount; ++triangleIndex) {
            const ClippingPoint& pointA{clipped[0]};
            const ClippingPoint& pointB{clipped[triangleIndex]};
            const ClippingPoint& pointC{clipped[triangleIndex + 1U]};

            const Geometry::Vector& cameraA{pointA.pos};
            const Geometry::Vector& cameraB{pointB.pos};
            const Geometry::Vector& cameraC{pointC.pos};

            if (backfaceCulling_) {
                const Geometry::Vector edgeAB{cameraB - cameraA};
                const Geometry::Vector edgeAC{cameraC - cameraA};

                const Geometry::Vector normal{edgeAB.Cross(edgeAC)};

                if (normal.lengthSq() <= normalEpsilonSquared) {
                    continue;
                }

                if (normal.Dot(cameraA) >= 0.0f) {
                    continue;
                }
            }

            if (cameraA.getX() < -cameraA.getZ() * horizontalLimit &&
                cameraB.getX() < -cameraB.getZ() * horizontalLimit &&
                cameraC.getX() < -cameraC.getZ() * horizontalLimit) {
                continue;
            }

            if (cameraA.getX() > cameraA.getZ() * horizontalLimit &&
                cameraB.getX() > cameraB.getZ() * horizontalLimit &&
                cameraC.getX() > cameraC.getZ() * horizontalLimit) {
                continue;
            }

            if (cameraA.getY() < -cameraA.getZ() * verticalLimit &&
                cameraB.getY() < -cameraB.getZ() * verticalLimit &&
                cameraC.getY() < -cameraC.getZ() * verticalLimit) {
                continue;
            }

            if (cameraA.getY() > cameraA.getZ() * verticalLimit &&
                cameraB.getY() > cameraB.getZ() * verticalLimit &&
                cameraC.getY() > cameraC.getZ() * verticalLimit) {
                continue;
            }

            projectedTriangles.emplace_back(project(cameraA), project(cameraB), project(cameraC), pointA.color, pointB.color, pointC.color);
        }
    }

    return projectedTriangles;
}

void Scene::renderTriangles( std::vector<TUI::Pixel>& colorBuffer, std::vector<float>& depthBuffer, int width, int height) const {
    if (width <= 0 || height <= 0) {
        return;
    }

    const std::vector<Geometry::Triangle> projectedTriangles{projectTriangles(width, height)};

    const float halfWidth{static_cast<float>(width) * 0.5f};
    const float halfHeight{static_cast<float>(height) * 0.5f};

    constexpr float denominatorEpsilon{1e-6f};
    constexpr float edgeTolerance{-1e-5f};

    for (const Geometry::Triangle& triangleValue : projectedTriangles) {
        const Geometry::Vector a{triangleValue.getA()};
        const Geometry::Vector b{triangleValue.getB()};
        const Geometry::Vector c{triangleValue.getC()};

        const float ax{a.getX() + halfWidth};
        const float ay{halfHeight - a.getY()};

        const float bx{b.getX() + halfWidth};
        const float by{halfHeight - b.getY()};

        const float cx{c.getX() + halfWidth};
        const float cy{halfHeight - c.getY()};

        if ((ax < 0.0f && bx < 0.0f && cx < 0.0f) ||
            (ax >= static_cast<float>(width) && bx >= static_cast<float>(width) && cx >= static_cast<float>(width)) || 
            (ay < 0.0f && by < 0.0f && cy < 0.0f) || 
            (ay >= static_cast<float>(height) && by >= static_cast<float>(height) && cy >= static_cast<float>(height))) {
            continue;}

        const int minX{std::clamp(static_cast<int>(std::floor(std::min({ax, bx, cx}))), 0, width - 1)};
        const int maxX{std::clamp(static_cast<int>(std::ceil(std::max({ax, bx, cx}))), 0, width - 1)};
        const int minY{std::clamp(static_cast<int>(std::floor(std::min({ay, by, cy}))), 0, height - 1)};
        const int maxY{std::clamp(static_cast<int>(std::ceil(std::max({ay, by, cy}))), 0, height - 1)};

        if (minX > maxX || minY > maxY) {
            continue;}

        const float denominator{(by - cy) * (ax - cx) + (cx - bx) * (ay - cy)};

        if (std::abs(denominator) <= denominatorEpsilon) {
            continue;}

        const float inverseDenominator{1.0f / denominator};

        const float weightAStepX{(by - cy) * inverseDenominator};
        const float weightAStepY{(cx - bx) * inverseDenominator};
        const float weightBStepX{(cy - ay) * inverseDenominator};
        const float weightBStepY{(ax - cx) * inverseDenominator};

        const float weightCStepX{-weightAStepX - weightBStepX};
        const float weightCStepY{-weightAStepY - weightBStepY};

        const float startX{static_cast<float>(minX) + 0.5f};
        const float startY{static_cast<float>(minY) + 0.5f};

        float rowWeightA{((by - cy) * (startX - cx) + (cx - bx) * (startY - cy)) * inverseDenominator};
        float rowWeightB{((cy - ay) * (startX - cx) + (ax - cx) * (startY - cy)) * inverseDenominator};
        float rowWeightC{1.0f - rowWeightA - rowWeightB};

        const float inverseZA{1.0f / a.getZ()};
        const float inverseZB{1.0f / b.getZ()};
        const float inverseZC{1.0f / c.getZ()};

        const TUI::Pixel colorA{triangleValue.getColorA()};
        const TUI::Pixel colorB{triangleValue.getColorB()};
        const TUI::Pixel colorC{triangleValue.getColorC()};

        const bool flatColor{samePixel(colorA, colorB) && samePixel(colorA, colorC)};
        const bool flatOpaque{flatColor && colorA.a == 255U};

        for (int y{minY}; y <= maxY; ++y) {
            float weightA{rowWeightA};
            float weightB{rowWeightB};
            float weightC{rowWeightC};

            std::size_t bufferIndex{static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(minX)};

            for (int x{minX}; x <= maxX; ++x, ++bufferIndex) {
                if (weightA >= edgeTolerance && weightB >= edgeTolerance && weightC >= edgeTolerance) {
                    const float interpolatedInverseZ{inverseZA * weightA + inverseZB * weightB + inverseZC * weightC};

                    if (interpolatedInverseZ > 0.0f) {
                        const float depth{1.0f / interpolatedInverseZ};

                        if (depth < depthBuffer[bufferIndex]) {
                            if (flatOpaque) {
                                colorBuffer[bufferIndex] = colorA;
                                depthBuffer[bufferIndex] = depth;
                            }
                            else {
                                TUI::Pixel sourceColor{};

                                if (flatColor) {
                                    sourceColor = colorA;
                                }
                                else {
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

                                    sourceColor = TUI::Pixel{
                                        static_cast<std::uint8_t>(std::clamp(redOverZ * depth, 0.0f, 255.0f)), 
                                        static_cast<std::uint8_t>(std::clamp(greenOverZ * depth, 0.0f, 255.0f)),
                                        static_cast<std::uint8_t>(std::clamp(blueOverZ * depth, 0.0f, 255.0f)),
                                        static_cast<std::uint8_t>(std::clamp(alphaOverZ * depth, 0.0f, 255.0f))
                                    };
                                }

                                if (sourceColor.a == 255U) {
                                    colorBuffer[bufferIndex] = sourceColor;
                                }
                                else if (sourceColor.a != 0U) {
                                    colorBuffer[bufferIndex] = alphaBlend(sourceColor, colorBuffer[bufferIndex]);
                                }

                                // Invisible fragments must not occlude later geometry.
                                if (sourceColor.a != 0U) depthBuffer[bufferIndex] = depth;
                            }
                        }
                    }
                }

                weightA += weightAStepX;
                weightB += weightBStepX;
                weightC += weightCStepX;
            }

            rowWeightA += weightAStepY;
            rowWeightB += weightBStepY;
            rowWeightC += weightCStepY;}
    }
}

void Scene::renderUI(std::vector<TUI::Pixel>& colorBuffer, int width, int height) const {
    if (uiElements_.empty() || width <= 0 || height <= 0) {
        return;
    }

    //Draw lower-priority elements first. The smallest Importance() value
    //remains visible, matching the previous per-pixel selection logic.
    std::vector<std::size_t> order(uiElements_.size());
    std::iota(order.begin(), order.end(), 0U);

    std::sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
        const int lhsImportance{uiElements_[lhs]->Importance()};
        const int rhsImportance{uiElements_[rhs]->Importance()};

        if (lhsImportance != rhsImportance) {
            return lhsImportance > rhsImportance;}

        //For equal importance, the first inserted element must win.
        return lhs > rhs;
    });

    for (const std::size_t elementIndex : order) {
        const UI::UIElement* const element{uiElements_[elementIndex].get()};
        const auto origin{element->Origin()};
        const auto ending{element->Ending()};

        const int minX{std::clamp((origin[0] * width) / 1000, 0, width - 1)};
        const int minY{std::clamp((origin[1] * height) / 1000, 0, height - 1)};
        const int maxX{std::clamp((ending[0] * width) / 1000, 0, width - 1)};
        const int maxY{std::clamp((ending[1] * height) / 1000, 0, height - 1)};

        if (minX > maxX || minY > maxY) {
            continue;}

        TUI::Pixel source{};

        if (const auto* square{dynamic_cast<const UI::UISquare*>(element)}) {
            source = square->Color();} else if (dynamic_cast<const UI::UIString*>(element) != nullptr) {
            source = TUI::Pixel{255, 255, 255, 255};} else {
            continue;}

        if (source.a == 0U) {
            continue;}

        for (int y{minY}; y <= maxY; ++y) {
            std::size_t bufferIndex{static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(minX)};

            for (int x{minX}; x <= maxX; ++x, ++bufferIndex) {
                if (source.a == 255U) {
                    colorBuffer[bufferIndex] = source;
                } else {
                    colorBuffer[bufferIndex] = alphaBlend(source, colorBuffer[bufferIndex]);
                }
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