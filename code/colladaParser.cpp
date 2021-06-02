#include "colladaParser.h"
#include "utility.h"

#include <windows.h>
#include <glad/glad.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "animatedModel.h"

std::vector<int> ParseStringIntVector(char* string)
{
    char* pNext;
    std::vector<int> vectorInt;
    int firstInt = strtol(string, &pNext, 10);
    vectorInt.push_back(firstInt);
    while(true)
    {
        char* pActual = pNext;
        int nextInt = strtol(pActual, &pNext, 10);
        if(pActual == pNext)
        {
            break;
        }
        vectorInt.push_back(nextInt);
    }
    return vectorInt;
}

std::vector<float> ParseStringFloatVector(char* string)
{
    char* pNext;
    std::vector<float> vectorFloats;
    float firstFloat = strtof(string, &pNext);
    vectorFloats.push_back(firstFloat);
    while(true)
    {
        char* pActual = pNext;
        float nextFloat = strtof(pActual, &pNext);
        if(pActual == pNext)
        {
            break;
        }
        vectorFloats.push_back(nextFloat);
    }
    return vectorFloats;
}

std::vector<char*> ParseStringStringVector(char* string)
{
    std::vector<char*> vectorStrings;
    char* token = std::strtok(string, " ");
    while(token != NULL)
    {
        vectorStrings.push_back(token);
        token = std::strtok(NULL, " ");
    }
    return vectorStrings;
}

bool LoadColladaFile(unsigned int* vao,
                     unsigned int* textId,
                     int* numberVertices,
                     const char* xmlFilePath,
                     const char* textureFilePath,
                     bool haveEBO)
{
    TiXmlDocument xmlDoc;
    if(!xmlDoc.LoadFile(xmlFilePath))
    {
        return false;
    }
    TiXmlElement* pRoot = xmlDoc.RootElement();
    TiXmlElement* geometries  = 0;
    TiXmlElement* animations  = 0;
    TiXmlElement* controllers = 0;
    TiXmlElement* visualScene = 0;

    for(TiXmlElement* e = pRoot->FirstChildElement();
        e != NULL;
        e = e->NextSiblingElement())
    {
        if(strcmp(e->Value(), "library_geometries") == 0)
        {
            geometries = e;
        }
        
        if(strcmp(e->Value(), "library_animations") == 0)
        {
            animations = e;
        }
        if(strcmp(e->Value(), "library_controllers") == 0)
        {
            controllers = e;
        }
        if(strcmp(e->Value(), "library_visual_scenes") == 0)
        {
            visualScene = e;
        }
    }
    
    if(geometries == NULL)
    {
        return false;
    }
 
    char* verticesString; 
    char* normalsString;
    char* textureString;
    char* indicesString; 
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> textures;
    std::vector<int>   indices; 
    int offset = 0;
    bool stopOffset = false;

    for(TiXmlElement* e = geometries->FirstChildElement()->FirstChildElement()->FirstChildElement();
        e != NULL;
        e = e->NextSiblingElement())
    {
        if(e->Attribute("id") != NULL)
        {
            if(strcmp(e->Attribute("id"), "Cube-mesh-positions") == 0)
            {
                verticesString = (char*)e->FirstChildElement()->GetText(); 
            }
            if(strcmp(e->Attribute("id"), "Cube-mesh-normals") == 0)
            {
                normalsString = (char*)e->FirstChildElement()->GetText();
            }
            if(strcmp(e->Attribute("id"), "Cube-mesh-map-0") == 0)
            {
                textureString = (char*)e->FirstChildElement()->GetText();
            }
        }
        if(strcmp(e->Value(), "triangles") == 0)
        {
            for(TiXmlElement* polylist = e->FirstChildElement();
                polylist != NULL;
                polylist = polylist->NextSiblingElement())
            {

                if(strcmp(polylist->Value(), "input") == 0)
                {
                    if(!stopOffset)
                        offset++; 
                }

                if(strcmp(polylist->Value(), "p") == 0)
                {
                    stopOffset = true;
                    indicesString = (char*)polylist->GetText();
                    std::vector<int> actualIndex = ParseStringIntVector(indicesString);
                    for(int i = 0; i < actualIndex.size(); i++)
                    {
                        indices.push_back(actualIndex[i]);
                    }
                }
            }
        }
    }

    vertices = ParseStringFloatVector(verticesString);
    normals  = ParseStringFloatVector(normalsString);
    textures = ParseStringFloatVector(textureString);    
    
    std::vector<float> vertexBufferTemp;
    for(int i = 0; i < indices.size(); i += offset)
    {
        for(int j = 0; j < 3; j++)
        {
            vertexBufferTemp.push_back(vertices[(indices[0 + i] * 3) + j]);
        } 
        for(int j = 0; j < 3; j++)
        {
            vertexBufferTemp.push_back(normals[(indices[1 + i] * 3) + j]);
        }
        for(int j = 0; j < 2; j++)
        {
            vertexBufferTemp.push_back(textures[(indices[2 + i] * 2) + j]);
        }
    }


    char* jointString;
    char* weightsString;
    char* vcountString;
    char* vString;
    std::vector<char*> joints;
    std::vector<float> weights;
    std::vector<int> vcount;
    std::vector<int> v;

    for(TiXmlElement* e = controllers->FirstChildElement()->FirstChildElement()->FirstChildElement();
        e != NULL;
        e = e->NextSiblingElement())
    {
        if(e->Attribute("id") != NULL)
        {
            if(strcmp(e->Attribute("id"), "Armature_Cube-skin-joints") == 0)
            {
                jointString = (char*)e->FirstChildElement()->GetText();
            }
            if(strcmp(e->Attribute("id"), "Armature_Cube-skin-weights") == 0)
            {
                weightsString = (char*)e->FirstChildElement()->GetText();
            }
        }

        if(strcmp(e->Value(), "vertex_weights") == 0)
        {
            for(TiXmlElement* v = e->FirstChildElement();
                v != NULL;
                v = v->NextSiblingElement())
            {
                if(strcmp(v->Value(), "vcount") == 0)
                {
                    vcountString = (char*)v->GetText();
                }
                if(strcmp(v->Value(), "v") == 0)
                {
                    vString = (char*)v->GetText(); 
                }
            }
        }
    }
    joints  = ParseStringStringVector(jointString);
    weights = ParseStringFloatVector(weightsString);
    vcount  = ParseStringIntVector(vcountString);
    v       = ParseStringIntVector(vString);

    AnimatedModel am;
    
    int index = 0;
    for(int i = 0; i < vcount.size(); i++)
    {
        int offset = index + (vcount[i] * 2);
        std::vector<float> actualWeight;
        std::vector<int> actualJointID;
        while(index < offset)
        {
            actualJointID.push_back(v[index]);
            actualWeight.push_back(weights[v[index + 1]]);
            index += 2;
        }
        Vec3  vec3Weight = {};
        IVec3 vec3JointsId = {};
        if(vcount[i] >= 3)
        {
            vec3Weight.x   = actualWeight[0]; 
            vec3Weight.y   = actualWeight[1];
            vec3Weight.z   = actualWeight[2];
            vec3JointsId.x = actualJointID[0];
            vec3JointsId.y = actualJointID[1]; 
            vec3JointsId.z = actualJointID[2];
        }
        if(vcount[i] == 2)
        {
            vec3Weight.x   = actualWeight[0]; 
            vec3Weight.y   = actualWeight[1];
            vec3JointsId.x = actualJointID[0];
            vec3JointsId.y = actualJointID[1]; 
        }
        if(vcount[i] == 1)
        {
            vec3Weight.x   = actualWeight[0]; 
            vec3JointsId.x = actualJointID[0];
        }
        am.jointIDs.push_back(vec3JointsId);
        am.weights.push_back(vec3Weight);
    }

    if(haveEBO)
    {
        std::vector<int> indexBuffer;
        for(int i = 0; i < indices.size(); i += offset)
        {
            indexBuffer.push_back((indices[i]));
        }
        int memoryToAllocate = (vertices.size() / 3) * 8;
        float* vertexBuffer = (float*)malloc(memoryToAllocate * sizeof(float)); 
        int vertexIndex = 0;
        for(int i = 0; i < indexBuffer.size(); i++)
        {
            for(int j = 0; j < 8; j++)
            {
                int index = (indexBuffer[i] * 8) + j;
                vertexBuffer[index] = vertexBufferTemp[vertexIndex + j];
            }
            vertexIndex += 8;        
        }
   
        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, memoryToAllocate * sizeof(float), vertexBuffer, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        unsigned int ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(int), indexBuffer.data(), GL_STATIC_DRAW);
    
        *numberVertices = indexBuffer.size();
        free(vertexBuffer);
    }
    else
    {
        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferTemp.size() * sizeof(float), vertexBufferTemp.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        *numberVertices = vertexBufferTemp.size() / 8;
    }
   
    uint32_t texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    *textId = texture1;
    // test loadd bmp file:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Texture texture = LoadBMP(textureFilePath);
    if(texture.pixels != NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height,
                                    0, GL_BGRA, GL_UNSIGNED_BYTE, texture.pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        OutputDebugString("ERROR::LOADING::BMP::FILE\n");
    }
    free(texture.pixels);
    

    return true;
}


