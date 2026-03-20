#pragma once
#include "Converter_Define.h"
#include "Converter_Util.h"
#include "BuiltIn_GUID.h"
#include "Material_Converter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <cmath>
#include <cfloat>

NS_BEGIN(Converter)

/* ------------------------------------------------------------
 * Binary Write Helpers
 * ------------------------------------------------------------ */

 /* 문자열을 length + bytes 형태로 저장하는 용도 */
    static void Write_String_Bin(std::ofstream& ofs, const std::string& strValue)
{
    const uint32_t iLength = static_cast<uint32_t>(strValue.size());
    ofs.write(reinterpret_cast<const char*>(&iLength), sizeof(iLength));

    if (iLength > 0)
        ofs.write(strValue.data(), iLength);
}

/* trivially copyable 데이터 하나를 저장하는 용도 */
template <typename T>
static void Write_Value_Bin(std::ofstream& ofs, const T& value)
{
    ofs.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

/* trivially copyable 벡터 전체를 저장하는 용도 */
template <typename T>
static void Write_Vector_Bin(std::ofstream& ofs, const std::vector<T>& vecValue)
{
    const uint32_t iCount = static_cast<uint32_t>(vecValue.size());
    ofs.write(reinterpret_cast<const char*>(&iCount), sizeof(iCount));

    if (!vecValue.empty())
        ofs.write(reinterpret_cast<const char*>(vecValue.data()), sizeof(T) * vecValue.size());
}

/* ------------------------------------------------------------
 * Assimp <-> Engine Convert Helpers
 * ------------------------------------------------------------ */

 /* aiMatrix4x4 -> _float4x4 변환 용도 */
static _float4x4 Convert_AiMatrix(const aiMatrix4x4& mat)
{
    _float4x4 out{};
    out._11 = mat.a1; out._12 = mat.b1; out._13 = mat.c1; out._14 = mat.d1;
    out._21 = mat.a2; out._22 = mat.b2; out._23 = mat.c2; out._24 = mat.d2;
    out._31 = mat.a3; out._32 = mat.b3; out._33 = mat.c3; out._34 = mat.d3;
    out._41 = mat.a4; out._42 = mat.b4; out._43 = mat.c4; out._44 = mat.d4;
    return out;
}

/* aiVector3D -> _float3 변환 용도 */
static _float3 Convert_AiVector3(const aiVector3D& v)
{
    _float3 out{};
    out.x = v.x;
    out.y = v.y;
    out.z = v.z;
    return out;
}

/* aiQuaternion -> _float4 변환 용도 */
static _float4 Convert_AiQuaternion(const aiQuaternion& q)
{
    _float4 out{};
    out.x = q.x;
    out.y = q.y;
    out.z = q.z;
    out.w = q.w;
    return out;
}

/* _float4 정규화 용도 */
static void Normalize_Quaternion(_float4& q)
{
    const float fLenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (fLenSq <= FLT_EPSILON)
    {
        q = _float4{ 0.f, 0.f, 0.f, 1.f };
        return;
    }

    const float fInvLen = 1.f / std::sqrt(fLenSq);
    q.x *= fInvLen;
    q.y *= fInvLen;
    q.z *= fInvLen;
    q.w *= fInvLen;
}

/* ------------------------------------------------------------
 * Bone Weight Helpers
 * ------------------------------------------------------------ */

 /* 정점당 최대 4개 Bone Weight를 유지하는 용도 */
static void Add_BoneWeight(BONE_WEIGHT_SLOT& slot, uint32_t iBoneIndex, float fWeight)
{
    if (fWeight <= 0.f)
        return;

    for (size_t i = 0; i < 4; ++i)
    {
        if (slot.fWeights[i] == 0.f)
        {
            slot.iIndices[i] = iBoneIndex;
            slot.fWeights[i] = fWeight;
            return;
        }
    }

    size_t iMinIndex = 0;
    for (size_t i = 1; i < 4; ++i)
    {
        if (slot.fWeights[i] < slot.fWeights[iMinIndex])
            iMinIndex = i;
    }

    if (fWeight > slot.fWeights[iMinIndex])
    {
        slot.iIndices[iMinIndex] = iBoneIndex;
        slot.fWeights[iMinIndex] = fWeight;
    }
}

/* Bone Weight 4개를 내림차순으로 정렬하는 용도 */
static void Sort_BoneWeightSlot(BONE_WEIGHT_SLOT& slot)
{
    for (size_t i = 0; i < 4; ++i)
    {
        for (size_t j = i + 1; j < 4; ++j)
        {
            if (slot.fWeights[j] > slot.fWeights[i])
            {
                std::swap(slot.fWeights[i], slot.fWeights[j]);
                std::swap(slot.iIndices[i], slot.iIndices[j]);
            }
        }
    }
}

/* Bone Weight 합을 1로 정규화하는 용도 */
static void Normalize_BoneWeightSlot(BONE_WEIGHT_SLOT& slot)
{
    Sort_BoneWeightSlot(slot);

    float fSum = 0.f;
    for (size_t i = 0; i < 4; ++i)
        fSum += slot.fWeights[i];

    if (fSum <= FLT_EPSILON)
    {
        slot.iIndices[0] = 0;
        slot.fWeights[0] = 0.f;
        slot.iIndices[1] = 0;
        slot.fWeights[1] = 0.f;
        slot.iIndices[2] = 0;
        slot.fWeights[2] = 0.f;
        slot.iIndices[3] = 0;
        slot.fWeights[3] = 0.f;
        return;
    }

    const float fInvSum = 1.f / fSum;
    for (size_t i = 0; i < 4; ++i)
        slot.fWeights[i] *= fInvSum;
}

/* ------------------------------------------------------------
 * Assimp Scene Load
 * ------------------------------------------------------------ */

 /* 애니메이션 모델용 Assimp Scene 로드 용도 */
static const aiScene* LoadScene_Assimp_Anim(Assimp::Importer& importer, const std::filesystem::path& fbxPath, uint32_t iFlag = 0)
{
    const uint32_t flags =
        aiProcess_ConvertToLeftHanded |             /* 왼손 좌표계 기준으로 변경 */
        aiProcessPreset_TargetRealtime_Fast |       /* 빠른 실시간 렌더링 용도 프리셋 */
        aiProcess_Triangulate |                     /* 모든 폴리곤 Triangle */
        aiProcess_JoinIdenticalVertices |           /* 중복 정점 줄여 최적화 */
        aiProcess_GenNormals |                      /* 노멀이 없다면 자동 생성 */
        aiProcess_CalcTangentSpace |                /* 탄젠트 계산 */
        aiProcess_LimitBoneWeights;                 /* Bone Weight 개수 제한 */

    return importer.ReadFile(fbxPath.string(), flags | iFlag);
}

/* ------------------------------------------------------------
 * Skeleton Convert
 * ------------------------------------------------------------ */

 /* 노드 트리를 전부 Bone 배열로 만드는 용도 */
static void Build_Skeleton_From_Node_Recursive(
    const aiNode* pAINode,
    int32_t iParentBoneIndex,
    const aiMatrix4x4& matParentCombined,
    CONVERTED_SKELETON& outSkeleton)
{
    if (!pAINode)
        return;

    CONVERTED_BONE bone{};
    bone.strName = pAINode->mName.C_Str();
    bone.iParentBoneIndex = iParentBoneIndex;

    bone.matLocalBind = Convert_AiMatrix(pAINode->mTransformation);

    const aiMatrix4x4 matCombined = pAINode->mTransformation * matParentCombined;
    bone.matCombinedBind = Convert_AiMatrix(matCombined);

    _float4x4 matIdentity{};
    matIdentity._11 = 1.f;
    matIdentity._22 = 1.f;
    matIdentity._33 = 1.f;
    matIdentity._44 = 1.f;
    bone.matOffset = matIdentity; /* 실제 Skin Bone이 아니면 기본은 Identity */

    const uint32_t iMyBoneIndex = static_cast<uint32_t>(outSkeleton.bones.size());
    outSkeleton.BoneNameToIndex[bone.strName] = iMyBoneIndex;
    outSkeleton.bones.push_back(std::move(bone));

    if (iParentBoneIndex < 0)
        outSkeleton.iRootBoneIndex = static_cast<int32_t>(iMyBoneIndex);
    else
        outSkeleton.bones[iParentBoneIndex].vecChildBoneIndices.push_back(iMyBoneIndex);

    for (uint32_t i = 0; i < pAINode->mNumChildren; ++i)
    {
        Build_Skeleton_From_Node_Recursive(
            pAINode->mChildren[i],
            static_cast<int32_t>(iMyBoneIndex),
            matCombined,
            outSkeleton);
    }
}

/* Scene 전체에서 Skeleton을 만드는 용도 */
static _bool Convert_Skeleton(const aiScene* pAIScene, CONVERTED_SKELETON& outSkeleton)
{
    if (!pAIScene || !pAIScene->mRootNode)
        return false;

    outSkeleton.bones.clear();
    outSkeleton.BoneNameToIndex.clear();
    outSkeleton.iRootBoneIndex = -1;

    Build_Skeleton_From_Node_Recursive(
        pAIScene->mRootNode,
        -1,
        aiMatrix4x4(),
        outSkeleton);

    return !outSkeleton.bones.empty();
}

/* Mesh Bone의 Offset 행렬을 Skeleton에 반영하는 용도 */
static void Apply_MeshBone_Offsets(const aiScene* pAIScene, CONVERTED_SKELETON& ioSkeleton)
{
    if (!pAIScene)
        return;

    for (uint32_t iMesh = 0; iMesh < pAIScene->mNumMeshes; ++iMesh)
    {
        const aiMesh* pAIMesh = pAIScene->mMeshes[iMesh];
        if (!pAIMesh)
            continue;

        for (uint32_t iBone = 0; iBone < pAIMesh->mNumBones; ++iBone)
        {
            const aiBone* pAIBone = pAIMesh->mBones[iBone];
            if (!pAIBone)
                continue;

            const std::string strBoneName = pAIBone->mName.C_Str();
            auto itFound = ioSkeleton.BoneNameToIndex.find(strBoneName);
            if (itFound == ioSkeleton.BoneNameToIndex.end())
                continue;

            ioSkeleton.bones[itFound->second].matOffset = Convert_AiMatrix(pAIBone->mOffsetMatrix);
        }
    }
}

/* ------------------------------------------------------------
 * Animation Sampling Helpers
 * ------------------------------------------------------------ */

 /* 해당 시간 이전/이후 키를 찾는 용도 */
template <typename TKey>
static uint32_t Find_Left_Key_Index(const TKey* pKeys, uint32_t iKeyCount, double fTime)
{
    if (!pKeys || iKeyCount == 0)
        return 0;

    if (iKeyCount == 1)
        return 0;

    for (uint32_t i = 0; i + 1 < iKeyCount; ++i)
    {
        if (fTime < pKeys[i + 1].mTime)
            return i;
    }

    return iKeyCount - 1;
}

/* Vector3 키를 보간 샘플링하는 용도 */
static _float3 Sample_Vector3_Channel(
    const aiVectorKey* pKeys,
    uint32_t iKeyCount,
    double fTime,
    const _float3& vDefaultValue)
{
    if (!pKeys || iKeyCount == 0)
        return vDefaultValue;

    if (iKeyCount == 1)
        return Convert_AiVector3(pKeys[0].mValue);

    const uint32_t iLeft = Find_Left_Key_Index(pKeys, iKeyCount, fTime);
    if (iLeft + 1 >= iKeyCount)
        return Convert_AiVector3(pKeys[iKeyCount - 1].mValue);

    const aiVectorKey& tLeft = pKeys[iLeft];
    const aiVectorKey& tRight = pKeys[iLeft + 1];

    const double fDenom = tRight.mTime - tLeft.mTime;
    if (fDenom <= DBL_EPSILON)
        return Convert_AiVector3(tLeft.mValue);

    const float fRatio = static_cast<float>((fTime - tLeft.mTime) / fDenom);

    aiVector3D vOut = tLeft.mValue + (tRight.mValue - tLeft.mValue) * fRatio;
    return Convert_AiVector3(vOut);
}

/* Quaternion 키를 보간 샘플링하는 용도 */
static _float4 Sample_Quaternion_Channel(
    const aiQuatKey* pKeys,
    uint32_t iKeyCount,
    double fTime,
    const _float4& vDefaultValue)
{
    if (!pKeys || iKeyCount == 0)
        return vDefaultValue;

    if (iKeyCount == 1)
    {
        _float4 out = Convert_AiQuaternion(pKeys[0].mValue);
        Normalize_Quaternion(out);
        return out;
    }

    const uint32_t iLeft = Find_Left_Key_Index(pKeys, iKeyCount, fTime);
    if (iLeft + 1 >= iKeyCount)
    {
        _float4 out = Convert_AiQuaternion(pKeys[iKeyCount - 1].mValue);
        Normalize_Quaternion(out);
        return out;
    }

    const aiQuatKey& tLeft = pKeys[iLeft];
    const aiQuatKey& tRight = pKeys[iLeft + 1];

    const double fDenom = tRight.mTime - tLeft.mTime;
    if (fDenom <= DBL_EPSILON)
    {
        _float4 out = Convert_AiQuaternion(tLeft.mValue);
        Normalize_Quaternion(out);
        return out;
    }

    const float fRatio = static_cast<float>((fTime - tLeft.mTime) / fDenom);

    aiQuaternion qOut;
    aiQuaternion::Interpolate(qOut, tLeft.mValue, tRight.mValue, fRatio);
    qOut.Normalize();

    _float4 out = Convert_AiQuaternion(qOut);
    Normalize_Quaternion(out);
    return out;
}

/* 채널의 모든 키 시간들을 하나로 모으는 용도 */
static void Collect_Channel_KeyTimes(const aiNodeAnim* pAINodeAnim, std::vector<double>& outTimes)
{
    outTimes.clear();
    if (!pAINodeAnim)
        return;

    outTimes.reserve(
        pAINodeAnim->mNumPositionKeys +
        pAINodeAnim->mNumRotationKeys +
        pAINodeAnim->mNumScalingKeys);

    for (uint32_t i = 0; i < pAINodeAnim->mNumPositionKeys; ++i)
        outTimes.push_back(pAINodeAnim->mPositionKeys[i].mTime);

    for (uint32_t i = 0; i < pAINodeAnim->mNumRotationKeys; ++i)
        outTimes.push_back(pAINodeAnim->mRotationKeys[i].mTime);

    for (uint32_t i = 0; i < pAINodeAnim->mNumScalingKeys; ++i)
        outTimes.push_back(pAINodeAnim->mScalingKeys[i].mTime);

    std::sort(outTimes.begin(), outTimes.end());
    outTimes.erase(
        std::unique(outTimes.begin(), outTimes.end(),
            [](double a, double b)
            {
                return std::abs(a - b) <= 1e-8;
            }),
        outTimes.end());
}

/* Bone 기본 자세에서 SRT를 추출하는 용도 */
static void Extract_DefaultSRT_From_Bone(const CONVERTED_BONE& tBone, _float3& outScale, _float4& outRotation, _float3& outTranslation)
{
    const _matrix matLocal = XMLoadFloat4x4(&tBone.matLocalBind);

    _vector vScale{};
    _vector vRotation{};
    _vector vTranslation{};

    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, matLocal);

    XMStoreFloat3(&outScale, vScale);
    XMStoreFloat4(&outRotation, vRotation);
    XMStoreFloat3(&outTranslation, vTranslation);

    Normalize_Quaternion(outRotation);
}

/* Assimp 채널을 런타임 채널 구조로 변환하는 용도 */
static _bool Convert_Animation_Channel(
    const aiNodeAnim* pAINodeAnim,
    const CONVERTED_SKELETON& tSkeleton,
    CONVERTED_ANIMATION_CHANNEL& outChannel)
{
    if (!pAINodeAnim)
        return false;

    outChannel = {};

    outChannel.strBoneName = pAINodeAnim->mNodeName.C_Str();

    auto itFound = tSkeleton.BoneNameToIndex.find(outChannel.strBoneName);
    if (itFound == tSkeleton.BoneNameToIndex.end())
        return false;

    outChannel.iBoneIndex = static_cast<int32_t>(itFound->second);

    std::vector<double> vecTimes;
    Collect_Channel_KeyTimes(pAINodeAnim, vecTimes);

    if (vecTimes.empty())
    {
        ANIM_KEYFRAME tKeyFrame{};
        tKeyFrame.fTrackPosition = 0.f;

        const CONVERTED_BONE& tBone = tSkeleton.bones[outChannel.iBoneIndex];
        Extract_DefaultSRT_From_Bone(tBone, tKeyFrame.vScale, tKeyFrame.vRotation, tKeyFrame.vTranslation);

        outChannel.vecKeyFrames.push_back(tKeyFrame);
        return true;
    }

    outChannel.vecKeyFrames.reserve(vecTimes.size());

    _float3 vDefaultScale{ 1.f, 1.f, 1.f };
    _float4 vDefaultRotation{ 0.f, 0.f, 0.f, 1.f };
    _float3 vDefaultTranslation{ 0.f, 0.f, 0.f };

    {
        const CONVERTED_BONE& tBone = tSkeleton.bones[outChannel.iBoneIndex];
        Extract_DefaultSRT_From_Bone(tBone, vDefaultScale, vDefaultRotation, vDefaultTranslation);
    }

    for (double fTime : vecTimes)
    {
        ANIM_KEYFRAME tKeyFrame{};
        tKeyFrame.fTrackPosition = static_cast<_float>(fTime);

        tKeyFrame.vScale = Sample_Vector3_Channel(
            pAINodeAnim->mScalingKeys,
            pAINodeAnim->mNumScalingKeys,
            fTime,
            vDefaultScale);

        tKeyFrame.vRotation = Sample_Quaternion_Channel(
            pAINodeAnim->mRotationKeys,
            pAINodeAnim->mNumRotationKeys,
            fTime,
            vDefaultRotation);

        tKeyFrame.vTranslation = Sample_Vector3_Channel(
            pAINodeAnim->mPositionKeys,
            pAINodeAnim->mNumPositionKeys,
            fTime,
            vDefaultTranslation);

        outChannel.vecKeyFrames.push_back(tKeyFrame);
    }

    return !outChannel.vecKeyFrames.empty();
}

/* Assimp 애니메이션 하나를 런타임 클립 구조로 변환하는 용도 */
static _bool Convert_Animation_Clip(
    const aiAnimation* pAIAnim,
    const CONVERTED_SKELETON& tSkeleton,
    CONVERTED_ANIMATION_CLIP& outClip,
    uint32_t iClipIndex)
{
    if (!pAIAnim)
        return false;

    outClip = {};

    if (pAIAnim->mName.length > 0)
        outClip.strName = pAIAnim->mName.C_Str();
    else
        outClip.strName = "AnimClip_" + std::to_string(iClipIndex);

    outClip.fDuration = static_cast<_float>(pAIAnim->mDuration);
    outClip.fTickPerSecond = (pAIAnim->mTicksPerSecond > 0.0)
        ? static_cast<_float>(pAIAnim->mTicksPerSecond)
        : 1.f;

    outClip.channels.reserve(pAIAnim->mNumChannels);

    for (uint32_t i = 0; i < pAIAnim->mNumChannels; ++i)
    {
        const aiNodeAnim* pAINodeAnim = pAIAnim->mChannels[i];
        if (!pAINodeAnim)
            continue;

        CONVERTED_ANIMATION_CHANNEL channel{};
        if (!Convert_Animation_Channel(pAINodeAnim, tSkeleton, channel))
            continue;

        outClip.channels.push_back(std::move(channel));
    }

    return true;
}

/* Scene 전체의 애니메이션 클립을 변환하는 용도 */
static _bool Convert_Animation_Clips(
    const aiScene* pAIScene,
    const CONVERTED_SKELETON& tSkeleton,
    std::vector<CONVERTED_ANIMATION_CLIP>& outClips)
{
    if (!pAIScene)
        return false;

    outClips.clear();
    outClips.reserve(pAIScene->mNumAnimations);

    for (uint32_t i = 0; i < pAIScene->mNumAnimations; ++i)
    {
        const aiAnimation* pAIAnim = pAIScene->mAnimations[i];
        if (!pAIAnim)
            continue;

        CONVERTED_ANIMATION_CLIP clip{};
        if (!Convert_Animation_Clip(pAIAnim, tSkeleton, clip, i))
            continue;

        outClips.push_back(std::move(clip));
    }

    return true;
}

/* ------------------------------------------------------------
 * Anim Mesh Convert
 * ------------------------------------------------------------ */

 /* 애니메이션 메쉬 하나를 변환하는 용도 */
static _bool Convert_SingleAnimMesh(
    const aiMesh* pAIMesh,
    const CONVERTED_SKELETON& tSkeleton,
    CONVERTED_ANIM_MESH& out,
    const float fImportScale)
{
    if (!pAIMesh || pAIMesh->mNumVertices == 0)
        return false;

    out.vertices.clear();
    out.indices.clear();

    const _bool hasNormals = (pAIMesh->mNormals != nullptr);
    const _bool hasTangents = (pAIMesh->mTangents != nullptr);
    const _bool hasUV0 = (pAIMesh->mTextureCoords[0] != nullptr);

    out.vertices.resize(pAIMesh->mNumVertices);

    for (size_t v = 0; v < pAIMesh->mNumVertices; ++v)
    {
        CopyFloat3_Scale(out.vertices[v].vPosition, pAIMesh->mVertices[v], fImportScale);

        if (hasNormals)  CopyFloat3(out.vertices[v].vNormal, pAIMesh->mNormals[v]);
        else             out.vertices[v].vNormal = _float3{ 0.f, 1.f, 0.f };

        if (hasTangents) CopyFloat3(out.vertices[v].vTangent, pAIMesh->mTangents[v]);
        else             out.vertices[v].vTangent = _float3{ 1.f, 0.f, 0.f };

        if (hasUV0)      CopyFloat2(out.vertices[v].vTexcoord, pAIMesh->mTextureCoords[0][v]);
        else             out.vertices[v].vTexcoord = _float2{ 0.f, 0.f };

        out.vertices[v].vBlendIndex = XMUINT4(0, 0, 0, 0);
        out.vertices[v].vBlendWeight = _float4(0.f, 0.f, 0.f, 0.f);
    }

    std::vector<BONE_WEIGHT_SLOT> vecWeightSlots;
    vecWeightSlots.resize(pAIMesh->mNumVertices);

    for (uint32_t iBone = 0; iBone < pAIMesh->mNumBones; ++iBone)
    {
        const aiBone* pAIBone = pAIMesh->mBones[iBone];
        if (!pAIBone)
            continue;

        const std::string strBoneName = pAIBone->mName.C_Str();
        auto itFound = tSkeleton.BoneNameToIndex.find(strBoneName);
        if (itFound == tSkeleton.BoneNameToIndex.end())
            continue;

        const uint32_t iBoneIndex = itFound->second;

        for (uint32_t iWeight = 0; iWeight < pAIBone->mNumWeights; ++iWeight)
        {
            const aiVertexWeight& tWeight = pAIBone->mWeights[iWeight];
            if (tWeight.mVertexId >= vecWeightSlots.size())
                continue;

            Add_BoneWeight(vecWeightSlots[tWeight.mVertexId], iBoneIndex, tWeight.mWeight);
        }
    }

    for (size_t v = 0; v < vecWeightSlots.size(); ++v)
    {
        Normalize_BoneWeightSlot(vecWeightSlots[v]);

        const float fWeightSum =
            vecWeightSlots[v].fWeights[0] +
            vecWeightSlots[v].fWeights[1] +
            vecWeightSlots[v].fWeights[2] +
            vecWeightSlots[v].fWeights[3];

        if (fWeightSum <= FLT_EPSILON)
        {
            std::cout << "Anim mesh vertex has no bone weight : mesh = "
                << pAIMesh->mName.C_Str()
                << ", vertex = " << v << "\n";
            return false;
        }

        out.vertices[v].vBlendIndex = XMUINT4(
            vecWeightSlots[v].iIndices[0],
            vecWeightSlots[v].iIndices[1],
            vecWeightSlots[v].iIndices[2],
            vecWeightSlots[v].iIndices[3]);

        out.vertices[v].vBlendWeight = _float4(
            vecWeightSlots[v].fWeights[0],
            vecWeightSlots[v].fWeights[1],
            vecWeightSlots[v].fWeights[2],
            vecWeightSlots[v].fWeights[3]);
    }
    out.indices.reserve(pAIMesh->mNumFaces * 3);

    for (uint32_t f = 0; f < pAIMesh->mNumFaces; ++f)
    {
        const aiFace& face = pAIMesh->mFaces[f];
        if (face.mNumIndices != 3)
            continue;

        out.indices.push_back(static_cast<uint32_t>(face.mIndices[0]));
        out.indices.push_back(static_cast<uint32_t>(face.mIndices[1]));
        out.indices.push_back(static_cast<uint32_t>(face.mIndices[2]));
    }

    return !out.vertices.empty() && !out.indices.empty();
}

/* Scene 전체의 애니메이션 모델을 변환하는 용도 */
static _bool Convert_AnimModel(
    const aiScene* pAIScene,
    CONVERTED_ANIM_MODEL& out,
    const float fImportScale)
{
    if (!pAIScene || pAIScene->mNumMeshes == 0)
        return false;

    out.parts.clear();
    out.tSkeleton = {};
    out.vecAnimClips.clear();

    if (!Convert_Skeleton(pAIScene, out.tSkeleton))
        return false;

    Apply_MeshBone_Offsets(pAIScene, out.tSkeleton);

    if (!Convert_Animation_Clips(pAIScene, out.tSkeleton, out.vecAnimClips))
        return false;

    out.parts.reserve(pAIScene->mNumMeshes);

    for (uint32_t i = 0; i < pAIScene->mNumMeshes; ++i)
    {
        const aiMesh* pAIMesh = pAIScene->mMeshes[i];
        if (!pAIMesh)
            continue;

        CONVERTED_ANIM_MODEL_PART part{};

        if (pAIMesh->mName.length > 0)
            part.strName = pAIMesh->mName.C_Str();
        else
            part.strName = "Part_" + std::to_string(i);

        part.iMaterialIndex = pAIMesh->mMaterialIndex;

        if (!Convert_SingleAnimMesh(pAIMesh, out.tSkeleton, part.mesh, fImportScale))
            continue;

        out.parts.push_back(std::move(part));
    }

    return !out.parts.empty();
}

/* ------------------------------------------------------------
 * Binary Save : Mesh
 * ------------------------------------------------------------ */

 /* 애니메이션 메쉬 바이너리 저장 용도 */
static _bool Save_Anim_MeshBin(const std::filesystem::path& outPath, const CONVERTED_ANIM_MESH& mesh)
{
    std::ofstream ofs(outPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    MESH_HEADER hdr{};
    hdr.iMagic = 0x4D534842; /* 'MSHB' */
    hdr.vertexStride = sizeof(VTXANIMMESH);
    hdr.vertexCount = static_cast<uint32_t>(mesh.vertices.size());
    hdr.indexCount = static_cast<uint32_t>(mesh.indices.size());

    ofs.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    if (!mesh.vertices.empty())
        ofs.write(reinterpret_cast<const char*>(mesh.vertices.data()), sizeof(VTXANIMMESH) * mesh.vertices.size());

    if (!mesh.indices.empty())
        ofs.write(reinterpret_cast<const char*>(mesh.indices.data()), sizeof(uint32_t) * mesh.indices.size());

    return ofs.good();
}

/* 애니메이션 메쉬 메타 저장 용도 */
static _bool Save_Anim_MeshMeta(
    const std::filesystem::path& metaPath,
    const std::string& strGUID,
    const std::filesystem::path& sourceFbx,
    const std::filesystem::path& cookedMeshbin,
    const CONVERTED_ANIM_MESH& mesh)
{
    std::ofstream ofs(metaPath);
    if (!ofs.is_open())
        return false;

    ofs << "guid=" << strGUID << "\n";
    ofs << "type=MESH\n";
    ofs << "source=" << sourceFbx.generic_string() << "\n";
    ofs << "cooked=" << cookedMeshbin.generic_string() << "\n";
    ofs << "vertexCount=" << mesh.vertices.size() << "\n";
    ofs << "indexCount=" << mesh.indices.size() << "\n";
    ofs << "skinned=true\n";

    return true;
}

/* ------------------------------------------------------------
 * Binary Save : Skeleton / Animation
 * ------------------------------------------------------------ */

 /* Bone 하나를 바이너리 저장하는 용도 */
static void Save_Bone_Bin(std::ofstream& ofs, const CONVERTED_BONE& bone)
{
    Write_String_Bin(ofs, bone.strName);

    BONE_HEADER_BIN hdr{};
    hdr.iParentBoneIndex = bone.iParentBoneIndex;
    hdr.childCount = static_cast<uint32_t>(bone.vecChildBoneIndices.size());
    Write_Value_Bin(ofs, hdr);

    if (!bone.vecChildBoneIndices.empty())
    {
        ofs.write(
            reinterpret_cast<const char*>(bone.vecChildBoneIndices.data()),
            sizeof(uint32_t) * bone.vecChildBoneIndices.size());
    }

    Write_Value_Bin(ofs, bone.matLocalBind);
    Write_Value_Bin(ofs, bone.matCombinedBind);
    Write_Value_Bin(ofs, bone.matOffset);
}

/* Skeleton 전체를 바이너리 저장하는 용도 */
static void Save_Skeleton_Bin(std::ofstream& ofs, const CONVERTED_SKELETON& tSkeleton)
{
    SKELETON_HEADER_BIN hdr{};
    hdr.boneCount = static_cast<uint32_t>(tSkeleton.bones.size());
    hdr.iRootBoneIndex = tSkeleton.iRootBoneIndex;
    Write_Value_Bin(ofs, hdr);

    for (const CONVERTED_BONE& bone : tSkeleton.bones)
        Save_Bone_Bin(ofs, bone);
}

/* 애니메이션 채널 하나를 바이너리 저장하는 용도 */
static void Save_Animation_Channel_Bin(std::ofstream& ofs, const CONVERTED_ANIMATION_CHANNEL& channel)
{
    Write_String_Bin(ofs, channel.strBoneName);

    ANIMATION_CHANNEL_HEADER_BIN hdr{};
    hdr.iBoneIndex = channel.iBoneIndex;
    hdr.keyFrameCount = static_cast<uint32_t>(channel.vecKeyFrames.size());
    Write_Value_Bin(ofs, hdr);

    if (!channel.vecKeyFrames.empty())
    {
        ofs.write(
            reinterpret_cast<const char*>(channel.vecKeyFrames.data()),
            sizeof(ANIM_KEYFRAME) * channel.vecKeyFrames.size());
    }
}

/* 애니메이션 클립 하나를 바이너리 저장하는 용도 */
static void Save_Animation_Clip_Bin(std::ofstream& ofs, const CONVERTED_ANIMATION_CLIP& clip)
{
    Write_String_Bin(ofs, clip.strName);

    ANIMATION_CLIP_HEADER_BIN hdr{};
    hdr.fDuration = clip.fDuration;
    hdr.fTickPerSecond = clip.fTickPerSecond;
    hdr.channelCount = static_cast<uint32_t>(clip.channels.size());
    Write_Value_Bin(ofs, hdr);

    for (const CONVERTED_ANIMATION_CHANNEL& channel : clip.channels)
        Save_Animation_Channel_Bin(ofs, channel);
}

/* ------------------------------------------------------------
 * Binary Save : Model File
 * ------------------------------------------------------------ */

 /* 파트 정보 문자열 묶음을 저장하는 용도 */
static void Save_Anim_ModelPartInfo_Bin(std::ofstream& ofs, const SAVED_ANIM_MODEL_PART_INFO& partInfo)
{
    Write_String_Bin(ofs, partInfo.strName);
    Write_String_Bin(ofs, partInfo.strMeshGUID);
    Write_String_Bin(ofs, partInfo.strMaterialGUID);
}

/* 애니메이션 모델 파일 전체를 바이너리 저장하는 용도 */
static _bool Save_Anim_ModelFile(
    const std::filesystem::path& modelPath,
    const CONVERTED_ANIM_MODEL& model,
    const std::vector<SAVED_ANIM_MODEL_PART_INFO>& parts)
{
    std::ofstream ofs(modelPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    ANIM_MODEL_HEADER hdr{};
    hdr.iMagic = 0x4D494E41; /* 'ANIM' */
    hdr.iVersion = 1;
    hdr.partCount = static_cast<uint32_t>(parts.size());
    hdr.clipCount = static_cast<uint32_t>(model.vecAnimClips.size());
    hdr.boneCount = static_cast<uint32_t>(model.tSkeleton.bones.size());

    Write_Value_Bin(ofs, hdr);

    for (const SAVED_ANIM_MODEL_PART_INFO& partInfo : parts)
        Save_Anim_ModelPartInfo_Bin(ofs, partInfo);

    Save_Skeleton_Bin(ofs, model.tSkeleton);

    for (const CONVERTED_ANIMATION_CLIP& clip : model.vecAnimClips)
        Save_Animation_Clip_Bin(ofs, clip);

    return true;
}

/* ------------------------------------------------------------
 * Full Convert Entry
 * ------------------------------------------------------------ */

 /* 애니메이션 모델 전체 변환 및 저장 진입점 */
inline static _bool Convert_Anim(
    std::filesystem::path& inPath,          /* Converter/FBXs/... */
    std::filesystem::path& textureRoot,     /* Client/Assets/Textures */
    std::filesystem::path& outMeshPath,     /* Client/Assets/Models/... .model */
    std::filesystem::path& outMatPath,      /* Client/Assets/Materials/... */
    std::filesystem::path& outMeshMeta,     /* Client/Assets/Models/... .model.meta */
    const _float fImportScale)
{
    Assimp::Importer importer;
    const aiScene* pAIScene = LoadScene_Assimp_Anim(importer, inPath);

    if (pAIScene == nullptr)
    {
        std::cout << "Assimp load failed : " << inPath.string() << "\n";
        return false;
    }

    CONVERTED_ANIM_MODEL model{};
    if (!Convert_AnimModel(pAIScene, model, fImportScale))
    {
        std::cout << "Convert_Anim model failed : " << inPath.string() << "\n";
        return false;
    }

    std::filesystem::create_directories(outMeshPath.parent_path());
    std::filesystem::create_directories(outMatPath.parent_path());

    std::vector<SAVED_ANIM_MODEL_PART_INFO> savedParts;
    savedParts.reserve(model.parts.size());

    const std::string strMeshBaseName = outMeshPath.stem().string();
    const std::filesystem::path parentMeshDir = outMeshPath.parent_path();
    const std::filesystem::path parentMatDir = outMatPath.parent_path();

    std::unordered_map<uint32_t, std::string> materialIndexToGUID;
    {
        const std::string strDefaultShaderGUID = To_String_Utf8(DEFAULT_ASSET_GUID::SHADER_VTXANIMMESH.value);
        const std::string strDefaultBaseMapGUID = To_String_Utf8(DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT.value);
        const std::string strDefaultNormalMapGUID = To_String_Utf8(DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT.value);

        for (uint32_t iMat = 0; iMat < pAIScene->mNumMaterials; ++iMat)
        {
            const aiMaterial* pAIMaterial = pAIScene->mMaterials[iMat];
            if (!pAIMaterial)
                continue;

            std::string strMaterialGUID = Generate_GUID_String();
            materialIndexToGUID[iMat] = strMaterialGUID;

            std::string strMaterialName = strMeshBaseName + "_Mat_" + std::to_string(iMat);

            const std::filesystem::path matPath = parentMatDir / (strMaterialName + ".mat");
            const std::filesystem::path matMetaPath = parentMatDir / (strMaterialName + ".mat.meta");

            const _float4 baseColor = Read_BaseColor(pAIMaterial);
            const _float fShininess = Read_Shininess(pAIMaterial);

            std::string strBaseMapGUID = strDefaultBaseMapGUID;
            std::string strNormalMapGUID = strDefaultNormalMapGUID;

            aiString strTexPath{};
            if (AI_SUCCESS == pAIMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &strTexPath))
            {
                strBaseMapGUID = Resolve_Texture_GUID_From_AssimpPath(
                    textureRoot, strTexPath, strDefaultBaseMapGUID);
            }

            if (AI_SUCCESS == pAIMaterial->GetTexture(aiTextureType_NORMALS, 0, &strTexPath))
            {
                strNormalMapGUID = Resolve_Texture_GUID_From_AssimpPath(
                    textureRoot, strTexPath, strDefaultNormalMapGUID);
            }

            if (!Save_MaterialFile(
                matPath,
                strMaterialGUID,
                strDefaultShaderGUID,
                baseColor,
                fShininess,
                strBaseMapGUID,
                strNormalMapGUID))
            {
                std::cout << "Save material file failed : " << matPath.string() << "\n";
                return false;
            }

            if (!Save_MaterialMeta(matMetaPath, strMaterialGUID, inPath, matPath))
            {
                std::cout << "Save material meta failed : " << matMetaPath.string() << "\n";
                return false;
            }
        }
    }

    for (size_t i = 0; i < model.parts.size(); ++i)
    {
        const auto& part = model.parts[i];

        const std::string strMeshGUID = Generate_GUID_String();
        const std::string strPartFileBase = strMeshBaseName + "_" + std::to_string(i);

        std::filesystem::path meshBinPath = parentMeshDir / (strPartFileBase + ".mesh");
        std::filesystem::path meshMetaPath = parentMeshDir / (strPartFileBase + ".mesh.meta");

        if (!Save_Anim_MeshBin(meshBinPath, part.mesh))
        {
            std::cout << "Save anim mesh bin failed : " << meshBinPath.string() << "\n";
            return false;
        }

        if (!Save_Anim_MeshMeta(meshMetaPath, strMeshGUID, inPath, meshBinPath, part.mesh))
        {
            std::cout << "Save anim mesh meta failed : " << meshMetaPath.string() << "\n";
            return false;
        }

        SAVED_ANIM_MODEL_PART_INFO info{};
        info.strName = part.strName;
        info.strMeshGUID = strMeshGUID;

        auto itFound = materialIndexToGUID.find(part.iMaterialIndex);
        if (itFound != materialIndexToGUID.end())
            info.strMaterialGUID = itFound->second;
        else
            info.strMaterialGUID.clear();

        savedParts.push_back(std::move(info));
    }

    const std::string strModelGUID = Generate_GUID_String();

    if (!Save_Anim_ModelFile(outMeshPath, model, savedParts))
    {
        std::cout << "Save anim model file failed : " << outMeshPath.string() << "\n";
        return false;
    }

    if (!Save_ModelMeta(outMeshMeta, strModelGUID))
    {
        std::cout << "Save model meta failed : " << outMeshMeta.string() << "\n";
        return false;
    }

    return true;
}

NS_END
