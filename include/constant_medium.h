#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "rtweekend.h"

#include "hittable.h"
#include "material.h"
#include "texture.h"

class ConstantMedium : public Hittable {
    public:
        shared_ptr<Hittable> boundary;
        double neg_inv_density;
        shared_ptr<Material> phase_function;

    public:
        ConstantMedium(shared_ptr<Hittable> b, double d, shared_ptr<Texture> a) : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<Isotropic>(a)) {}
        ConstantMedium(shared_ptr<Hittable> b, double d, Color c) : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<Isotropic>(c)) {}

        virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
            return boundary->bounding_box(time0, time1, output_box);
        }
};

bool ConstantMedium::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    const bool enableDebug = false;
    const bool debugging = enableDebug && random_double2() < 0.00001;

    HitRecord rec1, rec2;

    if (!boundary->hit(r, -INF, INF, rec1)) {
        return false;
    }

    if (!boundary->hit(r, rec1.t+0.0001, INF, rec2)) {
        return false;
    }

    if (debugging) std::cerr << "\nt_min=" << rec1.t << "t_max=" << rec2.t << '\n';

    if (rec1.t < t_min) rec1.t = t_min;
    if (rec2.t > t_max) rec2.t = t_max;

    if (rec1.t >= rec2.t) {
        return false;
    }

    if (rec1.t < 0) {
        rec1.t = 0;
    }

    const auto ray_length = r.direction().length();
    const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
    const auto hit_distance = neg_inv_density * log(random_double2());

    if (hit_distance > distance_inside_boundary) {
        return false;
    }

    rec.t = rec1.t + hit_distance / ray_length;
    rec.p = r.at(rec.t);

    if (debugging) {
        std::cerr << "hit_distance = " << hit_distance << '\n'
            << "rec.t = " << rec.t << '\n'
            << "rec.p = " << rec.p << '\n';
    }

    rec.normal = Vec3(1, 0, 0);
    rec.front_face = true;
    rec.mat_ptr = phase_function;

    return true;
}

#endif