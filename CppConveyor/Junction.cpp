﻿#include "Junction.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <algorithm> 
#include <array>
#include <random>
#include <chrono>

template <class RandomAccessIterator, class URNG>
void shuffle(RandomAccessIterator first, RandomAccessIterator last, URNG&& g)
{
	for (auto i = (last - first) - 1; i > 0; --i)
	{
		std::uniform_int_distribution<decltype(i)> d(0, i);
		swap(first[i], first[d(g)]);
	}
}

cpp_conv::Junction::Junction(int x, int y)
	: Entity(x, y, EntityKind::Junction)
	, m_pItem(nullptr)
	, m_uiTick(0)
{
}

void cpp_conv::Junction::Tick(const SceneContext& kContext)
{
	if (!m_pItem)
	{
		return;
	}

	m_uiTick++;
	std::array<std::tuple<cpp_conv::Entity*, Direction>, 4> arrDirectionEntities =
	{
		std::make_tuple(cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this, Direction::Up)), Direction::Up),
		std::make_tuple(cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this, Direction::Right)), Direction::Right),
		std::make_tuple(cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this, Direction::Down)), Direction::Down),
		std::make_tuple(cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this, Direction::Left)), Direction::Left),
	};

	std::default_random_engine engine(m_uiTick % 256);
	for (auto i = (arrDirectionEntities.end() - arrDirectionEntities.begin()) - 1; i > 0; --i)
	{
		std::uniform_int_distribution<decltype(i)> d(0, i);
		std::swap(arrDirectionEntities.begin()[i], arrDirectionEntities.begin()[d(engine)]);
	}

	for (auto entityDirectionPair : arrDirectionEntities)
	{
		cpp_conv::Entity* pEntity;
		Direction direction;

		std::tie(pEntity, direction) = entityDirectionPair;

		if (pEntity->m_eEntityKind != EntityKind::Conveyor)
		{
			continue;
		}

		Conveyor* pConveyor = reinterpret_cast<Conveyor*>(pEntity);
		if (pConveyor->m_direction != direction)
		{
			continue;
		}

		bool bFound = false;
		for (cpp_conv::Conveyor::Channel& rChannel : pConveyor->m_pChannels)
		{
			if (rChannel.m_pItems[0] || rChannel.m_pPendingItems[0])
			{
				continue;
			}

			rChannel.m_pPendingItems[0] = m_pItem;
			m_pItem = nullptr;
			bFound = true;
			break;
		}

		if (bFound)
		{
			break;
		}
	}
}

void cpp_conv::Junction::Draw(RenderContext& kContext) const
{
	cpp_conv::renderer::setPixel(kContext, L'↑', m_position.m_x * cpp_conv::renderer::c_gridScale + 1, m_position.m_y * cpp_conv::renderer::c_gridScale + 1, 1, true);
	cpp_conv::renderer::setPixel(kContext, L'→', m_position.m_x * cpp_conv::renderer::c_gridScale + 2, m_position.m_y * cpp_conv::renderer::c_gridScale + 1, 1, true);
	cpp_conv::renderer::setPixel(kContext, L'←', m_position.m_x * cpp_conv::renderer::c_gridScale + 1, m_position.m_y * cpp_conv::renderer::c_gridScale + 2, 1, true);
	cpp_conv::renderer::setPixel(kContext, L'↓', m_position.m_x * cpp_conv::renderer::c_gridScale + 2, m_position.m_y * cpp_conv::renderer::c_gridScale + 2, 1, true);
}

bool cpp_conv::Junction::AddItem(Item* pItem)
{
	if (m_pItem)
	{
		return false;
	}

	m_pItem = pItem;
	return true;
}
