#include "ItemRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "ItemDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"
#include "Profiler.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition>> g_vItems;

namespace
{
    void loadItems()
    {
        for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szItemsPaths) / sizeof(std::filesystem::path); i++)
        {
            const RegistryId asset = { i, 2 };
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::ItemDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vItems.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* itemAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));
    std::istringstream ss(copy);

    std::string id;
    std::string name;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        if (token.back() == '\r')
        {
            token.erase(token.size() - 1);
        }

        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        }

        idx++;
    }

    return new cpp_conv::ItemDefinition(cpp_conv::ItemId::FromStringId(id), rData.m_registryId, name);
}

cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> cpp_conv::resources::getItemDefinition(const ItemId id)
{
    PROFILE_FUNC();
    for (auto item : g_vItems)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::ItemDefinition, itemAssetHandler);
REGISTER_LOAD_HANDLER(loadItems);
