#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)

class CRope final
{
public :
    enum class ROPE_STATE : uint8_t { IDLE, EXTENDING, ANCHORED, RETURNING };

public :
    void Render();
    void Update();

private :
    void Process_Extending();
    void Process_Anchored();
    void Process_Retuning();



public :
    static std::unique_ptr<CRope> Create();

};

NS_END
