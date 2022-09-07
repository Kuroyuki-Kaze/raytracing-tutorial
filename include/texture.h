#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "rtw_stb_image.h"

#include "perlin.h"

#include <iostream>

class Texture {
    public:
        virtual Color value(double u, double v, const Point3 &p) const = 0;
};

class SolidColor : public Texture {
    private:
        Color color_value;

    public:
        SolidColor() {}
        SolidColor(Color c) : color_value(c) {}
        SolidColor(double red, double green, double blue) : SolidColor(Color(red, green, blue)) {}

        virtual Color value(double u, double v, const Point3 &p) const override {
            return color_value;
        }
};

class CheckerTexture : public Texture {
    public:
        shared_ptr<Texture> odd;
        shared_ptr<Texture> even;

    public:
        CheckerTexture() {}
        CheckerTexture(
            shared_ptr<Texture> _odd,
            shared_ptr<Texture> _even
        ) : odd(_odd), even(_even) {}
        CheckerTexture(
            Color c1,
            Color c2
        ): odd(make_shared<SolidColor>(c1)), even(make_shared<SolidColor>(c2)) {}

        virtual Color value(double u, double v, const Point3 &p) const override {
            auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
            if (sines < 0) {
                return odd->value(u, v, p);
            } else {
                return even->value(u, v, p);
            }
        }
};

class NoiseTexture : public Texture {
    public:
        Perlin noise;
        double scale;

    public:
        NoiseTexture() {}
        NoiseTexture(double sc) : scale(sc) {}
        
        virtual Color value(double u, double v, const Point3 &p) const override {
            return Color(1, 1, 1) * 0.5 * (1.0 + sin(scale * p.z() + 10 * noise.turb(p)));
        }
};

class ImageTexture : public Texture {
    private:
        unsigned char *data;
        int width, height;
        int bytes_per_scanline;

    public:
        const static int bytes_per_pixel = 3;
        
        ImageTexture() : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

        ImageTexture(const char *filename) {
            auto components_per_pixel = bytes_per_pixel;

            data = stbi_load(
                filename,
                &width,
                &height,
                &components_per_pixel,
                components_per_pixel
            );

            if (!data) {
                std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
                width = height = 0;
            }

            bytes_per_scanline = bytes_per_pixel * width;
        }

        ~ImageTexture() {
            delete data;
        }

        virtual Color value(double u, double v, const Vec3 &p) const override {
            if (data == nullptr) {
                return Color(0, 1, 1);
            }

            u = clamp(u, 0.0, 1.0);
            v = 1 - clamp(v, 0.0, 1.0);

            auto i = static_cast<int>(u * width);
            auto j = static_cast<int>(v * height);

            if (i >= width) i = width - 1;
            if (j >= height) j = height - 1;

            const auto color_scale = 1.0 / 255.0;
            auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;
            
            return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
        }
};

#endif