#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"

#include "aabb.h"

class Material;

struct HitRecord {
    Point3 p;
    Vec3 normal;
    shared_ptr<Material> mat_ptr;
    double t;
    double u;
    double v;
    bool front_face;

    inline void set_face_normal(const Ray &r, const Vec3 &outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
    public:
        virtual bool hit(
            __F_IN__ const Ray &r, 
            __F_IN__ double t_min,
            __F_IN__ double t_max,
            __F_OUT__ HitRecord &rec
        ) const = 0;

        virtual bool bounding_box(
            __F_IN__ double time0,
            __F_IN__ double time1,
            __F_OUT__ Aabb &output_box
        ) const = 0;

        virtual double pdf_value(
            __F_IN__ const Point3 &o,
            __F_IN__ const Vec3 &v
        ) const {
            return 0.0;
        }

        virtual Vec3 random(
            __F_IN__ const Vec3 &o
        ) const {
            return Vec3(1, 0, 0);
        }
};

class Translate : public Hittable {
    public:
        shared_ptr<Hittable> ptr;
        Vec3 offset;

    public:
        Translate(shared_ptr<Hittable> p, const Vec3 &displacement) : ptr(p), offset(displacement) {}

        virtual bool hit(
            const Ray &r, double t_min, double t_max, HitRecord &rec
        ) const override;

        virtual bool bounding_box(
            double time0, double time1, Aabb &output_box
        ) const override;
};

bool Translate::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    Ray moved_r(r.origin() - offset, r.direction(), r.time());
    if (!ptr->hit(moved_r, t_min, t_max, rec)) {
        return false;
    }

    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

bool Translate::bounding_box(double time0, double time1, Aabb &output_box) const {
    if (!ptr->bounding_box(time0, time1, output_box)) {
        return false;
    }

    output_box = Aabb(
        output_box.min() + offset,
        output_box.max() + offset
    );

    return true;
}

class RotateX: public Hittable {
    public:
        shared_ptr<Hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        Aabb bbox;

    public:
        RotateX(shared_ptr<Hittable> p, double angle);

        virtual bool hit(
            const Ray &r, double t_min, double t_max, HitRecord &rec
        ) const override;

        virtual bool bounding_box(
            double time0, double time1, Aabb &output_box
        ) const override {
            output_box = bbox;
            return hasbox;
        }
};

class RotateY: public Hittable {
    public:
        shared_ptr<Hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        Aabb bbox;

    public:
        RotateY(shared_ptr<Hittable> p, double angle);

        virtual bool hit(
            const Ray &r, double t_min, double t_max, HitRecord &rec
        ) const override;

        virtual bool bounding_box(
            double time0, double time1, Aabb &output_box
        ) const override {
            output_box = bbox;
            return hasbox;
        }
};

class RotateZ: public Hittable {
    public:
        shared_ptr<Hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        Aabb bbox;

    public:
        RotateZ(shared_ptr<Hittable> p, double angle);

        virtual bool hit(
            const Ray &r, double t_min, double t_max, HitRecord &rec
        ) const override;

        virtual bool bounding_box(
            double time0, double time1, Aabb &output_box
        ) const override {
            output_box = bbox;
            return hasbox;
        }
};

RotateX::RotateX(shared_ptr<Hittable> p, double angle): ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    Point3 min(INF, INF, INF);
    Point3 max(-INF, -INF, -INF);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
                auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
                auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

                auto newy = cos_theta * y - sin_theta * z;
                auto newz = sin_theta * y + cos_theta * z;

                Vec3 tester(newy, x, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = Aabb(min, max);
}

RotateY::RotateY(shared_ptr<Hittable> p, double angle): ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);
    
    Point3 min(INF, INF, INF);
    Point3 max(-INF, -INF, -INF);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
                auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
                auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

                auto newx = cos_theta * x + sin_theta * z;
                auto newz = -sin_theta * x + cos_theta * z;

                Vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = Aabb(min, max);
}

RotateZ::RotateZ(shared_ptr<Hittable> p, double angle): ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    Point3 min(INF, INF, INF);
    Point3 max(-INF, -INF, -INF);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
                auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
                auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

                auto newx = cos_theta * x - sin_theta * y;
                auto newy = sin_theta * x + cos_theta * y;

                Vec3 tester(newx, newy, z);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = Aabb(min, max);
}

bool RotateX::hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[1] = cos_theta * r.origin()[1] + sin_theta * r.origin()[2];
    origin[2] = -sin_theta * r.origin()[1] + cos_theta * r.origin()[2];

    direction[1] = cos_theta * r.direction()[1] + sin_theta * r.direction()[2];
    direction[2] = -sin_theta * r.direction()[1] + cos_theta * r.direction()[2];

    Ray rotated_r(origin, direction, r.time());

    if (!ptr->hit(rotated_r, t_min, t_max, rec)) {
        return false;
    }

    auto p = rec.p;
    auto normal = rec.normal;

    p[1] = cos_theta * rec.p[1] - sin_theta * rec.p[2];
    p[2] = sin_theta * rec.p[1] + cos_theta * rec.p[2];

    normal[1] = cos_theta * rec.normal[1] - sin_theta * rec.normal[2];
    normal[2] = sin_theta * rec.normal[1] + cos_theta * rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

bool RotateY::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

    Ray rotated_r(origin, direction, r.time());

    if (!ptr->hit(rotated_r, t_min, t_max, rec)) {
        return false;
    }

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
    p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

    normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
    normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

bool RotateZ::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = cos_theta * r.origin()[0] + sin_theta * r.origin()[1];
    origin[1] = -sin_theta * r.origin()[0] + cos_theta * r.origin()[1];

    direction[0] = cos_theta * r.direction()[0] + sin_theta * r.direction()[1];
    direction[1] = -sin_theta * r.direction()[0] + cos_theta * r.direction()[1];

    Ray rotated_r(origin, direction, r.time());

    if (!ptr->hit(rotated_r, t_min, t_max, rec)) {
        return false;
    }

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] = cos_theta * rec.p[0] - sin_theta * rec.p[1];
    p[1] = sin_theta * rec.p[0] + cos_theta * rec.p[1];

    normal[0] = cos_theta * rec.normal[0] - sin_theta * rec.normal[1];
    normal[1] = sin_theta * rec.normal[0] + cos_theta * rec.normal[1];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

class FlipFace : public Hittable {
    public:
        shared_ptr<Hittable> ptr;
    
    public:
        FlipFace(shared_ptr<Hittable> p): ptr(p) {}

        virtual bool hit(
            const Ray &r, double t_min, double t_max, HitRecord &rec
        ) const override {
            if (!ptr->hit(r, t_min, t_max, rec)) {
                return false;
            }

            rec.front_face = !rec.front_face;
            return true;
        }

        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
            return ptr->bounding_box(time0, time1, output_box);
        }
};

#endif