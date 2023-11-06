#include "tri.h"

Tri::Tri(std::string filename)
{
	std::ifstream fs;
	this->dataSize = std::filesystem::file_size(filename);

	fs.open(filename, std::ios::binary);
	uint8_t* p = new uint8_t[dataSize];
	fs.read((char*)p, dataSize);
	this->triData = p;
	fs.close();

	this->header = (TriHeader*)triData;
	initMemory();
}

Tri::~Tri()
{
	delete[] triData;
}

bool Tri::containsTexture(uint32_t strcode)
{
	return getIdx(strcode) > -1;
}

uint32_t Tri::getStrcodeAtIndex(int idx)
{
	TriInfo* info = (TriInfo*)&triData[0x20];

	return idx < header->numTexture ? info[idx].strcode : 0;
}

void Tri::getAllTextures()
{
	for (int i = 0; i < header->numTexture; i++)
	{
		int size, bpp;
		uint8_t* texture = getTextureIndexed(i, size);
		delete[] texture;
	}
}

int Tri::getIdx(uint32_t strcode)
{
	TriInfo* info = (TriInfo*)&triData[0x20];

	for (int i = 0; i < header->numTexture; i++) 
	{
		if (info[i].strcode == strcode) return i;
	}

	return -1;
}

uint8_t* Tri::getTexture(uint32_t strcode, int& size, int& bpp) {
	int idx = getIdx(strcode);
	if (idx == -1) return NULL;
	return getTextureIndexed(idx, size);
}

uint8_t* makeTGA(uint8_t* data, int dataSize, int16_t width, int16_t height) {
	uint32_t pad = 0;
	uint16_t twenty = 0x2020;
	uint32_t magic = 0x20000;
	uint8_t* tga = new uint8_t[0x12 + dataSize];

	memcpy(tga + 0x00, &magic, 4);
	memcpy(tga + 0x04, &pad, 4);
	memcpy(tga + 0x08, &pad, 4);
	memcpy(tga + 0x0C, &width, 2);
	memcpy(tga + 0x0E, &height, 2);
	memcpy(tga + 0x10, &twenty, 2);
	memcpy(tga + 0x12, data, dataSize);

	return tga;
}

void Tri::initMemory()
{
	int width = 64;
	writeTexPSMCT32(0, 1, 0, 0, width, header->height, 0, &triData[header->imageOffset]);
	writeTexPSMCT32(0, 1, 0, 0, width, header->clutHeight, 1, &triData[header->clutOffset]);
}

uint8_t extendAlpha(uint8_t a) {
	return uint8_t((a / 80) * 255);
}

uint8_t* Tri::paintPixels(TriColour* clut, uint8_t* pixels, int width, int height, int maxWidth, int& size, int16_t xOffset, int16_t yOffset)
{
	size = width * height * 4;
	uint8_t* texture = new uint8_t[size];
	int i = 0;

	for (int y = yOffset; y < yOffset + height; y++)
	{

		for (int x = xOffset; x < xOffset + width; x++)
		{
			int pixelPos = x + y * maxWidth;
			int pos = i * 4;

			texture[pos + 0] = clut[pixels[pixelPos]].b;
			texture[pos + 1] = clut[pixels[pixelPos]].g;
			texture[pos + 2] = clut[pixels[pixelPos]].r;
			texture[pos + 3] = extendAlpha(clut[pixels[pixelPos]].a);
			i++;
		}

	}
	return texture;
}

uint8_t* bpp4to8(uint8_t* src, int& size)
{
	size *= 2;
	uint8_t* expanded = new uint8_t[size];

	int pos = 0;
	for (int i = 0; i < size / 2; i++)
	{
		pos = i * 2;
		expanded[pos + 0] = (src[i] & 0xF0) >> 4;
		expanded[pos + 1] = src[i] & 0x0F;
	}

	return expanded;
}

void unswizzleClut(uint8_t* clutBuffer)
{
	char temp[32];

	for (int i = 1; i <= 29; i += 4)
	{
		memcpy(&temp, &clutBuffer[i * 32], 32);
		memcpy(&clutBuffer[i * 32], &clutBuffer[(i + 1) * 32], 32);
		memcpy(&clutBuffer[(i + 1) * 32], &temp, 32);
	}
}

uint8_t* Tri::getTextureIndexed(int idx, int& size)
{
	TriInfo* info = (TriInfo*)&triData[0x20];
	info = &info[idx];

	int imageWidth = (int)powf(2, (float)info->registerInfo2.TW);
	int imageHeight = (int)powf(2, (float)info->registerInfo2.TH);

	//calculation of actual texture sizes
	int texX = (int)(info->uOffset * imageWidth);
	int texY = (int)(info->vOffset * imageHeight);
	int texWidth = (int)(info->uScale * imageWidth) + 1;
	int texHeight = (int)(info->vScale * imageHeight) + 1;

	int memsize = 1024 * 1024 * 4;
	uint8_t* texBuffer = new uint8_t[memsize]();
	uint8_t* clutBuffer = new uint8_t[memsize]();

	int clutWidth = 8;
	int clutHeight = 2;

	//switch on psm
	switch (info->registerInfo2.PSM) {
	case 0x13:
		clutWidth = 16;
		clutHeight = 16;
		size = texWidth * texHeight;
		readTexPSMT8(info->registerInfo2.TBP0, info->registerInfo2.TBW, texX, texY, texWidth, texHeight, 0, (void*)texBuffer);
		break;
	case 0x14:
		clutWidth = 8;
		clutHeight = 2;
		size = texWidth * texHeight / 2;
		readTexPSMT4(info->registerInfo2.TBP0, info->registerInfo2.TBW, texX, texY, texWidth, texHeight, 0, (void*)texBuffer);
		break;
	}


	if ((info->registerInfo2.CPSM == 0) && (info->registerInfo2.CSM == 0))
	{
		readTexPSMCT32(info->registerInfo2.CBP, 1, (int)(info->registerInfo2.CSA * 8), 0, clutWidth, clutHeight, 1, (void*)clutBuffer);
		if (info->registerInfo2.PSM == 0x13) unswizzleClut(clutBuffer);
	}
	else
	{
		int dummy = 0;
	}

	uint8_t* expandedOut = texBuffer;

	if (info->registerInfo2.PSM == 0x14)
	{
		expandedOut = bpp4to8(texBuffer, size);
		delete[] texBuffer;
	}

	uint8_t* pixels = paintPixels((TriColour*)clutBuffer, expandedOut, texWidth, texHeight, texWidth, size, 0, 0);
	delete[] expandedOut;
	delete[] clutBuffer;

	uint8_t* tga = makeTGA(pixels, size, texWidth, texHeight);
	delete[] pixels;

	size += 0x12;

	return tga;
}