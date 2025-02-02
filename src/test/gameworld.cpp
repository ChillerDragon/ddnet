#include "test.h"
#include <gtest/gtest.h>

#include <base/system.h>
#include <game/server/gameworld.h>

TEST(GameWorld, basic)
{
	CGameWorld *pWorld = new CGameWorld();
	delete pWorld;
}
