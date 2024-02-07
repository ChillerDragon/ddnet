#include <base/system.h>

#include <engine/shared/config.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/ui_listbox.h>
#include <game/localization.h>

#include <cstring>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "menus.h"

#include <chrono>

using namespace FontIcons;
using namespace std::chrono_literals;

// void CMenus::RenderServerInfo(CUIRect MainView)

typedef void (*plugin_ptr_t)(CUIRect &View);

void CMenus::RenderSettingsTiles(CUIRect MainView)
{
	// CUIRect Left, Right;
	// MainView.VSplitMid(&Left, &Right, 5.0f);

	// Left.Draw(ColorRGBA(1, 1, 1, 0.25f), IGraphics::CORNER_ALL, 10.0f);
	// Right.Draw(ColorRGBA(1, 1, 1, 0.25f), IGraphics::CORNER_ALL, 10.0f);

	char *error;
	void *handle = dlopen("../hotui/sample.so", RTLD_LAZY);
	if(!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		return;
	}

	dlerror(); /* Clear any existing error */
	plugin_ptr_t tick_ptr;
	tick_ptr = (plugin_ptr_t)dlsym(handle, "ui_tick");

	error = dlerror();
	if(error != NULL)
	{
		fprintf(stderr, "%s\n", error);
		return;
	}

	tick_ptr(MainView);

	dlclose(handle);
}
