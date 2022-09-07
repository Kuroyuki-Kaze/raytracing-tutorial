#ifndef RAY_H
#define RAY_H

#include "vec3.h"
class Ray {
    public:
        Point3 orig;
        Vec3 dir;
        double tm;

    public:
        Ray() {}
        Ray(const Point3 &origin, const Vec3 &direction, double time = 0.0) : orig(origin), dir(direction), tm(time) {}

        Point3 origin() const { return orig; }
        Vec3 direction() const { return dir; }
        double time() const { return tm; }

        Point3 at(double t) const {
            return orig + t * dir;
        }
};

#endif