#include "tool.h"

const char* g_pPluginName = "mgs_master_collection_mdl";
const char* g_pPluginDesc = "Metal Gear Solid Master Collection MDL Handle By Jayveer";

bool checkMDL(BYTE* fileBuffer, int bufferLen, noeRAPI_t* rapi)
{
    uint32_t magic = *(uint32_t*)fileBuffer;
    return magic == 0x2043444D || magic == 0x2042444D || magic == 0x6C43444D;
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