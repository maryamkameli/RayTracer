//CSCI 5607 HW3 - Rays & Files
//This HW has three steps:
// 1. Compile and run the program (the program takes a single command line argument)
// 2. Understand the code in this file (rayTrace_vec3.cpp), in particular be sure to understand:
//     -How ray-sphere intersection works
//     -How the rays are being generated
//     -The pipeline from rays, to intersection, to pixel color
//    After you finish this step, and understand the math, take the HW quiz on canvas
// 3. Update the file parse_vec3.h so that the function parseSceneFile() reads the passed in file
//     and sets the relevant global variables for the rest of the code to product to correct image

//To Compile: g++ -fsanitize=address -std=c++11 rayTrace_vec3.cpp

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // For fopen and sscanf
#define _USE_MATH_DEFINES 
#endif

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h" //Defines an image class and a color class

//#Vec3 Library
#include "vec3.h"

//High resolution timer
#include <chrono>
#include <cmath>
#include <algorithm>
#include <limits>

//Scene file parser
#include "parse_vec3.h"

const float EPSILON = 0.0001f;
const float INF = std::numeric_limits<float>::infinity();


struct Ray {
    vec3 origin;
    vec3 direction;
    
    Ray(vec3 o, vec3 d) : origin(o), direction(d) {}
};

struct HitInfo {
    bool hit;
    float t;
    vec3 point;
    vec3 normal;
    Material material;
    
    HitInfo() : hit(false), t(INF) {}
};

HitInfo raySphereIntersect(const Ray& ray, const Sphere& sphere) {
    HitInfo info;
    vec3 oc = ray.origin - sphere.center;
    
    float a = dot(ray.direction, ray.direction);
    float b = 2.0f * dot(ray.direction, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return info;
    }
    float t0,t1;
    t0 = (-b - sqrt(discriminant))/(2.0f*a);
    t1 = (-b + sqrt(discriminant))/(2.0f*a);
    
    float t = t0;
    if (t < EPSILON) {
        t = t1;
        if (t < EPSILON) {
            return info;
        }
    }
    
    info.hit = true;
    info.t = t;
    info.point = ray.origin + t * ray.direction;
    info.normal = (info.point - sphere.center).normalized();
    info.material = sphere.material;
    
    return info;
}

HitInfo intersectScene(const Ray& ray) {
    HitInfo closest;
    
    for (const auto& sphere : spheres) {
        HitInfo info = raySphereIntersect(ray, sphere);
        if (info.hit && info.t < closest.t) {
            closest = info;
        }
    }
    
    return closest;
}

bool inShadow(const vec3& point, const vec3& lightDir, float lightDist) {
    Ray shadowRay(point + EPSILON * lightDir, lightDir);
    HitInfo shadowHit = intersectScene(shadowRay);
    return shadowHit.hit && shadowHit.t < lightDist - EPSILON;
}

vec3 shade(const HitInfo& hit, const Ray& ray, int depth);

// phong shading
vec3 computePhong(const HitInfo& hit, const vec3& viewDir) {
    
  // ambient component
    vec3 color = vec3(
        hit.material.ambient.x * ambientLight.x,
        hit.material.ambient.y * ambientLight.y,
        hit.material.ambient.z * ambientLight.z
    );

    for (const auto& light : pointLights) {
        vec3 lightDir = light.position - hit.point;
        float lightDist = lightDir.length();
        lightDir = lightDir.normalized();
        
        if (!inShadow(hit.point, lightDir, lightDist)) {
            
          // diffuse
            float diff = std::max(0.0f, dot(hit.normal, lightDir));
            vec3 diffuse = vec3(
                hit.material.diffuse.x * light.color.x * diff,
                hit.material.diffuse.y * light.color.y * diff,
                hit.material.diffuse.z * light.color.z * diff
            );
            
            // specular
            vec3 reflectDir = 2.0f * dot(lightDir, hit.normal) * hit.normal - lightDir;
            float spec = pow(std::max(0.0f, dot(reflectDir, viewDir)), hit.material.shininess);
            vec3 specular = vec3(
                hit.material.specular.x * light.color.x * spec,
                hit.material.specular.y * light.color.y * spec,
                hit.material.specular.z * light.color.z * spec
            );
            
            float attenuation = 1.0f / (lightDist * lightDist);
            color = color + attenuation * (diffuse + specular);
        }
    }
    
    // directional lights
    for (const auto& light : directionalLights) {
        vec3 lightDir = -1.0f * light.direction;
        
        if (!inShadow(hit.point, lightDir, INF)) {
            // diffuse
            float diff = std::max(0.0f, dot(hit.normal, lightDir));
            vec3 diffuse = vec3(
                hit.material.diffuse.x * light.color.x * diff,
                hit.material.diffuse.y * light.color.y * diff,
                hit.material.diffuse.z * light.color.z * diff
            );
            
            // specular
            vec3 reflectDir = 2.0f * dot(lightDir, hit.normal) * hit.normal - lightDir;
            float spec = pow(std::max(0.0f, dot(reflectDir, viewDir)), hit.material.shininess);
            vec3 specular = vec3(
                hit.material.specular.x * light.color.x * spec,
                hit.material.specular.y * light.color.y * spec,
                hit.material.specular.z * light.color.z * spec
            );
            
            color = color + diffuse + specular;
        }
    }
    
    // spot lights
    for (const auto& light : spotLights) {
        vec3 lightDir = light.position - hit.point;
        float lightDist = lightDir.length();
        lightDir = lightDir.normalized();
        
        float theta = acos(dot(-1.0f * lightDir, light.direction)) * 180.0f / M_PI;
        
        if (theta <= light.angle2) {
            float intensity = 1.0f;
            if (theta > light.angle1) {
                intensity = (light.angle2 - theta) / (light.angle2 - light.angle1);
            }
            
            if (!inShadow(hit.point, lightDir, lightDist)) {
                // diffuse
                float diff = std::max(0.0f, dot(hit.normal, lightDir));
                vec3 diffuse = intensity * vec3(
                    hit.material.diffuse.x * light.color.x * diff,
                    hit.material.diffuse.y * light.color.y * diff,
                    hit.material.diffuse.z * light.color.z * diff
                );
                
                // specular
                vec3 reflectDir = 2.0f * dot(lightDir, hit.normal) * hit.normal - lightDir;
                float spec = pow(std::max(0.0f, dot(reflectDir, viewDir)), hit.material.shininess);
                vec3 specular = intensity * vec3(
                    hit.material.specular.x * light.color.x * spec,
                    hit.material.specular.y * light.color.y * spec,
                    hit.material.specular.z * light.color.z * spec
                );
                
                float attenuation = 1.0f / (lightDist * lightDist);
                color = color + attenuation * (diffuse + specular);
            }
        }
    }
    
    return color;
}

vec3 shade(const HitInfo& hit, const Ray& ray, int depth) {
    if (depth <= 0) return vec3(0,0,0);
    
    vec3 viewDir = -1.0f * ray.direction;
    vec3 color = computePhong(hit, viewDir);
    
    // reflection
    float reflectivity = hit.material.specular.x + hit.material.specular.y + hit.material.specular.z;
    if (reflectivity > 0.01f && depth > 0) {
        vec3 reflectDir = ray.direction - 2.0f * dot(ray.direction, hit.normal) * hit.normal;
        Ray reflectRay(hit.point + EPSILON * hit.normal, reflectDir);
        HitInfo reflectHit = intersectScene(reflectRay);
        
        if (reflectHit.hit) {
            vec3 reflectColor = shade(reflectHit, reflectRay, depth - 1);
            color = color + vec3(
                hit.material.specular.x * reflectColor.x,
                hit.material.specular.y * reflectColor.y,
                hit.material.specular.z * reflectColor.z
            );
        }
    }
    
    return color;
}

// Trace a ray through the scene
vec3 traceRay(const Ray& ray, int depth) {
    HitInfo hit = intersectScene(ray);
    
    if (hit.hit) {
        return shade(hit, ray, depth);
    }
    
    return backgroundColor;
}


// //Tests is the ray intersects the sphere
// bool raySphereIntersect(vec3 start, vec3 dir, vec3 center, float radius){
//   float a = dot(dir,dir); //TODO - Understand: What do we know about "a" if "dir" is normalized on creation?
//   vec3 toStart = (start - center);
//   float b = 2 * dot(dir,toStart);
//   float c = dot(toStart,toStart) - radius*radius;
//   float discr = b*b - 4*a*c;
//   if (discr < 0) return false;
//   else{
//     float t0 = (-b + sqrt(discr))/(2*a);
//     float t1 = (-b - sqrt(discr))/(2*a);
//     if (t0 > 0 || t1 > 0) return true;
//   }
//   return false;
// }

int main(int argc, char** argv){

  //Read command line paramaters to get scene file
  if (argc != 2){
     std::cout << "Usage: ./a.out scenefile\n";
     return(0);
  }
  std::string secenFileName = argv[1];

  //Parse Scene File
  parseSceneFile(secenFileName);

  float imgW = img_width, imgH = img_height;
  float halfW = imgW/2, halfH = imgH/2;
  float d = halfH / tanf(halfAngleVFOV * (M_PI / 180.0f));
  std::vector<unsigned char> pixels(img_width * img_height * 3);

  // Image outputImg = Image(img_width,img_height);
  auto t_start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < img_width; i++){
    for (int j = 0; j < img_height; j++){
      //TODO - Understand: In what way does this assumes the basis is orthonormal?
      float u = (halfW - (imgW)*((i+0.5)/imgW));
      float v = (halfH - (imgH)*((j+0.5)/imgH));
      vec3 p = eye - d*forward + u*right + v*up;
      vec3 rayDir = (p - eye).normalized();  //Normalizing here is optional

      Ray ray(eye, rayDir);
      vec3 color = traceRay(ray, maxDepth);
      color = color.clampTo1();

      int idx = (j* img_width + i)*3;
      pixels[idx + 0] = (unsigned char)(color.x * 255); // r
      pixels[idx + 1] = (unsigned char)(color.y * 255); // g
      pixels[idx + 2] = (unsigned char)(color.z * 255); // b

      // bool hit = raySphereIntersect(eye,rayDir,spherePos,sphereRadius);
      // Color color;
      // if (hit) color = Color(1,1,1);
      // else color = Color(0,0,0);
      // outputImg.setPixel(i,j, color);
      //outputImg.setPixel(i,j, Color(fabs(i/imgW),fabs(j/imgH),fabs(0))); //TODO - Understand: Try this, what is it visualizing?
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());
  std::cout << "Writing image to " << imgName << std::endl;
  stbi_write_png(imgName.c_str(), img_width, img_height, 3, pixels.data(), img_width * 3);
  
  // outputImg.write(imgName.c_str());
  return 0;
}