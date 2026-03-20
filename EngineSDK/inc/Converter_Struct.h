#pragma once

#include "Render_Struct.h"
#include "Animation_Clip.h"
#include "Skeleton.h"
#include "Identity.h"
#include "Engine_Math.h"

NS_BEGIN(Engine)

/* 스킨드 메쉬 하나의 변환 결과를 담는 구조체 */
typedef struct tagConvertedAnimMesh
{
    std::vector<VTXANIMMESH> vertices;    /* 애니메이션 메쉬 정점 배열 */
    std::vector<uint32_t>    indices;     /* 인덱스 배열 */
} CONVERTED_ANIM_MESH;

/* 모델의 파트 하나를 담는 구조체 */
typedef struct tagConvertedAnimModelPart
{
    std::string         strName = "";         /* 파트 이름 */
    uint32_t            iMaterialIndex = 0;   /* Assimp Material Index */

    CONVERTED_ANIM_MESH mesh;                 /* 파트 메쉬 데이터 */
} CONVERTED_ANIM_MODEL_PART;

/* 컨버터에서 사용하는 Bone 정보 */
typedef struct tagConvertedBone
{
    std::string             strName = "";             /* Bone 이름 */

    int32_t                 iParentBoneIndex = -1;   /* 부모 Bone Index */
    std::vector<uint32_t>   vecChildBoneIndices;     /* 자식 Bone Index 목록 */

    _float4x4               matLocalBind{};          /* Bind Pose Local 행렬 */
    _float4x4               matCombinedBind{};       /* Bind Pose Combined 행렬 */
    _float4x4               matOffset{};             /* Skinning Offset 행렬 */
} CONVERTED_BONE;

/* 컨버터에서 사용하는 Skeleton 정보 */
typedef struct tagConvertedSkeleton
{
    std::vector<CONVERTED_BONE>                 bones;              /* 전체 Bone 배열 */

    int32_t                                     iRootBoneIndex = -1;/* 루트 Bone Index */
    std::unordered_map<std::string, uint32_t>  BoneNameToIndex;    /* Bone 이름 -> Index 매핑 */
} CONVERTED_SKELETON;

/* 컨버터에서 사용하는 애니메이션 채널 정보 */
typedef struct tagConvertedAnimationChannel
{
    std::string                 strBoneName = "";   /* 대상 Bone 이름 */
    int32_t                     iBoneIndex = -1;    /* 대상 Bone Index */

    std::vector<ANIM_KEYFRAME>  vecKeyFrames;       /* 키프레임 배열 */
} CONVERTED_ANIMATION_CHANNEL;

/* 컨버터에서 사용하는 애니메이션 클립 정보 */
typedef struct tagConvertedAnimationClip
{
    std::string                                 strName = "";       /* 클립 이름 */

    _float                                      fDuration = 0.f;    /* 클립 길이 */
    _float                                      fTickPerSecond = 0.f;/* 초당 Tick 수 */

    std::vector<CONVERTED_ANIMATION_CHANNEL>    channels;           /* 애니메이션 채널 목록 */
} CONVERTED_ANIMATION_CLIP;

/* 애니메이션 모델 전체 변환 결과 */
typedef struct tagConvertedAnimModel
{
    std::vector<CONVERTED_ANIM_MODEL_PART>  parts;          /* 모델 파트 목록 */
    CONVERTED_SKELETON                      tSkeleton;      /* 스켈레톤 정보 */
    std::vector<CONVERTED_ANIMATION_CLIP>   vecAnimClips;   /* 애니메이션 클립 목록 */
} CONVERTED_ANIM_MODEL;

/* Vertex Bone Weight 누적용 임시 구조체 */
typedef struct tagBoneWeightSlot
{
    uint32_t    iIndices[4] = { 0, 0, 0, 0 };      /* 최대 4개 Bone Index */
    _float      fWeights[4] = { 0.f, 0.f, 0.f, 0.f }; /* 최대 4개 Weight */
} BONE_WEIGHT_SLOT;

/* 애니메이션 메쉬 파일 헤더 */
typedef struct tagAnimMeshHeader
{
    uint32_t    vertexCount = 0;   /* 정점 개수 */
    uint32_t    indexCount = 0;    /* 인덱스 개수 */
} ANIM_MESH_HEADER;

/* 스켈레톤 블록 헤더 */
typedef struct tagSkeletonHeaderBin
{
    uint32_t    boneCount = 0;         /* Bone 개수 */
    int32_t     iRootBoneIndex = -1;   /* 루트 Bone Index */
} SKELETON_HEADER_BIN;

/* Bone 하나 저장할 때 사용하는 헤더 */
typedef struct tagBoneHeaderBin
{
    int32_t     iParentBoneIndex = -1; /* 부모 Bone Index */
    uint32_t    childCount = 0;        /* 자식 Bone 개수 */
} BONE_HEADER_BIN;

/* 애니메이션 클립 블록 헤더 */
typedef struct tagAnimationClipHeaderBin
{
    _float      fDuration = 0.f;       /* 클립 길이 */
    _float      fTickPerSecond = 0.f;  /* 초당 Tick 수 */
    uint32_t    channelCount = 0;      /* 채널 개수 */
} ANIMATION_CLIP_HEADER_BIN;

/* 애니메이션 채널 블록 헤더 */
typedef struct tagAnimationChannelHeaderBin
{
    int32_t     iBoneIndex = -1;       /* 대상 Bone Index */
    uint32_t    keyFrameCount = 0;     /* 키프레임 개수 */
} ANIMATION_CHANNEL_HEADER_BIN;

/* 애니메이션 모델 파일 최상단 헤더 */
typedef struct tagAnimModelHeader
{
    uint32_t    iMagic = 0x4D494E41;   /* 'ANIM' 식별자 */
    uint32_t    iVersion = 1;          /* 파일 버전 */

    uint32_t    partCount = 0;         /* 모델 파트 개수 */
    uint32_t    clipCount = 0;         /* 애니메이션 클립 개수 */
    uint32_t    boneCount = 0;         /* 전체 Bone 개수 */
} ANIM_MODEL_HEADER;

/* 모델 파일에 기록할 파트별 저장 정보 */
typedef struct tagSavedAnimModelPartInfo
{
    std::string strName;               /* 파트 이름 */
    std::string strMeshGUID;           /* 저장된 메쉬 GUID */
    std::string strMaterialGUID;       /* 연결할 머티리얼 GUID */
} SAVED_ANIM_MODEL_PART_INFO;

typedef struct tagConvertedMesh
{
    std::vector<Engine::VTXMESH> vertices;
    std::vector<uint32_t> indices;
} CONVERTED_MESH;

typedef struct tagConvertedModelPart
{
    std::string     strName;
    CONVERTED_MESH  mesh;
    uint32_t        iMaterialIndex = 0;

} CONVERTED_MODEL_PART;

typedef struct tagConvertedModel
{
    std::vector<CONVERTED_MODEL_PART> parts;
} CONVERTED_MODEL;

typedef struct tagSavedModelPartInfo
{
    std::string strName;
    std::string strMeshGUID;
    std::string strMaterialGUID;
} SAVED_MODEL_PART_INFO;

typedef struct tagModelPartDesc
{
    std::string strName;
    ASSET_GUID  tMeshGUID{};
    ASSET_GUID  tMaterialGUID{};
} MODEL_PART_DESC;

typedef struct tagModelDesc
{
    ASSET_GUID tGUID{};
    std::filesystem::path pathSource;
    std::vector<MODEL_PART_DESC> parts;
} MODEL_DESC;

typedef struct tagAnimModelPartDesc
{
    std::string strName;
    ASSET_GUID  tMeshGUID{};
    ASSET_GUID  tMaterialGUID{};

    std::vector<uint32_t>       vecBoneIndices;
    std::vector<_float4x4>      vecOffsetMatrices;
} ANIM_MODEL_PART_DESC;

typedef struct tagAnimModelDesc
{
    ASSET_GUID tGUID{};
    std::filesystem::path pathSource;

    std::vector<ANIM_MODEL_PART_DESC>   parts;
    SKELETON_ENTRY                      tSkeleton;
    std::vector<ANIMATION_CLIP_ENTRY>   vecAnimClips;
} ANIM_MODEL_DESC;

typedef struct tagMeshHeader
{
    /* Magic Number : 0x4D534842 -> 'MSHB' */
    uint32_t iMagic = 0x4D534842;

    uint32_t iVersion = 1;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    uint32_t vertexStride = sizeof(VTXMESH);
    uint32_t reserved0 = 0;
} MESH_HEADER;

typedef struct tagAnimModelBinHeader
{
    uint32_t    iMagic = 0x4C444F4D;   /* 'MODL' */
    uint32_t    iVersion = 1;

    uint32_t    meshCount = 0;
    uint32_t    boneCount = 0;
    uint32_t    animCount = 0;
    uint32_t    reserved0 = 0;
} ANIM_MODEL_BIN_HEADER;

typedef struct tagAnimModelPartBin
{
    uint32_t    nameLength = 0;
    uint32_t    materialGuidLength = 0;
    uint32_t    meshGuidLength = 0;
    uint32_t    boneCount = 0;
} ANIM_MODEL_PART_BIN;

typedef struct tagAnimModelPartBoneBin
{
    uint32_t    iBoneIndex = 0;
} ANIM_MODEL_PART_BONE_BIN;

typedef struct tagAnimModelBoneBin
{
    int32_t     iParentBoneIndex = -1;
    uint32_t    nameLength = 0;
} ANIM_MODEL_BONE_BIN;

typedef struct tagAnimModelAnimClipBin
{
    uint32_t    nameLength = 0;
    _float      fDuration = 0.f;
    _float      fTickPerSecond = 0.f;
    uint32_t    channelCount = 0;
} ANIM_MODEL_ANIM_CLIP_BIN;

typedef struct tagAnimModelAnimChannelBin
{
    uint32_t    boneNameLength = 0;
    int32_t     iBoneIndex = -1;
    uint32_t    keyCount = 0;
    uint32_t    reserved0 = 0;
} ANIM_MODEL_ANIM_CHANNEL_BIN;

typedef struct tagAnimModelAnimKeyBin
{
    _float      fTrackPosition = 0.f;
    _float3     vScale = _float3(1.f, 1.f, 1.f);
    _float4     vRotation = _float4(0.f, 0.f, 0.f, 1.f);
    _float3     vTranslation = _float3(0.f, 0.f, 0.f);
} ANIM_MODEL_ANIM_KEY_BIN;

NS_END
