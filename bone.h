#pragma once
#include "mgs/common/util.h"
#include "mgs/model/kms/kms.h"
#include "mgs/model/evm/evm.h"
#include "mgs/model/mdl/mdl.h"
#include "noesis/plugin/pluginshare.h"

inline
int findBoneIdx(char* boneName, modelBone_t* noeBones, int numBones) {
    for (int i = 0; i < numBones; i++) {
        if (!strcmp(noeBones[i].name, boneName))
            return i;
    }
    return -1;
}

inline
modelBone_t* bindBones(MdlBone* bones, int numBones, noeRAPI_t* rapi) {
    modelBone_t* noeBones = rapi->Noesis_AllocBones(numBones);

    for (int i = 0; i < numBones; i++) {
        std::string boneName = intToHexString(bones[i].strcode);
        strcpy_s(noeBones[i].name, boneName.c_str());

        RichVec4 bonePos = { bones[i].worldPos.x, bones[i].worldPos.y, bones[i].worldPos.z, bones[i].worldPos.w };
        RichVec3 bonePosV3 = bonePos.ToVec3();
        memcpy_s(&noeBones[i].mat.o, 12, &bonePosV3, 12);

        if (bones[i].parent > -1)
            noeBones[i].eData.parent = &noeBones[bones[i].parent];
    }

    rapi->rpgSetExData_Bones(noeBones, numBones);
    return noeBones;
}

inline
modelBone_t* bindKMSBones(KmsMesh* mesh, int numBones, noeRAPI_t* rapi)
{
    modelBone_t* noeBones = rapi->Noesis_AllocBones(numBones);

    for (int i = 0; i < numBones; i++) 
    {
        RichVec3 bonePosV3 = { mesh[i].pos.x, mesh[i].pos.y, mesh[i].pos.z };
        memcpy_s(&noeBones[i].mat.o, 12, &bonePosV3, 12);

        if (mesh[i].parent > -1) 
        {
            noeBones[i].eData.parent = &noeBones[mesh[i].parent];
            RichMat43 cMat(noeBones[i].mat);
            RichMat43 pMat(noeBones[i].eData.parent->mat);
            noeBones[i].mat = (cMat * pMat).m;
        }

    }

    rapi->rpgSetExData_Bones(noeBones, numBones);
    return noeBones;
}

inline
modelBone_t* bindEVMBones(EvmBone* bones, int numBones, noeRAPI_t* rapi)
{
    modelBone_t* noeBones = rapi->Noesis_AllocBones(numBones);

    for (int i = 0; i < numBones; i++)
    {
        RichVec3 bonePosV3 = { bones[i].worldPos.x, bones[i].worldPos.y, bones[i].worldPos.z };
        memcpy_s(&noeBones[i].mat.o, 12, &bonePosV3, 12);

        if (bones[i].parent > -1)
            noeBones[i].eData.parent = &noeBones[bones[i].parent];
    }

    rapi->rpgSetExData_Bones(noeBones, numBones);
    return noeBones;
}