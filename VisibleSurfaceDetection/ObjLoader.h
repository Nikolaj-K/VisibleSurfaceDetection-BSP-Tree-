//
// Created by paritosh on 21/10/17.
//

#ifndef POINTRENDERER_OBJLOADER_H
#define POINTRENDERER_OBJLOADER_H

#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<sstream>
#include<fstream>
#include <GL/glew.h>
#include<GL/freeglut.h>
#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <cmath>

struct Triangles
{
    glm::vec4 vertices[3];
    glm::vec3 normal[3];
    glm::vec3 uv[3];
    glm::vec3 color;
    std::string materialName;
    bool visible[6];
    long id;
};

int LoadModel(std::vector<Triangles>& triangleData);



#endif //POINTRENDERER_OBJLOADER_H
