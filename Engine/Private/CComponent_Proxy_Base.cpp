#include "CComponent_Proxy_Base.h"
#include "Component_System.h"

/*
    CComponent_Proxy_Base.h는 Component_System.h를 include할 수 없다 (Component_System → Processor → Pool → Proxy_Base 순환).
    그래서 헤더에는 선언만 두고, 시스템을 아는 정의는 여기에 둔다.
*/
bool Component_Is_Alive(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const void* pData)
{
    CComponent_System* pSystem = CComponent_System::GetInstancePtr(); /* 종료 중 재생성 방지: GetInstance 대신 */
    return pSystem && pSystem->Is_Alive(eComType, hComponent, pData);
}
