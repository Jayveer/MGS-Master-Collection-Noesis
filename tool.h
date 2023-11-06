#pragma once
#include "mat.h"
#include "bone.h"
#include "mesh.h"
#include "motion.h"


bool g_mgsMasterCollectionMtarPrompt = false;
const char* g_mgsMasterCollectionplugin_name = "Metal Gear Solid 3: Master Collection";

inline
int genericToolSet(bool& setting, int toolIdx)
{
    setting = !setting;
    g_nfn->NPAPI_CheckToolMenuItem(toolIdx, setting);
    return 1;
}

int mgs_mc_anim_prompt(int toolIdx, void* user_data)
{
    return genericToolSet(g_mgsMasterCollectionMtarPrompt, toolIdx);
}

inline
int makeTool(char* toolDesc, int (*toolMethod)(int toolIdx, void* userData))
{
    int handle = g_nfn->NPAPI_RegisterTool(toolDesc, toolMethod, NULL);
    g_nfn->NPAPI_SetToolSubMenuName(handle, g_mgsMasterCollectionplugin_name);
    return handle;
}

inline
void applyTools()
{
    makeTool("Prompt for Motion Archive", mgs_mc_anim_prompt);
}
