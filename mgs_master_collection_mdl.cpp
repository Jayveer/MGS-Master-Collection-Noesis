#include "tool.h"

#include "mgs/model/cmdl/cmdl.h"

const char* g_pPluginName = "mgs_master_collection_mdl";
const char* g_pPluginDesc = "Metal Gear Solid Master Collection MDL Handle By Jayveer";

bool checkMDL(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi)
{
    uint32_t magic = *(uint32_t*)fileBuffer;
    return magic == 0x2043444D || magic == 0x2042444D || magic == 0x6C43444D;
}

bool checkCMDL(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi)
{
    uint32_t magic = *(uint32_t*)fileBuffer;
    return magic == 0x4C444F4D;
}

bool checkEVM(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi)
{
    //there isn't really any way to check this
    return true;
}

bool checkKMS(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi)
{
    uint32_t magic = *(uint32_t*)fileBuffer;
    return magic == 0x00C81371 || magic == 0x00B253A3;
}


void readDec(std::vector<float>& dec, uint8_t* vertexBuffer, int32_t stride, int32_t numVertex) {
    for (int i = 0; i < numVertex; i++) {
        uint32_t bitstream = *(uint32_t*)vertexBuffer;

        RichBitStream bs = RichBitStream(&bitstream, 4);

        int32_t ax = bs.ReadBits(11); ax = negateBits(ax, 11);
        int32_t ay = bs.ReadBits(11); ay = negateBits(ay, 11);
        int32_t az = bs.ReadBits(10); az = negateBits(az, 10);

        float vx = ax / 1023.0f;
        float vy = ay / 1023.0f;
        float vz = az / 511.0f;

        dec.push_back(vx);
        dec.push_back(vy);
        dec.push_back(vz);

        vertexBuffer += stride;
    }
}

noesisModel_t* loadCMDL(BYTE* fileBuffer, int bufferLen, int& numMdl, noeRAPI_t* rapi)
{
    BYTE       *cmdlFile = &fileBuffer[0xC];
    CmdlHeader *header   = (CmdlHeader*)fileBuffer;
    CmdlType   *types    = (CmdlType*)&fileBuffer[0x10];

    void* ctx = rapi->rpgCreateContext();
    rapi->rpgSetOption(RPGOPT_GEOTWOSIDEDPRV, 1);

    std::vector<float>    vertexBuffer;
    std::vector<float>    normalBuffer;
    std::vector<uint32_t> faceBuffer;

    for (int i = 0; i < header->numTypes; i++)
    {
        uint32_t offset = types[i].offset + 0x0C;

        switch (types[i].name)
        {
        case 'POS0':
        {
            
            vec4Float* vec = (vec4Float*)&fileBuffer[offset];
            for (int j = 0; j < types[i].size / 16; j++)
            {                
                vertexBuffer.push_back(vec[j].x);
                vertexBuffer.push_back(vec[j].y);
                vertexBuffer.push_back(vec[j].z);
            }
        }
        break;
        case 'OIDX':
        {            
        }
        break;
        case 'NRM0':
        {
            readDec(normalBuffer, &fileBuffer[offset], 4, types[i].size / 4);
        }
        break;
        }
    }

    CmdlExtFace *face = (CmdlExtFace*)&cmdlFile[_byteswap_ulong(header->extOffset)];

    int x = 0;
    bool flip = 0;

    for (int i = 0; i < _byteswap_ulong(face->numFace); i+=3)
    {
        if (!flip) 
        {
            faceBuffer.push_back(_byteswap_ulong(face->faces[i + 0]));
            faceBuffer.push_back(_byteswap_ulong(face->faces[i + 1]));
            faceBuffer.push_back(_byteswap_ulong(face->faces[i + 2]));
        }
        else
        {
            faceBuffer.push_back(_byteswap_ulong(face->faces[i + 0]));
            faceBuffer.push_back(_byteswap_ulong(face->faces[i + 2]));
            faceBuffer.push_back(_byteswap_ulong(face->faces[i + 1]));
        }
    }

    rapi->rpgBindPositionBufferSafe(&vertexBuffer[0], RPGEODATA_FLOAT, 12, vertexBuffer.size() * 4);
    rapi->rpgBindNormalBufferSafe(&normalBuffer[0], RPGEODATA_FLOAT, 12, normalBuffer.size() * 4);
    rapi->rpgCommitTriangles(&faceBuffer[0], RPGEODATA_INT, faceBuffer.size(), RPGEO_TRIANGLE, 0);

    rapi->rpgClearBufferBinds();

    noesisModel_t* mdl = rapi->rpgConstructModel();
    if (mdl) numMdl = 1;

    rapi->rpgDestroyContext(ctx);
    return mdl;
}

noesisModel_t* loadEVM(BYTE* fileBuffer, int bufferLen, int& numMdl, noeRAPI_t* rapi)
{
    EvmHeader* header = (EvmHeader*)fileBuffer;
    EvmBone* bones = (EvmBone*)&fileBuffer[0x40];
    EvmVertexDefinition* vDef = (EvmVertexDefinition*)&fileBuffer[header->vertexDefinitionOffset];

    void* ctx = rapi->rpgCreateContext();

    modelBone_t* noeBones = (header->numBones) ? bindEVMBones(bones, header->numBones, rapi) : NULL;

    CArrayList<noesisTex_t*>      texList;
    CArrayList<noesisMaterial_t*> matList;

    for (int i = 0; i < header->numVertexDefinition; i++) {
        bindEVMMesh(&vDef[i], fileBuffer, rapi, matList, texList);
    }

    noesisMatData_t* md = rapi->Noesis_GetMatDataFromLists(matList, texList);
    rapi->rpgSetExData_Materials(md);

    if (g_mgsMasterCollectionMtarPrompt && header->numBones)
    {
        BYTE* motionFile = openMarMotion(rapi);
        if (motionFile) loadMarMotion(rapi, motionFile, noeBones, header->numBones);
    }

    noesisModel_t* mdl = rapi->rpgConstructModel();
    if (mdl) numMdl = 1;

    rapi->rpgDestroyContext(ctx);
    return mdl;
}

noesisModel_t* loadKMS(BYTE* fileBuffer, int bufferLen, int& numMdl, noeRAPI_t* rapi)
{
    KmsHeader* header = (KmsHeader*)fileBuffer;
    KmsMesh* mesh = (KmsMesh*)&fileBuffer[0x40];

    void* ctx = rapi->rpgCreateContext();

    modelBone_t* noeBones = bindKMSBones(mesh, header->numMesh, rapi);

    CArrayList<noesisTex_t*>      texList;
    CArrayList<noesisMaterial_t*> matList;

    for (int i = 0; i < header->numMesh; i++)
    {
        bindKMSMesh(&mesh[i], &noeBones[i], i, fileBuffer, rapi, matList, texList);
    }

    noesisMatData_t* md = rapi->Noesis_GetMatDataFromLists(matList, texList);
    rapi->rpgSetExData_Materials(md);

    if (g_mgsMasterCollectionMtarPrompt && header->numBones)
    {
        BYTE* motionFile = openMarMotion(rapi);
        if (motionFile) loadMarMotion(rapi, motionFile, noeBones, header->numMesh);
    }

    noesisModel_t* mdl = rapi->rpgConstructModel();
    if (mdl) numMdl = 1;

    rapi->rpgDestroyContext(ctx);
    return mdl;
}

noesisModel_t* loadMDL(BYTE* fileBuffer, int bufferLen, int& numMdl, noeRAPI_t* rapi)
{
    MdlHeader* header = (MdlHeader*)fileBuffer;
    bool isMDB = header->magic == 0x2042444D;
    bool isMDC1 = header->magic == 0x6C43444D;
    if (isMDB) mdlBHeadertoC(header);

    MdlBone  *bones  = (MdlBone*) &fileBuffer[header->boneOffset];
    MdlGroup *groups = (MdlGroup*)&fileBuffer[header->groupOffset];
    MdlMesh  *mesh   = (MdlMesh*) &fileBuffer[header->meshOffset];

    void* ctx = rapi->rpgCreateContext();
    rapi->rpgSetOption(RPGOPT_GEOTWOSIDEDPRV, 1);

    modelBone_t* noeBones = (header->numBones) ? bindBones(bones, header->numBones, rapi) : NULL;

    CArrayList<noesisTex_t*>      texList;
    CArrayList<noesisMaterial_t*> matList;

    for (int i = 0; i < header->numMesh; i++)
    {
        bindMesh(&mesh[i], fileBuffer, rapi, matList, texList, isMDB, isMDC1);
    }

    noesisMatData_t* md = rapi->Noesis_GetMatDataFromLists(matList, texList);
    rapi->rpgSetExData_Materials(md);

    if (g_mgsMasterCollectionMtarPrompt && header->numBones)
    {
        BYTE* motionFile = openMotion(rapi);
        if (motionFile) loadMotion(rapi, motionFile, noeBones, header->numBones);
    }

    noesisModel_t* mdl = rapi->rpgConstructModel();
    if (mdl) numMdl = 1;

    rapi->rpgDestroyContext(ctx);
    return mdl;

    return NULL;
}

bool NPAPI_InitLocal(void)
{
    int fh = g_nfn->NPAPI_Register("Metal Gear Solid 3 Master Collection", ".mdl");
    if (fh < 0) return false;

    g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh, checkMDL);
    g_nfn->NPAPI_SetTypeHandler_LoadModel(fh, loadMDL);

    fh = g_nfn->NPAPI_Register("Metal Gear Solid 2", ".evm");
    if (fh < 0) return false;

    g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh, checkEVM);
    g_nfn->NPAPI_SetTypeHandler_LoadModel(fh, loadEVM);

    fh = g_nfn->NPAPI_Register("Metal Gear Solid 2", ".kms");
    if (fh < 0) return false;

    g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh, checkKMS);
    g_nfn->NPAPI_SetTypeHandler_LoadModel(fh, loadKMS);

    fh = g_nfn->NPAPI_Register("Metal Gear Solid Master Collection CMDL", ".cmdl");
    if (fh < 0) return false;

    g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh, checkCMDL);
    g_nfn->NPAPI_SetTypeHandler_LoadModel(fh, loadCMDL);


    applyTools();

    return true;
}


void NPAPI_ShutdownLocal(void)
{

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}