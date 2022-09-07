#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"

#include "hittable.h"
#include "onb.h"

/// @brief Random Vector with cosine distribution
/// @return A random vector
inline Vec3 random_cosine_direction() {
    auto r1 = random_double2();
    auto r2 = random_double2();
    auto z = sqrt(1 - r2);

    auto phi = 2 * PI * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);

    return Vec3(x, y, z);
}

inline Vec3 random_to_sphere(double radius, double distance_squared) {
    auto r1 = random_double2();
    auto r2 = random_double2();
    auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

    auto phi = 2 * PI * r1;
    auto x = cos(phi) * sqrt(1 - z * z);
    auto y = sin(phi) * sqrt(1 - z * z);

    return Vec3(x, y, z);
}

class Pdf {
    public:

    public:
        virtual ~Pdf() {}

        virtual double value(__F_IN__ const Vec3 &direction) const = 0;
        virtual Vec3 generate() const = 0;
};

class CosinePdf : public Pdf {
    public:
        Onb uvw;

    public:
        CosinePdf(
            __F_IN__ const Vec3 &w
        ) { uvw.build_from_w(w); }

        virtual double value(
            __F_IN__ const Vec3 &direction
        ) const override {
            auto cosine = dot(normal(direction), uvw.w());
            return (cosine <= 0) ? 0 : cosine / PI;
        }

        virtual Vec3 generate() const override {
            return uvw.local(random_cosine_direction());
        }
};

class HittablePdf : public Pdf {
    public:
        Point3 o;
        shared_ptr<Hittable> ptr;

    public:
        HittablePdf(
            __F_IN__ const Point3 &origin,
            __F_IN__ shared_ptr<Hittable> p
        ) : o(origin), ptr(p) {}

        virtual double value(
            __F_IN__ const Vec3 &direction
        ) const override {
            return ptr->pdf_value(o, direction);
        }

        virtual Vec3 generate() const override {
            return ptr->random(o);
        }
};

class MixturePdf : public Pdf {
    public:
        shared_ptr<Pdf> p[2];

    public:
        MixturePdf(
            __F_IN__ shared_ptr<Pdf> p0,
            __F_IN__ shared_ptr<Pdf> p1
        ) {
            p[0] = p0;
            p[1] = p1;
        }

        virtual double value(const Vec3 &direction) const override {
            return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
        }

        virtual Vec3 generate() const override {
            if (random_double2() < 0.5) {
                return p[0]->generate();
            } else {
                return p[1]->generate();
            }
        }
};

#endif