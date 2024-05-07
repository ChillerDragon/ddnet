#include <base/system.h>
#include <engine/keys.h>

#include "editor.h"
#include "font_typer.h"

void CFontTyper::Init(CEditor *pEditor)
{
	CEditorComponent::Init(pEditor);

	m_CursorTextTexture = pEditor->Graphics()->LoadTexture("editor/cursor_text.png", IStorage::TYPE_ALL, 0);
}

void CFontTyper::SetTile(int X, int Y, unsigned char Index, const std::shared_ptr<CLayerTiles> &pLayer)
{
	CTile Tile = {
		Index, // index
		0, // flags
		0, // skip
		0, // reserved
	};
	pLayer->SetTile(X, Y, Tile);
}

bool CFontTyper::OnInput(const IInput::CEvent &Event)
{
	if(!IsActive())
	{
		if(Event.m_Key == KEY_T && Input()->ModifierIsPressed())
			TextModeOn();
		return false;
	}

	std::shared_ptr<CLayerTiles> pLayer = std::static_pointer_cast<CLayerTiles>(Editor()->GetSelectedLayerType(0, LAYERTYPE_TILES));
	if(!pLayer)
		return false;

	// only handle key down and not also key up
	if(!(Event.m_Flags & IInput::FLAG_PRESS))
		return false;

	// letters
	if(Event.m_Key > 3 && Event.m_Key < 30)
	{
		SetTile(m_TextIndexX++, m_TextIndexY, Event.m_Key - 4 + pLayer->m_LetterOffset, pLayer);
		m_TextLineLen++;
	}
	// numbers
	if(Event.m_Key >= 30 && Event.m_Key < 40)
	{
		SetTile(m_TextIndexX++, m_TextIndexY, Event.m_Key - 30 + pLayer->m_NumberOffset, pLayer);
		m_TextLineLen++;
	}
	// deletion
	if(Event.m_Key == KEY_BACKSPACE)
	{
		SetTile(--m_TextIndexX, m_TextIndexY, 0, pLayer);
		m_TextLineLen--;
	}
	// space
	if(Event.m_Key == KEY_SPACE)
	{
		SetTile(m_TextIndexX++, m_TextIndexY, 0, pLayer);
		m_TextLineLen++;
	}
	// newline
	if(Event.m_Key == KEY_RETURN)
	{
		m_TextIndexY++;
		m_TextIndexX -= m_TextLineLen;
		m_TextLineLen = 0;
	}
	// arrow key navigation
	if(Event.m_Key == KEY_LEFT)
	{
		m_TextIndexX--;
		m_TextLineLen--;
		if(Editor()->Input()->KeyIsPressed(KEY_LCTRL))
		{
			while(pLayer->GetTile(m_TextIndexX, m_TextIndexY).m_Index)
			{
				if(m_TextIndexX < 1 || m_TextIndexX > pLayer->m_Width - 2)
					break;
				if(Event.m_Key != KEY_LEFT)
					break;
				m_TextIndexX--;
				m_TextLineLen--;
			}
		}
	}
	if(Event.m_Key == KEY_RIGHT)
	{
		m_TextIndexX++;
		m_TextLineLen++;
		if(Editor()->Input()->KeyIsPressed(KEY_LCTRL))
		{
			while(pLayer->GetTile(m_TextIndexX, m_TextIndexY).m_Index)
			{
				if(m_TextIndexX < 1 || m_TextIndexX > pLayer->m_Width - 2)
					break;
				if(Event.m_Key != KEY_RIGHT)
					break;
				m_TextIndexX++;
				m_TextLineLen++;
			}
		}
	}
	if(Event.m_Key == KEY_UP)
		m_TextIndexY--;
	if(Event.m_Key == KEY_DOWN)
		m_TextIndexY++;
	m_NextCursorBlink = time_get() + time_freq();
	m_DrawCursor = true;
	float Dist = distance(
		vec2(m_TextIndexX, m_TextIndexY),
		vec2((Editor()->MapView()->GetWorldOffset().x + Editor()->MapView()->GetEditorOffset().x) / 32, (Editor()->MapView()->GetWorldOffset().y + Editor()->MapView()->GetEditorOffset().y) / 32));
	Dist /= Editor()->MapView()->GetWorldZoom();
	if(Dist > 10.0f)
	{
		Editor()->MapView()->SetWorldOffset(vec2(
			m_TextIndexX * 32 - Editor()->MapView()->GetEditorOffset().x,
			m_TextIndexY * 32 - Editor()->MapView()->GetEditorOffset().y));
	}

	return false;
}

void CFontTyper::TextModeOn()
{
	SetCursor();
	Editor()->m_Dialog = -1; // hack to not show picker when pressing space
	m_Active = true;

	std::shared_ptr<CLayerTiles> pLayer = std::static_pointer_cast<CLayerTiles>(Editor()->GetSelectedLayerType(0, LAYERTYPE_TILES));
	if(!pLayer)
		return;

	// open tile picker
	// to select offset
	if(pLayer->m_LetterOffset == -1)
	{
		m_PickOffsetLetters = true;
		dbg_msg("font_typer", "Missing letter offset");
	}
	if(pLayer->m_NumberOffset == -1)
	{
		m_PickOffsetNumbers = true;
		dbg_msg("font_typer", "Missing number offset");
	}
}

void CFontTyper::TextModeOff()
{
	m_Active = false;
	if(Editor()->m_Dialog == -1)
		Editor()->m_Dialog = DIALOG_NONE;
}

void CFontTyper::SetCursor()
{
	float Wx = Ui()->MouseWorldX();
	float Wy = Ui()->MouseWorldY();
	m_TextIndexX = (int)(Wx / 32);
	m_TextIndexY = (int)(Wy / 32);
	m_TextLineLen = 0;
}

void CFontTyper::OnShowPicker(float WorldX, float WorldY)
{
	if(!PickOffsets())
		return;
	std::shared_ptr<CLayer> pLayer = Editor()->GetSelectedLayer(0);
	int Tile = (int)WorldX / 32 + (int)WorldY / 32 * 16;
	if(Tile < 0 || Tile > 255)
		return;

	static bool s_Released = true;
	if(!Ui()->MouseButton(0))
		s_Released = true;

	if(Ui()->MouseButton(0) && s_Released)
	{
		s_Released = false;
		dbg_msg("font_typer", "clicked on tile %d", Tile);
		if(pLayer->m_LetterOffset == -1)
		{
			pLayer->m_LetterOffset = Tile;
			m_PickOffsetLetters = false;
		}
		else if(pLayer->m_NumberOffset == -1)
		{
			pLayer->m_NumberOffset = Tile;
			m_PickOffsetNumbers = false;
		}
		else
		{
			dbg_msg("font_typer", "warning tried to set letter offset again");
			m_PickOffsetLetters = false;
			m_PickOffsetNumbers = false;
		}
	}
}

void CFontTyper::OnRender(CUIRect View)
{
	if(!IsActive())
		return;

	if(Ui()->ConsumeHotkey(CUi::HOTKEY_ESCAPE))
	{
		if(PickOffsets())
		{
			m_PickOffsetLetters = false;
			m_PickOffsetNumbers = false;
		}
		TextModeOff();
	}
	if(PickOffsetLetters())
		str_copy(Editor()->m_aTooltip, "Click on the letter A");
	else if(PickOffsetNumbers())
		str_copy(Editor()->m_aTooltip, "Click on the number 1");
	else
		str_copy(Editor()->m_aTooltip, "Type on your keyboard to insert letters. Press Escape to end text mode.");

	std::shared_ptr<CLayerTiles> pLayer = std::static_pointer_cast<CLayerTiles>(Editor()->GetSelectedLayerType(0, LAYERTYPE_TILES));
	if(!pLayer)
		return;

	// exit if selected layer changes
	if(m_pLastLayer && m_pLastLayer != pLayer)
	{
		dbg_msg("font_typer", "layer changed");
		TextModeOff();
		m_pLastLayer = pLayer;
		return;
	}
	// exit if dialog or edit box pops up
	if(Editor()->m_Dialog != -1 || CLineInput::GetActiveInput())
	{
		dbg_msg("font_typer", "dialog opened");
		TextModeOff();
		return;
	}
	m_pLastLayer = pLayer;
	m_TextIndexX = clamp(m_TextIndexX, 0, pLayer->m_Width - 1);
	m_TextIndexY = clamp(m_TextIndexY, 0, pLayer->m_Height - 1);
	if(time_get() > m_NextCursorBlink)
	{
		m_NextCursorBlink = time_get() + time_freq() / 1.5;
		m_DrawCursor ^= true;
	}
	if(m_DrawCursor)
	{
		std::shared_ptr<CLayerGroup> pGroup = Editor()->GetSelectedGroup();
		pGroup->MapScreen();
		Editor()->Graphics()->WrapClamp();
		Editor()->Graphics()->TextureSet(m_CursorTextTexture);
		Editor()->Graphics()->QuadsBegin();
		Editor()->Graphics()->SetColor(1, 1, 1, 1);
		IGraphics::CQuadItem QuadItem(m_TextIndexX * 32, m_TextIndexY * 32, 32.0f, 32.0f);
		Editor()->Graphics()->QuadsDrawTL(&QuadItem, 1);
		Editor()->Graphics()->QuadsEnd();
		Editor()->Graphics()->WrapNormal();
		Editor()->Graphics()->MapScreen(Editor()->Ui()->Screen()->x, Editor()->Ui()->Screen()->y, Editor()->Ui()->Screen()->w, Editor()->Ui()->Screen()->h);
	}
}
