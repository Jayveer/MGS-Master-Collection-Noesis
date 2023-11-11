#pragma once
#include "../model.h"

struct JointVertexDefinition
{
	uint32_t flag;
	uint32_t numVertex;
	uint32_t textureStrcode;
	uint32_t texture2Strcode;
	uint32_t texture3Strcode;
	uint32_t numSkin;
	uint8_t* skinningTable;
	uint32_t vertexOffset;
	uint32_t normalOffset;
	uint32_t uvOffset;
	uint32_t uv2Offset;
	uint32_t uv3Offset;
	uint32_t weightOffset;
};

struct KmsVertexDefinition
{
	uint32_t flag;
	uint32_t numVertex;
	uint32_t textureStrcode;
	uint32_t pad;
	uint32_t texture2Strcode;
	uint32_t pad2;
	uint32_t texture3Strcode;
	uint32_t pad3;
	uint32_t vertexOffset;
	uint32_t pad4;
	uint32_t normalOffset;
	uint32_t pad5;
	uint32_t uvOffset;
	uint32_t pad6;
	uint32_t uv2Offset;
	uint32_t pad7;
	uint32_t uv3Offset;
	uint32_t pad8[7];
};

struct KmsMesh
{
	uint32_t flag;
	uint32_t numVertexDefinition;
	vec3Float max;
	vec3Float min;
	vec3Float pos;
	int32_t parent;
	uint32_t vertexDefinitionOffset;
	uint32_t pad[7];
};

struct KmsHeader
{
	uint32_t type;
	uint32_t numMesh;
	int32_t numBones;
	uint32_t pad;
	uint32_t strcode;
	uint32_t pad2[2];
	vec3Float min;
	vec3Float max;
	vec3Float pos;
};