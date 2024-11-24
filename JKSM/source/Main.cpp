#include "JKSM.hpp"
#include <3ds.h>

int main(void)
{
    JKSM::Initialize();
    while (JKSM::IsRunning() && aptMainLoop())
    {
        JKSM::Update();
        JKSM::Render();
    }
    JKSM::Exit();
    return 0;
}
