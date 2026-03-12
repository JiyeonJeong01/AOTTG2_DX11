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

    std::unordered_set<std::wstring> existing = Build_ExistingModelStemSet(modelPath);

    std::error_code ec;
    if (!std::filesystem::exists(fbxPath, ec))
    {
        std::cout << "FBX root not found: " << fbxPath.string() << "\n";
        return 1;
    }

    uint32_t convertedCount = 0;
    uint32_t skippedCount = 0;

    for (auto& it : std::filesystem::recursive_directory_iterator(fbxPath, ec))
    {
        if (ec)
            break;
        if (!it.is_regular_file(ec))
            continue;

        const std::filesystem::path inPath = it.path();
        if (inPath.extension() != L".fbx")
            continue;

        const std::wstring stem = inPath.stem().wstring();

        if (existing.find(stem) != existing.end())
        {
            ++skippedCount;
            continue;
        }

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
            outMeshMeta);

        if (!bOK)
        {
            std::cout << "Convert failed: " << inPath.string() << "\n";
            continue;
        }

        existing.insert(stem);
        ++convertedCount;

        std::cout << "Converted: " << inPath.string() << " -> " << outMeshPath.string() << "\n";
    }

    std::cout << "Done. converted=" << convertedCount << " skipped=" << skippedCount << "\n";

    system("pause");
    return 0;
}
