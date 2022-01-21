#include "Sequence.h"

#include "TargetingUtility.h"
#include "Junction.h"
#include "Underground.h"
#include "SceneContext.h"
#include "Direction.h"
#include "Profiler.h"
#include "WorldMap.h"
#include <cassert>

using namespace cpp_conv;
using cpp_conv::SequenceIterator;

SequenceIterator::IterateNode::IterateNode(Conveyor* pCurrent, const Conveyor* pEnd, cpp_conv::WorldMap& map, bool bEndSentinel)
    : m_pCurrent(pCurrent)
    , m_pEnd(pEnd)
    , m_rMap(map)
    , m_bEndSentinel(bEndSentinel)
{
}

SequenceIterator::IterateNode& SequenceIterator::IterateNode::operator++()
{
    if (m_pCurrent == m_pEnd)
    {
        m_bEndSentinel = true;
    }
    else
    {
        m_pCurrent = m_rMap.GetEntity<Conveyor>(grid::GetForwardPosition(*m_pCurrent), EntityKind::Conveyor);
    }

    return *this;
}

SequenceIterator::IterateNode SequenceIterator::IterateNode::operator++(int)
{
    IterateNode tmp = *this; ++(*this);
    return tmp;
}

SequenceIterator::SequenceIterator(Sequence& sequence, cpp_conv::WorldMap& map)
    : m_sequence(sequence)
    , m_rMap(map)
{
}

SequenceIterator::IterateNode SequenceIterator::begin()
{
    return { const_cast<Conveyor*>(m_sequence.GetTailConveyor()), m_sequence.GetHeadConveyor(), m_rMap, false };
}

SequenceIterator::IterateNode SequenceIterator::end()
{
    return { const_cast<Conveyor*>(m_sequence.GetHeadConveyor()), m_sequence.GetHeadConveyor(), m_rMap, true };
}

Conveyor* cpp_conv::TraceHeadConveyor(cpp_conv::WorldMap& map, Conveyor& searchStart)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);
    while (true)
    {
        Vector3 forwardPosition = grid::GetForwardPosition(*pCurrentConveyor);
        Entity* pTargetConveyor = map.GetEntity<Conveyor>(forwardPosition, EntityKind::Conveyor);
        if (pTargetConveyor == nullptr)
        {
            break;
        }

        const Entity* vPotentialNeighbours[4];
        vPotentialNeighbours[(int)RelativeDirection::Backwards] = map.GetEntity<Conveyor>(grid::GetBackwardsPosition(*pTargetConveyor), EntityKind::Conveyor);
        vPotentialNeighbours[(int)RelativeDirection::Right] = map.GetEntity<Conveyor>(grid::GetRightPosition(*pTargetConveyor), EntityKind::Conveyor);
        vPotentialNeighbours[(int)RelativeDirection::Left] = map.GetEntity<Conveyor>(grid::GetLeftPosition(*pTargetConveyor), EntityKind::Conveyor);

        for (auto direction : directionPriority)
        {
            const Entity* pDirectionEntity = vPotentialNeighbours[(int)direction];
            if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
            {
                continue;
            }

            if (pDirectionEntity == pCurrentConveyor)
            {
                break;
            }

            if (grid::GetForwardPosition(*pDirectionEntity) == forwardPosition)
            {
                return pCurrentConveyor;
            }
        }

        if (pTargetConveyor == &searchStart)
        {
            break;
        }

        pCurrentConveyor = reinterpret_cast<Conveyor*>(pTargetConveyor);
    }

    return pCurrentConveyor;
}

const Conveyor* cpp_conv::TraceTailConveyor(cpp_conv::WorldMap& map, Conveyor& searchStart, Conveyor& head, std::vector<Conveyor*>& vOutConveyors)
{
    Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);

    assert(pCurrentConveyor != nullptr); // If it is null here, we are misusing this method.

    while (true)
    {
        vOutConveyors.push_back(pCurrentConveyor);

        //if (vOutConveyors.size() >= Sequence::c_uiMaxSequenceLength)
        {
            //break;
        }

        Entity* pTargetConveyor = cpp_conv::targeting_util::FindNextTailConveyor(map, *pCurrentConveyor);
        if (!pTargetConveyor || pTargetConveyor->m_eEntityKind != EntityKind::Conveyor || pTargetConveyor == &searchStart || pTargetConveyor == &head)
        {
            break;
        }

        pCurrentConveyor = reinterpret_cast<Conveyor*>(pTargetConveyor);
    }

    std::reverse(vOutConveyors.begin(), vOutConveyors.end());
    return pCurrentConveyor;
}

std::vector<Sequence> cpp_conv::InitializeSequences(cpp_conv::WorldMap& map, const std::vector<Conveyor*>& conveyors)
{
    std::vector<Sequence> vSequences;
    cpp_conveyor::vector_set<const Conveyor*> alreadyProcessedConveyors(conveyors.size());

    int iId = 0;
    for (auto& conveyor : conveyors)
    {
        if (alreadyProcessedConveyors.contains(conveyor))
        {
            continue;
        }
         
        iId++;
        std::vector<Conveyor*> vConveyors;
        Conveyor* pHeadConveyor = TraceHeadConveyor(map, *conveyor);
        const Conveyor* pTailConveyor = TraceTailConveyor(map, *pHeadConveyor, *pHeadConveyor, vConveyors);

        vSequences.emplace_back(pHeadConveyor, pTailConveyor, vConveyors, iId);
        Sequence& sequence = vSequences.back();
        for (auto& rNode : vConveyors)
        {
            rNode->m_pSequenceId = iId;
            alreadyProcessedConveyors.insert(rNode);
        }
    }

    return vSequences;
}

bool cpp_conv::IsCircular(const cpp_conv::WorldMap& map, std::vector<Sequence>& sequences, Sequence* pStartSequence)
{
    Sequence* pCurrentSequence = pStartSequence;
    int checkedDepth = 0;
    while (true)
    {
        checkedDepth++;
        if (checkedDepth > c_maxCircularCheckDepth)
        {
            return false;
        }

        const Conveyor* pHead = pCurrentSequence->GetHeadConveyor();
        const Entity* pForwardEntity = map.GetEntity(grid::GetForwardPosition(*pHead));
        if (pForwardEntity->m_eEntityKind != EntityKind::Conveyor)
        {
            return false;
        }

        Sequence* pNextSequence = nullptr;
        for (auto& sequence : sequences)
        {
            if (sequence.GetTailConveyor() == pForwardEntity)
            {
                pNextSequence = &sequence;
                break;
            }
        }

        if (pNextSequence == nullptr)
        {
            return false;
        }

        if (pNextSequence == pStartSequence)
        {
            return true;
        }
    }

    return false;
}

std::tuple<int, Direction> cpp_conv::GetInnerMostCornerChannel(const cpp_conv::WorldMap& map, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    const Entity* pBackConverter = cpp_conv::targeting_util::FindNextTailConveyor(map, rConveyor);
    if (pBackConverter == nullptr || pBackConverter->GetDirection() == rConveyor.m_direction)
    {
        return std::make_tuple(-1, Direction::Up);
    }
     
    Direction selfDirection = rConveyor.m_direction;
    Direction backDirection = pBackConverter->GetDirection();
    while (selfDirection != Direction::Up)
    {
        selfDirection = cpp_conv::direction::Rotate90DegreeClockwise(selfDirection);
        backDirection = cpp_conv::direction::Rotate90DegreeClockwise(backDirection);
    }

    return std::make_tuple(backDirection == Direction::Right ? 1 : 0, pBackConverter->GetDirection());
}

void Sequence::MoveItemToForwardsNode(cpp_conv::SceneContext& kContext, cpp_conv::Conveyor* pNode, cpp_conv::Entity* pForwardEntity, cpp_conv::Conveyor::Channel& rChannel, ItemInstance& frontMostItem)
{
    if (pNode == GetHeadConveyor())
    {
        if (pForwardEntity && pForwardEntity->SupportsInsertion() && pForwardEntity->TryInsert(kContext, *pNode, frontMostItem.m_Item, rChannel.m_ChannelLane))
        {
            frontMostItem = ItemInstance::Empty();
        }
    }
    else
    {
        cpp_conv::Conveyor* pForwardNode = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);
        cpp_conv::Conveyor::Channel& forwardChannel = pForwardNode->m_pChannels[rChannel.m_ChannelLane];
        ItemInstance& forwardTargetItem = forwardChannel.m_pSlots[0].m_Item;
        ItemInstance& forwardPendingItem = forwardChannel.m_pPendingItems[0];
        if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
        {
            pForwardNode->AddItemToSlot(
                kContext.m_rMap,
                &forwardChannel,
                0,
                frontMostItem.m_Item,
                *pNode,
                rChannel.m_ChannelLane,
                rChannel.m_LaneLength - 1);
            frontMostItem = ItemInstance::Empty();
        }
    }
}

void Sequence::MoveLanesForwards(cpp_conv::SceneContext& kContext, cpp_conv::Conveyor* pNode, cpp_conv::Conveyor::Channel& rChannel)
{
    for (int iChannelSlot = rChannel.m_LaneLength - 2; iChannelSlot >= 0; iChannelSlot--)
    {
        ItemInstance& currentItem = rChannel.m_pSlots[iChannelSlot].m_Item;
        if (currentItem.m_CurrentTick < currentItem.m_TargetTick)
        {
            continue;
        }

        ItemInstance& forwardTargetItem = rChannel.m_pSlots[iChannelSlot + 1].m_Item;
        ItemInstance& forwardPendingItem = rChannel.m_pSlots[iChannelSlot + 1].m_Item;

        if (!currentItem.IsEmpty() && forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
        {
            pNode->AddItemToSlot(kContext.m_rMap, &rChannel, iChannelSlot + 1, currentItem.m_Item, *pNode, rChannel.m_ChannelLane, iChannelSlot);
            currentItem = ItemInstance::Empty();
        }
    }
}

__declspec(noinline) void cpp_conv::Sequence::TickNode(cpp_conv::SceneContext& kContext, cpp_conv::Conveyor* pNode, cpp_conv::Entity* pForwardEntity)
{
    if (!pNode->m_bHasWork)
    {
        return;
    }

    bool bIsCornerConveyor = pNode->IsCorner();
    for (cpp_conv::Conveyor::Channel& rChannel : pNode->m_pChannels)
    {
        ItemInstance& frontMostItem = rChannel.m_pSlots[rChannel.m_LaneLength - 1].m_Item;
        if (!frontMostItem.IsEmpty() && frontMostItem.m_CurrentTick >= frontMostItem.m_TargetTick)
        {
            MoveItemToForwardsNode(kContext, pNode, pForwardEntity, rChannel, frontMostItem);
        }

        MoveLanesForwards(kContext, pNode, rChannel);        
    }
}

void cpp_conv::Sequence::Tick(SceneContext& kContext)
{
    for(int i = 0; i < m_pNodes.size(); i++)
    {
        cpp_conv::Conveyor* pNode = m_pNodes[i];
        cpp_conv::Entity* pForwardEntity = nullptr;
        if (i < m_pNodes.size() - 1)
        {
            pForwardEntity = m_pNodes[i + 1];
        }
        else
        {
            pForwardEntity = kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(*pNode));
        }

        TickNode(kContext, pNode, pForwardEntity);
    }
}
