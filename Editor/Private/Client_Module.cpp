#include "Client_Module.h"

#include "Core_System.h"
#include "Engine_Log.h"

NS_BEGIN(Editor)

namespace
{
    HMODULE                 g_hClient = nullptr;
    std::filesystem::path   g_LoadedCopy;
    uint32_t                g_iCopyIndex = 0;

    std::filesystem::path Exe_Dir()
    {
        wchar_t szPath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, szPath, MAX_PATH);
        return std::filesystem::path(szPath).parent_path();
    }

    /* 이전 실행이 비정상 종료해 남은 복사본 정리 (잠긴 파일은 건너뛴다) */
    void Remove_Stale_Copies(const std::filesystem::path& dir)
    {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
        {
            const std::wstring name = entry.path().filename().wstring();
            if (name.rfind(L"Client_hot_", 0) == 0 && entry.path().extension() == L".dll")
                std::filesystem::remove(entry.path(), ec);
        }
    }
}

_bool ClientModule::Load()
{
    const std::filesystem::path dir = Exe_Dir();
    if (nullptr == g_hClient && 0 == g_iCopyIndex)
        Remove_Stale_Copies(dir);

    const std::filesystem::path src = dir / L"Client.dll";
    const std::filesystem::path dst = dir / (L"Client_hot_" + std::to_wstring(g_iCopyIndex++) + L".dll");

    std::error_code ec;
    std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
    IF_TRUE_RETURN_MSG_BREAK(ec, false, "Client.dll copy failed : %s", ec.message().c_str());

    HMODULE hModule = LoadLibraryW(dst.c_str());
    IF_NULL_RETURN_MSG_BREAK(hModule, false, "LoadLibrary failed : Client copy");

    using RegisterFn = void(*)();
    RegisterFn pfnRegister = reinterpret_cast<RegisterFn>(GetProcAddress(hModule, "Client_Register_AllScripts"));
    if (nullptr == pfnRegister)
    {
        FreeLibrary(hModule);
        std::filesystem::remove(dst, ec);
        _DEBUG_ERROR_BREAK("Client_Register_AllScripts not found.");
        return false;
    }

    g_hClient = hModule;
    g_LoadedCopy = dst;
    pfnRegister();

    return true;
}

_bool ClientModule::Reload()
{
    /* 1. 씬 오브젝트(Script 인스턴스 · 리스너)와 Script 함수 주소를 이전 DLL이 살아 있을 때 정리 */
    if (!SYS_CORE.Unload_Scripts())
        return false;

    /* 2. DLL 교체 */
    Unload();
    if (!Load())
        return false;

    /* 3. 같은 씬 파일을 다시 로드 (Initialize_From_Spec이 새 VTable을 복사한다) */
    SYS_CORE.Restart();

    LOG_INFO("Client hot reload done.");
    return true;
}

void ClientModule::Unload()
{
    if (nullptr == g_hClient)
        return;

    FreeLibrary(g_hClient);
    g_hClient = nullptr;

    std::error_code ec;
    std::filesystem::remove(g_LoadedCopy, ec);
    g_LoadedCopy.clear();
}

NS_END
