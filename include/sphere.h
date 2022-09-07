#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class Sphere : public Hittable {
    public:
        Point3 center;
        double radius;
        shared_ptr<Material> mat_ptr;

    public:
        Sphere() {}
        Sphere(Point3 cen, double r, shared_ptr<Material> m): center(cen), radius(r), mat_ptr(m) {};

        virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
        virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;
        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;
        virtual Vec3 random(const Point3 &o) const override;

    private:
        static void get_sphere_uv(const Point3 &p, double &u, double &v) {
            // p: a point on the sphere of radius 1, centered at the origin.
            // u: returned value [0,1] of angle around the Y axis from X=-1.
            // v: returned value [0,1] of angle from Y=-1 to Y=+1.
            // <1 0 0> -> u = 0.5, v = 0.5
            // <-1 0 0> -> u = 0, v = 0.5
            // <0 1 0> -> u = 0.5, v = 1.0
            // <0 -1 0> -> u = 0.5, v = 0.0
            // <0 0 1> -> u = 0.25, v = 0.5
            // <0 0 -1> -> u = 0.75, v = 0.5

            auto theta = acos(-p.y());
            auto phi = atan2(-p.z(), p.x()) + PI;

            u = phi / (2 * PI);
            v = theta / PI;
        }
};

double Sphere::pdf_value(const Point3 &o, const Vec3 &v) const {
    HitRecord rec;
    if (!this->hit(Ray(o, v), 0.001, INF, rec)) {
        return 0;
    }

    auto cos_theta_max = sqrt(1 - radius * radius / (center - o).length_squared());
    auto solid_angle = 2 * PI * (1 - cos_theta_max);

    return 1 / solid_angle;
}

bool Sphere::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    Vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0) return false;

    auto sqrtd = sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root) {
            return false;
        }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat_ptr = mat_ptr;

    return true;
}

bool Sphere::bounding_box(double time0, double time1, Aabb &output_box) const {
    output_box = Aabb(
        center - Vec3(radius, radius, radius),
        center + Vec3(radius, radius, radius)
    );

    return true;
}

Vec3 Sphere::random(const Point3 &o) const {
    Vec3 direction = center - o;
    auto distance_squared = direction.length_squared();
    Onb uvw;
    uvw.build_from_w(direction);
    return uvw.local(random_to_sphere(radius, distance_squared));
}

#endif