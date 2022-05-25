#pragma once

#include <cmath>
namespace geo {
    const int RADIUS_EARTH = 6371000;

    struct Coordinates {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
    };

    inline double ComputeDistance(Coordinates from, Coordinates to);
}