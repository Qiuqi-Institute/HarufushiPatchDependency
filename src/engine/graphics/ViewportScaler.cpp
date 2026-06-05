#include "engine/graphics/ViewportScaler.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace haru::engine::graphics {

ViewportScaler::ViewportScaler(Size designSize) : designSize_(designSize) {
    if (designSize.width <= 0 || designSize.height <= 0) {
        throw std::invalid_argument("viewport design size must be positive");
    }
}

Size ViewportScaler::designSize() const {
    return designSize_;
}

Rect ViewportScaler::presentationRect(Size targetSize, int scalePercent) const {
    if (targetSize.width <= 0 || targetSize.height <= 0) {
        return {0, 0, 0, 0};
    }

    const double fitScale = std::min(static_cast<double>(targetSize.width) /
                                         static_cast<double>(designSize_.width),
                                     static_cast<double>(targetSize.height) /
                                         static_cast<double>(designSize_.height));
    const double requestedScale =
        fitScale * (static_cast<double>(std::clamp(scalePercent, 10, 200)) / 100.0);
    const double scale = std::min(std::max(requestedScale, 0.0), fitScale);
    const int width = std::max(1, static_cast<int>(std::round(designSize_.width * scale)));
    const int height = std::max(1, static_cast<int>(std::round(designSize_.height * scale)));
    return {(targetSize.width - width) / 2, (targetSize.height - height) / 2, width, height};
}

std::optional<Point> ViewportScaler::mapPointToDesign(Point targetPoint,
                                                      Size targetSize,
                                                      int scalePercent) const {
    const Rect rect = presentationRect(targetSize, scalePercent);
    if (rect.width <= 0 || rect.height <= 0 || targetPoint.x < rect.x ||
        targetPoint.y < rect.y || targetPoint.x >= rect.x + rect.width ||
        targetPoint.y >= rect.y + rect.height) {
        return std::nullopt;
    }

    return Point{((targetPoint.x - rect.x) * designSize_.width) / rect.width,
                 ((targetPoint.y - rect.y) * designSize_.height) / rect.height};
}

} // namespace haru::engine::graphics
