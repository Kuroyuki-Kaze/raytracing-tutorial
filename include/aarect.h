#ifndef AARECT_H
#define AARECT_H

#include "rtweekend.h"

#include "hittable.h"

class XYRect: public Hittable {
    public:
        double x0, x1, y0, y1, k;
        shared_ptr<Material> mp;

    public:
        XYRect() {}
        XYRect(double _x0, double _x1, double _y0, double _y1, double _k, shared_ptr<Material> mat):
            x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {}

        virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
            output_box = Aabb(Point3(x0, y0, k - 0.0001), Point3(x1, y1, k + 0.0001));
            return true;
        }
};

class XZRect: public Hittable {
    public:
        double x0, x1, z0, z1, k;
        shared_ptr<Material> mp;
    
    public:
        XZRect() {}
        XZRect(double _x0, double _x1, double _z0, double _z1, double _k, shared_ptr<Material> mat):
            x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {}

        virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
            output_box = Aabb(Point3(x0, k - 0.0001, z0), Point3(x1, k + 0.0001, z1));
            return true;
        }

        virtual double pdf_value(const Point3 &origin, const Vec3 &v) const override {
            HitRecord rec;
            if (!this->hit(Ray(origin, v), 0.001, INF, rec)) {
                return 0;
            }

            auto area = (x1 - x0) * (z1 - z0);
            auto distance_squared = rec.t * rec.t * v.length_squared();
            auto cosine = fabs(dot(v, rec.normal) / v.length());

            return distance_squared / (cosine * area);
        }

        virtual Vec3 random(const Point3 &origin) const override {
            auto random_point = Point3(random_double2(x0, x1), k, random_double2(z0, z1));
            return random_point - origin;
        }
};

class YZRect: public Hittable {
    public:
        double y0, y1, z0, z1, k;
        shared_ptr<Material> mp;
    
    public:
        YZRect() {}
        YZRect(double _y0, double _y1, double _z0, double _z1, double _k, shared_ptr<Material> mat):
            y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {}

        virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
            output_box = Aabb(Point3(k - 0.0001, y0, z0), Point3(k + 0.0001, y1, z1));
            return true;
        }
};

bool XYRect::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    auto t = (k - r.origin().z()) / r.direction().z();
    if (t < t_min || t > t_max) {
        return false;
    }

    auto x = r.origin().x() + t * r.direction().x();
    auto y = r.origin().y() + t * r.direction().y();

    if (x < x0 || x > x1 || y < y0 || y > y1) {
        return false;
    }

    rec.u = (x - x0) / (x1 - x0);
    rec.v = (y - y0) / (y1 - y0);
    rec.t = t;

    auto outward_normal = Vec3(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool XZRect::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    auto t = (k - r.origin().y()) / r.direction().y();
    if (t < t_min || t > t_max) {
        return false;
    }

    auto x = r.origin().x() + t * r.direction().x();
    auto z = r.origin().z() + t * r.direction().z();

    if (x < x0 || x > x1 || z < z0 || z > z1) {
        return false;
    }

    rec.u = (x - x0) / (x1 - x0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;

    auto outward_normal = Vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool YZRect::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    auto t = (k - r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max) {
        return false;
    }

    auto y = r.origin().y() + t * r.direction().y();
    auto z = r.origin().z() + t * r.direction().z();

    if (y < y0 || y > y1 || z < z0 || z > z1) {
        return false;
    }

    rec.u = (y - y0) / (y1 - y0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;

    auto outward_normal = Vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

#endif