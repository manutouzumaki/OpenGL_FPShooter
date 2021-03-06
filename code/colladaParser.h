#ifndef COLLADAPARSER_H
#define COLLADAPARSER_H

#include "tinyxml.h"
#include "math.h"
#include "animatedModel.h"
#include "animation.h"
// c and c++ libraries
#include <vector>

struct JointIDsAndWeights
{
    std::vector<Vec3>  a;
    std::vector<IVec3> b;
};

struct VertexAndIndices
{
    std::vector<float> a;
    std::vector<int>   b;
    int offset;
    int size;
};

std::vector<int> ParseStringIntVector(char* string);
std::vector<float> ParseStringFloatVector(char* string);
std::vector<char*> ParseStringStringVector(char* string);
JointIDsAndWeights GetJointsIdsAndWeight(TiXmlElement* controllers);
VertexAndIndices   GetVertexAndIndices(TiXmlElement* geometries);
Joint GetJointsStructure(TiXmlElement* visualScene);
bool LoadColladaFile(unsigned int* vao,
                     unsigned int* textId,
                     Animation* animation,
                     Joint* jointStruct,
                     int* jointCount,
                     int* numberVertices,
                     const char* xmlFilePath,
                     const char* textureFilePath,
                     bool haveEBO);

#endif
