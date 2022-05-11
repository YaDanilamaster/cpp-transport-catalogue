#include "geo.h"

bool geo::Coordinates::operator==(const Coordinates& other) const
{
    return lat == other.lat && lng == other.lng;
}

bool geo::Coordinates::operator!=(const Coordinates& other) const
{
    return !(*this == other);
}
