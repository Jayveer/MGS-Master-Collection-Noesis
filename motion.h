#pragma once
#include "mgs/common/util.h"
#include "mgs/motion/mtar/mtar.h"
#include "mgs/motion/mtar/mtcm.h"
#include "noesis/plugin/pluginshare.h"

#include "include/half/half.h"

const double g_mgs3mc_PI = acos(-1);
const float  g_mgs3mc_GAME_FRAMERATE = 30.0f;

struct MoveAnimation
{
    int keyframe;
    float x;
    float y;
    float z;
};

struct RotAnimation
{
    int keyframe;
    float x;
    float y;
    float z;
    float w;
};

inline
float shiftRadix(const float& f, const int& exponent)
{
    return f * pow(2, exponent);
}

inline
float short2Half(const uint16_t& n)
{
    half x;
    x.setBits(n);
    float r = x;
    return r;
}

inline
float short2HalfExp(const uint16_t& n)
{
    half x;
    x.setBits(n);
    float r = x;
    return shiftRadix(r, 7);
}

inline
bool checkMagic(BYTE* motionFile)
{
    MtarHeader* header = (MtarHeader*)motionFile;
    return header->magic == 0x7261744D;
}

inline
BYTE* openMotion(noeRAPI_t* rapi)
{
    int len;
    char out[MAX_NOESIS_PATH];
    BYTE* mtarFile = rapi->Noesis_LoadPairedFile("load mtar", ".mtar", len, out);
    if (!mtarFile) return NULL;
    return checkMagic(mtarFile) ? mtarFile : NULL;
}

inline
RichQuat angleAxisToQuat(const float& angle, RichVec3 axis)
{
    RichQuat quat;

    axis.Normalize();
    double sinAngle = sin(angle / 2.0f);

    quat[0] = axis[0] * sinAngle;
    quat[1] = axis[1] * sinAngle;
    quat[2] = axis[2] * sinAngle;
    quat[3] = cos(angle / 2.0f);

    return quat;
}

inline
std::vector<RotAnimation> readRotBitstream(uint16_t* rotBitStream, const int& size, const int& quatLength, const uint32_t& numFrames)
{
    int keyFrame = 0;
    std::vector<RotAnimation> ra;
    RichBitStream bs = RichBitStream(rotBitStream, size * 2);

    while (keyFrame < numFrames)
    {
        keyFrame += bs.ReadBits(8);

        float theta = shiftRadix(bs.ReadBits(quatLength), -quatLength) * g_mgs3mc_PI;
        float axisX = shiftRadix(bs.ReadBits(quatLength), -quatLength);
        float axisY = shiftRadix(bs.ReadBits(quatLength), -quatLength);

        bool negCheck = bs.ReadBits(1);
        bool negCheck2 = bs.ReadBits(1);
        bool negCheck3 = bs.ReadBits(1);

        float axisZ = 1.0f - axisX - axisY;

        if (negCheck) { axisX = -axisX; }
        if (negCheck2) { axisY = -axisY; }
        if (negCheck3) { axisZ = -axisZ; }

        RichQuat qt = angleAxisToQuat(theta, { axisX, axisY, axisZ });

        ra.push_back({ keyFrame, qt[0], qt[1], qt[2], qt[3] });
    }

    return ra;
}

inline
std::vector<MoveAnimation> readMoveBitstream(uint16_t* moveBitStream, const int& size, const uint32_t& numFrames)
{
    int keyFrame = 0;
    std::vector<MoveAnimation> ma;
    RichBitStream bs = RichBitStream(moveBitStream, size * 2);

    while (keyFrame < numFrames)
    {
        float x = short2HalfExp(bs.ReadBits(16));
        float y = short2HalfExp(bs.ReadBits(16));
        float z = short2HalfExp(bs.ReadBits(16));
        keyFrame += bs.ReadBits(8);
        ma.push_back({ keyFrame, x , y, z });
    }

    return ma;
}

inline
int determineArchiveSize(uint8_t* mtcm, int i)
{
    MtcmHeader* mtcmHeader = (MtcmHeader*)mtcm;
    int rotOffset = mtcmHeader->quatOffset[i];

    int nextOffset = (i == mtcmHeader->numJoints - 1) ? mtcmHeader->rootOffset : mtcmHeader->quatOffset[i + 1];
    int size = nextOffset - rotOffset;
    return size <= 0 ? mtcmHeader->rootOffset - rotOffset : size;
}

inline
noeKeyFrameData_t createTransKFData(const MoveAnimation& trans, std::vector<float>& aniData) 
{
    noeKeyFrameData_t data = {};
    data.dataIndex = aniData.size();
    data.time = trans.keyframe / g_mgs3mc_GAME_FRAMERATE;

    aniData.push_back(trans.x);
    aniData.push_back(trans.y);
    aniData.push_back(trans.z);

    return data;
}

inline
noeKeyFrameData_t createRotKFData(const RotAnimation& rot, std::vector<float>& aniData)
{
    noeKeyFrameData_t data = {};
    data.dataIndex = aniData.size();
    data.time = rot.keyframe / g_mgs3mc_GAME_FRAMERATE;

    RichQuat quat = { rot.x, rot.y, rot.z, rot.w };
    quat.Transpose();

    aniData.push_back(quat[0]);
    aniData.push_back(quat[1]);
    aniData.push_back(quat[2]);
    aniData.push_back(quat[3]);

    return data;
}

inline
std::vector<noeKeyFrameData_t> createKFData(const std::vector<MoveAnimation>& trans, const std::vector<RotAnimation>& rot, const std::vector<MoveAnimation>& scale, std::vector<float>& aniData)
{
    std::vector<noeKeyFrameData_t> kfData;

    for (int i = 0; i < trans.size(); i++) {
        noeKeyFrameData_t transData = createTransKFData(trans[i], aniData);
        kfData.push_back(transData);
    }

    for (int i = 0; i < rot.size(); i++) {
        noeKeyFrameData_t rotData = createRotKFData(rot[i], aniData);
        kfData.push_back(rotData);
    }

    for (int i = 0; i < scale.size(); i++) {
        noeKeyFrameData_t scaleData = createTransKFData(scale[i], aniData);
        kfData.push_back(scaleData);
    }

    return kfData;
}

inline
noeKeyFramedBone_t createKFBone(uint32_t boneID, modelBone_t* noeBones, int numBones, int numFrames, const std::vector<MoveAnimation>& trans, const std::vector<RotAnimation>& rot, const std::vector<MoveAnimation>& scale, std::vector<float>& aniData, std::vector<std::vector<noeKeyFrameData_t>>& kfData)
{
    noeKeyFramedBone_t kfBone = {};

    int boneIdx = boneID;
    if (boneIdx == -1) return kfBone;

    kfBone.boneIndex = boneIdx;

    kfBone.scaleInterpolation = NOEKF_INTERPOLATE_LINEAR;
    kfBone.rotationInterpolation = NOEKF_INTERPOLATE_LINEAR;
    kfBone.translationInterpolation = NOEKF_INTERPOLATE_LINEAR;

    kfBone.scaleType = NOEKF_SCALE_VECTOR_3;
    kfBone.rotationType = NOEKF_ROTATION_QUATERNION_4;
    kfBone.translationType = NOEKF_TRANSLATION_VECTOR_3;

    kfBone.numScaleKeys = scale.size();
    kfBone.numRotationKeys = rot.size();
    kfBone.numTranslationKeys = trans.size();

    kfBone.maxTime = numFrames / g_mgs3mc_GAME_FRAMERATE;

    std::vector<noeKeyFrameData_t> nkfData = createKFData(trans, rot, scale, aniData);
    kfData.push_back(nkfData);

    int transPos = 0;
    int rotPos = (trans.size());
    int scalePos = (trans.size() + rot.size());

    if (kfBone.numScaleKeys)        kfBone.scaleKeys = &kfData[kfData.size() - 1][scalePos];
    if (kfBone.numRotationKeys)     kfBone.rotationKeys = &kfData[kfData.size() - 1][rotPos];
    if (kfBone.numTranslationKeys)  kfBone.translationKeys = &kfData[kfData.size() - 1][transPos];

    return kfBone;
}

inline
noeKeyFramedAnim_t createKFAnim(char* animName, modelBone_t* noeBones, int numBones, std::vector<noeKeyFramedBone_t>& kfBones, std::vector<float>& data)
{
    noeKeyFramedAnim_t kfAnim = {};

    kfAnim.name = animName;
    kfAnim.numKfBones = kfBones.size();
    kfAnim.kfBones = kfBones.data();
    kfAnim.numBones = numBones;
    kfAnim.framesPerSecond = g_mgs3mc_GAME_FRAMERATE;

    kfAnim.data = data.data();
    kfAnim.numDataFloats = data.size();

    return kfAnim;
}

inline
noesisAnim_t* bindMtcm(uint8_t* mtcm, uint32_t* boneTable, noeRAPI_t* rapi, modelBone_t* noeBones, int numBones)
{
    MtcmHeader* mtcmHeader = (MtcmHeader*)mtcm;

    uint8_t* mtcmArchive = &mtcm[mtcmHeader->archiveOffset];

    std::vector<float> aniData;
    std::vector<noeKeyFramedBone_t> kfBones;
    std::vector <std::vector<noeKeyFrameData_t>> kfData;

    RichBitStream bs = RichBitStream(&mtcmHeader->bitCheck0, 128);

    for (int j = 0; j < mtcmHeader->numJoints; j++) {
        int check = bs.ReadBits(1);
        std::vector<RotAnimation>  rot;
        std::vector<MoveAnimation> trans;
        std::vector<MoveAnimation> scale;

        if (j == 0) {
            uint16_t* moveBitstream = (uint16_t*)mtcmArchive;
            trans = readMoveBitstream(moveBitstream, mtcmHeader->quatOffset[0], mtcmHeader->numFrames);
        }

        int boneID = j;
        int size = determineArchiveSize(mtcm, j);
        int rotOffset = mtcmHeader->quatOffset[j];
        uint16_t* rotBitstream = (uint16_t*)&mtcmArchive[rotOffset * 2];

        int quatLength = check ? mtcmHeader->quatHighBit : mtcmHeader->quatLowBit;
        rot = readRotBitstream(rotBitstream, size, quatLength, mtcmHeader->numFrames);

        noeKeyFramedBone_t kfBone = createKFBone(boneID, noeBones, numBones, mtcmHeader->numFrames, trans, rot, scale, aniData, kfData);
        kfBones.push_back(kfBone);
    }

    if (aniData.empty()) return NULL;

    std::string animName = intToHexString(mtcmHeader->name);
    noeKeyFramedAnim_t kfAnim = createKFAnim((char*)animName.c_str(), noeBones, numBones, kfBones, aniData);

    noesisAnim_t* anim = rapi->Noesis_AnimFromBonesAndKeyFramedAnim(noeBones, numBones, &kfAnim, true);
    return anim;
}

inline
void loadMotion(noeRAPI_t* rapi, BYTE* motionFile, modelBone_t* noeBones, int numBones)
{
    MtarHeader* mtarHeader = (MtarHeader*)motionFile;

    if (mtarHeader->maxJoint > numBones) return;

    mtarHeader->resvBoneTableOffset = 0x20;
    mtarHeader->resvUnknownOffset = mtarHeader->resvBoneTableOffset + (mtarHeader->maxJoint * 4);
    mtarHeader->resvDataTableOffset = mtarHeader->resvUnknownOffset + (mtarHeader->maxJoint * 4);

    uint8_t  *mtcm      = (uint8_t*) &motionFile[mtarHeader->mtcmOffset];
    uint32_t *boneTable = (uint32_t*)&motionFile[mtarHeader->resvBoneTableOffset];
    MtarData *mtcmTable = (MtarData*)&motionFile[mtarHeader->resvDataTableOffset + 0x18];

    CArrayList<noesisAnim_t*> animList;

    for (int i = 0; i < mtarHeader->numMotion; i++)
    {
        uint8_t* thisMtcm = &motionFile[mtcmTable[i].mtcmOffset];

        noesisAnim_t* anim = bindMtcm(thisMtcm, boneTable, rapi, noeBones, numBones);
        if (anim) animList.Append(anim);
    }

    noesisAnim_t* anims = rapi->Noesis_AnimFromAnimsList(animList, animList.Num());
    rapi->rpgSetExData_AnimsNum(anims, 1);
}