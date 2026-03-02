#include "Mesh_Converter.h"

int main()
{
    std::filesystem::path fbxPath = L"../../Converter/Bin/FBXs";
    std::filesystem::path meshPath = Engine::ProjectConfig::PATH + Engine::ProjectConfig::MESH;

    if (!std::filesystem::exists(fbxPath))
    {
        std::cout << "Input file not found: " << fbxPath.string() << "\n";
        return 1;
    }

    /* 이미 만들어진 .mesh 목록 빌드 */
    std::unordered_set<std::wstring> existing = Build_ExistingMeshStemSet(meshPath);


    /* /FBXs/ 폴더의 중복되지 않는 .fbx만 추출 */
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
        if (ec) break;
        if (!it.is_regular_file(ec)) continue;

        const std::filesystem::path inPath = it.path();
        if (inPath.extension() != L".fbx")
            continue;

        const std::wstring stem = inPath.stem().wstring();

        if (existing.find(stem) != existing.end())
        {
            ++skippedCount;
            continue;
        }

        std::filesystem::path outPath = meshPath / (stem + L".mesh");
        outPath.make_preferred();
        std::filesystem::path outMeta = outPath;
        outMeta += L".meta";

        _bool bOK = Converter::Convert(const_cast<std::filesystem::path&>(inPath),
            outPath,
            outMeta);
        if (!bOK)
        {
            std::cout << "Convert failed: " << inPath.string() << "\n";
            continue;
        }

        existing.insert(stem);
        ++convertedCount;

        std::cout << "Converted: " << inPath.string() << " -> " << outPath.string() << "\n";
    }

    std::cout << "Done. converted=" << convertedCount << " skipped=" << skippedCount << "\n";

    system("pause");
    return 0;
}
