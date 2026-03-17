#pragma comment(lib, "Engine.lib")

#include "NonAnim_Converter.h"
#include "Anim_Converter.h"

#pragma region Convert Entry

int Convert_NonAnim_Model(
    std::filesystem::path& inPath,
    std::filesystem::path& textureRoot,
    std::filesystem::path& outMeshPath,
    std::filesystem::path& outMatPath,
    std::filesystem::path& outMeshMeta,
    const _float fImportScale)
{
    _bool bOK = Converter::Convert_NonAnim(
        inPath,
        textureRoot,
        outMeshPath,
        outMatPath,
        outMeshMeta,
        fImportScale);

    if (!bOK)
    {
        std::wcout << L"Convert_NonAnim failed: " << inPath.wstring() << L"\n";
        return 1;
    }

    std::wcout << L"[NONANIM] Converted: " << inPath.wstring()
        << L" -> " << outMeshPath.wstring()
        << L" (Scale=" << fImportScale << L")\n";

    return 0;
}

int Convert_Anim_Model(
    std::filesystem::path& inPath,
    std::filesystem::path& textureRoot,
    std::filesystem::path& outMeshPath,
    std::filesystem::path& outMatPath,
    std::filesystem::path& outMeshMeta,
    const _float fImportScale)
{
    _bool bOK = Converter::Convert_Anim(
        inPath,
        textureRoot,
        outMeshPath,
        outMatPath,
        outMeshMeta,
        fImportScale);

    if (!bOK)
    {
        std::wcout << L"Convert_Anim failed: " << inPath.wstring() << L"\n";
        return 1;
    }

    std::wcout << L"[ANIM] Converted: " << inPath.wstring()
        << L" -> " << outMeshPath.wstring()
        << L" (Scale=" << fImportScale << L")\n";

    return 0;
}

#pragma endregion

int main()
{
    std::filesystem::path fbxPath = L"../../Converter/Bin/FBXs";
    std::filesystem::path modelPath = Engine::ProjectConfig::PATH + Engine::ProjectConfig::MESH;
    std::filesystem::path matPath = Engine::ProjectConfig::PATH + Engine::ProjectConfig::MATERIAL;
    std::filesystem::path textureRoot = Engine::ProjectConfig::PATH + Engine::ProjectConfig::TEXTURE;

    if (!std::filesystem::exists(fbxPath))
    {
        std::cout << "Input file not found: " << fbxPath.string() << "\n";
        return 1;
    }

    std::wstring fileName;
    std::wcout << L"Enter the name of the file to convert under the FBXs folder (excluding extension) : ";
    std::getline(std::wcin, fileName);

    if (fileName.empty())
    {
        std::cout << "File name is empty.\n";
        return 1;
    }

    std::wstring strModelType;
    std::wcout << L"Enter model type (0 = NONANIM, 1 = ANIM) : ";
    std::getline(std::wcin, strModelType);

    if (strModelType.empty())
    {
        std::cout << "Model type is empty.\n";
        return 1;
    }

    int iModelType = 0;
    try
    {
        iModelType = std::stoi(strModelType);
    }
    catch (...)
    {
        std::cout << "Invalid model type.\n";
        return 1;
    }

    if (iModelType != 0 && iModelType != 1)
    {
        std::cout << "Model type must be 0 or 1.\n";
        return 1;
    }

    std::wstringstream ssScale;
    std::wstring strScale;

    std::wcout << L"enter the scale value (e.g., 1.0 / 0.01) : ";
    std::getline(std::wcin, strScale);

    if (strScale.empty())
    {
        std::cout << "Scale is empty.\n";
        return 1;
    }

    ssScale << strScale;

    _float fImportScale = 1.f;
    ssScale >> fImportScale;

    if (ssScale.fail() || fImportScale <= 0.f)
    {
        std::cout << "Invalid scale value.\n";
        return 1;
    }

    std::filesystem::path inPath = fbxPath / (fileName + L".fbx");
    inPath.make_preferred();

    if (!std::filesystem::exists(inPath))
    {
        std::wcout << L"FBX file not found: " << inPath.wstring() << L"\n";
        return 1;
    }

    const std::wstring stem = inPath.stem().wstring();

    std::filesystem::path outMeshPath = modelPath / (stem + L".model");
    outMeshPath.make_preferred();

    std::filesystem::path outMeshMeta = modelPath / (stem + L".model.meta");
    outMeshMeta.make_preferred();

    std::filesystem::path outMatPath = matPath / (stem + L".mat");
    outMatPath.make_preferred();

    std::filesystem::path fbxFile = inPath;
    std::filesystem::path texRoot = textureRoot;

    int iResult = 0;

    if (iModelType == static_cast<int>(Engine::MODEL_TYPE::NONANIM))
    {
        iResult = Convert_NonAnim_Model(
            fbxFile,
            texRoot,
            outMeshPath,
            outMatPath,
            outMeshMeta,
            fImportScale);
    }
    else
    {
        iResult = Convert_Anim_Model(
            fbxFile,
            texRoot,
            outMeshPath,
            outMatPath,
            outMeshMeta,
            fImportScale);
    }

    system("pause");
    return iResult;
}
