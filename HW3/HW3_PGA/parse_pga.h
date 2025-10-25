
//Set the global scene parameter variables
//TODO: Set the scene parameters based on the values in the scene file

#ifndef PARSE_PGA_H
#define PARSE_PGA_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 800, img_height = 600;
std::string imgName = "raytraced.png";

//Camera Parameters
Point3D eye = Point3D(0,0,0); 
Dir3D forward = Dir3D(0,0,-1).normalized();
Dir3D up = Dir3D(0,1,0).normalized();
Dir3D right = Dir3D(-1,0,0).normalized();
float halfAngleVFOV = 35; 

//Scene (Sphere) Parameters
Point3D spherePos = Point3D(0,0,2);
float sphereRadius = 1; 

void parseSceneFile(std::string fileName){
  //TODO: Override the default values with new data from the file "fileName"
  std::ifstream file(fileName);
  
  std::string line;
  while (std::getline(file,line)){
    // skip empty 
    if (line.empty()) 
    continue;
    // skip line starting with #
    if (line[0]=='#') 
    continue;

    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) 
    continue;

    std::string command = line.substr(0,colonPos);
    std::string params = line.substr(colonPos+1);

    // parse each cmd
    if(command == "sphere"){
      float x,y,z,r;
      if (sscanf(params.c_str(), "%f %f %f %f", &x, &y, &z, &r) == 4){
        spherePos = Point3D(x,y,z);
        sphereRadius = r;
        
      }

    }
    else if (command == "image_resolution"){
      int w,h;
      if(sscanf(params.c_str(), "%d %d", &w, &h) == 2){
        img_width =w;
        img_height=h;
      }
    }

    else if ( command == "output_image"){
      size_t start = params.find_first_not_of(" \t");
      size_t end = params.find_last_not_of(" \t\r\n");
      
      if (start!= std::string::npos && end!=std::string::npos){
        imgName = params.substr(start, end-start+1);
      }
    }

    else if (command=="camera_pos"){
      float x, y, z; 
      if (sscanf(params.c_str(), "%f %f %f", &x, &y, &z) == 3){
        eye = Point3D(x,y,z);
      }
    }

    else if (command=="camera_fwd"){
      float fx, fy, fz; 
      if (sscanf(params.c_str(), "%f %f %f", &fx, &fy, &fz) == 3){
        forward = Dir3D(fx,fy,fz);
      }
    }

    else if (command=="camera_up"){
      float ux, uy, uz; 
      if (sscanf(params.c_str(), "%f %f %f", &ux, &uy, &uz) == 3){
        up = Dir3D(ux,uy,uz);
      }
    }

    else if (command=="camera_fov_ha"){
      float ha; 
      if (sscanf(params.c_str(), "%f", &ha) == 1){
        halfAngleVFOV = ha;
      }
    }
  }

  file.close();

  //TODO: Create an orthogonal camera basis, based on the provided up and right vectors

  forward = forward.normalized(); 
  right = cross(up,forward).normalized(); 
  up = cross(forward,right).normalized();

  printf("Orthogonal Camera Basis:\n");
  forward.print("forward");
  right.print("right");
  up.print("up");
}

#endif