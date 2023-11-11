#pragma once
#include "mat.h"
#include <vector>
#include "mgs/model/kms/kms.h"
#include "mgs/model/evm/evm.h"
#include "mgs/model/mdl/mdl.h"

inline
MdlVertexDefinitionAB makeJointVertexDef(MdlVertexDefinition* vertDef, MdlMesh* mesh, bool isMDB)
{
    bool vdB = isMDB ? 0 : mesh->flag & 0x0F;
    MdlVertexDefinitionAB vertDefAB;

    vertDefAB.flag = vdB ? vertDef->B.flag : vertDef->A.flag;
    vertDefAB.numVertexIndex = vdB ? vertDef->B.numVertexIndex : vertDef->A.numVertexIndex;
    vertDefAB.stride = vdB ? vertDef->B.stride : vertDef->A.stride;
    vertDefAB.textureStrcode = vdB ? vertDef->B.textureStrcode : vertDef->A.textureStrcode;
    vertDefAB.texture2Strcode = vdB ? vertDef->B.texture2Strcode : vertDef->A.texture2Strcode;
    vertDefAB.texture3Strcode = vdB ? vertDef->B.texture3Strcode : vertDef->A.texture3Strcode;
    vertDefAB.vertexIndexOffset = vdB ? vertDef->B.vertexIndexOffset : vertDef->A.vertexIndexOffset;
    vertDefAB.colourOffset = vdB ? 0 : vertDef->A.colourOffset;
    vertDefAB.numWeights = vdB ? vertDef->B.numWeights : 2;
    vertDefAB.skinningTable = vdB ? vertDef->B.skinningTable : mesh->skinningTable;

    return vertDefAB;
}

inline
void bindVertex(int16_t* fvf, std::vector<float>& vertexBuffer, float scale) {
    vertexBuffer.push_back(fvf[0] * scale);
    vertexBuffer.push_back(fvf[1] * scale);
    vertexBuffer.push_back(fvf[2] * scale);
}

inline
void bindNormal(int16_t* fvf, std::vector<float>& normalBuffer) {
    float scale = 1.0f / 4096.0f;

    normalBuffer.push_back(fvf[4] * scale);
    normalBuffer.push_back(fvf[5] * scale);
    normalBuffer.push_back(fvf[6] * scale);
}

inline
void bindFace(int16_t* fvf, uint16_t fc, std::vector<uint16_t>& faceBuffer, bool& flip)
{
    if (fvf[3] & 0x8000) return;

    uint16_t fa = fc < 3 ? 0 : fc - 2;
    uint16_t fb = fc < 2 ? 0 : fc - 1;

    if (!flip) 
    {
        faceBuffer.push_back(fa);
        faceBuffer.push_back(fb);
        faceBuffer.push_back(fc);
    }
    else
    {
        faceBuffer.push_back(fa);
        faceBuffer.push_back(fc);
        faceBuffer.push_back(fb);
    }

    flip = !flip;
}

inline
void bindUV(int16_t* fvf, uint32_t flag, std::vector<float>& uvBuffer, std::vector<float>& uvBuffer2, std::vector<float>& uvBuffer3)
{
    float scale = 1.0f / 4096.0f;

    if (flag & 0x40)
    {
        uvBuffer.push_back(fvf[8] * scale);
        uvBuffer.push_back(fvf[9] * scale);
    }

    if (flag & 0x80)
    {
        uvBuffer2.push_back(fvf[12] * scale);
        uvBuffer2.push_back(fvf[13] * scale);
    }

    if (flag & 0x100)
    {
        uvBuffer3.push_back(fvf[16] * scale);
        uvBuffer3.push_back(fvf[17] * scale);
    }
}

inline
void bindUV(int16_t* uvs, std::vector<float>& uvBuffer) {
    float scale = 1.0f / 4096.0f;
    uvBuffer.push_back(uvs[0] * scale);
    uvBuffer.push_back(uvs[1] * scale);
}

inline
void bindSkin(int16_t* fvf, uint16_t numWeights, uint8_t* skinningTable, std::vector<float>& weightBuffer, std::vector<uint8_t>& boneBuffer, bool isExtended)
{
    if (isExtended)
    {
        for (int x = 7; x < 4 + (numWeights * 4); x += 4)
        {
            int idx = fvf[x] >> 2;
            boneBuffer.push_back(skinningTable[idx]);
        }

        for (int x = 20; x < 20 + numWeights; x++)
        {
            weightBuffer.push_back(fvf[x] / 4096.0);
        }
    }
    else
    {
        float weight = fvf[7] / 4096.0;
        int idx = fvf[11] >> 2;

        boneBuffer.push_back(skinningTable[0]);   weightBuffer.push_back(weight);
        boneBuffer.push_back(skinningTable[idx]); weightBuffer.push_back(1.0 - weight);
    }
}

inline
void setOrigin(MdlMesh* mesh, noeRAPI_t* rapi)
{
    modelMatrix_t t = g_identityMatrix;
    g_mfn->Math_VecCopy(&mesh->min.x, t.o);
    rapi->rpgSetTransform(&t);
}

inline
void setOrigin(KmsMesh* mesh, modelBone_t* noeBone, noeRAPI_t* rapi)
{
    modelMatrix_t t = g_identityMatrix;
    g_mfn->Math_VecCopy(noeBone->mat.o, t.o);
    rapi->rpgSetTransform(&t);
}

inline
void resetWinding(int16_t* index, bool& flip)
{
    if (index[3] & 0x8000) flip = 1;
}

inline
void bindMesh(MdlMesh* mesh, BYTE* fileBuffer, noeRAPI_t* rapi, CArrayList<noesisMaterial_t*>& matList, CArrayList<noesisTex_t*>& texList, bool isMDB, bool isMDC1)
{
    float scale = 1.0f;
    if (!isMDB && !isMDC1) scale /= 16.0f;

    if (isMDB) setOrigin(mesh, rapi);

    int x = 1;
    for (int i = 0; i < mesh->numVertexDefinition; i++)
    {
        std::vector<float> uvBuffer, uvBuffer2, uvBuffer3, weightBuffer, vertexBuffer;
        std::vector<uint8_t>  boneBuffer;
        std::vector<uint16_t> faceBuffer;
        std::vector<float> normalBuffer;

        MdlVertexDefinition* vertDef = (MdlVertexDefinition*)&fileBuffer[mesh->vertexDefinitionOffset];
        MdlVertexDefinitionAB vertDefAB = makeJointVertexDef(&vertDef[i], mesh, isMDB);
        void* vertexIndex = &fileBuffer[vertDefAB.vertexIndexOffset];

        bool flip = 0;
        for (int j = 0; j < vertDefAB.numVertexIndex; j++)
        {

            int16_t* fvf = (int16_t*)vertexIndex;

            bindVertex(fvf, vertexBuffer, scale);
            bindFace(fvf, j, faceBuffer, flip);
            bindNormal(fvf, normalBuffer);
            if (!isMDB) bindSkin(fvf, vertDefAB.numWeights, vertDefAB.skinningTable, weightBuffer, boneBuffer, vertDefAB.stride == 6);
            bindUV(fvf, vertDefAB.flag, uvBuffer, uvBuffer2, uvBuffer3);

            if (fvf[3] & 0x8000) flip = 0;

            vertexIndex = (uint8_t*)vertexIndex + (vertDefAB.stride * 8);
            x++;
        }

        uint32_t textures[3] = { vertDefAB.textureStrcode, vertDefAB.texture2Strcode, vertDefAB.texture3Strcode };
        bindMat(textures, fileBuffer, rapi, matList, texList);

        if (!isMDB)
        {
            rapi->rpgBindBoneWeightBuffer(&weightBuffer[0], RPGEODATA_FLOAT, vertDefAB.numWeights * 4, vertDefAB.numWeights);
            rapi->rpgBindBoneIndexBuffer(&boneBuffer[0], RPGEODATA_UBYTE, vertDefAB.numWeights, vertDefAB.numWeights);
        }

        if (!uvBuffer.empty()) rapi->rpgBindUV1BufferSafe(&uvBuffer[0], RPGEODATA_FLOAT, 8, uvBuffer.size() * 4);
        if (!uvBuffer2.empty()) rapi->rpgBindUV2BufferSafe(&uvBuffer2[0], RPGEODATA_FLOAT, 8, uvBuffer2.size() * 4);
        if (!uvBuffer3.empty()) rapi->rpgBindUVXBufferSafe(&uvBuffer3[0], RPGEODATA_FLOAT, 8, 2, 2, uvBuffer3.size() * 4);

        rapi->rpgBindNormalBufferSafe(&normalBuffer[0], RPGEODATA_FLOAT, 12, normalBuffer.size() * 4);
        rapi->rpgBindPositionBufferSafe(&vertexBuffer[0], RPGEODATA_FLOAT, 12, vertexBuffer.size() * 4);
        rapi->rpgCommitTriangles(&faceBuffer[0], RPGEODATA_USHORT, faceBuffer.size(), RPGEO_TRIANGLE, 0);
        rapi->rpgClearBufferBinds();
    }
}

inline
void bindKMSNormal(int16_t* normals, std::vector<float>& normalBuffer)
{
    float scale = 1.0f / 4096.0f;
    normalBuffer.push_back(normals[0] * scale);
    normalBuffer.push_back(normals[1] * scale);
    normalBuffer.push_back(normals[2] * scale);
}

inline
void bindKMSSkin(int16_t* vertices, KmsMesh* mesh, int meshNum, std::vector<float>& weightBuffer, std::vector<uint8_t>& boneBuffer)
{
    float weight = vertices[3] / 4096.0f;
    weightBuffer.push_back(weight);
    boneBuffer.push_back(meshNum);

    if (mesh->parent > -1)
    {
        weightBuffer.push_back(1.0f - weight);
        boneBuffer.push_back(mesh->parent);
    }
}

inline
void bindKMSMesh(KmsMesh* mesh, modelBone_t* noeBone, int meshNum, BYTE* fileBuffer, noeRAPI_t* rapi, CArrayList<noesisMaterial_t*>& matList, CArrayList<noesisTex_t*>& texList)
{
    setOrigin(mesh, noeBone, rapi);

    KmsVertexDefinition* vDef = (KmsVertexDefinition*)&fileBuffer[mesh->vertexDefinitionOffset];

    float scale = 1.0f;

    for (int i = 0; i < mesh->numVertexDefinition; i++) {
        std::vector<float>  vertexBuffer, normalBuffer, uvBuffer, uvBuffer2, uvBuffer3, weightBuffer;
        std::vector<uint16_t> faceBuffer;
        std::vector<uint8_t> boneBuffer;

        int16_t* uvIndex = (int16_t*)&fileBuffer[vDef[i].uvOffset];
        int16_t* uv2Index = (int16_t*)&fileBuffer[vDef[i].uv2Offset];
        int16_t* uv3Index = (int16_t*)&fileBuffer[vDef[i].uv3Offset];
        int16_t* vertexIndex = (int16_t*)&fileBuffer[vDef[i].vertexOffset];
        int16_t* normalIndex = (int16_t*)&fileBuffer[vDef[i].normalOffset];

        bool flip = 1;
        for (int j = 0; j < vDef[i].numVertex; j++) {

            bindVertex(vertexIndex, vertexBuffer, scale);
            bindKMSNormal(normalIndex, normalBuffer);
            bindFace(normalIndex, j, faceBuffer, flip);

            if (vDef->uvOffset)  bindUV(uvIndex, uvBuffer);
            if (vDef->uv2Offset) bindUV(uv2Index, uvBuffer2);
            if (vDef->uv3Offset) bindUV(uv3Index, uvBuffer3);

            resetWinding(normalIndex, flip);
            bindKMSSkin(vertexIndex, mesh, meshNum, weightBuffer, boneBuffer);

            uvIndex += 2;
            uv2Index += 2;
            uv3Index += 2;
            vertexIndex += 4;
            normalIndex += 4;
        }

        uint32_t textures[3] = { vDef[i].textureStrcode, vDef[i].texture2Strcode, vDef[i].texture3Strcode };
        bindMat(textures, fileBuffer, rapi, matList, texList);

        int numWeight = 1 + (mesh->parent > -1);

        if (!uvBuffer.empty())  rapi->rpgBindUV1BufferSafe(&uvBuffer[0], RPGEODATA_FLOAT, 8, uvBuffer.size() * 4);
        if (!uvBuffer2.empty()) rapi->rpgBindUV2BufferSafe(&uvBuffer2[0], RPGEODATA_FLOAT, 8, uvBuffer2.size() * 4);
        if (!uvBuffer3.empty()) rapi->rpgBindUVXBufferSafe(&uvBuffer3[0], RPGEODATA_FLOAT, 8, 2, 2, uvBuffer3.size() * 4);

        rapi->rpgBindBoneIndexBuffer(&boneBuffer[0], RPGEODATA_UBYTE, numWeight, numWeight);
        rapi->rpgBindBoneWeightBuffer(&weightBuffer[0], RPGEODATA_FLOAT, numWeight * 4, numWeight);
        rapi->rpgBindNormalBufferSafe(&normalBuffer[0], RPGEODATA_FLOAT, 12, normalBuffer.size() * 4);
        rapi->rpgBindPositionBufferSafe(&vertexBuffer[0], RPGEODATA_FLOAT, 12, vertexBuffer.size() * 4);
        rapi->rpgCommitTriangles(&faceBuffer[0], RPGEODATA_USHORT, faceBuffer.size(), RPGEO_TRIANGLE, 0);

        rapi->rpgClearBufferBinds();
    }

}

inline
void bindEVMSkin(uint8_t* skin, uint8_t* skinningTable, int32_t numSkin, std::vector<float>& weightBuffer, std::vector<uint8_t>& boneBuffer) {
    for (int i = 0; i < numSkin; i++) {
        weightBuffer.push_back(skin[i] / 128.0f);
    }

    for (int i = 4; i < 4 + numSkin; i++) {
        int idx = skin[i] >> 2;
        boneBuffer.push_back(skinningTable[idx]);
    }
}

inline
void bindEVMMesh(EvmVertexDefinition* vDef, BYTE* fileBuffer, noeRAPI_t* rapi, CArrayList<noesisMaterial_t*>& matList, CArrayList<noesisTex_t*>& texList) {
    std::vector<float>  vertexBuffer, normalBuffer, uvBuffer, uvBuffer2, uvBuffer3, weightBuffer;
    std::vector<uint16_t> faceBuffer;
    std::vector<uint8_t> boneBuffer;

    int16_t* uvIndex     = (int16_t*)&fileBuffer[vDef->uvOffset];
    int16_t* uv2Index    = (int16_t*)&fileBuffer[vDef->uv2Offset];
    int16_t* uv3Index    = (int16_t*)&fileBuffer[vDef->uv3Offset];
    uint8_t* weightIndex = (uint8_t*)&fileBuffer[vDef->weightOffset];
    int16_t* vertexIndex = (int16_t*)&fileBuffer[vDef->vertexOffset];
    int16_t* normalIndex = (int16_t*)&fileBuffer[vDef->normalOffset];

    float scale = 1.0f / 16.0f;

    bool flip = 1;
    for (int i = 0; i < vDef->numVertex; i++)
    {

        if (vDef->vertexOffset) bindVertex(vertexIndex, vertexBuffer, scale);
        if (vDef->vertexOffset) bindFace(vertexIndex, i, faceBuffer, flip);
        if (vDef->normalOffset) bindKMSNormal(normalIndex, normalBuffer);

        if (vDef->uvOffset)  bindUV(uvIndex, uvBuffer);
        if (vDef->uv2Offset) bindUV(uv2Index, uvBuffer2);
        if (vDef->uv3Offset) bindUV(uv3Index, uvBuffer3);

        resetWinding(vertexIndex, flip);
        if (vDef->weightOffset) bindEVMSkin(weightIndex, vDef->skinningTable, vDef->numSkin, weightBuffer, boneBuffer);

        uvIndex += 4;
        uv2Index += 4;
        uv3Index += 4;
        weightIndex += 8;
        vertexIndex += 4;
        normalIndex += 4;
    }

    uint32_t textures[3] = { vDef->textureStrcode, vDef->texture2Strcode, vDef->texture3Strcode };
    bindMat(textures, fileBuffer, rapi, matList, texList);

    if (!uvBuffer.empty())     rapi->rpgBindUV1BufferSafe(&uvBuffer[0], RPGEODATA_FLOAT, 8, uvBuffer.size() * 4);
    if (!uvBuffer2.empty())    rapi->rpgBindUV2BufferSafe(&uvBuffer2[0], RPGEODATA_FLOAT, 8, uvBuffer2.size() * 4);
    if (!uvBuffer3.empty())    rapi->rpgBindUVXBufferSafe(&uvBuffer3[0], RPGEODATA_FLOAT, 8, 2, 2, uvBuffer3.size() * 4);
    if (!normalBuffer.empty()) rapi->rpgBindNormalBufferSafe(&normalBuffer[0], RPGEODATA_FLOAT, 12, normalBuffer.size() * 4);
    if (!vertexBuffer.empty()) rapi->rpgBindPositionBufferSafe(&vertexBuffer[0], RPGEODATA_FLOAT, 12, vertexBuffer.size() * 4);

    if (!weightBuffer.empty()) rapi->rpgBindBoneIndexBuffer(&boneBuffer[0], RPGEODATA_UBYTE, vDef->numSkin, vDef->numSkin);
    if (!weightBuffer.empty()) rapi->rpgBindBoneWeightBuffer(&weightBuffer[0], RPGEODATA_FLOAT, vDef->numSkin * 4, vDef->numSkin);

    if (!faceBuffer.empty())   rapi->rpgCommitTriangles(&faceBuffer[0], RPGEODATA_USHORT, faceBuffer.size(), RPGEO_TRIANGLE, 0);
    rapi->rpgClearBufferBinds();
}

