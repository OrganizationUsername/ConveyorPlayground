#include "MapLoadHandler.h"
#include "ResourceManager.h"
#include "Map.h"
#include "Entity.h"
#include "Conveyor.h"
#include "Junction.h"
#include "Factory.h"
#include "Underground.h"
#include "Storage.h"
#include "ItemRegistry.h"
#include "FactoryRegistry.h"

#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "Inserter.h"

cpp_conv::resources::ResourceAsset* mapAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::istringstream ss(pStrData);
    std::string strLine;

    auto pMap = new cpp_conv::resources::Map();
    int iRow = 0;
    while (std::getline(ss, strLine))
    {
        for (int iCol = 0; iCol < strLine.size(); iCol++)
        {
            cpp_conv::Entity* pEntity = nullptr;
            switch (strLine[iCol])
            {
            case '>': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Right); break;
            case '<': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Left); break;
            case '^': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Down); break;
            case 'v': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Up); break;
            case 'I': pEntity = new cpp_conv::Inserter(iCol, iRow, Direction::Down, cpp_conv::InserterId::FromStringId("INSERTER_BASIC")); break;
            case 'U': pEntity = new cpp_conv::Inserter(iCol, iRow, Direction::Up, cpp_conv::InserterId::FromStringId("INSERTER_BASIC")); break;
            case 'T': pEntity = new cpp_conv::Inserter(iCol, iRow, Direction::Left, cpp_conv::InserterId::FromStringId("INSERTER_BASIC")); break;
            case 'Y': pEntity = new cpp_conv::Inserter(iCol, iRow, Direction::Right, cpp_conv::InserterId::FromStringId("INSERTER_BASIC")); break;
            case 'A': pEntity = new cpp_conv::Factory(iCol, iRow, Direction::Right, cpp_conv::FactoryId::FromStringId("FACTORY_COPPER_MINE")); break;
            case 'D': pEntity = new cpp_conv::Factory(iCol, iRow, Direction::Left, cpp_conv::FactoryId::FromStringId("FACTORY_COPPER_MINE")); break;
            case 'F': pEntity = new cpp_conv::Factory(iCol, iRow, Direction::Down, cpp_conv::FactoryId::FromStringId("FACTORY_COPPER_MINE")); break;
            case 'G': pEntity = new cpp_conv::Factory(iCol, iRow, Direction::Up, cpp_conv::FactoryId::FromStringId("FACTORY_COPPER_MINE")); break;
            case 'C': pEntity = new cpp_conv::Factory(iCol, iRow, Direction::Right, cpp_conv::FactoryId::FromStringId("FACTORY_COPPER_SMELTER")); break;
            case 'J': pEntity = new cpp_conv::Junction(iCol, iRow); break;
            case 'S': pEntity = new cpp_conv::Storage(iCol, iRow, 16, 256); break;
            case 'u': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Down); break;
            case 'y': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Up); break;
            case 'i': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Left); break;
            case 'o': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Right); break;
            } 

            if (pEntity)
            {
                if (pEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    pMap->GetConveyors().push_back(reinterpret_cast<cpp_conv::Conveyor*>(pEntity));
                }
                else
                {
                    pMap->GetOtherEntities().push_back(pEntity);
                }
            }
        }

        iRow++;
    }

    return pMap;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::resources::Map, mapAssetHandler);
