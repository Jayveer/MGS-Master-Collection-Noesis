#pragma once
#include "mgs/common/util.h"
#include "mgs/texture/tri.h"
#include "noesis/plugin/pluginshare.h"

inline
int findMaterialIdx(char* matName, CArrayList<noesisMaterial_t*>& matList)
{
    for (int i = 0; i < matList.Num(); i++)
    {
        if (!strcmp(matList[i]->name, matName)) return i;
    }
    return -1;
}

inline
int findTextureIdx(char* texName, CArrayList<noesisTex_t*>& texList) {
    for (int i = 0; i < texList.Num(); i++) {
        if (!strcmp(texList[i]->filename, texName))
            return i;
    }
    return -1;
}

inline
std::string findTri(noeRAPI_t* rapi, uint32_t& strcode)
{
    std::filesystem::path p{ rapi->Noesis_GetInputName() };
    p = p.parent_path();
    p = p.parent_path();
    p = p.parent_path();
    std::filesystem::path tri{ p.string() + "\\tri"};

    for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator(tri))
    {
        if (file.path().extension() == ".tri")
        {
            Tri tri = Tri(file.path().u8string());
            if (tri.containsTexture(strcode))
                return file.path().u8string();
        }
    }

    return "";
}

inline
noesisTex_t* loadTexture(noeRAPI_t* rapi, uint32_t& strcode)
{
    std::string triFile = findTri(rapi, strcode);
    if (triFile == "") return NULL;

    int size, bpp;

    Tri tri = Tri(triFile);
    uint8_t* texData = tri.getTexture(strcode, size, bpp);

    noesisTex_t* noeTexture = rapi->Noesis_LoadTexByHandler(texData, size, ".tga");
    delete[] texData;
    return noeTexture;
}

inline
void bindMat(uint32_t strcode[3], BYTE* fileBuffer, noeRAPI_t* rapi, CArrayList<noesisMaterial_t*>& matList, CArrayList<noesisTex_t*>& texList)
{
    
    std::string str[3];
    str[0] = intToHexString(strcode[0]);
    str[1] = intToHexString(strcode[1]);
    str[2] = intToHexString(strcode[2]);

    std::string matStr = str[0] + str[1] + str[2];

    //set mat name
    char matName[19];
    strcpy_s(matName, matStr.c_str());

    //check if material already exists
    int x = findMaterialIdx(matName, matList);

    //use existing mat if it does
    if (x > -1) 
    {
        rapi->rpgSetMaterial(matName);
        return;
    }

    //create material
    noesisMaterial_t* noeMat = rapi->Noesis_GetMaterialList(1, false);
    noeMat->name = rapi->Noesis_PooledString(matName);

    //set tex name
    for (int i = 0; i < 3; i++)
    {

        if (!strcode[i]) continue;

        std::string fullName = str[i] + ".tga";

        char texName[11];
        strcpy_s(texName, fullName.c_str());

        //check if texture already exists
        int y = findTextureIdx(texName, texList);

        //set tex to mat if it does
        if (y > -1)
        {
            switch (i) {
            case 0:
                noeMat->texIdx = y; break;
            case 1:
                noeMat->specularTexIdx = y; break;
            case 2:
                noeMat->envTexIdx = y; break;
            }
            matList.Append(noeMat);
            rapi->rpgSetMaterial(matName);
            return;
        }

        //load texture
        noesisTex_t* noeTexture = loadTexture(rapi, strcode[i]);
        if (!noeTexture) return;
        noeTexture->filename = rapi->Noesis_PooledString(texName);

        //set tex to mat
        switch (i)
        {
        //also depends on vertdef flag(& 0x200); but I'll do that another day
        case 0:
            noeMat->texIdx = texList.Num();  break;
        case 1:
            noeMat->specularTexIdx = texList.Num(); break;
        case 2:
            noeMat->envTexIdx = texList.Num(); break;
        }

        matList.Append(noeMat);

        texList.Append(noeTexture);
    }
    //set material
    rapi->rpgSetMaterial(noeMat->name);
    
}