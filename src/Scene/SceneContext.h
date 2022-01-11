#pragma once

#include <vector>
#include <chrono>
#include "EntityGrid.h"
#include "Sequence.h"

namespace cpp_conv { class WorldMap; }

namespace cpp_conv
{
    struct SceneContext
    {
        Position m_player;
        cpp_conv::WorldMap& m_rMap;
        std::vector<cpp_conv::Sequence>& m_sequences;

        struct
        {
            std::chrono::steady_clock::time_point m_lastPlayerMove;
        } m_debounce;
    };
}
