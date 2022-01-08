#pragma once

#include <cstdint>
#include <string>
#include "DataId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class FactoryDefinition;
}

namespace cpp_conv::resources
{
    const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> getFactoryDefinition(cpp_conv::FactoryId id);
}
