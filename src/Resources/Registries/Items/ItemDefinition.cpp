#include "ItemDefinition.h"
#include "AssetPtr.h"
#include "ResourceManager.h"

cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> cpp_conv::ItemDefinition::GetTile() const
{
    if (m_pTile)
    {
        return m_pTile;
    }

    // TODO[CJones] Need to add some multi-threaded protection here if I'm going to abuse mutable like that.
    m_pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>({ m_registryId.m_index, m_registryId.m_category + 1 });
    return m_pTile;
}
