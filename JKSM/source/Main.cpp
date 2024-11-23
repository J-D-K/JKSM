#include "JKSM.hpp"

int main(void)
{
    JKSM::Initialize();
    while (JKSM::IsRunning())
    {
        JKSM::Update();
        JKSM::Render();
    }
    JKSM::Exit();
    return 0;
}
