#pragma once
#include "../common/fileutil.h"
#include "../../PS2/PS2Textures.h"

struct GsTex0
{
	uint64_t TBP0 : 14; // Texture Buffer Base Pointer (Address/256)
	uint64_t TBW : 6; // Texture Buffer Width (Texels/64)
	uint64_t PSM : 6; // Pixel Storage Format (0 = 32bit RGBA)
	uint64_t TW : 4; // width = 2^TW
	uint64_t TH : 4; // height = 2^TH
	uint64_t TCC : 1; // 0 = RGB, 1 = RGBA
	uint64_t TFX : 2; // TFX  - Texture Function (0=modulate, 1=decal, 2=hilight, 3=hilight2)
	uint64_t CBP : 14; // CLUT Buffer Base Pointer
	uint64_t CPSM : 4; // CLUT Storage Format
	uint64_t CSM : 1; // CLUT Storage Mode
	uint64_t CSA : 5; // CLUT Offset
	uint64_t CLD : 3; // CLUT Load Control
};

struct TriHeader
{
	uint32_t pad;
	int32_t width;
	int32_t height;
	uint32_t clutHeight;
	int32_t numTexture;
	uint32_t pad1;
	int32_t imageOffset;
	int32_t clutOffset;
};
struct TriInfo
{
	float uOffset;
	float vOffset;
	float uScale;
	float vScale;
	uint32_t strcode;
	uint32_t pad0;
	uint32_t pad1;
	uint32_t pad2;
	uint32_t padc[8];
	uint32_t unknownA;
	uint32_t unknownB;
	uint32_t unknownC;
	uint32_t pad3;
	uint32_t unknownD;
	uint32_t unknownE;
	uint32_t unknownF;
	uint32_t pad4;
	GsTex0 registerInfo;
	uint32_t unknownG;
	uint32_t unknownH;
	GsTex0 registerInfo2;
	uint32_t unknownI;
	uint32_t unknownJ;
	uint32_t unknownK;
	uint32_t unknownL;
	uint32_t unknownM;
	uint32_t unknownN;
	uint32_t unknownO;
	uint32_t unknownP;
	uint32_t unknownQ;
	uint32_t unknownR;
	float u1;
	float v1;
	float u2;
	float v2;
	float u3;
	float v3;
	uint32_t pad5;
	uint32_t pad6;
};

struct TriColour
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

class Tri
{
public:
	Tri(std::string filename);
	~Tri();
	int getNumInfo();
	void getAllTextures();
	uint32_t getStrcodeAtIndex(int idx);
	bool containsTexture(uint32_t strcode);
	uint8_t* getTextureIndexed(int idx, int& size);
	uint8_t* getTexture(uint32_t strcode, int& size, int& bpp);
private:
	int dataSize;
	uint8_t* triData;
	TriHeader* header;

	void initMemory();
	int getIdx(uint32_t strcode);
	uint8_t* paintPixels(TriColour* clut, uint8_t* pixels, int width, int height, int maxWidth, int& size, int16_t xOffset, int16_t yOffset);
};