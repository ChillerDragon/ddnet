#ifndef GAME_EDITOR_FONT_TYPER_H
#define GAME_EDITOR_FONT_TYPER_H

#include <engine/graphics.h>
#include <game/editor/mapitems/layer_tiles.h>

#include <memory>

#include "component.h"

class CFontTyper : public CEditorComponent
{
	int m_TextIndexX = 0;
	int m_TextIndexY = 0;
	int m_TextLineLen = 0;
	bool m_DrawCursor = false;
	bool m_Active = false;
	bool m_PickOffsetLetters = false;
	bool m_PickOffsetNumbers = false;
	int64_t m_NextCursorBlink = 0;
	IGraphics::CTextureHandle m_CursorTextTexture;
	std::shared_ptr<class CLayer> m_pLastLayer;

public:
	void OnRender(CUIRect View) override;
	bool OnInput(const IInput::CEvent &Event) override;
	void Init(CEditor *pEditor) override;

	void SetCursor();
	void TextModeOff();
	void TextModeOn();
	void SetTile(int X, int Y, unsigned char Index, const std::shared_ptr<CLayerTiles> &pLayer);

	bool IsActive() const { return m_Active; }
	bool PickOffsetLetters() const { return m_PickOffsetLetters; }
	bool PickOffsetNumbers() const { return m_PickOffsetNumbers; }
	bool PickOffsets() const { return PickOffsetLetters() || PickOffsetNumbers(); }

	// TODO: move this to CEditorComponent and also use it for the explanations
	void OnShowPicker(float WorldX, float WorldY);
};

#endif
