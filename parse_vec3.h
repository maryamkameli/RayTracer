
//Set the global scene parameter variables
//TODO: Set the scene parameters based on the values in the scene file

#ifndef PARSE_VEC3_H
#define PARSE_VEC3_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include "vec3.h"
#include <vector>
#include <sstream>

struct Material;
struct Sphere;
struct PointLight;
struct DirectionalLight;
struct SpotLight;

//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 800, img_height = 600;
std::string imgName = "raytraced.png";

//Camera Parameters
vec3 eye = vec3(0,0,0); 
vec3 forward = vec3(0,0,-1).normalized();
vec3 up = vec3(0,1,0).normalized();
vec3 right;
float halfAngleVFOV = 35; 

vec3 backgroundColor = vec3(0,0,0);
int maxDepth = 5;

//Scene (Sphere) Parameters
// vec3 spherePos = vec3(0,0,2);
// float sphereRadius = 1; 

std::vector<Sphere> spheres;
vec3 ambientLight = vec3(0,0,0);
std::vector<PointLight> pointLights;
std::vector<DirectionalLight> directionalLights;
std::vector<SpotLight> spotLights;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    vec3 transmissive;
    float ior;
    
    Material() : ambient(0,0,0), diffuse(1,1,1), specular(0,0,0), 
                 shininess(5), transmissive(0,0,0), ior(1) {}
};

Material currentMaterial;

struct Sphere {
    vec3 center;
    float radius;
    Material material;
    
    Sphere(vec3 c, float r) : center(c), radius(r) {}
};

struct PointLight {
    vec3 color;
    vec3 position;
    PointLight(vec3 c, vec3 p) : color(c), position(p) {}
};

struct DirectionalLight {
    vec3 color;
    vec3 direction;
    DirectionalLight(vec3 c, vec3 d) : color(c), direction(d.normalized()) {}
};

struct SpotLight {
    vec3 color;
    vec3 position;
    vec3 direction;
    float angle1, angle2;
    
    SpotLight(vec3 c, vec3 p, vec3 d, float a1, float a2) 
        : color(c), position(p), direction(d.normalized()), angle1(a1), angle2(a2) {}
};

void parseSceneFile(std::string fileName){
  //TODO: Override the default values with new data from the file "fileName"
  std::ifstream file(fileName);
    std::string line;
    
    while (std::getline(file, line)) {

      if (line.empty() or line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (!command.empty() && command.back() == ':') {
            command.pop_back();
        }
        
        if (command == "camera_pos") {
            iss >> eye.x >> eye.y >> eye.z;
        }
        else if (command == "camera_fwd") {
            iss >> forward.x >> forward.y >> forward.z;
        }
        else if (command == "camera_up") {
            iss >> up.x >> up.y >> up.z;
        }
        else if (command == "camera_fov_ha") {
            iss >> halfAngleVFOV;
        }
        else if (command == "film_resolution") {
            iss >> img_width >> img_height;
        }
        else if (command == "output_image") {
            iss >> imgName;
        }
        else if (command == "background") {
            iss >> backgroundColor.x >> backgroundColor.y >> backgroundColor.z;
        }
        else if (command == "material") {
            iss >> currentMaterial.ambient.x >> currentMaterial.ambient.y >> currentMaterial.ambient.z
                >> currentMaterial.diffuse.x >> currentMaterial.diffuse.y >> currentMaterial.diffuse.z
                >> currentMaterial.specular.x >> currentMaterial.specular.y >> currentMaterial.specular.z
                >> currentMaterial.shininess
                >> currentMaterial.transmissive.x >> currentMaterial.transmissive.y >> currentMaterial.transmissive.z
                >> currentMaterial.ior;
        }
        else if (command == "sphere") {
            vec3 center;
            float radius;
            iss >> center.x >> center.y >> center.z >> radius;
            Sphere s(center, radius);
            s.material = currentMaterial;
            spheres.push_back(s);
        }
        else if (command == "ambient_light") {
            iss >> ambientLight.x >> ambientLight.y >> ambientLight.z;
        }
        else if (command == "point_light") {
            vec3 color, pos;
            iss >> color.x >> color.y >> color.z >> pos.x >> pos.y >> pos.z;
            pointLights.push_back(PointLight(color, pos));
        }
        else if (command == "directional_light") {
            vec3 color, dir;
            iss >> color.x >> color.y >> color.z >> dir.x >> dir.y >> dir.z;
            directionalLights.push_back(DirectionalLight(color, dir));
        }
        else if (command == "spot_light") {
            vec3 color, pos, dir;
            float a1, a2;
            iss >> color.x >> color.y >> color.z 
                >> pos.x >> pos.y >> pos.z
                >> dir.x >> dir.y >> dir.z
                >> a1 >> a2;
            spotLights.push_back(SpotLight(color, pos, dir, a1, a2));
        }
        else if (command == "max_depth") {
            iss >> maxDepth;
        }
    }
    
    file.close();

    // orthogonal camera basis



  //TODO: Create an orthogonal camera basis, based on the provided up and right vectors
  forward = forward.normalized();
  right = cross(forward, up).normalized();
  up = cross(right, forward).normalized();  
  printf("Orthogonal Camera Basis:\n");
  printf("forward: %f,%f,%f\n",forward.x,forward.y,forward.z);
  printf("right: %f,%f,%f\n",right.x,right.y,right.z);
  printf("up: %f,%f,%f\n",up.x,up.y,up.z);
}

#endif