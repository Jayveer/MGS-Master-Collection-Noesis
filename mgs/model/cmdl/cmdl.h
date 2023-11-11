#pragma once
#include "../model.h"

typedef struct CmdlHeader
{
	uint32_t magic   = 'MODL';
	uint32_t version = 1;
	uint32_t extOffset;
	uint32_t numTypes;
} CmdlHeader;

typedef struct CmdlType
{
	uint32_t name;
	uint16_t field_04;
	uint16_t field_06;
	uint32_t offset;
	uint32_t pad;
	uint32_t size;
	uint32_t pad2[3];
};

//big_endian
typedef struct CmdlExtFace
{
	uint32_t numFace;
	uint32_t faces[];
} CmdlExtFace;

typedef struct CmdlExtDef
{
	vec3Float vec[2];
	uint16_t field_18;
	uint8_t field_19;
} CmdlExtDef;
