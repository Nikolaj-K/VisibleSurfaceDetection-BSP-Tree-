//
// Created by paritosh on 21/10/17.
//

#include "ObjLoader.h"

struct FaceData
{
    std::vector<unsigned int> allVertexIndices{};
    std::vector<unsigned int> allNormalIndices{};
    std::vector<unsigned int> allUVIndices{};
};

std::vector<glm::vec3> allVertices{};
std::vector<glm::vec3> allNormals{};
std::vector<glm::vec3> allUV{};
FaceData currentFaceData;
std::vector<unsigned int> allVertexIndices{};
std::vector<unsigned int> allNormalIndices{};
std::vector<unsigned int> allUVIndices{};
std::map<std::string, FaceData> materialMapping;
std::stringstream objFileContents{ "" };
std::stringstream mtlFileContents{ "" };
std::string line{};
std::string currentMaterial{};
std::string newMaterial{};

int nthOccurrence(const std::string& str, const std::string& findMe, int nth)
{
    size_t  pos = 0;
    int     cnt = 0;

    while( cnt != nth )
    {
        pos+=1;
        pos = str.find(findMe, pos);
        if ( pos == std::string::npos )
            return -1;
        cnt++;
    }
    return pos;
}

int LoadModel(std::vector<Triangles>& triangleData) {

    //Read file in stream
    std::ifstream objFile{"triObj.obj"};
    std::ifstream mtlFile{"triObj.mtl"};

    objFileContents << objFile.rdbuf();
    mtlFileContents << mtlFile.rdbuf();

    objFile.close();
    mtlFile.close();

    if (objFileContents.rdbuf()->in_avail() == 0) {
        std::cout << "Obj file contains no data!!";
        return -1;
    }

//Parse obj file
    while (std::getline(objFileContents, line)) {

        if (line.find("vt") != std::string::npos && line.size() > 1) {

            unsigned firstNumPos = nthOccurrence(line, " ", 2);
            unsigned lastNumPos = line.find_last_of(" ");

            std::string strX = line.substr(2, firstNumPos - 1);
            std::string strY = line.substr(firstNumPos + 1, lastNumPos - firstNumPos - 1);
            std::string strZ = line.substr(lastNumPos + 1);

            allUV.push_back(glm::vec3(std::stof(strX), std::stof(strY), std::stof(strZ)));


        } else if (line.find("vn") != std::string::npos && line.size() > 1) {
            unsigned firstNumPos = nthOccurrence(line, " ", 2);
            unsigned lastNumPos = line.find_last_of(" ");

            std::string strX = line.substr(2, firstNumPos - 1);
            std::string strY = line.substr(firstNumPos + 1, lastNumPos - firstNumPos - 1);
            std::string strZ = line.substr(lastNumPos + 1);

            allNormals.push_back(glm::vec3(std::stof(strX), std::stof(strY), std::stof(strZ)));
        } else if (line.find("v") != std::string::npos && line.size() > 1) {
            unsigned firstNumPos = nthOccurrence(line, " ", 2);
            unsigned lastNumPos = line.find_last_of(" ");

            std::string strX = line.substr(2, firstNumPos - 1);
            std::string strY = line.substr(firstNumPos + 1, lastNumPos - firstNumPos - 1);
            std::string strZ = line.substr(lastNumPos + 1);

            allVertices.push_back(glm::vec3(std::stof(strX), std::stof(strY), std::stof(strZ)));


        } else if (line.find("usemtl") != std::string::npos && line.size() > 1) {
            if (currentMaterial.size() != 0) {
                materialMapping[currentMaterial].allVertexIndices.insert(
                        materialMapping[currentMaterial].allVertexIndices.end(), allVertexIndices.begin(),
                        allVertexIndices.end());
                materialMapping[currentMaterial].allNormalIndices.insert(
                        materialMapping[currentMaterial].allNormalIndices.end(), allNormalIndices.begin(),
                        allNormalIndices.end());
                materialMapping[currentMaterial].allUVIndices.insert(
                        materialMapping[currentMaterial].allUVIndices.end(), allUVIndices.begin(), allUVIndices.end());
            }

            unsigned first = line.find_first_of(" ");
            currentMaterial = line.substr(first + 1);


//Reset the face data as new material is coming
            allVertexIndices.clear();
            allNormalIndices.clear();
            allUVIndices.clear();
        } else if (line.find("f") != std::string::npos && line.size() > 1 && line.find("mtllib") == std::string::npos) {
            unsigned firstNumPos = nthOccurrence(line, " ", 3);
            unsigned lastNumPos = line.find_last_of(" ");

            std::string face1 = line.substr(2, firstNumPos - 1);
            std::string face2 = line.substr(firstNumPos + 1, lastNumPos - firstNumPos - 1);
            std::string face3 = line.substr(lastNumPos + 1);

            unsigned first = face1.find_first_of("/");
            unsigned last = face1.find_last_of("/");

            std::string vertX = face1.substr(0, first);
            std::string uvX = face1.substr(first + 1, last - first - 1);
            std::string normalX = face1.substr(last + 1);

            first = face2.find_first_of("/");
            last = face2.find_last_of("/");

            std::string vertY = face2.substr(0, first);
            std::string uvY = face2.substr(first + 1, last - first - 1);
            std::string normalY = face2.substr(last + 1);

            first = face3.find_first_of("/");
            last = face3.find_last_of("/");

            std::string vertZ = face3.substr(0, first);
            std::string uvZ = face3.substr(first + 1, last - first - 1);
            std::string normalZ = face3.substr(last + 1);


            allVertexIndices.push_back(std::stoi(vertX));
            allVertexIndices.push_back(std::stoi(vertY));
            allVertexIndices.push_back(std::stoi(vertZ));

            allNormalIndices.push_back(std::stoi(normalX));
            allNormalIndices.push_back(std::stoi(normalY));
            allNormalIndices.push_back(std::stoi(normalZ));

            allUVIndices.push_back(std::stoi(uvX));
            allUVIndices.push_back(std::stoi(uvY));
            allUVIndices.push_back(std::stoi(uvZ));


        }
    }

    std::map <std::string, glm::vec3> colorMaterialMapping;
    std::string tempMaterialName{};
    glm::vec3 color;
//Parse mtl  file
    while (std::getline(mtlFileContents, line)) {

        if (line.find("newmtl") != std::string::npos) {
            if (tempMaterialName.size() != 0) {
                colorMaterialMapping[tempMaterialName] = color;
            }

            unsigned first = line.find_first_of(" ");
            tempMaterialName = line.substr(first + 1);
        } else if (line.find("Kd") != std::string::npos) {
            unsigned firstNumPos = nthOccurrence(line, " ", 3);
            unsigned lastNumPos = line.find_last_of(" ");

            std::string strX = line.substr(2, firstNumPos - 1);
            std::string strY = line.substr(firstNumPos + 1, lastNumPos - firstNumPos - 1);
            std::string strZ = line.substr(lastNumPos + 1);

            color = glm::vec3(std::stof(strX), std::stof(strY), std::stof(strZ));
        }
    }


//For the last material
    if (currentMaterial.size() != 0) {
        materialMapping[currentMaterial].allVertexIndices.insert(
                materialMapping[currentMaterial].allVertexIndices.end(), allVertexIndices.begin(),
                allVertexIndices.end());
        materialMapping[currentMaterial].allNormalIndices.insert(
                materialMapping[currentMaterial].allNormalIndices.end(), allNormalIndices.begin(),
                allNormalIndices.end());
        materialMapping[currentMaterial].allUVIndices.insert(materialMapping[currentMaterial].allUVIndices.end(),
                                                             allUVIndices.begin(), allUVIndices.end());
    }

    if (tempMaterialName.size() != 0) {
        colorMaterialMapping[tempMaterialName] = color;
    }


//Now lets populate the structure for triangle data
    Triangles *tempTriData = new Triangles;
    long trianglID = 1;

    for (const auto &materialMap : materialMapping) {
//Assume initially triangle is not visible from any side
        for (size_t j = 0; j < 6; j++) {
            tempTriData->visible[j] = false;
        }

        for (size_t i = 0; i < materialMap.second.allVertexIndices.size(); i += 3) {
            //Fill triangles vertex data
            tempTriData->vertices[0] = glm::vec4(allVertices[materialMap.second.allVertexIndices[i] - 1], 1.0);
            tempTriData->vertices[1] = glm::vec4(allVertices[materialMap.second.allVertexIndices[i + 1] - 1], 1.0);
            tempTriData->vertices[2] = glm::vec4(allVertices[materialMap.second.allVertexIndices[i + 2] - 1], 1.0);

            tempTriData->normal[0] = allNormals[materialMap.second.allNormalIndices[i] - 1];
            tempTriData->normal[1] = allNormals[materialMap.second.allNormalIndices[i + 1] - 1];
            tempTriData->normal[2] = allNormals[materialMap.second.allNormalIndices[i + 2] - 1];

            tempTriData->uv[0] = allUV[materialMap.second.allUVIndices[i] - 1];
            tempTriData->uv[1] = allUV[materialMap.second.allUVIndices[i + 1] - 1];
            tempTriData->uv[2] = allUV[materialMap.second.allUVIndices[i + 2] - 1];
            tempTriData->color = colorMaterialMapping[materialMap.first];
            tempTriData->materialName = materialMap.first;

            tempTriData->id = trianglID;
            ++trianglID;
            triangleData.push_back(*tempTriData);
        }
    }

    delete tempTriData;
}