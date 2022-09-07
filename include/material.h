#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"

#include "onb.h"
#include "pdf.h"
#include "texture.h"

struct HitRecord;

struct ScatterRecord {
    Ray specular_ray;
    bool is_specular;
    Color attenuation;
    shared_ptr<Pdf> pdf_ptr;
};

class Material {
    public:
    
    public:
        virtual bool scatter(
            __F_IN__ const Ray &r_in,
            __F_IN__ const HitRecord &rec,
            __F_OUT__ ScatterRecord &srec
        ) const {
            return false;
        }

        virtual double scattering_pdf(
            __F_IN__ const Ray &r_in,
            __F_IN__ const HitRecord &rec,
            __F_IN__ const Ray &scattered
        ) const {
            return 0;
        }

        virtual Color emitted(
            __F_IN__ const Ray &r_in,
            __F_IN__ const HitRecord &rec,
            __F_IN__ double u,
            __F_IN__ double v,
            __F_IN__ const Point3 &p
        ) const {
            return Color(0, 0, 0);
        }
};

class Lambertian : public Material {
    public:
        shared_ptr<Texture> albedo;

    public:
        Lambertian(const Color &a) : albedo(make_shared<SolidColor>(a)) {}
        Lambertian(shared_ptr<Texture> a) : albedo(a) {}

        virtual bool scatter(
            const Ray &r_in, const HitRecord &rec, ScatterRecord &srec
        ) const override {
            srec.is_specular = false;
            srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
            srec.pdf_ptr = make_shared<CosinePdf>(rec.normal);
            return true;
        }

        double scattering_pdf(
            const Ray &r_in, const HitRecord &rec, const Ray &scattered
        ) const {
            auto cosine = dot(rec.normal, normal(scattered.direction()));
            return cosine < 0 ? 0 : cosine / PI;
        }
};

class Metal : public Material {
    public:
        Color albedo;
        double fuzz;

    public:
        Metal(const Color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

        virtual bool scatter(
            const Ray &r_in, const HitRecord &rec, ScatterRecord &srec
        ) const override {
            Vec3 reflected = reflect(normal(r_in.direction()), rec.normal);
            srec.specular_ray = Ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
            srec.attenuation = albedo;
            srec.is_specular = true;
            srec.pdf_ptr = nullptr;
            return true;
        }
};

class Dielectric : public Material {
    public:
        double ir;
    
    public:
        Dielectric(double refraction_index): ir(refraction_index) {}

        virtual bool scatter(
            const Ray &r_in, const HitRecord &rec, ScatterRecord &srec
        ) const override {
            srec.is_specular = true;
            srec.pdf_ptr = nullptr;
            srec.attenuation = Color(1.0, 1.0, 1.0);

            double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

            Vec3 unit_direction = normal(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            Vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double2()) {
                direction = reflect(unit_direction, rec.normal);
            } else {
                direction = refract(unit_direction, rec.normal, refraction_ratio);
            }
            
            srec.specular_ray = Ray(rec.p, direction, r_in.time());
            return true;
        }

    private:
        static double reflectance(double cosine, double ref_idx) {
            auto r0 = (1 - ref_idx) / (1 + ref_idx);
            r0 = r0 * r0;
            return r0 + (1 - r0) * pow((1 - cosine), 5);
        }
};

class DiffuseLight : public Material {
    public:
        shared_ptr<Texture> emit;

    public:
        DiffuseLight(shared_ptr<Texture> a): emit(a) {}
        DiffuseLight(Color c) : emit(make_shared<SolidColor>(c)) {}

        virtual Color emitted(
            const Ray &r_in, const HitRecord &rec, double u, double v, const Point3 &p
        ) const override {
            if (!rec.front_face) {
                return Color(0, 0, 0);
            }
            return emit->value(u, v, p);
        }
};

class Isotropic : public Material {
    public:
        shared_ptr<Texture> albedo;

    public:
        Isotropic(Color c): albedo(make_shared<SolidColor>(c)) {}
        Isotropic(shared_ptr<Texture> a): albedo(a) {}

        // virtual bool scatter(
        //     const Ray &r_in, const HitRecord &rec, Color &alb, Ray &scattered, double &pdf
        // ) const override {
        //     scattered = Ray(rec.p, random_in_unit_sphere(), r_in.time());
        //     alb = albedo->value(rec.u, rec.v, rec.p);

        //     return true;
        // }
};

#endif