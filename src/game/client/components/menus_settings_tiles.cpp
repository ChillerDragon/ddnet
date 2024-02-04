#include <base/system.h>

#include <engine/shared/config.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/ui_listbox.h>
#include <game/localization.h>

#include "menus.h"

#include <chrono>

using namespace FontIcons;
using namespace std::chrono_literals;

// void CMenus::RenderServerInfo(CUIRect MainView)

void CMenus::RenderSettingsTiles(CUIRect MainView)
{
	CUIRect Left, Right;
	MainView.VSplitMid(&Left, &Right, 5.0f);

	Left.Draw(ColorRGBA(1, 1, 1, 0.25f), IGraphics::CORNER_ALL, 10.0f);
	Right.Draw(ColorRGBA(1, 1, 1, 0.25f), IGraphics::CORNER_ALL, 10.0f);
}
