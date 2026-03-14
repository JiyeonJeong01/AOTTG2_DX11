#include "Mesh_Converter.h"

#pragma comment(lib, "Engine.lib")

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

    _bool bOK = Converter::Convert(
        fbxFile,
        texRoot,
        outMeshPath,
        outMatPath,
        outMeshMeta,
        fImportScale);

    if (!bOK)
    {
        std::wcout << L"Convert failed: " << inPath.wstring() << L"\n";
        system("pause");
        return 1;
    }

    std::wcout << L"Converted: " << inPath.wstring()
        << L" -> " << outMeshPath.wstring()
        << L" (Scale=" << fImportScale << L")\n";

    system("pause");
    return 0;
}
