#pragma once

#include "Just/Common.h"
#include "Just/Math/Vector.h"

struct Ray
{
    Point3f origin;
    Vector3f direction;
    mutable float tMax;
    mutable float tMin;
    Ray() : tMax(std::numeric_limits<float>::max()), tMin(kEpsilon) {}
    Ray(const Point3f &origin, const Vector3f &direction) :
            origin(origin), direction(Normalize(direction)), tMax(std::numeric_limits<float>::max()), tMin(kEpsilon) {}
    ~Ray() = default;
    Point3f At(float t) const { return origin + direction * t; }
};