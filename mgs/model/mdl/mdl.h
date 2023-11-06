#pragma once
#include <inttypes.h>

struct vec3Float {
	float x;
	float y;
	float z;
};

struct vec4Float {
	float x;
	float y;
	float z;
	float w;
};

struct vec2Short {
	int16_t x;
	int16_t y;
};

struct vec3Short {
	int16_t x;
	int16_t y;
	int16_t z;
};

struct vec4Short {
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t w;
};

struct MdlHeader {
	uint32_t magic;
	uint32_t numBones;
	uint32_t numGroups;
	uint32_t numMesh;
	uint32_t fileStrcode;
	uint32_t pad;
	uint32_t boneOffset;
	uint32_t pad2;
	uint32_t groupOffset;
	uint32_t pad3;
	uint32_t meshOffset;
	uint32_t pad4;
	vec4Float max;
	vec4Float min;
};

struct MdlBone {
	uint32_t strcode;
	uint32_t flag;
	int32_t parent;
	uint32_t pad;
	vec4Float parentPos;
	vec4Float worldPos;
	vec4Float max;
	vec4Float min;
};

struct MdlGroup {
	uint32_t strcode;
	uint32_t unknown;
	uint32_t parent;
	uint32_t padding;
};

struct MdlMesh
{
	uint32_t strcode;
	uint32_t flag;
	uint32_t numVertexDefinition;
	uint32_t pad;
	uint32_t vertexDefinitionOffset;
	uint32_t pad2[3];
	uint32_t numSkin;
	uint32_t padding;
	uint8_t skinningTable[8];
	vec4Float max;
	vec4Float min;
};


struct MdlMeshB
{
	uint32_t strcode;
	uint32_t flag;
	uint32_t numVertexDefinition;
	uint32_t pad;
	uint32_t vertexDefinitionOffset;
	uint32_t pad2[3];
	vec4Float max;
	vec4Float min;
	vec4Float unk;
};

struct MdlVertexDefinition {
	union {
		struct A {
			uint32_t flag;
			uint32_t numVertexIndex;
			uint32_t stride;
			uint32_t pad;
			uint32_t textureStrcode;
			uint32_t pad2;
			uint32_t texture2Strcode;
			uint32_t pad3;
			uint32_t texture3Strcode;
			uint32_t pad4;
			uint32_t vertexIndexOffset;
			uint32_t pad5;
			uint32_t colourOffset;
			uint32_t pad6[3];
		} A;
		struct B {
			uint16_t flag;
			uint16_t numWeights;
			uint8_t skinningTable[8];
			uint16_t numVertexIndex;
			uint16_t stride;
			uint32_t textureStrcode;
			uint32_t pad2;
			uint32_t texture2Strcode;
			uint32_t pad3;
			uint32_t texture3Strcode;
			uint32_t pad4;
			uint32_t vertexIndexOffset;
			uint32_t pad5;
			uint32_t colourOffset;
			uint32_t pad6[3];
		} B;
	};
};

struct MdlVertexDefinitionAB {
	uint32_t flag;
	uint32_t numVertexIndex;
	uint32_t stride;
	uint32_t textureStrcode;
	uint32_t texture2Strcode;
	uint32_t texture3Strcode;
	uint32_t vertexIndexOffset;
	uint32_t colourOffset;
	uint16_t numWeights;
	uint8_t* skinningTable;
};

struct MdlVertexIndexA {
	vec3Short vertices;
	uint16_t face;
	vec3Short normals;
	uint8_t w1;
	uint8_t w2;
	vec2Short uv;
	uint8_t w3;
	uint8_t w4;
	uint8_t w5;
	uint8_t w6;
};

struct MdlVertexIndexB {
	vec3Short vertices;
	uint16_t face;
	vec3Short normals;
	uint8_t w1;
	uint8_t w2;
	vec2Short uv;
	uint8_t w3;
	uint8_t w4;
	uint8_t w5;
	uint8_t w6;
	vec2Short uv2;
	uint16_t unknown3;
	uint16_t unknown4;
};

struct MdlVertexIndexC {
	vec3Short vertices;
	uint16_t face;
	vec3Short normals;
	uint8_t w1;
	uint8_t w2;
	vec2Short uv;
	uint8_t w3;
	uint8_t w4;
	uint8_t w5;
	uint8_t w6;
	vec2Short uv2;
	uint16_t unknown3;
	uint16_t unknown4;
	vec2Short uv3;
	uint16_t unknown7;
	uint16_t unknown8;
	uint16_t unknown9;
	uint16_t unknown10;
	uint16_t unknown11;
	uint16_t unknown12;
};

inline
void mdlBHeadertoC(MdlHeader* header) 
{
	header->meshOffset = header->groupOffset;
	header->groupOffset = header->boneOffset;
	header->boneOffset = 0;
	header->numBones = 0;
}