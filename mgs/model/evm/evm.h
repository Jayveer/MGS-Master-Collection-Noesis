#pragma once
#include "../model.h"

struct EvmHeader
{
	uint32_t numUnknown;
	uint32_t numBones;
	vec3Float max;
	vec3Float min;
	uint32_t strcode;
	uint32_t pad;
	uint32_t flag;
	int32_t numVertexDefinition;
	uint32_t vertexDefinitionOffset;
	uint32_t pad2[3];
};

struct EvmBone
{
	uint32_t pad;
	int32_t parent;
	vec3Float parentPos;
	vec3Float worldPos;
	vec4Float max;
	vec4Float min;
};

struct EvmVertexDefinition
{
	uint32_t flag;
	uint32_t pad;
	uint32_t textureStrcode;
	uint32_t pad2;
	uint32_t texture2Strcode;
	uint32_t pad3;
	uint32_t texture3Strcode;
	uint32_t pad4;
	uint32_t numVertex;
	uint32_t numSkin;
	uint8_t skinningTable[8];
	uint32_t vertexOffset;
	uint32_t pad5;
	uint32_t normalOffset;
	uint32_t pad6;
	uint32_t uvOffset;
	uint32_t pad7;
	uint32_t uv2Offset;
	uint32_t pad8;
	uint32_t uv3Offset;
	uint32_t pad9;
	uint32_t weightOffset;
	uint32_t pad10;
	uint32_t pad11[4];
};