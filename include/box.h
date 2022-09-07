#ifndef BOX_H
#define BOX_H

#include "rtweekend.h"

#include "aarect.h"
#include "hittable_list.h"

class Box : public Hittable {
    public:
        Point3 box_min;
        Point3 box_max;
        HittableList sides;

    public:
        Box() {}
        Box(const Point3 &p0, const Point3 &p1, shared_ptr<Material> mp);

        virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

        virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
            output_box = Aabb(box_min, box_max);
            return true;
        }
};

Box::Box(const Point3 &p0, const Point3 &p1, shared_ptr<Material> mp) {
    box_min = p0;
    box_max = p1;

    sides.add(make_shared<XYRect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), mp));
    sides.add(make_shared<XYRect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), mp));

    sides.add(make_shared<XZRect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), mp));
    sides.add(make_shared<XZRect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), mp));

    sides.add(make_shared<YZRect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), mp));
    sides.add(make_shared<YZRect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), mp));
}

bool Box::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
    return sides.hit(r, t_min, t_max, rec);
}

#endif