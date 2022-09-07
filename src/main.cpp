#include "../include/rtweekend.h"

#include "../include/aarect.h"
#include "../include/box.h"
#include "../include/bvh.h"
#include "../include/camera.h"
#include "../include/color.h"
#include "../include/constant_medium.h"
#include "../include/hittable_list.h"
#include "../include/material.h"
#include "../include/moving_sphere.h"
#include "../include/pdf.h"
#include "../include/sphere.h"

#include "../include/external/ctpl_stl.h"

#include <algorithm>
#include <iostream>
#include <chrono>
#include <map>

Color ray_color(
    __F_IN__ const Ray &r,
    __F_IN__ const Color &background,
    __F_IN__ const Hittable &world,
    __F_IN__ shared_ptr<Hittable> lights,
    __F_IN__ int depth
) {
    HitRecord rec;

    if (depth <= 0) {
        return Color(0, 0, 0);
    }

    if (!world.hit(r, 0.001, INF, rec)) {
        return background;
    }

    ScatterRecord srec;
    Color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec)) {
        return emitted;
    }

    auto light_ptr = make_shared<HittablePdf>(rec.p, lights);
    MixturePdf p(light_ptr, srec.pdf_ptr);

    Ray scattered = Ray(rec.p, p.generate(), r.time());
    auto pdf_val = p.value(scattered.direction());

    return emitted
        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                           * ray_color(scattered, background, world, lights, depth - 1) / pdf_val;
}

HittableList random_scene() {
    HittableList world;

    auto checker = make_shared<CheckerTexture>(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));
    world.add(make_shared<Sphere>(Point3(0, -1000.0, 0), 1000, make_shared<Lambertian>(checker)));

    for (int a = -11; a < 11; a++){
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double2();
            Point3 center(a + 0.9 * random_double2(), 0.2, b + 0.9 * random_double2());

            std::cerr << "center: " << center << std::endl;
            std::cerr << "material: " << choose_mat << std::endl;

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<Material> sphere_material;
                
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Color::random() * Color::random();
                    sphere_material = make_shared<Lambertian>(albedo);
                    auto center2 = center + Vec3(0, random_double2(0, 0.5), 0);
                    world.add(make_shared<MovingSphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double2(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<Dielectric>(1.5);
    world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    return world;
}

HittableList two_spheres() {
    HittableList objects;

    auto checker = make_shared<CheckerTexture>(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));

    objects.add(make_shared<Sphere>(Point3(0, -10, 0), 10, make_shared<Lambertian>(checker)));
    objects.add(make_shared<Sphere>(Point3(0, 10, 0), 10, make_shared<Lambertian>(checker)));

    return objects;
}

HittableList two_perlin_spheres() {
    HittableList objects;

    auto light = make_shared<DiffuseLight>(Color(10, 10, 10));
    objects.add(make_shared<XZRect>(123, 423, 147, 412, 554, light));

    auto pertext = make_shared<NoiseTexture>(0.1);
    objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(pertext)));
    objects.add(make_shared<Sphere>(Point3(220, 280, 300), 80, make_shared<Lambertian>(pertext)));

    return objects;
}

HittableList earth() {
    auto earth_texture = make_shared<ImageTexture>("assets/earthmap.jpg");
    auto earth_surface = make_shared<Lambertian>(earth_texture);
    auto globe = make_shared<Sphere>(Point3(0, 0, 0), 2, earth_surface);

    return HittableList(globe);
}

HittableList simple_light() {
    HittableList objects;

    auto pertext = make_shared<NoiseTexture>(4);
    objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(pertext)));
    objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Lambertian>(pertext)));

    auto difflight = make_shared<DiffuseLight>(Color(4, 4, 4));
    // objects.add(make_shared<XYRect>(3, 5, 1, 3, -2, difflight));
    objects.add(make_shared<Sphere>(Point3(0, 7, 0), 2, difflight));

    return objects;
}

HittableList cornell_box() {
    HittableList objects;

    auto red = make_shared<Lambertian>(Color(.65, .05, .05));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

    objects.add(make_shared<YZRect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<YZRect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<XZRect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<FlipFace>(make_shared<XZRect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<XZRect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<XYRect>(0, 555, 0, 555, 555, white));

    shared_ptr<Hittable> box1 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vec3(265, 0, 295));
    objects.add(box1);

    shared_ptr<Hittable> box2 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
    box2 = make_shared<RotateY>(box2, -18);
    box2 = make_shared<Translate>(box2, Vec3(130, 0, 65));
    objects.add(box2);

    return objects;
}

HittableList cornell_smoke() {
    HittableList objects;

    auto red = make_shared<Lambertian>(Color(.65, .05, .05));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));

    objects.add(make_shared<YZRect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<YZRect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<XZRect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<XZRect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<XZRect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<XYRect>(0, 555, 0, 555, 555, white));

    shared_ptr<Hittable> box1 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vec3(265, 0, 295));

    shared_ptr<Hittable> box2 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
    box2 = make_shared<RotateY>(box2, -18);
    box2 = make_shared<Translate>(box2, Vec3(130, 0, 65));

    objects.add(make_shared<ConstantMedium>(box1, 0.01, Color(0,0,0)));
    objects.add(make_shared<ConstantMedium>(box2, 0.01, Color(1,1,1)));
    return objects;
}

HittableList final_scene() {
    HittableList boxes1;
    auto ground = make_shared<Lambertian>(Color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;

    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double2(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<Box>(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
        }
    }

    HittableList objects;

    objects.add(make_shared<BVHNode>(boxes1, 0, 1));

    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));
    objects.add(make_shared<XZRect>(123, 423, 147, 412, 554, light));

    auto center1 = Point3(400, 400, 200);
    auto center2 = center1 + Vec3(30, 0, 0);

    auto moving_sphere_material = make_shared<Lambertian>(Color(0.7, 0.3, 0.1));
    objects.add(make_shared<MovingSphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(make_shared<Sphere>(Point3(260, 150, 45), 50, make_shared<Dielectric>(1.5)));

    objects.add(make_shared<Sphere>(Point3(0, 150, 145), 50, make_shared<Metal>(Color(0.8, 0.8, 0.9), 1.0)));

    auto boundary = make_shared<Sphere>(Point3(360, 150, 145), 70, make_shared<Dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<ConstantMedium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));
    boundary = make_shared<Sphere>(Point3(0, 0, 0), 5000, make_shared<Dielectric>(1.5));
    objects.add(make_shared<ConstantMedium>(boundary, .0001, Color(1, 1, 1)));

    auto emat = make_shared<Lambertian>(make_shared<ImageTexture>("assets/earthmap.jpg"));
    objects.add(make_shared<Sphere>(Point3(400, 200, 400), 100, emat));

    auto pertext = make_shared<NoiseTexture>(0.1);
    objects.add(make_shared<Sphere>(Point3(220, 280, 300), 80, make_shared<Lambertian>(pertext)));

    HittableList boxes2;

    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    int ns = 1000;

    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<Sphere>(Point3::random(0, 165), 10, white));
    }

    objects.add(make_shared<Translate>(
        make_shared<RotateY>(
            make_shared<BVHNode>(boxes2, 0.0, 1.0), 15),
            Vec3(-100, 270, 395)
        )
    );

    return objects;
}

HittableList huh() {
    HittableList boxes1;
    auto ground = make_shared<Lambertian>(Color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;

    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double2(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<Box>(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
        }
    }

    HittableList objects;

    objects.add(make_shared<BVHNode>(boxes1, 0, 1));

    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));
    objects.add(make_shared<XZRect>(123, 423, 147, 412, 554, light));

    objects.add(make_shared<Sphere>(Point3(220, 280, 300), 200, make_shared<Dielectric>(1.5)));

    return objects;
}

int main() {
    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 500;
    const int max_depth = 50;

    // World
    
    HittableList world;
    Point3 lookfrom;
    Point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    int samples_per_pixel = 500;
    Color background(0, 0, 0);
    auto time0 = 0.0;
    auto time1 = 1.0;
    auto lights = make_shared<HittableList>();
    world = cornell_box();
    lights->add(make_shared<XZRect>(213, 343, 227, 332, 554, shared_ptr<Material>()));
    // lights->add(make_shared<Sphere>(Point3(190, 90, 190), 90, shared_ptr<Material>()));
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 100;
    background = Color(0, 0, 0);
    lookfrom = Point3(278, 278, -800);
    lookat = Point3(278, 278, 0);
    vfov = 40.0;

    // switch (6)
    // {
    //     case 1:
    //     {
    //         world = random_scene();
    //         background = Color(0.7, 0.8, 1.0);
    //         lookfrom = Point3(13, 2, 3);
    //         lookat = Point3(0, 0, 0);
    //         vfov = 20.0;
    //         aperture = 0.1;
    //         break;
    //     }
        
    //     case 2:
    //     {
    //         world = two_spheres();
    //         background = Color(0.7, 0.8, 1.0);
    //         lookfrom = Point3(13, 2, 3);
    //         lookat = Point3(0, 0, 0);
    //         vfov = 20.0;
    //         break;
    //     }

    //     case 3:
    //     {
    //         world = two_perlin_spheres();
    //         background = Color(0, 0, 0);
    //         lookfrom = Point3(478, 278, -600);
    //         lookat = Point3(278, 278, 0);
    //         vfov = 40.0;
    //         break;
    //     }

    //     case 4:
    //     {
    //         world = earth();
    //         lookfrom = Point3(13, 2, 3);
    //         background = Color(0.7, 0.8, 1.0);
    //         lookat = Point3(0, 0, 0);
    //         vfov = 20.0;
    //         break;
    //     }

    //     case 5:
    //     {
    //         world = simple_light();
    //         samples_per_pixel = 400;
    //         background = Color(0.0, 0.0, 0.0);
    //         lookfrom = Point3(26, 3, 6);
    //         lookat = Point3(0, 2, 0);
    //         vfov = 20.0;
    //     }

    //     case 6:
    //     {
    //         world = cornell_box();
    //         lights->add(make_shared<XZRect>(213, 343, 227, 332, 554, shared_ptr<Material>()));
    //         aspect_ratio = 1.0;
    //         image_width = 600;
    //         samples_per_pixel = 100;
    //         background = Color(0, 0, 0);
    //         lookfrom = Point3(278, 278, -800);
    //         lookat = Point3(278, 278, 0);
    //         vfov = 40.0;
    //         break;
    //     }

    //     case 7:
    //     {
    //         world = cornell_smoke();
    //         aspect_ratio = 1.0;
    //         image_width = 600;
    //         samples_per_pixel = 200;
    //         lookfrom = Point3(278, 278, -800);
    //         lookat = Point3(278, 278, 0);
    //         vfov = 40.0;
    //         break;
    //     }

    //     case 8:
    //     {
    //         world = final_scene();
    //         aspect_ratio = 1.0;
    //         image_width = 800;
    //         samples_per_pixel = 10000;
    //         background = Color(0, 0, 0);
    //         lookfrom = Point3(478, 278, -600);
    //         lookat = Point3(278, 278, 0);
    //         vfov = 40.0;
    //         break;
    //     }

    //     default:
    //     {
    //         world = huh();
    //         samples_per_pixel = 2000;
    //         background = Color(0, 0, 0);
    //         lookfrom = Point3(478, 278, -600);
    //         lookat = Point3(278, 278, 0);
    //         vfov = 40.0;
    //         break;
    //     }
    // }


    // HittableList world;

    // auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    // auto material_center = make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    // auto material_left = make_shared<Dielectric>(1.33);
    // auto material_right = make_shared<Metal>(Color(0.8, 0.6, 0.2), 1.0);
    
    // world.add(make_shared<Sphere>(Point3(0, -100.5, -1), 100, material_ground));
    // world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5, material_center));
    // world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));
    // world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), -0.4, material_left));
    // world.add(make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));

    // auto R = cos(PI / 4);

    // auto material_left = make_shared<Lambertian>(Color(0, 0, 1));
    // auto material_right = make_shared<Lambertian>(Color(1, 0, 0));

    // world.add(make_shared<Sphere>(Point3(-R, 0, -1), R, material_left));
    // world.add(make_shared<Sphere>(Point3(R, 0, -1), R, material_right));
    
    // Camera

    Vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    int image_height = static_cast<int>(image_width / aspect_ratio);

    Camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, time0, time1);

    // Render

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    auto last_counter = std::chrono::high_resolution_clock::now();

    // for (int j = image_height - 1; j >= 0; --j) {
    //     std::cerr << "\nScanlines remaining: " << j << ' ' << std::flush;
    //     for (int i = 0; i < image_width; ++i) {
    //         Color pixel_color(0, 0, 0);

    //         for (int s = 0; s < samples_per_pixel; ++s) {
    //             auto u = (i + random_double2()) / (image_width - 1);
    //             auto v = (j + random_double2()) / (image_height - 1);
    //             Ray r = cam.get_ray(u, v);
    //             pixel_color += ray_color(r, world, max_depth);
    //         }

    //         write_color(std::cout, pixel_color, samples_per_pixel);
    //     }
    //     auto stop = std::chrono::high_resolution_clock::now();
    //     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - last_counter);
    //     std::cerr << "\nTook: " << duration.count() << "ms" << std::flush;
    //     last_counter = stop;
    // }


    ctpl::thread_pool p(std::thread::hardware_concurrency());

    long long total_time = 0;

    // std::map<std::pair<int, int>, Color> pixels;

    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\nScanlines remaining: " << j << ' ' << std::flush;
        std::vector<std::pair<std::pair<int, int>, Color>> pixels(image_width);
        std::vector<std::future<std::pair<std::pair<int, int>, Color>>> results(image_width);
        
        for (int i = 0; i < image_width; ++i) {
            results[i] = p.push([i, j, &world, &cam, samples_per_pixel, max_depth, image_width, image_height, &background, &lights](int) {
                Color pixel_color(0, 0, 0);

                for (int s = 0; s < samples_per_pixel; s++) {
                    auto u = (i + random_double2()) / (image_width - 1);
                    auto v = (j + random_double2()) / (image_height - 1);
                    Ray r = cam.get_ray(u, v);
                    pixel_color += ray_color(r, background, world, lights, max_depth);
                }

                std::pair<std::pair<int, int>, Color> pix = std::make_pair(std::make_pair(j, i), pixel_color);
                // std::cerr << "(" << pix.first.first << ", " << pix.first.second << "), " << pix.second.e[0] << ", " << pix.second.e[1] << ", " << pix.second.e[2] << std::endl;
                return pix;
            });
        }

        // for (auto& result : results) {
        //     std::cerr << "getting result" << std::flush;
        //     result.get();
        // }
        
        for (int i = 0; i < image_width; ++i) {
            pixels[i] = results[i].get();
        }

        results.clear();

        std::sort(pixels.begin(), pixels.end(), [](const std::pair<std::pair<int, int>, Color> &a, const std::pair<std::pair<int, int>, Color> &b) {
            if (a.first.first == b.first.first) {
                return a.first.second < b.first.second;
            }
            return a.first.first > b.first.first;
        });

        for (auto& pixel : pixels) {
            write_color(std::cout, pixel.second, samples_per_pixel);
        }

        auto end_counter = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_counter - last_counter);
        std::cerr << "\nTook: " << duration.count() << "ms" << std::flush;
        total_time += duration.count();
        last_counter = end_counter;
        pixels.clear();
    }

    std::cerr << "\nDone.\n";
    std::cerr << "Total time: " << total_time << "ms / " << total_time / 1000.0 << "s" << std::endl;
}

// int fact(int n) {
//     if (n <= 1) return 1;
//     else return n * fact(n - 1);
// }

// int main (int argc, char *argv[]) {
//     double n = 18;
//     double r = 3;

//     double answer = fact(n) / (fact(r) * fact(n - r));

//     std::cout << answer << std::endl;
    
//     return 0;
// }