#pragma once

#include "Geometry.hpp"

#include <optional>

namespace haru::engine::graphics {

class ViewportScaler {
public:
    explicit ViewportScaler(Size designSize);

    Size designSize() const;
    Rect presentationRect(Size targetSize, int scalePercent = 100) const;
    std::optional<Point> mapPointToDesign(Point targetPoint,
                                          Size targetSize,
                                          int scalePercent = 100) const;

private:
    Size designSize_;
};

} // namespace haru::engine::graphics
