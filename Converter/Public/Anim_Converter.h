#pragma once
#include "Converter_Define.h"
#include "Converter_Util.h"
#include "BuiltIn_GUID.h"
#include "Material_Converter.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <set>

NS_BEGIN(Converter)

namespace {

    typedef struct tagBoneWeightSlot
    {
        uint32_t    iIndices[4] = { 0, 0, 0, 0 };
        _float      fWeights[4] = { 0.f, 0.f, 0.f };
    } BONE_WEIGHT_SLOT;

    static void Add_BoneWeight(BONE_WEIGHT_SLOT& slot, uint32_t iBoneIndex, float fWeight)
    {
        if (fWeight <= 0.f)
            return;

        for (size_t i = 0; i < 4; ++i)
        {
            if (slot.fWeights[i] == 0.f) /* 가중치 빈 곳에 넣기 */
            {
                slot.iIndices[i] = iBoneIndex;
                slot.fWeights[i] = fWeight;
                return;
            }
        }
    }

    /* fWeight의 합이 1.0 이상이 되는 경우 방지 */
    static void Normalize_BoneWeight(BONE_WEIGHT_SLOT& slot)
    {
        float fSum = slot.fWeights[0] + slot.fWeights[1] + slot.fWeights[2] + slot.fWeights[3];
        if (fSum <= 0.f)
        {
            slot.iIndices[0] = 0;
            slot.fWeights[0] = 1.f;
            return;
        }

        for (size_t i = 0; i < 4; ++i)
            slot.fWeights[i] /= fSum;
    }

    static _float4x4 To_Float4x4(const aiMatrix4x4& mat)
    {
        _float4x4 out{};
        out._11 = mat.a1; out._12 = mat.b1; out._13 = mat.c1; out._14 = mat.d1;
        out._21 = mat.a2; out._22 = mat.b2; out._23 = mat.c2; out._24 = mat.d2;
        out._31 = mat.a3; out._32 = mat.b3; out._33 = mat.c3; out._34 = mat.d3;
        out._41 = mat.a4; out._42 = mat.b4; out._43 = mat.c4; out._44 = mat.d4;
        return out;
    }

    static void Apply_Scale_To_LocalBind(_float4x4& mat, float fImportScale)
    {
        mat._41 *= fImportScale;
        mat._42 *= fImportScale;
        mat._43 *= fImportScale;
    }

    /* aiNode로 이름 찾기 */
    static const aiNode* Find_Node_By_Name_Recursive(const aiNode* pNode, const std::string& strName)
    {
        if (!pNode)
            return nullptr;

        if (strName == pNode->mName.C_Str())
            return pNode;

        for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
        {
            const aiNode* pFound = Find_Node_By_Name_Recursive(pNode->mChildren[i], strName);
            if (pFound)
                return pFound;
        }

        return nullptr;
    }

    /* 메쉬에서 Bone 이름 추출하기 */
    static void Gather_BoneNames_From_Meshes(const aiScene* pAIScene, std::set<std::string>& outBoneNames)
    {
        outBoneNames.clear();

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

                outBoneNames.insert(pAIBone->mName.C_Str());
            }
        }
    }

    /* 애니메이션으로부터 이름 추출하기 */
    static void Gather_BoneNames_From_Animations(const aiScene* pAIScene, std::set<std::string>& outBoneNames)
    {
        if (!pAIScene)
            return;

        for (uint32_t iAnim = 0; iAnim < pAIScene->mNumAnimations; ++iAnim)
        {
            const aiAnimation* pAIAnim = pAIScene->mAnimations[iAnim];
            if (!pAIAnim)
                continue;

            for (uint32_t iChannel = 0; iChannel < pAIAnim->mNumChannels; ++iChannel)
            {
                const aiNodeAnim* pAIChannel = pAIAnim->mChannels[iChannel];
                if (!pAIChannel)
                    continue;

                outBoneNames.insert(pAIChannel->mNodeName.C_Str());
            }
        }
    }

}

static void Build_Skeleton_Recursive(const aiNode* pNode, int32_t iParentBoneIndex, const std::set<std::string>& boneNames, SKELETON_ENTRY& out, float fImportScale)
{
    if (!pNode)
        return;

    const std::string strNodeName = pNode->mName.C_Str();

    int32_t iThisBoneIndex = iParentBoneIndex;

    if (boneNames.find(strNodeName) != boneNames.end())
    {
        BONE_ENTRY tBone{};
        tBone.strName = strNodeName;
        tBone.iParentBoneIndex = iParentBoneIndex;
        tBone.matLocalBind = To_Float4x4(pNode->mTransformation);
        Apply_Scale_To_LocalBind(tBone.matLocalBind, fImportScale);
        tBone.matCombinedBind = Math::Identity();
        tBone.matOffset = Math::Identity();

        iThisBoneIndex = static_cast<int32_t>(out.bones.size());
        out.bones.push_back(tBone);
        out.BoneNameToIndex[strNodeName] = static_cast<uint32_t>(iThisBoneIndex);

        if (iParentBoneIndex >= 0)
            out.bones[iParentBoneIndex].vecChildBoneIndices.push_back(static_cast<uint32_t>(iThisBoneIndex));
        else if (out.iRootBoneIndex < 0)
            out.iRootBoneIndex = iThisBoneIndex;
    }

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
        Build_Skeleton_Recursive(pNode->mChildren[i], iThisBoneIndex, boneNames, out, fImportScale);
}

static void Build_Skeleton(const aiScene* pAIScene, SKELETON_ENTRY& out, float fImportScale)
{
    out.bones.clear();
    out.BoneNameToIndex.clear();
    out.iRootBoneIndex = -1;

    if (!pAIScene || !pAIScene->mRootNode)
        return;

    std::set<std::string> boneNames;
    Gather_BoneNames_From_Meshes(pAIScene, boneNames);
    Gather_BoneNames_From_Animations(pAIScene, boneNames);

    Build_Skeleton_Recursive(pAIScene->mRootNode, -1, boneNames, out, fImportScale);

    for (size_t i = 0; i < out.bones.size(); ++i)
    {
        const int32_t iParent = out.bones[i].iParentBoneIndex;

        if (iParent < 0)
        {
            out.bones[i].matCombinedBind = out.bones[i].matLocalBind;
        }
        else
        {
            const _matrix matLocal = XMLoadFloat4x4(&out.bones[i].matLocalBind);
            const _matrix matParentCombined = XMLoadFloat4x4(&out.bones[iParent].matCombinedBind);
            XMStoreFloat4x4(&out.bones[i].matCombinedBind, matLocal * matParentCombined);
        }
    }

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

            auto it = out.BoneNameToIndex.find(pAIBone->mName.C_Str());
            if (it == out.BoneNameToIndex.end())
                continue;

            out.bones[it->second].matOffset = To_Float4x4(pAIBone->mOffsetMatrix);
        }
    }
}

static ANIM_KEYFRAME Sample_Channel_KeyFrame(const aiNodeAnim* pAIChannel, double fTimeTick, float fImportScale)
{
    ANIM_KEYFRAME out{};

    out.fTrackPosition = (_float)fTimeTick;
    out.vScale = _float3(1.f, 1.f, 1.f);
    out.vRotation = _float4(0.f, 0.f, 0.f, 1.f);
    out.vTranslation = _float3(0.f, 0.f, 0.f);

    if (pAIChannel->mNumScalingKeys > 0)
    {
        uint32_t iKey = 0;
        while ((iKey + 1) < pAIChannel->mNumScalingKeys &&
            pAIChannel->mScalingKeys[iKey + 1].mTime <= fTimeTick)
        {
            ++iKey;
        }

        if ((iKey + 1) < pAIChannel->mNumScalingKeys)
        {
            const aiVectorKey& left = pAIChannel->mScalingKeys[iKey];
            const aiVectorKey& right = pAIChannel->mScalingKeys[iKey + 1];
            const double denom = right.mTime - left.mTime;
            const float ratio = (denom <= 0.0) ? 0.f : float((fTimeTick - left.mTime) / denom);

            XMFLOAT3 l(left.mValue.x, left.mValue.y, left.mValue.z);
            XMFLOAT3 r(right.mValue.x, right.mValue.y, right.mValue.z);

            out.vScale.x = l.x + (r.x - l.x) * ratio;
            out.vScale.y = l.y + (r.y - l.y) * ratio;
            out.vScale.z = l.z + (r.z - l.z) * ratio;
        }
        else
        {
            const aiVector3D& v = pAIChannel->mScalingKeys[pAIChannel->mNumScalingKeys - 1].mValue;
            out.vScale = _float3(v.x, v.y, v.z);
        }
    }

    if (pAIChannel->mNumRotationKeys > 0)
    {
        uint32_t iKey = 0;
        while ((iKey + 1) < pAIChannel->mNumRotationKeys &&
            pAIChannel->mRotationKeys[iKey + 1].mTime <= fTimeTick)
        {
            ++iKey;
        }

        if ((iKey + 1) < pAIChannel->mNumRotationKeys)
        {
            const aiQuatKey& left = pAIChannel->mRotationKeys[iKey];
            const aiQuatKey& right = pAIChannel->mRotationKeys[iKey + 1];
            const double denom = right.mTime - left.mTime;
            const float ratio = (denom <= 0.0) ? 0.f : float((fTimeTick - left.mTime) / denom);

            const _vector vLeft = XMVectorSet(left.mValue.x, left.mValue.y, left.mValue.z, left.mValue.w);
            const _vector vRight = XMVectorSet(right.mValue.x, right.mValue.y, right.mValue.z, right.mValue.w);
            _float4 q{};
            XMStoreFloat4(&q, XMQuaternionSlerp(vLeft, vRight, ratio));
            out.vRotation = q;
        }
        else
        {
            const aiQuaternion& q = pAIChannel->mRotationKeys[pAIChannel->mNumRotationKeys - 1].mValue;
            out.vRotation = _float4(q.x, q.y, q.z, q.w);
        }
    }

    if (pAIChannel->mNumPositionKeys > 0)
    {
        uint32_t iKey = 0;
        while ((iKey + 1) < pAIChannel->mNumPositionKeys &&
            pAIChannel->mPositionKeys[iKey + 1].mTime <= fTimeTick)
        {
            ++iKey;
        }

        if ((iKey + 1) < pAIChannel->mNumPositionKeys)
        {
            const aiVectorKey& left = pAIChannel->mPositionKeys[iKey];
            const aiVectorKey& right = pAIChannel->mPositionKeys[iKey + 1];
            const double denom = right.mTime - left.mTime;
            const float ratio = (denom <= 0.0) ? 0.f : float((fTimeTick - left.mTime) / denom);

            XMFLOAT3 l(left.mValue.x, left.mValue.y, left.mValue.z);
            XMFLOAT3 r(right.mValue.x, right.mValue.y, right.mValue.z);

            out.vTranslation.x = (l.x + (r.x - l.x) * ratio) * fImportScale;
            out.vTranslation.y = (l.y + (r.y - l.y) * ratio) * fImportScale;
            out.vTranslation.z = (l.z + (r.z - l.z) * ratio) * fImportScale;
        }
        else
        {
            const aiVector3D& v = pAIChannel->mPositionKeys[pAIChannel->mNumPositionKeys - 1].mValue;
            out.vTranslation = _float3(v.x * fImportScale, v.y * fImportScale, v.z * fImportScale);
        }
    }

    return out;
}

static _bool Convert_Animations(
    const aiScene* pAIScene,
    const SKELETON_ENTRY& tSkeleton,
    std::vector<ANIMATION_CLIP_ENTRY>& outClips,
    float fImportScale)
{
    outClips.clear();

    if (!pAIScene || pAIScene->mNumAnimations == 0)
        return true;

    for (uint32_t iAnim = 0; iAnim < pAIScene->mNumAnimations; ++iAnim)
    {
        const aiAnimation* pAIAnim = pAIScene->mAnimations[iAnim];
        if (!pAIAnim)
            continue;

        ANIMATION_CLIP_ENTRY tClip{};
        ASSET_GUID::Try_Utf8_To_GUID(Generate_GUID_String(), tClip.tGUID);
        tClip.strName = (pAIAnim->mName.length > 0) ? pAIAnim->mName.C_Str() : ("Anim_" + std::to_string(iAnim));
        tClip.fDuration = (_float)pAIAnim->mDuration;
        tClip.fTickPerSecond = (_float)((pAIAnim->mTicksPerSecond > 0.0) ? pAIAnim->mTicksPerSecond : 25.0);

        tClip.channels.reserve(pAIAnim->mNumChannels);

        for (uint32_t iChannel = 0; iChannel < pAIAnim->mNumChannels; ++iChannel)
        {
            const aiNodeAnim* pAIChannel = pAIAnim->mChannels[iChannel];
            if (!pAIChannel)
                continue;

            ANIMATION_CHANNEL_ENTRY tChannel{};
            tChannel.strBoneName = pAIChannel->mNodeName.C_Str();

            auto itBone = tSkeleton.BoneNameToIndex.find(tChannel.strBoneName);
            if (itBone == tSkeleton.BoneNameToIndex.end())
                continue;

            tChannel.iBoneIndex = (int32_t)itBone->second;

            std::set<double> keyTimes;
            for (uint32_t i = 0; i < pAIChannel->mNumScalingKeys; ++i)   keyTimes.insert(pAIChannel->mScalingKeys[i].mTime);
            for (uint32_t i = 0; i < pAIChannel->mNumRotationKeys; ++i)  keyTimes.insert(pAIChannel->mRotationKeys[i].mTime);
            for (uint32_t i = 0; i < pAIChannel->mNumPositionKeys; ++i)  keyTimes.insert(pAIChannel->mPositionKeys[i].mTime);

            if (keyTimes.empty())
            {
                ANIM_KEYFRAME tKey{};
                tKey.fTrackPosition = 0.f;
                tChannel.vecKeyFrames.push_back(tKey);
            }
            else
            {
                tChannel.vecKeyFrames.reserve(keyTimes.size());
                for (double fTimeTick : keyTimes)
                {
                    ANIM_KEYFRAME tKey = Sample_Channel_KeyFrame(pAIChannel, fTimeTick, fImportScale);
                    tKey.fTrackPosition = (_float)fTimeTick;
                    tChannel.vecKeyFrames.push_back(tKey);
                }
            }

            tClip.channels.push_back(std::move(tChannel));
        }

        outClips.push_back(std::move(tClip));
    }

    return true;
}

static _bool Convert_SingleAnimMesh(
    const aiMesh* pAIMesh,
    const SKELETON_ENTRY& tSkeleton,
    Engine::CONVERTED_ANIM_MESH& out,
    std::vector<uint32_t>& outBoneIndices,
    std::vector<_float4x4>& outOffsetMatrices,
    const float fImportScale)
{
    if (!pAIMesh || pAIMesh->mNumVertices == 0)
        return false;

    out.vertices.clear();
    out.indices.clear();
    outBoneIndices.clear();
    outOffsetMatrices.clear();

    const _bool hasNormals = (pAIMesh->mNormals != nullptr);
    const _bool hasTangents = (pAIMesh->mTangents != nullptr);
    const _bool hasUV0 = (pAIMesh->mTextureCoords[0] != nullptr);

    out.vertices.resize(pAIMesh->mNumVertices);
    std::vector<BONE_WEIGHT_SLOT> weightSlots(pAIMesh->mNumVertices);

    for (size_t v = 0; v < pAIMesh->mNumVertices; ++v)
    {
        CopyFloat3_Scale(out.vertices[v].vPosition, pAIMesh->mVertices[v], fImportScale);

        if (hasNormals)  CopyFloat3(out.vertices[v].vNormal, pAIMesh->mNormals[v]);
        else             out.vertices[v].vNormal = _float3{ 0.f, 1.f, 0.f };

        if (hasTangents) CopyFloat3(out.vertices[v].vTangent, pAIMesh->mTangents[v]);
        else             out.vertices[v].vTangent = _float3{ 1.f, 0.f, 0.f };

        if (hasUV0)      CopyFloat2(out.vertices[v].vTexcoord, pAIMesh->mTextureCoords[0][v]);
        else             out.vertices[v].vTexcoord = _float2{ 0.f, 0.f };
    }

    for (uint32_t iBone = 0; iBone < pAIMesh->mNumBones; ++iBone)
    {
        const aiBone* pAIBone = pAIMesh->mBones[iBone];
        if (!pAIBone)
            continue;

        auto it = tSkeleton.BoneNameToIndex.find(pAIBone->mName.C_Str());
        if (it == tSkeleton.BoneNameToIndex.end())
            continue;

        const uint32_t iSkeletonBoneIndex = it->second;
        const uint32_t iPaletteIndex = static_cast<uint32_t>(outBoneIndices.size());

        outBoneIndices.push_back(iSkeletonBoneIndex);
        outOffsetMatrices.push_back(To_Float4x4(pAIBone->mOffsetMatrix));

        for (uint32_t iWeight = 0; iWeight < pAIBone->mNumWeights; ++iWeight)
        {
            const aiVertexWeight& w = pAIBone->mWeights[iWeight];
            if (w.mVertexId >= pAIMesh->mNumVertices)
                continue;

            Add_BoneWeight(weightSlots[w.mVertexId], iSkeletonBoneIndex, w.mWeight);
        }
    }

    for (size_t v = 0; v < pAIMesh->mNumVertices; ++v)
    {
        Normalize_BoneWeight(weightSlots[v]);

        out.vertices[v].vBlendIndex = XMUINT4(
            weightSlots[v].iIndices[0],
            weightSlots[v].iIndices[1],
            weightSlots[v].iIndices[2],
            weightSlots[v].iIndices[3]);

        out.vertices[v].vBlendWeight = _float4(
            weightSlots[v].fWeights[0],
            weightSlots[v].fWeights[1],
            weightSlots[v].fWeights[2],
            weightSlots[v].fWeights[3]);
    }

    out.indices.reserve(pAIMesh->mNumFaces * 3);

    for (uint32_t f = 0; f < pAIMesh->mNumFaces; ++f)
    {
        const aiFace& face = pAIMesh->mFaces[f];
        if (face.mNumIndices != 3)
            continue;

        out.indices.push_back((uint32_t)face.mIndices[0]);
        out.indices.push_back((uint32_t)face.mIndices[1]);
        out.indices.push_back((uint32_t)face.mIndices[2]);
    }

    return !out.vertices.empty() && !out.indices.empty();
}

static _bool Convert_AnimModel(const aiScene* pAIScene, CONVERTED_ANIM_MODEL& out, const float fImportScale)
{
    if (!pAIScene || pAIScene->mNumMeshes == 0)
        return false;

    out.parts.clear();
    out.tSkeleton = {};
    out.animClips.clear();

    Build_Skeleton(pAIScene, out.tSkeleton, fImportScale);
    if (out.tSkeleton.bones.empty())
        return false;

    if (!Convert_Animations(pAIScene, out.tSkeleton, out.animClips, fImportScale))
        return false;

    for (size_t i = 0; i < pAIScene->mNumMeshes; ++i)
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

        if (!Convert_SingleAnimMesh(
            pAIMesh,
            out.tSkeleton,
            part.mesh,
            part.vecBoneIndices,
            part.vecOffsetMatrices,
            fImportScale))
        {
            continue;
        }

        out.parts.push_back(std::move(part));
    }

    return !out.parts.empty();
}

static const aiScene* LoadScene_Assimp_Anim(Assimp::Importer& importer, const std::filesystem::path& fbxPath, uint32_t iFlag = 0)
{
    const uint32_t flags =
        aiProcess_ConvertToLeftHanded |
        aiProcessPreset_TargetRealtime_Fast |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace;

    return importer.ReadFile(fbxPath.string(), flags | iFlag);
}

/* ------------------------------------------------------------
    저장부
------------------------------------------------------------ */

typedef struct tagAnimMeshHeader
{
    uint32_t iMagic = 0x4D534842; /* 'MSHB' */
    uint32_t iVersion = 2;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    uint32_t vertexStride = sizeof(VTXANIMMESH);
    uint32_t reserved0 = 0;
} ANIM_MESH_HEADER;

static _bool Save_Anim_MeshBin(const std::filesystem::path& outPath, const CONVERTED_ANIM_MESH& mesh)
{
    std::ofstream ofs(outPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    ANIM_MESH_HEADER hdr{};
    hdr.vertexCount = (uint32_t)mesh.vertices.size();
    hdr.indexCount = (uint32_t)mesh.indices.size();
    hdr.vertexStride = sizeof(VTXANIMMESH);

    ofs.write((const char*)&hdr, sizeof(hdr));
    ofs.write((const char*)mesh.vertices.data(), sizeof(Engine::VTXANIMMESH) * mesh.vertices.size());
    ofs.write((const char*)mesh.indices.data(), sizeof(uint32_t) * mesh.indices.size());

    return true;
}

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
    ofs << "vertexFormat=VTXANIMMESH\n";

    return true;
}

static _bool Save_Anim_ModelFile(
    const std::filesystem::path& modelPath,
    const std::filesystem::path& sourceFbx,
    const std::vector<SAVED_MODEL_PART_INFO>& parts,
    const SKELETON_ENTRY& tSkeleton,
    const std::vector<ANIMATION_CLIP_ENTRY>& vecAnimClips,
    const CONVERTED_ANIM_MODEL& model)
{
    std::ofstream ofs(modelPath);
    if (!ofs.is_open())
        return false;

    ofs << "source=" << sourceFbx.generic_string() << "\n";
    ofs << "meshCount=" << parts.size() << "\n";
    ofs << "boneCount=" << tSkeleton.bones.size() << "\n";
    ofs << "animCount=" << vecAnimClips.size() << "\n";

    for (size_t i = 0; i < parts.size(); ++i)
    {
        ofs << "part" << i << "Name=" << parts[i].strName << "\n";
        ofs << "part" << i << "MeshGuid=" << parts[i].strMeshGUID << "\n";
        ofs << "part" << i << "MaterialGuid=" << parts[i].strMaterialGUID << "\n";

        if (i < model.parts.size())
        {
            ofs << "part" << i << "BoneCount=" << model.parts[i].vecBoneIndices.size() << "\n";

            for (size_t j = 0; j < model.parts[i].vecBoneIndices.size(); ++j)
            {
                ofs << "part" << i << "BoneIndex" << j << "=" << model.parts[i].vecBoneIndices[j] << "\n";

                const _float4x4& mat = model.parts[i].vecOffsetMatrices[j];
                ofs << "part" << i << "Offset" << j << "="
                    << mat._11 << "," << mat._12 << "," << mat._13 << "," << mat._14 << ","
                    << mat._21 << "," << mat._22 << "," << mat._23 << "," << mat._24 << ","
                    << mat._31 << "," << mat._32 << "," << mat._33 << "," << mat._34 << ","
                    << mat._41 << "," << mat._42 << "," << mat._43 << "," << mat._44 << "\n";
            }
        }
    }

    for (size_t i = 0; i < tSkeleton.bones.size(); ++i)
    {
        const auto& bone = tSkeleton.bones[i];
        ofs << "bone" << i << "Name=" << bone.strName << "\n";
        ofs << "bone" << i << "Parent=" << bone.iParentBoneIndex << "\n";
        ofs << "bone" << i << "LocalBind="
            << bone.matLocalBind._11 << "," << bone.matLocalBind._12 << "," << bone.matLocalBind._13 << "," << bone.matLocalBind._14 << ","
            << bone.matLocalBind._21 << "," << bone.matLocalBind._22 << "," << bone.matLocalBind._23 << "," << bone.matLocalBind._24 << ","
            << bone.matLocalBind._31 << "," << bone.matLocalBind._32 << "," << bone.matLocalBind._33 << "," << bone.matLocalBind._34 << ","
            << bone.matLocalBind._41 << "," << bone.matLocalBind._42 << "," << bone.matLocalBind._43 << "," << bone.matLocalBind._44 << "\n";
        ofs << "bone" << i << "Offset="
            << bone.matOffset._11 << "," << bone.matOffset._12 << "," << bone.matOffset._13 << "," << bone.matOffset._14 << ","
            << bone.matOffset._21 << "," << bone.matOffset._22 << "," << bone.matOffset._23 << "," << bone.matOffset._24 << ","
            << bone.matOffset._31 << "," << bone.matOffset._32 << "," << bone.matOffset._33 << "," << bone.matOffset._34 << ","
            << bone.matOffset._41 << "," << bone.matOffset._42 << "," << bone.matOffset._43 << "," << bone.matOffset._44 << "\n";
    }

    for (size_t i = 0; i < vecAnimClips.size(); ++i)
    {
        const auto& clip = vecAnimClips[i];
        ofs << "anim" << i << "Name=" << clip.strName << "\n";
        ofs << "anim" << i << "Duration=" << clip.fDuration << "\n";
        ofs << "anim" << i << "TickPerSecond=" << clip.fTickPerSecond << "\n";
        ofs << "anim" << i << "ChannelCount=" << clip.channels.size() << "\n";

        for (size_t c = 0; c < clip.channels.size(); ++c)
        {
            const auto& ch = clip.channels[c];
            ofs << "anim" << i << "Channel" << c << "BoneName=" << ch.strBoneName << "\n";
            ofs << "anim" << i << "Channel" << c << "BoneIndex=" << ch.iBoneIndex << "\n";
            ofs << "anim" << i << "Channel" << c << "KeyCount=" << ch.vecKeyFrames.size() << "\n";

            for (size_t k = 0; k < ch.vecKeyFrames.size(); ++k)
            {
                const auto& key = ch.vecKeyFrames[k];
                ofs << "anim" << i << "Channel" << c << "Key" << k << "="
                    << key.fTrackPosition << "|"
                    << key.vScale.x << "," << key.vScale.y << "," << key.vScale.z << "|"
                    << key.vRotation.x << "," << key.vRotation.y << "," << key.vRotation.z << "," << key.vRotation.w << "|"
                    << key.vTranslation.x << "," << key.vTranslation.y << "," << key.vTranslation.z
                    << "\n";
            }
        }
    }

    return true;
}

/* ------------------------------------------------------------
    최종 진입점
------------------------------------------------------------ */

inline static _bool Convert_Anim(
    std::filesystem::path& inPath,
    std::filesystem::path& textureRoot,
    std::filesystem::path& outMeshPath,
    std::filesystem::path& outMatPath,
    std::filesystem::path& outMeshMeta,
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
        std::cout << "Convert_AnimModel failed : " << inPath.string() << "\n";
        return false;
    }

    std::filesystem::create_directories(outMeshPath.parent_path());
    std::filesystem::create_directories(outMatPath.parent_path());

    std::vector<SAVED_MODEL_PART_INFO> savedParts;
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

        SAVED_MODEL_PART_INFO info{};
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
    if (!Save_Anim_ModelFile(outMeshPath, inPath, savedParts, model.tSkeleton, model.animClips, model))
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
