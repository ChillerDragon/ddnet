/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_NULL_H
#define GAME_CLIENT_UI_NULL_H

#include <base/system.h>
#include <engine/textrender.h>
#include <string>
#include <vector>

class CUIRect;
class CUIElement
{
public:
	struct SUIElementRect;
};

class CUINull
{
public:
	void Init(class IGraphics *pGraphics, class ITextRender *pTextRender) {}
	class IGraphics *Graphics() const { return NULL; }
	class ITextRender *TextRender() const { return NULL; }

	CUINull() {}
	~CUINull() {}

	void ResetUIElement(CUIElement &UIElement) {}

	CUIElement *GetNewUIElement(int RequestedRectCount) { return NULL; }

	void AddUIElement(CUIElement *pElement) {}
	void OnElementsReset() {}
	void OnWindowResize() {}
	void OnLanguageChange() {}

	int Update(float mx, float my, float Mwx, float Mwy, int m_Buttons) { return 0; }

	float MouseDeltaX() const { return 1.0f; }
	float MouseDeltaY() const { return 1.0f; }
	float MouseX() const { return 1.0f; }
	float MouseY() const { return 1.0f; }
	float MouseWorldX() const { return 1.0f; }
	float MouseWorldY() const { return 1.0f; }
	int MouseButton(int Index) const { return 0; }
	int MouseButtonClicked(int Index) const { return 0; }
	int MouseButtonReleased(int Index) const { return 0; }

	void SetHotItem(const void *pID) {}
	void SetActiveItem(const void *pID) {}
	void ClearLastActiveItem() { }
	const void *HotItem() const { return NULL; }
	const void *NextHotItem() const { return NULL; }
	const void *ActiveItem() const { return NULL; }
	const void *LastActiveItem() const { return NULL; }

	bool MouseInside(const CUIRect *pRect) const { return false; }
	void ConvertMouseMove(float *x, float *y) const { }

	float ButtonColorMulActive() { return 0.5f; }
	float ButtonColorMulHot() { return 1.5f; }
	float ButtonColorMulDefault() { return 1.0f; }
	float ButtonColorMul(const void *pID) { return 1.0f; }

	CUIRect *Screen() { return NULL; }
	void MapScreen() {}
	float PixelSize() { return 1.0f; }
	void ClipEnable(const CUIRect *pRect) {}
	void ClipDisable() {}

	void SetScale(float s) {}
	float Scale() const { return 1.0f; }

	int DoButtonLogic(const void *pID, int Checked, const CUIRect *pRect) { return 0; }
	int DoButtonLogic(const void *pID, const char *pText, int Checked, const CUIRect *pRect) { return 0; }
	int DoPickerLogic(const void *pID, const CUIRect *pRect, float *pX, float *pY) { return 0; }

	float DoTextLabel(float x, float y, float w, float h, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, bool StopAtEnd = false, class CTextCursor *pSelCursor = NULL) { return 1.0f; }
	void DoLabel(const CUIRect *pRect, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, class CTextCursor *pSelCursor = NULL) {}
	void DoLabelScaled(const CUIRect *pRect, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1) {}

	void DoLabel(CUIElement::SUIElementRect &RectEl, const CUIRect *pRect, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, bool StopAtEnd = false, int StrLen = -1, class CTextCursor *pReadCursor = NULL) {}
	void DoLabelStreamed(CUIElement::SUIElementRect &RectEl, float x, float y, float w, float h, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, bool StopAtEnd = false, int StrLen = -1, class CTextCursor *pReadCursor = NULL) {}
	void DoLabelStreamed(CUIElement::SUIElementRect &RectEl, const CUIRect *pRect, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, bool StopAtEnd = false, int StrLen = -1, class CTextCursor *pReadCursor = NULL) {}
};

#endif
