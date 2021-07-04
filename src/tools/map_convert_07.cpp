/* (c) DDNet developers. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.  */

#include <base/math.h>
#include <base/system.h>
#include <engine/graphics.h>
#include <engine/shared/datafile.h>
#include <engine/storage.h>
#include <game/gamecore.h>
#include <game/mapitems.h>

#include <game/mapitems_ex.h>
#include <game/editor/editor.h>
#include <game/editor/auto_map.h>

#include <game/mapitems.h>
#include <game/mapitems.cpp>

/*
	Usage: map_convert_07 <source map filepath> <dest map filepath>
*/

static IStorage *gs_pStorage;


template<typename T>
static int MakeVersion(int i, const T &v)
{
	return (i << 16) + sizeof(T);
}

CEditorSound::~CEditorSound()
{
	m_pEditor->Sound()->UnloadSample(m_SoundID);
	if(m_pData)
	{
		free(m_pData);
		m_pData = 0x0;
	}
}

CEditorImage::~CEditorImage()
{
	m_pEditor->Graphics()->UnloadTexture(m_Texture);
	if(m_pData)
	{
		free(m_pData);
		m_pData = 0;
	}
}

CLayerGroup::CLayerGroup()
{
	m_aName[0] = 0;
	m_Visible = true;
	m_Collapse = false;
	m_GameGroup = false;
	m_OffsetX = 0;
	m_OffsetY = 0;
	m_ParallaxX = 100;
	m_ParallaxY = 100;

	m_UseClipping = 0;
	m_ClipX = 0;
	m_ClipY = 0;
	m_ClipW = 0;
	m_ClipH = 0;
}

CLayerGroup::~CLayerGroup()
{
	Clear();
}

// layer_sounds.cpp

static const float s_SourceVisualSize = 32.0f;

CLayerSounds::CLayerSounds()
{
	m_Type = LAYERTYPE_SOUNDS;
	m_aName[0] = '\0';
	m_Sound = -1;
}

CLayerSounds::~CLayerSounds()
{
}

void CLayerSounds::Render(bool Tileset)
{
}

CSoundSource *CLayerSounds::NewSource(int x, int y)
{
	m_pEditor->m_Map.m_Modified = true;

	CSoundSource *pSource = &m_lSources[m_lSources.add(CSoundSource())];

	pSource->m_Position.x = f2fx(x);
	pSource->m_Position.y = f2fx(y);

	pSource->m_Loop = 1;
	pSource->m_Pan = 1;
	pSource->m_TimeDelay = 0;

	pSource->m_PosEnv = -1;
	pSource->m_PosEnvOffset = 0;
	pSource->m_SoundEnv = -1;
	pSource->m_SoundEnvOffset = 0;

	pSource->m_Falloff = 80;

	pSource->m_Shape.m_Type = CSoundShape::SHAPE_CIRCLE;
	pSource->m_Shape.m_Circle.m_Radius = 1500;

	/*
	pSource->m_Shape.m_Type = CSoundShape::SHAPE_RECTANGLE;
	pSource->m_Shape.m_Rectangle.m_Width = f2fx(1500.0f);
	pSource->m_Shape.m_Rectangle.m_Height = f2fx(1000.0f);
	*/

	return pSource;
}

void CLayerSounds::BrushSelecting(CUIRect Rect)
{
}

int CLayerSounds::BrushGrab(CLayerGroup *pBrush, CUIRect Rect)
{
	// create new layer
	CLayerSounds *pGrabbed = new CLayerSounds();
	pGrabbed->m_pEditor = m_pEditor;
	pGrabbed->m_Sound = m_Sound;
	pBrush->AddLayer(pGrabbed);

	for(int i = 0; i < m_lSources.size(); i++)
	{
		CSoundSource *pSource = &m_lSources[i];
		float px = fx2f(pSource->m_Position.x);
		float py = fx2f(pSource->m_Position.y);

		if(px > Rect.x && px < Rect.x + Rect.w && py > Rect.y && py < Rect.y + Rect.h)
		{
			CSoundSource n;
			n = *pSource;

			n.m_Position.x -= f2fx(Rect.x);
			n.m_Position.y -= f2fx(Rect.y);

			pGrabbed->m_lSources.add(n);
		}
	}

	return pGrabbed->m_lSources.size() ? 1 : 0;
}

void CLayerSounds::BrushPlace(CLayer *pBrush, float wx, float wy)
{
	CLayerSounds *l = (CLayerSounds *)pBrush;
	for(int i = 0; i < l->m_lSources.size(); i++)
	{
		CSoundSource n = l->m_lSources[i];

		n.m_Position.x += f2fx(wx);
		n.m_Position.y += f2fx(wy);

		m_lSources.add(n);
	}
	m_pEditor->m_Map.m_Modified = true;
}

int CLayerSounds::RenderProperties(CUIRect *pToolBox)
{
	return 0;
}

void CLayerSounds::ModifySoundIndex(INDEX_MODIFY_FUNC Func)
{
	Func(&m_Sound);
}

void CLayerSounds::ModifyEnvelopeIndex(INDEX_MODIFY_FUNC Func)
{
	for(int i = 0; i < m_lSources.size(); i++)
	{
		Func(&m_lSources[i].m_SoundEnv);
		Func(&m_lSources[i].m_PosEnv);
	}
}

// layer_quads.cpp

CLayerQuads::CLayerQuads()
{
	m_Type = LAYERTYPE_QUADS;
	m_aName[0] = '\0';
	m_Image = -1;
}

CLayerQuads::~CLayerQuads()
{
}

void CLayerQuads::Render(bool QuadPicker)
{
}

CQuad *CLayerQuads::NewQuad(int x, int y, int Width, int Height)
{
	m_pEditor->m_Map.m_Modified = true;

	CQuad *q = &m_lQuads[m_lQuads.add(CQuad())];

	q->m_PosEnv = -1;
	q->m_ColorEnv = -1;
	q->m_PosEnvOffset = 0;
	q->m_ColorEnvOffset = 0;

	Width /= 2;
	Height /= 2;
	q->m_aPoints[0].x = i2fx(x - Width);
	q->m_aPoints[0].y = i2fx(y - Height);
	q->m_aPoints[1].x = i2fx(x + Width);
	q->m_aPoints[1].y = i2fx(y - Height);
	q->m_aPoints[2].x = i2fx(x - Width);
	q->m_aPoints[2].y = i2fx(y + Height);
	q->m_aPoints[3].x = i2fx(x + Width);
	q->m_aPoints[3].y = i2fx(y + Height);

	q->m_aPoints[4].x = i2fx(x); // pivot
	q->m_aPoints[4].y = i2fx(y);

	q->m_aTexcoords[0].x = i2fx(0);
	q->m_aTexcoords[0].y = i2fx(0);

	q->m_aTexcoords[1].x = i2fx(1);
	q->m_aTexcoords[1].y = i2fx(0);

	q->m_aTexcoords[2].x = i2fx(0);
	q->m_aTexcoords[2].y = i2fx(1);

	q->m_aTexcoords[3].x = i2fx(1);
	q->m_aTexcoords[3].y = i2fx(1);

	q->m_aColors[0].r = 255;
	q->m_aColors[0].g = 255;
	q->m_aColors[0].b = 255;
	q->m_aColors[0].a = 255;
	q->m_aColors[1].r = 255;
	q->m_aColors[1].g = 255;
	q->m_aColors[1].b = 255;
	q->m_aColors[1].a = 255;
	q->m_aColors[2].r = 255;
	q->m_aColors[2].g = 255;
	q->m_aColors[2].b = 255;
	q->m_aColors[2].a = 255;
	q->m_aColors[3].r = 255;
	q->m_aColors[3].g = 255;
	q->m_aColors[3].b = 255;
	q->m_aColors[3].a = 255;

	return q;
}

void CLayerQuads::BrushSelecting(CUIRect Rect)
{
}

int CLayerQuads::BrushGrab(CLayerGroup *pBrush, CUIRect Rect)
{
	// create new layers
	CLayerQuads *pGrabbed = new CLayerQuads();
	pGrabbed->m_pEditor = m_pEditor;
	pGrabbed->m_Image = m_Image;
	pBrush->AddLayer(pGrabbed);

	//dbg_msg("", "%f %f %f %f", rect.x, rect.y, rect.w, rect.h);
	for(int i = 0; i < m_lQuads.size(); i++)
	{
		CQuad *q = &m_lQuads[i];
		float px = fx2f(q->m_aPoints[4].x);
		float py = fx2f(q->m_aPoints[4].y);

		if(px > Rect.x && px < Rect.x + Rect.w && py > Rect.y && py < Rect.y + Rect.h)
		{
			CQuad n;
			n = *q;

			for(auto &Point : n.m_aPoints)
			{
				Point.x -= f2fx(Rect.x);
				Point.y -= f2fx(Rect.y);
			}

			pGrabbed->m_lQuads.add(n);
		}
	}

	return pGrabbed->m_lQuads.size() ? 1 : 0;
}

void CLayerQuads::BrushPlace(CLayer *pBrush, float wx, float wy)
{
	CLayerQuads *l = (CLayerQuads *)pBrush;
	for(int i = 0; i < l->m_lQuads.size(); i++)
	{
		CQuad n = l->m_lQuads[i];

		for(auto &Point : n.m_aPoints)
		{
			Point.x += f2fx(wx);
			Point.y += f2fx(wy);
		}

		m_lQuads.add(n);
	}
	m_pEditor->m_Map.m_Modified = true;
}

void Swap(CPoint &a, CPoint &b)
{
	CPoint Tmp;
	Tmp.x = a.x;
	Tmp.y = a.y;

	a.x = b.x;
	a.y = b.y;

	b.x = Tmp.x;
	b.y = Tmp.y;
}

void CLayerQuads::BrushFlipX()
{
	for(int i = 0; i < m_lQuads.size(); i++)
	{
		CQuad *q = &m_lQuads[i];

		Swap(q->m_aPoints[0], q->m_aPoints[1]);
		Swap(q->m_aPoints[2], q->m_aPoints[3]);
	}
	m_pEditor->m_Map.m_Modified = true;
}

void CLayerQuads::BrushFlipY()
{
	for(int i = 0; i < m_lQuads.size(); i++)
	{
		CQuad *q = &m_lQuads[i];

		Swap(q->m_aPoints[0], q->m_aPoints[2]);
		Swap(q->m_aPoints[1], q->m_aPoints[3]);
	}
	m_pEditor->m_Map.m_Modified = true;
}

void Rotate(vec2 *pCenter, vec2 *pPoint, float Rotation)
{
	float x = pPoint->x - pCenter->x;
	float y = pPoint->y - pCenter->y;
	pPoint->x = x * cosf(Rotation) - y * sinf(Rotation) + pCenter->x;
	pPoint->y = x * sinf(Rotation) + y * cosf(Rotation) + pCenter->y;
}

void CLayerQuads::BrushRotate(float Amount)
{
	vec2 Center;
	GetSize(&Center.x, &Center.y);
	Center.x /= 2;
	Center.y /= 2;

	for(int i = 0; i < m_lQuads.size(); i++)
	{
		CQuad *q = &m_lQuads[i];

		for(auto &Point : q->m_aPoints)
		{
			vec2 Pos(fx2f(Point.x), fx2f(Point.y));
			Rotate(&Center, &Pos, Amount);
			Point.x = f2fx(Pos.x);
			Point.y = f2fx(Pos.y);
		}
	}
}

void CLayerQuads::GetSize(float *w, float *h)
{
	*w = 0;
	*h = 0;

	for(int i = 0; i < m_lQuads.size(); i++)
	{
		for(auto &Point : m_lQuads[i].m_aPoints)
		{
			*w = maximum(*w, fx2f(Point.x));
			*h = maximum(*h, fx2f(Point.y));
		}
	}
}

int CLayerQuads::RenderProperties(CUIRect *pToolBox)
{
	return 0;
}

void CLayerQuads::ModifyImageIndex(INDEX_MODIFY_FUNC Func)
{
	Func(&m_Image);
}

void CLayerQuads::ModifyEnvelopeIndex(INDEX_MODIFY_FUNC Func)
{
	for(int i = 0; i < m_lQuads.size(); i++)
	{
		Func(&m_lQuads[i].m_PosEnv);
		Func(&m_lQuads[i].m_ColorEnv);
	}
}

// layer_tiles.cpp

CLayerTiles::CLayerTiles(int w, int h)
{
	m_Type = LAYERTYPE_TILES;
	m_aName[0] = '\0';
	m_Width = w;
	m_Height = h;
	m_Image = -1;
	m_Game = 0;
	m_Color.r = 255;
	m_Color.g = 255;
	m_Color.b = 255;
	m_Color.a = 255;
	m_ColorEnv = -1;
	m_ColorEnvOffset = 0;

	m_Tele = 0;
	m_Speedup = 0;
	m_Front = 0;
	m_Switch = 0;
	m_Tune = 0;
	m_AutoMapperConfig = -1;
	m_Seed = 0;
	m_AutoAutoMap = false;

	m_pTiles = new CTile[m_Width * m_Height];
	mem_zero(m_pTiles, (size_t)m_Width * m_Height * sizeof(CTile));
}

CLayerTiles::~CLayerTiles()
{
	delete[] m_pTiles;
}

CTile CLayerTiles::GetTile(int x, int y)
{
	return m_pTiles[y * m_Width + x];
}

void CLayerTiles::SetTile(int x, int y, CTile tile)
{
	m_pTiles[y * m_Width + x] = tile;
}

void CLayerTiles::PrepareForSave()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
			m_pTiles[y * m_Width + x].m_Flags &= TILEFLAG_VFLIP | TILEFLAG_HFLIP | TILEFLAG_ROTATE;

	if(m_Image != -1 && m_Color.a == 255)
	{
		for(int y = 0; y < m_Height; y++)
			for(int x = 0; x < m_Width; x++)
				m_pTiles[y * m_Width + x].m_Flags |= m_pEditor->m_Map.m_lImages[m_Image]->m_aTileFlags[m_pTiles[y * m_Width + x].m_Index];
	}
}

void CLayerTiles::MakePalette()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
			m_pTiles[y * m_Width + x].m_Index = y * 16 + x;
}

void CLayerTiles::Render(bool Tileset)
{
}

int CLayerTiles::ConvertX(float x) const { return (int)(x / 32.0f); }
int CLayerTiles::ConvertY(float y) const { return (int)(y / 32.0f); }

void CLayerTiles::Convert(CUIRect Rect, RECTi *pOut)
{
	pOut->x = ConvertX(Rect.x);
	pOut->y = ConvertY(Rect.y);
	pOut->w = ConvertX(Rect.x + Rect.w + 31) - pOut->x;
	pOut->h = ConvertY(Rect.y + Rect.h + 31) - pOut->y;
}

void CLayerTiles::Snap(CUIRect *pRect)
{
	RECTi Out;
	Convert(*pRect, &Out);
	pRect->x = Out.x * 32.0f;
	pRect->y = Out.y * 32.0f;
	pRect->w = Out.w * 32.0f;
	pRect->h = Out.h * 32.0f;
}

void CLayerTiles::Clamp(RECTi *pRect)
{
	if(pRect->x < 0)
	{
		pRect->w += pRect->x;
		pRect->x = 0;
	}

	if(pRect->y < 0)
	{
		pRect->h += pRect->y;
		pRect->y = 0;
	}

	if(pRect->x + pRect->w > m_Width)
		pRect->w = m_Width - pRect->x;

	if(pRect->y + pRect->h > m_Height)
		pRect->h = m_Height - pRect->y;

	if(pRect->h < 0)
		pRect->h = 0;
	if(pRect->w < 0)
		pRect->w = 0;
}

bool CLayerTiles::IsEmpty(CLayerTiles *pLayer)
{
	for(int y = 0; y < pLayer->m_Height; y++)
		for(int x = 0; x < pLayer->m_Width; x++)
		{
			int Index = pLayer->GetTile(x, y).m_Index;
			if(Index)
			{
				if(pLayer->m_Game)
				{
					if(m_pEditor->m_AllowPlaceUnusedTiles || IsValidGameTile(Index))
						return false;
				}
				else if(pLayer->m_Front)
				{
					if(m_pEditor->m_AllowPlaceUnusedTiles || IsValidFrontTile(Index))
						return false;
				}
				else
					return false;
			}
		}

	return true;
}

void CLayerTiles::BrushSelecting(CUIRect Rect)
{
}

int CLayerTiles::BrushGrab(CLayerGroup *pBrush, CUIRect Rect)
{
	return 1;
}

void CLayerTiles::FillSelection(bool Empty, CLayer *pBrush, CUIRect Rect)
{
	if(m_Readonly || (!Empty && pBrush->m_Type != LAYERTYPE_TILES))
		return;

	Snap(&Rect);

	int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);

	CLayerTiles *pLt = static_cast<CLayerTiles *>(pBrush);

	bool Destructive = m_pEditor->m_BrushDrawDestructive || Empty || IsEmpty(pLt);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			bool HasTile = GetTile(fx, fy).m_Index;
			if(!Empty && pLt->GetTile(x, y).m_Index == TILE_THROUGH_CUT)
			{
				if(m_Game && m_pEditor->m_Map.m_pFrontLayer)
				{
					HasTile = HasTile || m_pEditor->m_Map.m_pFrontLayer->GetTile(fx, fy).m_Index;
				}
				else if(m_Front)
				{
					HasTile = HasTile || m_pEditor->m_Map.m_pGameLayer->GetTile(fx, fy).m_Index;
				}
			}

			if(!Destructive && HasTile)
				continue;

			if(Empty)
				m_pTiles[fy * m_Width + fx].m_Index = 0;
			else
				SetTile(fx, fy, pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)]);
		}
	}
	FlagModified(sx, sy, w, h);
}

void CLayerTiles::BrushDraw(CLayer *pBrush, float wx, float wy)
{
	if(m_Readonly)
		return;

	//
	CLayerTiles *l = (CLayerTiles *)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);

	bool Destructive = m_pEditor->m_BrushDrawDestructive || IsEmpty(l);

	for(int y = 0; y < l->m_Height; y++)
		for(int x = 0; x < l->m_Width; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			bool HasTile = GetTile(fx, fy).m_Index;
			if(l->GetTile(x, y).m_Index == TILE_THROUGH_CUT)
			{
				if(m_Game && m_pEditor->m_Map.m_pFrontLayer)
				{
					HasTile = HasTile || m_pEditor->m_Map.m_pFrontLayer->GetTile(fx, fy).m_Index;
				}
				else if(m_Front)
				{
					HasTile = HasTile || m_pEditor->m_Map.m_pGameLayer->GetTile(fx, fy).m_Index;
				}
			}

			if(!Destructive && HasTile)
				continue;

			SetTile(fx, fy, l->GetTile(x, y));
		}

	FlagModified(sx, sy, l->m_Width, l->m_Height);
}

void CLayerTiles::BrushFlipX()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width / 2; x++)
		{
			CTile Tmp = m_pTiles[y * m_Width + x];
			m_pTiles[y * m_Width + x] = m_pTiles[y * m_Width + m_Width - 1 - x];
			m_pTiles[y * m_Width + m_Width - 1 - x] = Tmp;
		}

	if(m_Tele || m_Speedup || m_Tune)
		return;

	bool Rotate = !(m_Game || m_Front || m_Switch) || m_pEditor->m_AllowPlaceUnusedTiles;
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
			if(!Rotate && !IsRotatableTile(m_pTiles[y * m_Width + x].m_Index))
				m_pTiles[y * m_Width + x].m_Flags = 0;
			else
				m_pTiles[y * m_Width + x].m_Flags ^= m_pTiles[y * m_Width + x].m_Flags & TILEFLAG_ROTATE ? TILEFLAG_HFLIP : TILEFLAG_VFLIP;
}

void CLayerTiles::BrushFlipY()
{
	for(int y = 0; y < m_Height / 2; y++)
		for(int x = 0; x < m_Width; x++)
		{
			CTile Tmp = m_pTiles[y * m_Width + x];
			m_pTiles[y * m_Width + x] = m_pTiles[(m_Height - 1 - y) * m_Width + x];
			m_pTiles[(m_Height - 1 - y) * m_Width + x] = Tmp;
		}

	if(m_Tele || m_Speedup || m_Tune)
		return;

	bool Rotate = !(m_Game || m_Front || m_Switch) || m_pEditor->m_AllowPlaceUnusedTiles;
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
			if(!Rotate && !IsRotatableTile(m_pTiles[y * m_Width + x].m_Index))
				m_pTiles[y * m_Width + x].m_Flags = 0;
			else
				m_pTiles[y * m_Width + x].m_Flags ^= m_pTiles[y * m_Width + x].m_Flags & TILEFLAG_ROTATE ? TILEFLAG_VFLIP : TILEFLAG_HFLIP;
}

void CLayerTiles::BrushRotate(float Amount)
{
	int Rotation = (round_to_int(360.0f * Amount / (pi * 2)) / 90) % 4; // 0=0°, 1=90°, 2=180°, 3=270°
	if(Rotation < 0)
		Rotation += 4;

	if(Rotation == 1 || Rotation == 3)
	{
		// 90° rotation
		CTile *pTempData = new CTile[m_Width * m_Height];
		mem_copy(pTempData, m_pTiles, (size_t)m_Width * m_Height * sizeof(CTile));
		CTile *pDst = m_pTiles;
		bool Rotate = !(m_Game || m_Front) || m_pEditor->m_AllowPlaceUnusedTiles;
		for(int x = 0; x < m_Width; ++x)
			for(int y = m_Height - 1; y >= 0; --y, ++pDst)
			{
				*pDst = pTempData[y * m_Width + x];
				if(!Rotate && !IsRotatableTile(pDst->m_Index))
					pDst->m_Flags = 0;
				else
				{
					if(pDst->m_Flags & TILEFLAG_ROTATE)
						pDst->m_Flags ^= (TILEFLAG_HFLIP | TILEFLAG_VFLIP);
					pDst->m_Flags ^= TILEFLAG_ROTATE;
				}
			}

		int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempData;
	}

	if(Rotation == 2 || Rotation == 3)
	{
		BrushFlipX();
		BrushFlipY();
	}
}

void CLayerTiles::Resize(int NewW, int NewH)
{
	CTile *pNewData = new CTile[NewW * NewH];
	mem_zero(pNewData, (size_t)NewW * NewH * sizeof(CTile));

	// copy old data
	for(int y = 0; y < minimum(NewH, m_Height); y++)
		mem_copy(&pNewData[y * NewW], &m_pTiles[y * m_Width], minimum(m_Width, NewW) * sizeof(CTile));

	// replace old
	delete[] m_pTiles;
	m_pTiles = pNewData;
	m_Width = NewW;
	m_Height = NewH;

	// resize tele layer if available
	if(m_Game && m_pEditor->m_Map.m_pTeleLayer && (m_pEditor->m_Map.m_pTeleLayer->m_Width != NewW || m_pEditor->m_Map.m_pTeleLayer->m_Height != NewH))
		m_pEditor->m_Map.m_pTeleLayer->Resize(NewW, NewH);

	// resize speedup layer if available
	if(m_Game && m_pEditor->m_Map.m_pSpeedupLayer && (m_pEditor->m_Map.m_pSpeedupLayer->m_Width != NewW || m_pEditor->m_Map.m_pSpeedupLayer->m_Height != NewH))
		m_pEditor->m_Map.m_pSpeedupLayer->Resize(NewW, NewH);

	// resize front layer
	if(m_Game && m_pEditor->m_Map.m_pFrontLayer && (m_pEditor->m_Map.m_pFrontLayer->m_Width != NewW || m_pEditor->m_Map.m_pFrontLayer->m_Height != NewH))
		m_pEditor->m_Map.m_pFrontLayer->Resize(NewW, NewH);

	// resize switch layer if available
	if(m_Game && m_pEditor->m_Map.m_pSwitchLayer && (m_pEditor->m_Map.m_pSwitchLayer->m_Width != NewW || m_pEditor->m_Map.m_pSwitchLayer->m_Height != NewH))
		m_pEditor->m_Map.m_pSwitchLayer->Resize(NewW, NewH);

	// resize tune layer if available
	if(m_Game && m_pEditor->m_Map.m_pTuneLayer && (m_pEditor->m_Map.m_pTuneLayer->m_Width != NewW || m_pEditor->m_Map.m_pTuneLayer->m_Height != NewH))
		m_pEditor->m_Map.m_pTuneLayer->Resize(NewW, NewH);
}

void CLayerTiles::Shift(int Direction)
{
	int o = m_pEditor->m_ShiftBy;

	switch(Direction)
	{
	case 1:
	{
		// left
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pTiles[y * m_Width], &m_pTiles[y * m_Width + o], (m_Width - o) * sizeof(CTile));
			mem_zero(&m_pTiles[y * m_Width + (m_Width - o)], o * sizeof(CTile));
		}
	}
	break;
	case 2:
	{
		// right
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pTiles[y * m_Width + o], &m_pTiles[y * m_Width], (m_Width - o) * sizeof(CTile));
			mem_zero(&m_pTiles[y * m_Width], o * sizeof(CTile));
		}
	}
	break;
	case 4:
	{
		// up
		for(int y = 0; y < m_Height - o; ++y)
		{
			mem_copy(&m_pTiles[y * m_Width], &m_pTiles[(y + o) * m_Width], m_Width * sizeof(CTile));
			mem_zero(&m_pTiles[(y + o) * m_Width], m_Width * sizeof(CTile));
		}
	}
	break;
	case 8:
	{
		// down
		for(int y = m_Height - 1; y >= o; --y)
		{
			mem_copy(&m_pTiles[y * m_Width], &m_pTiles[(y - o) * m_Width], m_Width * sizeof(CTile));
			mem_zero(&m_pTiles[(y - o) * m_Width], m_Width * sizeof(CTile));
		}
	}
	}
}

void CLayerTiles::ShowInfo()
{
}

int CLayerTiles::RenderProperties(CUIRect *pToolBox)
{
	return 0;
}

void CLayerTiles::FlagModified(int x, int y, int w, int h)
{
	m_pEditor->m_Map.m_Modified = true;
	if(m_Seed != 0 && m_AutoMapperConfig != -1 && m_AutoAutoMap && m_Image >= 0)
	{
		m_pEditor->m_Map.m_lImages[m_Image]->m_AutoMapper.ProceedLocalized(this, m_AutoMapperConfig, m_Seed, x, y, w, h);
	}
}

void CLayerTiles::ModifyImageIndex(INDEX_MODIFY_FUNC Func)
{
	Func(&m_Image);
}

void CLayerTiles::ModifyEnvelopeIndex(INDEX_MODIFY_FUNC Func)
{
	Func(&m_ColorEnv);
}

CLayerTele::CLayerTele(int w, int h) :
	CLayerTiles(w, h)
{
	//m_Type = LAYERTYPE_TELE;
	str_copy(m_aName, "Tele", sizeof(m_aName));
	m_Tele = 1;

	m_pTeleTile = new CTeleTile[w * h];
	mem_zero(m_pTeleTile, (size_t)w * h * sizeof(CTeleTile));
}

CLayerTele::~CLayerTele()
{
	delete[] m_pTeleTile;
}

void CLayerTele::Resize(int NewW, int NewH)
{
	// resize tele data
	CTeleTile *pNewTeleData = new CTeleTile[NewW * NewH];
	mem_zero(pNewTeleData, (size_t)NewW * NewH * sizeof(CTeleTile));

	// copy old data
	for(int y = 0; y < minimum(NewH, m_Height); y++)
		mem_copy(&pNewTeleData[y * NewW], &m_pTeleTile[y * m_Width], minimum(m_Width, NewW) * sizeof(CTeleTile));

	// replace old
	delete[] m_pTeleTile;
	m_pTeleTile = pNewTeleData;

	// resize tile data
	CLayerTiles::Resize(NewW, NewH);

	// resize gamelayer too
	if(m_pEditor->m_Map.m_pGameLayer->m_Width != NewW || m_pEditor->m_Map.m_pGameLayer->m_Height != NewH)
		m_pEditor->m_Map.m_pGameLayer->Resize(NewW, NewH);
}

void CLayerTele::Shift(int Direction)
{
	CLayerTiles::Shift(Direction);
	int o = m_pEditor->m_ShiftBy;

	switch(Direction)
	{
	case 1:
	{
		// left
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pTeleTile[y * m_Width], &m_pTeleTile[y * m_Width + o], (m_Width - o) * sizeof(CTeleTile));
			mem_zero(&m_pTeleTile[y * m_Width + (m_Width - o)], o * sizeof(CTeleTile));
		}
	}
	break;
	case 2:
	{
		// right
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pTeleTile[y * m_Width + o], &m_pTeleTile[y * m_Width], (m_Width - o) * sizeof(CTeleTile));
			mem_zero(&m_pTeleTile[y * m_Width], o * sizeof(CTeleTile));
		}
	}
	break;
	case 4:
	{
		// up
		for(int y = 0; y < m_Height - o; ++y)
		{
			mem_copy(&m_pTeleTile[y * m_Width], &m_pTeleTile[(y + o) * m_Width], m_Width * sizeof(CTeleTile));
			mem_zero(&m_pTeleTile[(y + o) * m_Width], m_Width * sizeof(CTeleTile));
		}
	}
	break;
	case 8:
	{
		// down
		for(int y = m_Height - 1; y >= o; --y)
		{
			mem_copy(&m_pTeleTile[y * m_Width], &m_pTeleTile[(y - o) * m_Width], m_Width * sizeof(CTeleTile));
			mem_zero(&m_pTeleTile[(y - o) * m_Width], m_Width * sizeof(CTeleTile));
		}
	}
	}
}

bool CLayerTele::IsEmpty(CLayerTiles *pLayer)
{
	for(int y = 0; y < pLayer->m_Height; y++)
		for(int x = 0; x < pLayer->m_Width; x++)
			if(IsValidTeleTile(pLayer->GetTile(x, y).m_Index))
				return false;

	return true;
}

void CLayerTele::BrushDraw(CLayer *pBrush, float wx, float wy)
{
	if(m_Readonly)
		return;

	CLayerTele *l = (CLayerTele *)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);
	if(str_comp(l->m_aFileName, m_pEditor->m_aFileName))
		m_pEditor->m_TeleNumber = l->m_TeleNum;

	bool Destructive = m_pEditor->m_BrushDrawDestructive || IsEmpty(l);

	for(int y = 0; y < l->m_Height; y++)
		for(int x = 0; x < l->m_Width; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(IsValidTeleTile(l->m_pTiles[y * l->m_Width + x].m_Index))
			{
				if(m_pEditor->m_TeleNumber != l->m_TeleNum)
				{
					m_pTeleTile[fy * m_Width + fx].m_Number = m_pEditor->m_TeleNumber;
				}
				else if(l->m_pTeleTile[y * l->m_Width + x].m_Number)
					m_pTeleTile[fy * m_Width + fx].m_Number = l->m_pTeleTile[y * l->m_Width + x].m_Number;
				else
				{
					if(!m_pEditor->m_TeleNumber)
					{
						m_pTeleTile[fy * m_Width + fx].m_Number = 0;
						m_pTeleTile[fy * m_Width + fx].m_Type = 0;
						m_pTiles[fy * m_Width + fx].m_Index = 0;
						continue;
					}
					else
						m_pTeleTile[fy * m_Width + fx].m_Number = m_pEditor->m_TeleNumber;
				}

				m_pTeleTile[fy * m_Width + fx].m_Type = l->m_pTiles[y * l->m_Width + x].m_Index;
				m_pTiles[fy * m_Width + fx].m_Index = l->m_pTiles[y * l->m_Width + x].m_Index;
			}
			else
			{
				m_pTeleTile[fy * m_Width + fx].m_Number = 0;
				m_pTeleTile[fy * m_Width + fx].m_Type = 0;
				m_pTiles[fy * m_Width + fx].m_Index = 0;
			}
		}
	FlagModified(sx, sy, l->m_Width, l->m_Height);
}

void CLayerTele::BrushFlipX()
{
	CLayerTiles::BrushFlipX();

	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width / 2; x++)
		{
			CTeleTile Tmp = m_pTeleTile[y * m_Width + x];
			m_pTeleTile[y * m_Width + x] = m_pTeleTile[y * m_Width + m_Width - 1 - x];
			m_pTeleTile[y * m_Width + m_Width - 1 - x] = Tmp;
		}
}

void CLayerTele::BrushFlipY()
{
	CLayerTiles::BrushFlipY();

	for(int y = 0; y < m_Height / 2; y++)
		for(int x = 0; x < m_Width; x++)
		{
			CTeleTile Tmp = m_pTeleTile[y * m_Width + x];
			m_pTeleTile[y * m_Width + x] = m_pTeleTile[(m_Height - 1 - y) * m_Width + x];
			m_pTeleTile[(m_Height - 1 - y) * m_Width + x] = Tmp;
		}
}

void CLayerTele::BrushRotate(float Amount)
{
	int Rotation = (round_to_int(360.0f * Amount / (pi * 2)) / 90) % 4; // 0=0°, 1=90°, 2=180°, 3=270°
	if(Rotation < 0)
		Rotation += 4;

	if(Rotation == 1 || Rotation == 3)
	{
		// 90° rotation
		CTeleTile *pTempData1 = new CTeleTile[m_Width * m_Height];
		CTile *pTempData2 = new CTile[m_Width * m_Height];
		mem_copy(pTempData1, m_pTeleTile, (size_t)m_Width * m_Height * sizeof(CTeleTile));
		mem_copy(pTempData2, m_pTiles, (size_t)m_Width * m_Height * sizeof(CTile));
		CTeleTile *pDst1 = m_pTeleTile;
		CTile *pDst2 = m_pTiles;
		for(int x = 0; x < m_Width; ++x)
			for(int y = m_Height - 1; y >= 0; --y, ++pDst1, ++pDst2)
			{
				*pDst1 = pTempData1[y * m_Width + x];
				*pDst2 = pTempData2[y * m_Width + x];
			}

		int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempData1;
		delete[] pTempData2;
	}

	if(Rotation == 2 || Rotation == 3)
	{
		BrushFlipX();
		BrushFlipY();
	}
}

void CLayerTele::FillSelection(bool Empty, CLayer *pBrush, CUIRect Rect)
{
	if(m_Readonly || (!Empty && pBrush->m_Type != LAYERTYPE_TILES))
		return;

	Snap(&Rect); // corrects Rect; no need of <=

	Snap(&Rect);

	int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);

	CLayerTele *pLt = static_cast<CLayerTele *>(pBrush);

	bool Destructive = m_pEditor->m_BrushDrawDestructive || Empty || IsEmpty(pLt);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(Empty || !IsValidTeleTile((pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)]).m_Index))
			{
				m_pTiles[fy * m_Width + fx].m_Index = 0;
				m_pTeleTile[fy * m_Width + fx].m_Type = 0;
				m_pTeleTile[fy * m_Width + fx].m_Number = 0;
			}
			else
			{
				m_pTiles[fy * m_Width + fx] = pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)];
				if(pLt->m_Tele && m_pTiles[fy * m_Width + fx].m_Index > 0)
				{
					if((!pLt->m_pTeleTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Number && m_pEditor->m_TeleNumber) || m_pEditor->m_TeleNumber != pLt->m_TeleNum)
						m_pTeleTile[fy * m_Width + fx].m_Number = m_pEditor->m_TeleNumber;
					else
						m_pTeleTile[fy * m_Width + fx].m_Number = pLt->m_pTeleTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Number;
					m_pTeleTile[fy * m_Width + fx].m_Type = m_pTiles[fy * m_Width + fx].m_Index;
				}
			}
		}
	}
	FlagModified(sx, sy, w, h);
}

bool CLayerTele::ContainsElementWithId(int Id)
{
	for(int y = 0; y < m_Height; ++y)
	{
		for(int x = 0; x < m_Width; ++x)
		{
			if(m_pTeleTile[y * m_Width + x].m_Number == Id)
			{
				return true;
			}
		}
	}

	return false;
}

CLayerSpeedup::CLayerSpeedup(int w, int h) :
	CLayerTiles(w, h)
{
	//m_Type = LAYERTYPE_SPEEDUP;
	str_copy(m_aName, "Speedup", sizeof(m_aName));
	m_Speedup = 1;

	m_pSpeedupTile = new CSpeedupTile[w * h];
	mem_zero(m_pSpeedupTile, (size_t)w * h * sizeof(CSpeedupTile));
}

CLayerSpeedup::~CLayerSpeedup()
{
	delete[] m_pSpeedupTile;
}

void CLayerSpeedup::Resize(int NewW, int NewH)
{
	// resize speedup data
	CSpeedupTile *pNewSpeedupData = new CSpeedupTile[NewW * NewH];
	mem_zero(pNewSpeedupData, (size_t)NewW * NewH * sizeof(CSpeedupTile));

	// copy old data
	for(int y = 0; y < minimum(NewH, m_Height); y++)
		mem_copy(&pNewSpeedupData[y * NewW], &m_pSpeedupTile[y * m_Width], minimum(m_Width, NewW) * sizeof(CSpeedupTile));

	// replace old
	delete[] m_pSpeedupTile;
	m_pSpeedupTile = pNewSpeedupData;

	// resize tile data
	CLayerTiles::Resize(NewW, NewH);

	// resize gamelayer too
	if(m_pEditor->m_Map.m_pGameLayer->m_Width != NewW || m_pEditor->m_Map.m_pGameLayer->m_Height != NewH)
		m_pEditor->m_Map.m_pGameLayer->Resize(NewW, NewH);
}

void CLayerSpeedup::Shift(int Direction)
{
	CLayerTiles::Shift(Direction);
	int o = m_pEditor->m_ShiftBy;

	switch(Direction)
	{
	case 1:
	{
		// left
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pSpeedupTile[y * m_Width], &m_pSpeedupTile[y * m_Width + o], (m_Width - o) * sizeof(CSpeedupTile));
			mem_zero(&m_pSpeedupTile[y * m_Width + (m_Width - o)], o * sizeof(CSpeedupTile));
		}
	}
	break;
	case 2:
	{
		// right
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pSpeedupTile[y * m_Width + o], &m_pSpeedupTile[y * m_Width], (m_Width - o) * sizeof(CSpeedupTile));
			mem_zero(&m_pSpeedupTile[y * m_Width], o * sizeof(CSpeedupTile));
		}
	}
	break;
	case 4:
	{
		// up
		for(int y = 0; y < m_Height - o; ++y)
		{
			mem_copy(&m_pSpeedupTile[y * m_Width], &m_pSpeedupTile[(y + o) * m_Width], m_Width * sizeof(CSpeedupTile));
			mem_zero(&m_pSpeedupTile[(y + o) * m_Width], m_Width * sizeof(CSpeedupTile));
		}
	}
	break;
	case 8:
	{
		// down
		for(int y = m_Height - 1; y >= o; --y)
		{
			mem_copy(&m_pSpeedupTile[y * m_Width], &m_pSpeedupTile[(y - o) * m_Width], m_Width * sizeof(CSpeedupTile));
			mem_zero(&m_pSpeedupTile[(y - o) * m_Width], m_Width * sizeof(CSpeedupTile));
		}
	}
	}
}

bool CLayerSpeedup::IsEmpty(CLayerTiles *pLayer)
{
	for(int y = 0; y < pLayer->m_Height; y++)
		for(int x = 0; x < pLayer->m_Width; x++)
			if(IsValidSpeedupTile(pLayer->GetTile(x, y).m_Index))
				return false;

	return true;
}

void CLayerSpeedup::BrushDraw(CLayer *pBrush, float wx, float wy)
{
	if(m_Readonly)
		return;

	CLayerSpeedup *l = (CLayerSpeedup *)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);
	if(str_comp(l->m_aFileName, m_pEditor->m_aFileName))
	{
		m_pEditor->m_SpeedupAngle = l->m_SpeedupAngle;
		m_pEditor->m_SpeedupForce = l->m_SpeedupForce;
		m_pEditor->m_SpeedupMaxSpeed = l->m_SpeedupMaxSpeed;
	}

	bool Destructive = m_pEditor->m_BrushDrawDestructive || IsEmpty(l);

	for(int y = 0; y < l->m_Height; y++)
		for(int x = 0; x < l->m_Width; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(IsValidSpeedupTile(l->m_pTiles[y * l->m_Width + x].m_Index))
			{
				if(m_pEditor->m_SpeedupAngle != l->m_SpeedupAngle || m_pEditor->m_SpeedupForce != l->m_SpeedupForce || m_pEditor->m_SpeedupMaxSpeed != l->m_SpeedupMaxSpeed)
				{
					m_pSpeedupTile[fy * m_Width + fx].m_Force = m_pEditor->m_SpeedupForce;
					m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = m_pEditor->m_SpeedupMaxSpeed;
					m_pSpeedupTile[fy * m_Width + fx].m_Angle = m_pEditor->m_SpeedupAngle;
					m_pSpeedupTile[fy * m_Width + fx].m_Type = l->m_pTiles[y * l->m_Width + x].m_Index;
					m_pTiles[fy * m_Width + fx].m_Index = l->m_pTiles[y * l->m_Width + x].m_Index;
				}
				else if(l->m_pSpeedupTile[y * l->m_Width + x].m_Force)
				{
					m_pSpeedupTile[fy * m_Width + fx].m_Force = l->m_pSpeedupTile[y * l->m_Width + x].m_Force;
					m_pSpeedupTile[fy * m_Width + fx].m_Angle = l->m_pSpeedupTile[y * l->m_Width + x].m_Angle;
					m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = l->m_pSpeedupTile[y * l->m_Width + x].m_MaxSpeed;
					m_pSpeedupTile[fy * m_Width + fx].m_Type = l->m_pTiles[y * l->m_Width + x].m_Index;
					m_pTiles[fy * m_Width + fx].m_Index = l->m_pTiles[y * l->m_Width + x].m_Index;
				}
				else if(m_pEditor->m_SpeedupForce)
				{
					m_pSpeedupTile[fy * m_Width + fx].m_Force = m_pEditor->m_SpeedupForce;
					m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = m_pEditor->m_SpeedupMaxSpeed;
					m_pSpeedupTile[fy * m_Width + fx].m_Angle = m_pEditor->m_SpeedupAngle;
					m_pSpeedupTile[fy * m_Width + fx].m_Type = l->m_pTiles[y * l->m_Width + x].m_Index;
					m_pTiles[fy * m_Width + fx].m_Index = l->m_pTiles[y * l->m_Width + x].m_Index;
				}
				else
				{
					m_pSpeedupTile[fy * m_Width + fx].m_Force = 0;
					m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = 0;
					m_pSpeedupTile[fy * m_Width + fx].m_Angle = 0;
					m_pSpeedupTile[fy * m_Width + fx].m_Type = 0;
					m_pTiles[fy * m_Width + fx].m_Index = 0;
				}
			}
			else
			{
				m_pSpeedupTile[fy * m_Width + fx].m_Force = 0;
				m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = 0;
				m_pSpeedupTile[fy * m_Width + fx].m_Angle = 0;
				m_pSpeedupTile[fy * m_Width + fx].m_Type = 0;
				m_pTiles[fy * m_Width + fx].m_Index = 0;
			}
		}
	FlagModified(sx, sy, l->m_Width, l->m_Height);
}

void CLayerSpeedup::BrushFlipX()
{
	CLayerTiles::BrushFlipX();

	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width / 2; x++)
		{
			CSpeedupTile Tmp = m_pSpeedupTile[y * m_Width + x];
			m_pSpeedupTile[y * m_Width + x] = m_pSpeedupTile[y * m_Width + m_Width - 1 - x];
			m_pSpeedupTile[y * m_Width + m_Width - 1 - x] = Tmp;
		}
}

void CLayerSpeedup::BrushFlipY()
{
	CLayerTiles::BrushFlipY();

	for(int y = 0; y < m_Height / 2; y++)
		for(int x = 0; x < m_Width; x++)
		{
			CSpeedupTile Tmp = m_pSpeedupTile[y * m_Width + x];
			m_pSpeedupTile[y * m_Width + x] = m_pSpeedupTile[(m_Height - 1 - y) * m_Width + x];
			m_pSpeedupTile[(m_Height - 1 - y) * m_Width + x] = Tmp;
		}
}

void CLayerSpeedup::BrushRotate(float Amount)
{
	int Rotation = (round_to_int(360.0f * Amount / (pi * 2)) / 90) % 4; // 0=0°, 1=90°, 2=180°, 3=270°
	if(Rotation < 0)
		Rotation += 4;

	if(Rotation == 1 || Rotation == 3)
	{
		// 90° rotation
		CSpeedupTile *pTempData1 = new CSpeedupTile[m_Width * m_Height];
		CTile *pTempData2 = new CTile[m_Width * m_Height];
		mem_copy(pTempData1, m_pSpeedupTile, (size_t)m_Width * m_Height * sizeof(CSpeedupTile));
		mem_copy(pTempData2, m_pTiles, (size_t)m_Width * m_Height * sizeof(CTile));
		CSpeedupTile *pDst1 = m_pSpeedupTile;
		CTile *pDst2 = m_pTiles;
		for(int x = 0; x < m_Width; ++x)
			for(int y = m_Height - 1; y >= 0; --y, ++pDst1, ++pDst2)
			{
				*pDst1 = pTempData1[y * m_Width + x];
				*pDst2 = pTempData2[y * m_Width + x];
			}

		int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempData1;
		delete[] pTempData2;
	}

	if(Rotation == 2 || Rotation == 3)
	{
		BrushFlipX();
		BrushFlipY();
	}
}

void CLayerSpeedup::FillSelection(bool Empty, CLayer *pBrush, CUIRect Rect)
{
	if(m_Readonly || (!Empty && pBrush->m_Type != LAYERTYPE_TILES))
		return;

	Snap(&Rect); // corrects Rect; no need of <=

	Snap(&Rect);

	int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);

	CLayerSpeedup *pLt = static_cast<CLayerSpeedup *>(pBrush);

	bool Destructive = m_pEditor->m_BrushDrawDestructive || Empty || IsEmpty(pLt);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(Empty || !IsValidSpeedupTile((pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)]).m_Index)) // no speed up tile chosen: reset
			{
				m_pTiles[fy * m_Width + fx].m_Index = 0;
				m_pSpeedupTile[fy * m_Width + fx].m_Force = 0;
				m_pSpeedupTile[fy * m_Width + fx].m_Angle = 0;
			}
			else
			{
				m_pTiles[fy * m_Width + fx] = pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)];
				if(pLt->m_Speedup && m_pTiles[fy * m_Width + fx].m_Index > 0)
				{
					if((!pLt->m_pSpeedupTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Force && m_pEditor->m_SpeedupForce) || m_pEditor->m_SpeedupForce != pLt->m_SpeedupForce)
						m_pSpeedupTile[fy * m_Width + fx].m_Force = m_pEditor->m_SpeedupForce;
					else
						m_pSpeedupTile[fy * m_Width + fx].m_Force = pLt->m_pSpeedupTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Force;
					if((!pLt->m_pSpeedupTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Angle && m_pEditor->m_SpeedupAngle) || m_pEditor->m_SpeedupAngle != pLt->m_SpeedupAngle)
						m_pSpeedupTile[fy * m_Width + fx].m_Angle = m_pEditor->m_SpeedupAngle;
					else
						m_pSpeedupTile[fy * m_Width + fx].m_Angle = pLt->m_pSpeedupTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Angle;
					if((!pLt->m_pSpeedupTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_MaxSpeed && m_pEditor->m_SpeedupMaxSpeed) || m_pEditor->m_SpeedupMaxSpeed != pLt->m_SpeedupMaxSpeed)
						m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = m_pEditor->m_SpeedupMaxSpeed;
					else
						m_pSpeedupTile[fy * m_Width + fx].m_MaxSpeed = pLt->m_pSpeedupTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_MaxSpeed;
					m_pSpeedupTile[fy * m_Width + fx].m_Type = m_pTiles[fy * m_Width + fx].m_Index;
				}
			}
		}
	}
	FlagModified(sx, sy, w, h);
}

CLayerFront::CLayerFront(int w, int h) :
	CLayerTiles(w, h)
{
	//m_Type = LAYERTYPE_FRONT;
	str_copy(m_aName, "Front", sizeof(m_aName));
	m_Front = 1;
}

void CLayerFront::SetTile(int x, int y, CTile tile)
{
	if(tile.m_Index == TILE_THROUGH_CUT)
	{
		CTile nohook = {TILE_NOHOOK};
		m_pEditor->m_Map.m_pGameLayer->CLayerTiles::SetTile(x, y, nohook); // NOLINT(bugprone-parent-virtual-call)
	}
	else if(tile.m_Index == TILE_AIR && CLayerTiles::GetTile(x, y).m_Index == TILE_THROUGH_CUT)
	{
		CTile air = {TILE_AIR};
		m_pEditor->m_Map.m_pGameLayer->CLayerTiles::SetTile(x, y, air); // NOLINT(bugprone-parent-virtual-call)
	}
	if(m_pEditor->m_AllowPlaceUnusedTiles || IsValidFrontTile(tile.m_Index))
	{
		CLayerTiles::SetTile(x, y, tile);
	}
	else
	{
		CTile air = {TILE_AIR};
		CLayerTiles::SetTile(x, y, air);
		if(!m_pEditor->m_PreventUnusedTilesWasWarned)
		{
			m_pEditor->m_PopupEventType = m_pEditor->POPEVENT_PREVENTUNUSEDTILES;
			m_pEditor->m_PopupEventActivated = true;
			m_pEditor->m_PreventUnusedTilesWasWarned = true;
		}
	}
}

void CLayerFront::Resize(int NewW, int NewH)
{
	// resize tile data
	CLayerTiles::Resize(NewW, NewH);

	// resize gamelayer too
	if(m_pEditor->m_Map.m_pGameLayer->m_Width != NewW || m_pEditor->m_Map.m_pGameLayer->m_Height != NewH)
		m_pEditor->m_Map.m_pGameLayer->Resize(NewW, NewH);
}

CLayerSwitch::CLayerSwitch(int w, int h) :
	CLayerTiles(w, h)
{
	//m_Type = LAYERTYPE_SWITCH;
	str_copy(m_aName, "Switch", sizeof(m_aName));
	m_Switch = 1;

	m_pSwitchTile = new CSwitchTile[w * h];
	mem_zero(m_pSwitchTile, (size_t)w * h * sizeof(CSwitchTile));
}

CLayerSwitch::~CLayerSwitch()
{
	delete[] m_pSwitchTile;
}

void CLayerSwitch::Resize(int NewW, int NewH)
{
	// resize switch data
	CSwitchTile *pNewSwitchData = new CSwitchTile[NewW * NewH];
	mem_zero(pNewSwitchData, (size_t)NewW * NewH * sizeof(CSwitchTile));

	// copy old data
	for(int y = 0; y < minimum(NewH, m_Height); y++)
		mem_copy(&pNewSwitchData[y * NewW], &m_pSwitchTile[y * m_Width], minimum(m_Width, NewW) * sizeof(CSwitchTile));

	// replace old
	delete[] m_pSwitchTile;
	m_pSwitchTile = pNewSwitchData;

	// resize tile data
	CLayerTiles::Resize(NewW, NewH);

	// resize gamelayer too
	if(m_pEditor->m_Map.m_pGameLayer->m_Width != NewW || m_pEditor->m_Map.m_pGameLayer->m_Height != NewH)
		m_pEditor->m_Map.m_pGameLayer->Resize(NewW, NewH);
}

void CLayerSwitch::Shift(int Direction)
{
	CLayerTiles::Shift(Direction);
	int o = m_pEditor->m_ShiftBy;

	switch(Direction)
	{
	case 1:
	{
		// left
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pSwitchTile[y * m_Width], &m_pSwitchTile[y * m_Width + o], (m_Width - o) * sizeof(CSwitchTile));
			mem_zero(&m_pSwitchTile[y * m_Width + (m_Width - o)], o * sizeof(CSwitchTile));
		}
	}
	break;
	case 2:
	{
		// right
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pSwitchTile[y * m_Width + o], &m_pSwitchTile[y * m_Width], (m_Width - o) * sizeof(CSwitchTile));
			mem_zero(&m_pSwitchTile[y * m_Width], o * sizeof(CSwitchTile));
		}
	}
	break;
	case 4:
	{
		// up
		for(int y = 0; y < m_Height - o; ++y)
		{
			mem_copy(&m_pSwitchTile[y * m_Width], &m_pSwitchTile[(y + o) * m_Width], m_Width * sizeof(CSwitchTile));
			mem_zero(&m_pSwitchTile[(y + o) * m_Width], m_Width * sizeof(CSwitchTile));
		}
	}
	break;
	case 8:
	{
		// down
		for(int y = m_Height - 1; y >= o; --y)
		{
			mem_copy(&m_pSwitchTile[y * m_Width], &m_pSwitchTile[(y - o) * m_Width], m_Width * sizeof(CSwitchTile));
			mem_zero(&m_pSwitchTile[(y - o) * m_Width], m_Width * sizeof(CSwitchTile));
		}
	}
	}
}

bool CLayerSwitch::IsEmpty(CLayerTiles *pLayer)
{
	for(int y = 0; y < pLayer->m_Height; y++)
		for(int x = 0; x < pLayer->m_Width; x++)
			if(IsValidSwitchTile(pLayer->GetTile(x, y).m_Index))
				return false;

	return true;
}

void CLayerSwitch::BrushDraw(CLayer *pBrush, float wx, float wy)
{
	if(m_Readonly)
		return;

	CLayerSwitch *l = (CLayerSwitch *)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);
	if(str_comp(l->m_aFileName, m_pEditor->m_aFileName))
	{
		m_pEditor->m_SwitchNum = l->m_SwitchNumber;
		m_pEditor->m_SwitchDelay = l->m_SwitchDelay;
	}

	bool Destructive = m_pEditor->m_BrushDrawDestructive || IsEmpty(l);

	for(int y = 0; y < l->m_Height; y++)
		for(int x = 0; x < l->m_Width; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(IsValidSwitchTile(l->m_pTiles[y * l->m_Width + x].m_Index))
			{
				if(m_pEditor->m_SwitchNum != l->m_SwitchNumber || m_pEditor->m_SwitchDelay != l->m_SwitchDelay)
				{
					m_pSwitchTile[fy * m_Width + fx].m_Number = m_pEditor->m_SwitchNum;
					m_pSwitchTile[fy * m_Width + fx].m_Delay = m_pEditor->m_SwitchDelay;
				}
				else if(l->m_pSwitchTile[y * l->m_Width + x].m_Number)
				{
					m_pSwitchTile[fy * m_Width + fx].m_Number = l->m_pSwitchTile[y * l->m_Width + x].m_Number;
					m_pSwitchTile[fy * m_Width + fx].m_Delay = l->m_pSwitchTile[y * l->m_Width + x].m_Delay;
				}
				else
				{
					m_pSwitchTile[fy * m_Width + fx].m_Number = m_pEditor->m_SwitchNum;
					m_pSwitchTile[fy * m_Width + fx].m_Delay = m_pEditor->m_SwitchDelay;
				}

				m_pSwitchTile[fy * m_Width + fx].m_Type = l->m_pTiles[y * l->m_Width + x].m_Index;
				m_pSwitchTile[fy * m_Width + fx].m_Flags = l->m_pTiles[y * l->m_Width + x].m_Flags;
				m_pTiles[fy * m_Width + fx].m_Index = l->m_pTiles[y * l->m_Width + x].m_Index;
				m_pTiles[fy * m_Width + fx].m_Flags = l->m_pTiles[y * l->m_Width + x].m_Flags;
			}
			else
			{
				m_pSwitchTile[fy * m_Width + fx].m_Number = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Type = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Flags = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Delay = 0;
				m_pTiles[fy * m_Width + fx].m_Index = 0;
			}

			if(l->m_pTiles[y * l->m_Width + x].m_Index == TILE_FREEZE)
			{
				m_pSwitchTile[fy * m_Width + fx].m_Flags = 0;
			}
			else if(l->m_pTiles[y * l->m_Width + x].m_Index == TILE_DFREEZE || l->m_pTiles[y * l->m_Width + x].m_Index == TILE_DUNFREEZE)
			{
				m_pSwitchTile[fy * m_Width + fx].m_Flags = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Delay = 0;
			}
		}
	FlagModified(sx, sy, l->m_Width, l->m_Height);
}

void CLayerSwitch::BrushFlipX()
{
	CLayerTiles::BrushFlipX();

	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width / 2; x++)
		{
			CSwitchTile Tmp = m_pSwitchTile[y * m_Width + x];
			m_pSwitchTile[y * m_Width + x] = m_pSwitchTile[y * m_Width + m_Width - 1 - x];
			m_pSwitchTile[y * m_Width + m_Width - 1 - x] = Tmp;
		}
}

void CLayerSwitch::BrushFlipY()
{
	CLayerTiles::BrushFlipY();

	for(int y = 0; y < m_Height / 2; y++)
		for(int x = 0; x < m_Width; x++)
		{
			CSwitchTile Tmp = m_pSwitchTile[y * m_Width + x];
			m_pSwitchTile[y * m_Width + x] = m_pSwitchTile[(m_Height - 1 - y) * m_Width + x];
			m_pSwitchTile[(m_Height - 1 - y) * m_Width + x] = Tmp;
		}
}

void CLayerSwitch::BrushRotate(float Amount)
{
	int Rotation = (round_to_int(360.0f * Amount / (pi * 2)) / 90) % 4; // 0=0°, 1=90°, 2=180°, 3=270°
	if(Rotation < 0)
		Rotation += 4;

	if(Rotation == 1 || Rotation == 3)
	{
		// 90° rotation
		CSwitchTile *pTempData1 = new CSwitchTile[m_Width * m_Height];
		CTile *pTempData2 = new CTile[m_Width * m_Height];
		mem_copy(pTempData1, m_pSwitchTile, (size_t)m_Width * m_Height * sizeof(CSwitchTile));
		mem_copy(pTempData2, m_pTiles, (size_t)m_Width * m_Height * sizeof(CTile));
		CSwitchTile *pDst1 = m_pSwitchTile;
		CTile *pDst2 = m_pTiles;
		for(int x = 0; x < m_Width; ++x)
			for(int y = m_Height - 1; y >= 0; --y, ++pDst1, ++pDst2)
			{
				*pDst1 = pTempData1[y * m_Width + x];
				*pDst2 = pTempData2[y * m_Width + x];
				if(IsRotatableTile(pDst2->m_Index))
				{
					if(pDst2->m_Flags & TILEFLAG_ROTATE)
						pDst2->m_Flags ^= (TILEFLAG_HFLIP | TILEFLAG_VFLIP);
					pDst2->m_Flags ^= TILEFLAG_ROTATE;
				}
			}

		int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempData1;
		delete[] pTempData2;
	}

	if(Rotation == 2 || Rotation == 3)
	{
		BrushFlipX();
		BrushFlipY();
	}
}

void CLayerSwitch::FillSelection(bool Empty, CLayer *pBrush, CUIRect Rect)
{
	if(m_Readonly || (!Empty && pBrush->m_Type != LAYERTYPE_TILES))
		return;

	Snap(&Rect); // corrects Rect; no need of <=

	Snap(&Rect);

	int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);

	CLayerSwitch *pLt = static_cast<CLayerSwitch *>(pBrush);

	bool Destructive = m_pEditor->m_BrushDrawDestructive || Empty || IsEmpty(pLt);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(Empty || !IsValidSwitchTile((pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)]).m_Index))
			{
				m_pTiles[fy * m_Width + fx].m_Index = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Type = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Number = 0;
				m_pSwitchTile[fy * m_Width + fx].m_Delay = 0;
			}
			else
			{
				m_pTiles[fy * m_Width + fx] = pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)];
				m_pSwitchTile[fy * m_Width + fx].m_Type = m_pTiles[fy * m_Width + fx].m_Index;
				if(pLt->m_Switch && m_pEditor->m_SwitchNum && m_pTiles[fy * m_Width + fx].m_Index > 0)
				{
					if((!pLt->m_pSwitchTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Number) || m_pEditor->m_SwitchNum != pLt->m_SwitchNumber)
						m_pSwitchTile[fy * m_Width + fx].m_Number = m_pEditor->m_SwitchNum;
					else
						m_pSwitchTile[fy * m_Width + fx].m_Number = pLt->m_pSwitchTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Number;
					if((!pLt->m_pSwitchTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Delay) || m_pEditor->m_SwitchDelay != pLt->m_SwitchDelay)
						m_pSwitchTile[fy * m_Width + fx].m_Delay = m_pEditor->m_SwitchDelay;
					else
						m_pSwitchTile[fy * m_Width + fx].m_Delay = pLt->m_pSwitchTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Delay;
					m_pSwitchTile[fy * m_Width + fx].m_Flags = pLt->m_pSwitchTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Flags;
				}
			}
		}
	}
	FlagModified(sx, sy, w, h);
}

bool CLayerSwitch::ContainsElementWithId(int Id)
{
	for(int y = 0; y < m_Height; ++y)
	{
		for(int x = 0; x < m_Width; ++x)
		{
			if(m_pSwitchTile[y * m_Width + x].m_Number == Id)
			{
				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------

CLayerTune::CLayerTune(int w, int h) :
	CLayerTiles(w, h)
{
	//m_Type = LAYERTYPE_TUNE;
	str_copy(m_aName, "Tune", sizeof(m_aName));
	m_Tune = 1;

	m_pTuneTile = new CTuneTile[w * h];
	mem_zero(m_pTuneTile, (size_t)w * h * sizeof(CTuneTile));
}

CLayerTune::~CLayerTune()
{
	delete[] m_pTuneTile;
}

void CLayerTune::Resize(int NewW, int NewH)
{
	// resize Tune data
	CTuneTile *pNewTuneData = new CTuneTile[NewW * NewH];
	mem_zero(pNewTuneData, (size_t)NewW * NewH * sizeof(CTuneTile));

	// copy old data
	for(int y = 0; y < minimum(NewH, m_Height); y++)
		mem_copy(&pNewTuneData[y * NewW], &m_pTuneTile[y * m_Width], minimum(m_Width, NewW) * sizeof(CTuneTile));

	// replace old
	delete[] m_pTuneTile;
	m_pTuneTile = pNewTuneData;

	// resize tile data
	CLayerTiles::Resize(NewW, NewH);

	// resize gamelayer too
	if(m_pEditor->m_Map.m_pGameLayer->m_Width != NewW || m_pEditor->m_Map.m_pGameLayer->m_Height != NewH)
		m_pEditor->m_Map.m_pGameLayer->Resize(NewW, NewH);
}

void CLayerTune::Shift(int Direction)
{
	CLayerTiles::Shift(Direction);
	int o = m_pEditor->m_ShiftBy;

	switch(Direction)
	{
	case 1:
	{
		// left
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pTuneTile[y * m_Width], &m_pTuneTile[y * m_Width + o], (m_Width - o) * sizeof(CTuneTile));
			mem_zero(&m_pTuneTile[y * m_Width + (m_Width - o)], o * sizeof(CTuneTile));
		}
	}
	break;
	case 2:
	{
		// right
		for(int y = 0; y < m_Height; ++y)
		{
			mem_move(&m_pTuneTile[y * m_Width + o], &m_pTuneTile[y * m_Width], (m_Width - o) * sizeof(CTuneTile));
			mem_zero(&m_pTuneTile[y * m_Width], o * sizeof(CTuneTile));
		}
	}
	break;
	case 4:
	{
		// up
		for(int y = 0; y < m_Height - o; ++y)
		{
			mem_copy(&m_pTuneTile[y * m_Width], &m_pTuneTile[(y + o) * m_Width], m_Width * sizeof(CTuneTile));
			mem_zero(&m_pTuneTile[(y + o) * m_Width], m_Width * sizeof(CTuneTile));
		}
	}
	break;
	case 8:
	{
		// down
		for(int y = m_Height - 1; y >= o; --y)
		{
			mem_copy(&m_pTuneTile[y * m_Width], &m_pTuneTile[(y - o) * m_Width], m_Width * sizeof(CTuneTile));
			mem_zero(&m_pTuneTile[(y - o) * m_Width], m_Width * sizeof(CTuneTile));
		}
	}
	}
}

bool CLayerTune::IsEmpty(CLayerTiles *pLayer)
{
	for(int y = 0; y < pLayer->m_Height; y++)
		for(int x = 0; x < pLayer->m_Width; x++)
			if(IsValidTuneTile(pLayer->GetTile(x, y).m_Index))
				return false;

	return true;
}

void CLayerTune::BrushDraw(CLayer *pBrush, float wx, float wy)
{
	if(m_Readonly)
		return;

	CLayerTune *l = (CLayerTune *)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);
	if(str_comp(l->m_aFileName, m_pEditor->m_aFileName))
	{
		m_pEditor->m_TuningNum = l->m_TuningNumber;
	}

	bool Destructive = m_pEditor->m_BrushDrawDestructive || IsEmpty(l);

	for(int y = 0; y < l->m_Height; y++)
		for(int x = 0; x < l->m_Width; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(IsValidTuneTile(l->m_pTiles[y * l->m_Width + x].m_Index))
			{
				if(m_pEditor->m_TuningNum != l->m_TuningNumber)
				{
					m_pTuneTile[fy * m_Width + fx].m_Number = m_pEditor->m_TuningNum;
				}
				else if(l->m_pTuneTile[y * l->m_Width + x].m_Number)
					m_pTuneTile[fy * m_Width + fx].m_Number = l->m_pTuneTile[y * l->m_Width + x].m_Number;
				else
				{
					if(!m_pEditor->m_TuningNum)
					{
						m_pTuneTile[fy * m_Width + fx].m_Number = 0;
						m_pTuneTile[fy * m_Width + fx].m_Type = 0;
						m_pTiles[fy * m_Width + fx].m_Index = 0;
						continue;
					}
					else
						m_pTuneTile[fy * m_Width + fx].m_Number = m_pEditor->m_TuningNum;
				}

				m_pTuneTile[fy * m_Width + fx].m_Type = l->m_pTiles[y * l->m_Width + x].m_Index;
				m_pTiles[fy * m_Width + fx].m_Index = l->m_pTiles[y * l->m_Width + x].m_Index;
			}
			else
			{
				m_pTuneTile[fy * m_Width + fx].m_Number = 0;
				m_pTuneTile[fy * m_Width + fx].m_Type = 0;
				m_pTiles[fy * m_Width + fx].m_Index = 0;
			}
		}
	FlagModified(sx, sy, l->m_Width, l->m_Height);
}

void CLayerTune::BrushFlipX()
{
	CLayerTiles::BrushFlipX();

	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width / 2; x++)
		{
			CTuneTile Tmp = m_pTuneTile[y * m_Width + x];
			m_pTuneTile[y * m_Width + x] = m_pTuneTile[y * m_Width + m_Width - 1 - x];
			m_pTuneTile[y * m_Width + m_Width - 1 - x] = Tmp;
		}
}

void CLayerTune::BrushFlipY()
{
	CLayerTiles::BrushFlipY();

	for(int y = 0; y < m_Height / 2; y++)
		for(int x = 0; x < m_Width; x++)
		{
			CTuneTile Tmp = m_pTuneTile[y * m_Width + x];
			m_pTuneTile[y * m_Width + x] = m_pTuneTile[(m_Height - 1 - y) * m_Width + x];
			m_pTuneTile[(m_Height - 1 - y) * m_Width + x] = Tmp;
		}
}

void CLayerTune::BrushRotate(float Amount)
{
	int Rotation = (round_to_int(360.0f * Amount / (pi * 2)) / 90) % 4; // 0=0°, 1=90°, 2=180°, 3=270°
	if(Rotation < 0)
		Rotation += 4;

	if(Rotation == 1 || Rotation == 3)
	{
		// 90° rotation
		CTuneTile *pTempData1 = new CTuneTile[m_Width * m_Height];
		CTile *pTempData2 = new CTile[m_Width * m_Height];
		mem_copy(pTempData1, m_pTuneTile, (size_t)m_Width * m_Height * sizeof(CTuneTile));
		mem_copy(pTempData2, m_pTiles, (size_t)m_Width * m_Height * sizeof(CTile));
		CTuneTile *pDst1 = m_pTuneTile;
		CTile *pDst2 = m_pTiles;
		for(int x = 0; x < m_Width; ++x)
			for(int y = m_Height - 1; y >= 0; --y, ++pDst1, ++pDst2)
			{
				*pDst1 = pTempData1[y * m_Width + x];
				*pDst2 = pTempData2[y * m_Width + x];
			}

		int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempData1;
		delete[] pTempData2;
	}

	if(Rotation == 2 || Rotation == 3)
	{
		BrushFlipX();
		BrushFlipY();
	}
}

void CLayerTune::FillSelection(bool Empty, CLayer *pBrush, CUIRect Rect)
{
	if(m_Readonly || (!Empty && pBrush->m_Type != LAYERTYPE_TILES))
		return;

	Snap(&Rect); // corrects Rect; no need of <=

	int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);

	CLayerTune *pLt = static_cast<CLayerTune *>(pBrush);

	bool Destructive = m_pEditor->m_BrushDrawDestructive || Empty || IsEmpty(pLt);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int fx = x + sx;
			int fy = y + sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(!Destructive && GetTile(fx, fy).m_Index)
				continue;

			if(Empty || !IsValidTuneTile((pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)]).m_Index)) // \o/ this fixes editor bug; TODO: use IsUsedInThisLayer here
			{
				m_pTiles[fy * m_Width + fx].m_Index = 0;
				m_pTuneTile[fy * m_Width + fx].m_Type = 0;
				m_pTuneTile[fy * m_Width + fx].m_Number = 0;
			}
			else
			{
				m_pTiles[fy * m_Width + fx] = pLt->m_pTiles[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)];
				if(pLt->m_Tune && m_pTiles[fy * m_Width + fx].m_Index > 0)
				{
					if((!pLt->m_pTuneTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Number && m_pEditor->m_TuningNum) || m_pEditor->m_TuningNum != pLt->m_TuningNumber)
						m_pTuneTile[fy * m_Width + fx].m_Number = m_pEditor->m_TuningNum;
					else
						m_pTuneTile[fy * m_Width + fx].m_Number = pLt->m_pTuneTile[(y * pLt->m_Width + x % pLt->m_Width) % (pLt->m_Width * pLt->m_Height)].m_Number;
					m_pTuneTile[fy * m_Width + fx].m_Type = m_pTiles[fy * m_Width + fx].m_Index;
				}
			}
		}
	}

	FlagModified(sx, sy, w, h);
}















































void CEditorMap::MakeTeleLayer(CLayer *pLayer)
{
	m_pTeleLayer = (CLayerTele *)pLayer;
	m_pTeleLayer->m_pEditor = m_pEditor;
	m_pTeleLayer->m_Texture = m_pEditor->m_TeleTexture;
}

void CEditorMap::MakeSpeedupLayer(CLayer *pLayer)
{
	m_pSpeedupLayer = (CLayerSpeedup *)pLayer;
	m_pSpeedupLayer->m_pEditor = m_pEditor;
	m_pSpeedupLayer->m_Texture = m_pEditor->m_SpeedupTexture;
}

void CEditorMap::MakeFrontLayer(CLayer *pLayer)
{
	m_pFrontLayer = (CLayerFront *)pLayer;
	m_pFrontLayer->m_pEditor = m_pEditor;
	m_pFrontLayer->m_Texture = m_pEditor->m_FrontTexture;
}

void CEditorMap::MakeSwitchLayer(CLayer *pLayer)
{
	m_pSwitchLayer = (CLayerSwitch *)pLayer;
	m_pSwitchLayer->m_pEditor = m_pEditor;
	m_pSwitchLayer->m_Texture = m_pEditor->m_SwitchTexture;
}

void CEditorMap::MakeTuneLayer(CLayer *pLayer)
{
	m_pTuneLayer = (CLayerTune *)pLayer;
	m_pTuneLayer->m_pEditor = m_pEditor;
	m_pTuneLayer->m_Texture = m_pEditor->m_TuneTexture;
}

void CLayerGroup::AddLayer(CLayer *l)
{
	m_pMap->m_Modified = true;
	m_lLayers.add(l);
}

void CLayerGroup::DeleteLayer(int Index)
{
	if(Index < 0 || Index >= m_lLayers.size())
		return;
	delete m_lLayers[Index];
	m_lLayers.remove_index(Index);
	m_pMap->m_Modified = true;
	m_pMap->m_UndoModified++;
}

void CLayerGroup::GetSize(float *w, float *h) const
{
	*w = 0;
	*h = 0;
	for(int i = 0; i < m_lLayers.size(); i++)
	{
		float lw, lh;
		m_lLayers[i]->GetSize(&lw, &lh);
		*w = maximum(*w, lw);
		*h = maximum(*h, lh);
	}
}

int CLayerGroup::SwapLayers(int Index0, int Index1)
{
	if(Index0 < 0 || Index0 >= m_lLayers.size())
		return Index0;
	if(Index1 < 0 || Index1 >= m_lLayers.size())
		return Index0;
	if(Index0 == Index1)
		return Index0;
	m_pMap->m_Modified = true;
	m_pMap->m_UndoModified++;
	swap(m_lLayers[Index0], m_lLayers[Index1]);
	return Index1;
}

void CEditorImage::AnalyseTileFlags()
{
	mem_zero(m_aTileFlags, sizeof(m_aTileFlags));

	int tw = m_Width / 16; // tilesizes
	int th = m_Height / 16;
	if(tw == th && m_Format == CImageInfo::FORMAT_RGBA)
	{
		unsigned char *pPixelData = (unsigned char *)m_pData;

		int TileID = 0;
		for(int ty = 0; ty < 16; ty++)
			for(int tx = 0; tx < 16; tx++, TileID++)
			{
				bool Opaque = true;
				for(int x = 0; x < tw; x++)
					for(int y = 0; y < th; y++)
					{
						int p = (ty * tw + y) * m_Width + tx * tw + x;
						if(pPixelData[p * 4 + 3] < 250)
						{
							Opaque = false;
							break;
						}
					}

				if(Opaque)
					m_aTileFlags[TileID] |= TILEFLAG_OPAQUE;
			}
	}
}

// editor.cpp

const void *CEditor::ms_pUiGotContext;

static bool ImageNameLess(const CEditorImage *const &a, const CEditorImage *const &b)
{
	return str_comp(a->m_aName, b->m_aName) < 0;
}

static int *gs_pSortedIndex = 0;
static void ModifySortedIndex(int *pIndex)
{
	if(*pIndex >= 0)
		*pIndex = gs_pSortedIndex[*pIndex];
}

void CEditor::SortImages()
{
	if(!std::is_sorted(m_Map.m_lImages.base_ptr(), m_Map.m_lImages.base_ptr() + m_Map.m_lImages.size(), ImageNameLess))
	{
		array<CEditorImage *> lTemp = m_Map.m_lImages;
		gs_pSortedIndex = new int[lTemp.size()];

		std::sort(m_Map.m_lImages.base_ptr(), m_Map.m_lImages.base_ptr() + m_Map.m_lImages.size(), ImageNameLess);
		for(int OldIndex = 0; OldIndex < lTemp.size(); OldIndex++)
		{
			for(int NewIndex = 0; NewIndex < m_Map.m_lImages.size(); NewIndex++)
			{
				if(lTemp[OldIndex] == m_Map.m_lImages[NewIndex])
				{
					gs_pSortedIndex[OldIndex] = NewIndex;
					break;
				}
			}
		}
		m_Map.ModifyImageIndex(ModifySortedIndex);

		delete[] gs_pSortedIndex;
		gs_pSortedIndex = 0;
	}
}

CLayerGroup *CEditor::GetSelectedGroup() const
{
	if(m_SelectedGroup >= 0 && m_SelectedGroup < m_Map.m_lGroups.size())
		return m_Map.m_lGroups[m_SelectedGroup];
	return 0x0;
}

CLayer *CEditor::GetSelectedLayer(int Index) const
{
	CLayerGroup *pGroup = GetSelectedGroup();
	if(!pGroup)
		return 0x0;

	if(Index < 0 || Index >= m_lSelectedLayers.size())
		return 0x0;

	int LayerIndex = m_lSelectedLayers[Index];

	if(LayerIndex >= 0 && LayerIndex < m_Map.m_lGroups[m_SelectedGroup]->m_lLayers.size())
		return pGroup->m_lLayers[LayerIndex];
	return 0x0;
}

CLayer *CEditor::GetSelectedLayerType(int Index, int Type) const
{
	CLayer *p = GetSelectedLayer(Index);
	if(p && p->m_Type == Type)
		return p;
	return 0x0;
}

array<CQuad *> CEditor::GetSelectedQuads()
{
	CLayerQuads *ql = (CLayerQuads *)GetSelectedLayerType(0, LAYERTYPE_QUADS);
	array<CQuad *> lQuads;
	if(!ql)
		return lQuads;
	lQuads.set_size(m_lSelectedQuads.size());
	for(int i = 0; i < m_lSelectedQuads.size(); ++i)
		lQuads[i] = &ql->m_lQuads[m_lSelectedQuads[i]];
	return lQuads;
}

CSoundSource *CEditor::GetSelectedSource()
{
	CLayerSounds *pSounds = (CLayerSounds *)GetSelectedLayerType(0, LAYERTYPE_SOUNDS);
	if(!pSounds)
		return 0;
	if(m_SelectedSource >= 0 && m_SelectedSource < pSounds->m_lSources.size())
		return &pSounds->m_lSources[m_SelectedSource];
	return 0;
}

void CEditor::SelectLayer(int LayerIndex, int GroupIndex)
{
	if(GroupIndex != -1)
		m_SelectedGroup = GroupIndex;

	m_lSelectedLayers.clear();
	m_lSelectedLayers.add(LayerIndex);
}

void CEditor::SelectQuad(int Index)
{
	m_lSelectedQuads.clear();
	m_lSelectedQuads.add(Index);
}

void CEditor::SelectGameLayer()
{
	for(int g = 0; g < m_Map.m_lGroups.size(); g++)
	{
		for(int i = 0; i < m_Map.m_lGroups[g]->m_lLayers.size(); i++)
		{
			if(m_Map.m_lGroups[g]->m_lLayers[i] == m_Map.m_pGameLayer)
			{
				SelectLayer(i, g);
				return;
			}
		}
	}
}

void CEditorMap::CreateDefault(IGraphics::CTextureHandle EntitiesTexture)
{
	// add background
	CLayerGroup *pGroup = NewGroup();
	pGroup->m_ParallaxX = 0;
	pGroup->m_ParallaxY = 0;
	CLayerQuads *pLayer = new CLayerQuads;
	pLayer->m_pEditor = m_pEditor;
	CQuad *pQuad = pLayer->NewQuad(0, 0, 1600, 1200);
	pQuad->m_aColors[0].r = pQuad->m_aColors[1].r = 94;
	pQuad->m_aColors[0].g = pQuad->m_aColors[1].g = 132;
	pQuad->m_aColors[0].b = pQuad->m_aColors[1].b = 174;
	pQuad->m_aColors[2].r = pQuad->m_aColors[3].r = 204;
	pQuad->m_aColors[2].g = pQuad->m_aColors[3].g = 232;
	pQuad->m_aColors[2].b = pQuad->m_aColors[3].b = 255;
	pGroup->AddLayer(pLayer);

	// add game layer and reset front, tele, speedup, tune and switch layer pointers
	MakeGameGroup(NewGroup());
	MakeGameLayer(new CLayerGame(50, 50));
	m_pGameGroup->AddLayer(m_pGameLayer);

	m_pFrontLayer = 0x0;
	m_pTeleLayer = 0x0;
	m_pSpeedupLayer = 0x0;
	m_pSwitchLayer = 0x0;
	m_pTuneLayer = 0x0;
}


void CEditor::Init()
{
	// m_pInput = Kernel()->RequestInterface<IInput>();
	// m_pClient = Kernel()->RequestInterface<IClient>();
	// m_pConfig = CreateConfigManager();
	// m_pConsole = CreateConsole(CFGFLAG_CLIENT);
	// m_pGraphics = Kernel()->RequestInterface<IGraphics>();
	// m_pTextRender = Kernel()->RequestInterface<ITextRender>();
	m_pStorage = gs_pStorage;
	// m_pSound = Kernel()->RequestInterface<ISound>();
	// CGameClient *pGameClient = (CGameClient *)Kernel()->RequestInterface<IGameClient>();
	// m_RenderTools.Init(m_pGraphics, &m_UI, pGameClient);
	// m_UI.SetGraphics(m_pGraphics, m_pTextRender);
	// m_Map.m_pEditor = this;

	// m_UIEx.Init(UI(), Kernel(), RenderTools(), Input()->GetEventsRaw(), Input()->GetEventCountRaw());

	// m_CheckerTexture = Graphics()->LoadTexture("editor/checker.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	// m_BackgroundTexture = Graphics()->LoadTexture("editor/background.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	// m_CursorTexture = Graphics()->LoadTexture("editor/cursor.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	// int TextureLoadFlag = Graphics()->HasTextureArrays() ? IGraphics::TEXLOAD_TO_2D_ARRAY_TEXTURE : IGraphics::TEXLOAD_TO_3D_TEXTURE;
	// m_EntitiesTexture = Graphics()->LoadTexture("editor/entities/DDNet.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);

	// m_FrontTexture = Graphics()->LoadTexture("editor/front.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);
	// m_TeleTexture = Graphics()->LoadTexture("editor/tele.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);
	// m_SpeedupTexture = Graphics()->LoadTexture("editor/speedup.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);
	// m_SwitchTexture = Graphics()->LoadTexture("editor/switch.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);
	// m_TuneTexture = Graphics()->LoadTexture("editor/tune.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureLoadFlag);

	// m_TilesetPicker.m_pEditor = this;
	// m_TilesetPicker.MakePalette();
	// m_TilesetPicker.m_Readonly = true;

	// m_QuadsetPicker.m_pEditor = this;
	// m_QuadsetPicker.NewQuad(0, 0, 64, 64);
	// m_QuadsetPicker.m_Readonly = true;

	m_Brush.m_pMap = &m_Map;

	// Reset();
	// m_Map.m_Modified = false;
	// m_Map.m_UndoModified = 0;
	// m_LastUndoUpdateTime = time_get();

	// ms_PickerColor = ColorHSVA(1.0f, 0.0f, 0.0f);
}

void CEditor::UpdateAndRender()
{
}

void CEditorMap::MakeGameLayer(CLayer *pLayer)
{
	m_pGameLayer = (CLayerGame *)pLayer;
	m_pGameLayer->m_pEditor = m_pEditor;
	m_pGameLayer->m_Texture = m_pEditor->m_EntitiesTexture;
}

void CEditorMap::MakeGameGroup(CLayerGroup *pGroup)
{
	m_pGameGroup = pGroup;
	m_pGameGroup->m_GameGroup = true;
	str_copy(m_pGameGroup->m_aName, "Game", sizeof(m_pGameGroup->m_aName));
}

void CEditorMap::Clean()
{
	for(int i = 0; i < m_lGroups.size(); i++)
	{
		m_lGroups[i]->m_lLayers.delete_all();
	}
	m_lGroups.delete_all();
	m_lEnvelopes.delete_all();
	m_lImages.delete_all();
	m_lSounds.delete_all();

	m_MapInfo.Reset();

	m_lSettings.clear();

	m_pGameLayer = 0x0;
	m_pGameGroup = 0x0;

	m_Modified = false;

	m_pTeleLayer = 0x0;
	m_pSpeedupLayer = 0x0;
	m_pFrontLayer = 0x0;
	m_pSwitchLayer = 0x0;
	m_pTuneLayer = 0x0;
}

//

void CEditor::Reset(bool CreateDefault)
{
	m_Map.Clean();

	//delete undo file
	char aBuffer[1024];
	m_pStorage->GetCompletePath(IStorage::TYPE_SAVE, "editor/", aBuffer, sizeof(aBuffer));
	m_lUndoSteps.clear();

	mem_zero(m_apSavedBrushes, sizeof m_apSavedBrushes);

	// create default layers
	if(CreateDefault)
		m_Map.CreateDefault(m_EntitiesTexture);

	SelectGameLayer();
	m_lSelectedQuads.clear();
	m_SelectedPoints = 0;
	m_SelectedEnvelope = 0;
	m_SelectedImage = 0;
	m_SelectedSound = 0;
	m_SelectedSource = -1;

	m_WorldOffsetX = 0;
	m_WorldOffsetY = 0;
	m_EditorOffsetX = 0.0f;
	m_EditorOffsetY = 0.0f;

	m_WorldZoom = 1.0f;
	m_ZoomLevel = 200;

	m_MouseDeltaX = 0;
	m_MouseDeltaY = 0;
	m_MouseDeltaWx = 0;
	m_MouseDeltaWy = 0;

	m_Map.m_Modified = false;
	m_Map.m_UndoModified = 0;
	m_LastUndoUpdateTime = time_get();
	m_UndoRunning = false;

	m_ShowEnvelopePreview = 0;
	m_ShiftBy = 1;

	m_Map.m_Modified = false;
	m_Map.m_UndoModified = 0;
	m_LastUndoUpdateTime = time_get();
}

int CEditor::Save(const char *pFilename)
{
	return m_Map.Save(Kernel()->RequestInterface<IStorage>(), pFilename);
}

int CEditorMap::Save(class IStorage *pStorage, const char *pFileName)
{
	dbg_msg("editor", "saving to '%s'...", pFileName);
	CDataFileWriter df;
	if(!df.Open(pStorage, pFileName))
	{
		dbg_msg("editor", "failed to open file '%s'...", pFileName);
		return 0;
	}

	// save version
	{
		CMapItemVersion Item;
		Item.m_Version = 1;
		df.AddItem(MAPITEMTYPE_VERSION, 0, sizeof(Item), &Item);
	}

	// save map info
	{
		CMapItemInfoSettings Item;
		Item.m_Version = 1;

		if(m_MapInfo.m_aAuthor[0])
			Item.m_Author = df.AddData(str_length(m_MapInfo.m_aAuthor) + 1, m_MapInfo.m_aAuthor);
		else
			Item.m_Author = -1;
		if(m_MapInfo.m_aVersion[0])
			Item.m_MapVersion = df.AddData(str_length(m_MapInfo.m_aVersion) + 1, m_MapInfo.m_aVersion);
		else
			Item.m_MapVersion = -1;
		if(m_MapInfo.m_aCredits[0])
			Item.m_Credits = df.AddData(str_length(m_MapInfo.m_aCredits) + 1, m_MapInfo.m_aCredits);
		else
			Item.m_Credits = -1;
		if(m_MapInfo.m_aLicense[0])
			Item.m_License = df.AddData(str_length(m_MapInfo.m_aLicense) + 1, m_MapInfo.m_aLicense);
		else
			Item.m_License = -1;

		Item.m_Settings = -1;
		if(m_lSettings.size())
		{
			int Size = 0;
			for(int i = 0; i < m_lSettings.size(); i++)
			{
				Size += str_length(m_lSettings[i].m_aCommand) + 1;
			}

			char *pSettings = (char *)malloc(maximum(Size, 1));
			char *pNext = pSettings;
			for(int i = 0; i < m_lSettings.size(); i++)
			{
				int Length = str_length(m_lSettings[i].m_aCommand) + 1;
				mem_copy(pNext, m_lSettings[i].m_aCommand, Length);
				pNext += Length;
			}
			Item.m_Settings = df.AddData(Size, pSettings);
			free(pSettings);
		}

		df.AddItem(MAPITEMTYPE_INFO, 0, sizeof(Item), &Item);
	}

	// save images
	for(int i = 0; i < m_lImages.size(); i++)
	{
		CEditorImage *pImg = m_lImages[i];

		// analyse the image for when saving (should be done when we load the image)
		// TODO!
		pImg->AnalyseTileFlags();

		CMapItemImage Item;
		Item.m_Version = 1;

		Item.m_Width = pImg->m_Width;
		Item.m_Height = pImg->m_Height;
		Item.m_External = pImg->m_External;
		Item.m_ImageName = df.AddData(str_length(pImg->m_aName) + 1, pImg->m_aName);
		if(pImg->m_External)
			Item.m_ImageData = -1;
		else
		{
			if(pImg->m_Format == CImageInfo::FORMAT_RGB)
			{
				// Convert to RGBA
				unsigned char *pDataRGBA = (unsigned char *)malloc((size_t)Item.m_Width * Item.m_Height * 4);
				unsigned char *pDataRGB = (unsigned char *)pImg->m_pData;
				for(int i = 0; i < Item.m_Width * Item.m_Height; i++)
				{
					pDataRGBA[i * 4] = pDataRGB[i * 3];
					pDataRGBA[i * 4 + 1] = pDataRGB[i * 3 + 1];
					pDataRGBA[i * 4 + 2] = pDataRGB[i * 3 + 2];
					pDataRGBA[i * 4 + 3] = 255;
				}
				Item.m_ImageData = df.AddData(Item.m_Width * Item.m_Height * 4, pDataRGBA);
				free(pDataRGBA);
			}
			else
			{
				Item.m_ImageData = df.AddData(Item.m_Width * Item.m_Height * 4, pImg->m_pData);
			}
		}
		df.AddItem(MAPITEMTYPE_IMAGE, i, sizeof(Item), &Item);
	}

	// save sounds
	for(int i = 0; i < m_lSounds.size(); i++)
	{
		CEditorSound *pSound = m_lSounds[i];

		CMapItemSound Item;
		Item.m_Version = 1;

		Item.m_External = 0;
		Item.m_SoundName = df.AddData(str_length(pSound->m_aName) + 1, pSound->m_aName);
		Item.m_SoundData = df.AddData(pSound->m_DataSize, pSound->m_pData);
		Item.m_SoundDataSize = pSound->m_DataSize;

		df.AddItem(MAPITEMTYPE_SOUND, i, sizeof(Item), &Item);
	}

	// save layers
	int LayerCount = 0, GroupCount = 0;
	int AutomapperCount = 0;
	for(int g = 0; g < m_lGroups.size(); g++)
	{
		CLayerGroup *pGroup = m_lGroups[g];
		CMapItemGroup GItem;
		GItem.m_Version = CMapItemGroup::CURRENT_VERSION;

		GItem.m_ParallaxX = pGroup->m_ParallaxX;
		GItem.m_ParallaxY = pGroup->m_ParallaxY;
		GItem.m_OffsetX = pGroup->m_OffsetX;
		GItem.m_OffsetY = pGroup->m_OffsetY;
		GItem.m_UseClipping = pGroup->m_UseClipping;
		GItem.m_ClipX = pGroup->m_ClipX;
		GItem.m_ClipY = pGroup->m_ClipY;
		GItem.m_ClipW = pGroup->m_ClipW;
		GItem.m_ClipH = pGroup->m_ClipH;
		GItem.m_StartLayer = LayerCount;
		GItem.m_NumLayers = 0;

		// save group name
		StrToInts(GItem.m_aName, sizeof(GItem.m_aName) / sizeof(int), pGroup->m_aName);

		for(int l = 0; l < pGroup->m_lLayers.size(); l++)
		{
			if(pGroup->m_lLayers[l]->m_Type == LAYERTYPE_TILES)
			{
				dbg_msg("editor", "saving tiles layer");
				CLayerTiles *pLayer = (CLayerTiles *)pGroup->m_lLayers[l];
				pLayer->PrepareForSave();

				CMapItemLayerTilemap Item;
				Item.m_Version = 3;

				Item.m_Layer.m_Version = 0; // was previously uninitialized, do not rely on it being 0
				Item.m_Layer.m_Flags = pLayer->m_Flags;
				Item.m_Layer.m_Type = pLayer->m_Type;

				Item.m_Color = pLayer->m_Color;
				Item.m_ColorEnv = pLayer->m_ColorEnv;
				Item.m_ColorEnvOffset = pLayer->m_ColorEnvOffset;

				Item.m_Width = pLayer->m_Width;
				Item.m_Height = pLayer->m_Height;
				// Item.m_Flags = pLayer->m_Game ? TILESLAYERFLAG_GAME : 0;

				if(pLayer->m_Tele)
					Item.m_Flags = TILESLAYERFLAG_TELE;
				else if(pLayer->m_Speedup)
					Item.m_Flags = TILESLAYERFLAG_SPEEDUP;
				else if(pLayer->m_Front)
					Item.m_Flags = TILESLAYERFLAG_FRONT;
				else if(pLayer->m_Switch)
					Item.m_Flags = TILESLAYERFLAG_SWITCH;
				else if(pLayer->m_Tune)
					Item.m_Flags = TILESLAYERFLAG_TUNE;
				else
					Item.m_Flags = pLayer->m_Game ? TILESLAYERFLAG_GAME : 0;

				Item.m_Image = pLayer->m_Image;

				// the following values were previously uninitialized, do not rely on them being -1 when unused
				Item.m_Tele = -1;
				Item.m_Speedup = -1;
				Item.m_Front = -1;
				Item.m_Switch = -1;
				Item.m_Tune = -1;

				if(Item.m_Flags && !(pLayer->m_Game))
				{
					CTile *pEmptyTiles = (CTile *)calloc((size_t)pLayer->m_Width * pLayer->m_Height, sizeof(CTile));
					mem_zero(pEmptyTiles, (size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CTile));
					Item.m_Data = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CTile), pEmptyTiles);
					free(pEmptyTiles);

					if(pLayer->m_Tele)
						Item.m_Tele = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CTeleTile), ((CLayerTele *)pLayer)->m_pTeleTile);
					else if(pLayer->m_Speedup)
						Item.m_Speedup = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CSpeedupTile), ((CLayerSpeedup *)pLayer)->m_pSpeedupTile);
					else if(pLayer->m_Front)
						Item.m_Front = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CTile), pLayer->m_pTiles);
					else if(pLayer->m_Switch)
						Item.m_Switch = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CSwitchTile), ((CLayerSwitch *)pLayer)->m_pSwitchTile);
					else if(pLayer->m_Tune)
						Item.m_Tune = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CTuneTile), ((CLayerTune *)pLayer)->m_pTuneTile);
				}
				else
					Item.m_Data = df.AddData((size_t)pLayer->m_Width * pLayer->m_Height * sizeof(CTile), pLayer->m_pTiles);

				// save layer name
				StrToInts(Item.m_aName, sizeof(Item.m_aName) / sizeof(int), pLayer->m_aName);

				df.AddItem(MAPITEMTYPE_LAYER, LayerCount, sizeof(Item), &Item);

				// save auto mapper of each tile layer (not physics layer)
				if(!Item.m_Flags)
				{
					CMapItemAutoMapperConfig ItemAutomapper;
					ItemAutomapper.m_Version = CMapItemAutoMapperConfig::CURRENT_VERSION;
					ItemAutomapper.m_GroupId = GroupCount;
					ItemAutomapper.m_LayerId = GItem.m_NumLayers;
					ItemAutomapper.m_AutomapperConfig = pLayer->m_AutoMapperConfig;
					ItemAutomapper.m_AutomapperSeed = pLayer->m_Seed;
					ItemAutomapper.m_Flags = 0;
					if(pLayer->m_AutoAutoMap)
						ItemAutomapper.m_Flags |= CMapItemAutoMapperConfig::FLAG_AUTOMATIC;

					df.AddItem(MAPITEMTYPE_AUTOMAPPER_CONFIG, AutomapperCount, sizeof(ItemAutomapper), &ItemAutomapper);
					AutomapperCount++;
				}

				GItem.m_NumLayers++;
				LayerCount++;
			}
			else if(pGroup->m_lLayers[l]->m_Type == LAYERTYPE_QUADS)
			{
				dbg_msg("editor", "saving quads layer");
				CLayerQuads *pLayer = (CLayerQuads *)pGroup->m_lLayers[l];
				if(pLayer->m_lQuads.size())
				{
					CMapItemLayerQuads Item;
					Item.m_Version = 2;
					Item.m_Layer.m_Version = 0; // was previously uninitialized, do not rely on it being 0
					Item.m_Layer.m_Flags = pLayer->m_Flags;
					Item.m_Layer.m_Type = pLayer->m_Type;
					Item.m_Image = pLayer->m_Image;

					// add the data
					Item.m_NumQuads = pLayer->m_lQuads.size();
					Item.m_Data = df.AddDataSwapped(pLayer->m_lQuads.size() * sizeof(CQuad), pLayer->m_lQuads.base_ptr());

					// save layer name
					StrToInts(Item.m_aName, sizeof(Item.m_aName) / sizeof(int), pLayer->m_aName);

					df.AddItem(MAPITEMTYPE_LAYER, LayerCount, sizeof(Item), &Item);

					// clean up
					//mem_free(quads);

					GItem.m_NumLayers++;
					LayerCount++;
				}
			}
			else if(pGroup->m_lLayers[l]->m_Type == LAYERTYPE_SOUNDS)
			{
				dbg_msg("editor", "saving sounds layer");
				CLayerSounds *pLayer = (CLayerSounds *)pGroup->m_lLayers[l];
				if(pLayer->m_lSources.size())
				{
					CMapItemLayerSounds Item;
					Item.m_Version = CMapItemLayerSounds::CURRENT_VERSION;
					Item.m_Layer.m_Version = 0; // was previously uninitialized, do not rely on it being 0
					Item.m_Layer.m_Flags = pLayer->m_Flags;
					Item.m_Layer.m_Type = pLayer->m_Type;
					Item.m_Sound = pLayer->m_Sound;

					// add the data
					Item.m_NumSources = pLayer->m_lSources.size();
					Item.m_Data = df.AddDataSwapped(pLayer->m_lSources.size() * sizeof(CSoundSource), pLayer->m_lSources.base_ptr());

					// save layer name
					StrToInts(Item.m_aName, sizeof(Item.m_aName) / sizeof(int), pLayer->m_aName);

					df.AddItem(MAPITEMTYPE_LAYER, LayerCount, sizeof(Item), &Item);
					GItem.m_NumLayers++;
					LayerCount++;
				}
			}
		}

		df.AddItem(MAPITEMTYPE_GROUP, GroupCount++, sizeof(GItem), &GItem);
	}

	// save envelopes
	int PointCount = 0;
	for(int e = 0; e < m_lEnvelopes.size(); e++)
	{
		CMapItemEnvelope Item;
		Item.m_Version = CMapItemEnvelope::CURRENT_VERSION;
		Item.m_Channels = m_lEnvelopes[e]->m_Channels;
		Item.m_StartPoint = PointCount;
		Item.m_NumPoints = m_lEnvelopes[e]->m_lPoints.size();
		Item.m_Synchronized = m_lEnvelopes[e]->m_Synchronized;
		StrToInts(Item.m_aName, sizeof(Item.m_aName) / sizeof(int), m_lEnvelopes[e]->m_aName);

		df.AddItem(MAPITEMTYPE_ENVELOPE, e, sizeof(Item), &Item);
		PointCount += Item.m_NumPoints;
	}

	// save points
	int TotalSize = sizeof(CEnvPoint) * PointCount;
	CEnvPoint *pPoints = (CEnvPoint *)calloc(maximum(PointCount, 1), sizeof(*pPoints));
	PointCount = 0;

	for(int e = 0; e < m_lEnvelopes.size(); e++)
	{
		int Count = m_lEnvelopes[e]->m_lPoints.size();
		mem_copy(&pPoints[PointCount], m_lEnvelopes[e]->m_lPoints.base_ptr(), sizeof(CEnvPoint) * Count);
		PointCount += Count;
	}

	df.AddItem(MAPITEMTYPE_ENVPOINTS, 0, TotalSize, pPoints);
	free(pPoints);

	// finish the data file
	df.Finish();
	dbg_msg("editor", "saving done");
	return 1;
}

int CEditor::Load(const char *pFileName, int StorageType)
{
	Reset();
	int result = m_Map.Load(Kernel()->RequestInterface<IStorage>(), pFileName, StorageType);
	if(result)
	{
		str_copy(m_aFileName, pFileName, 512);
		SortImages();
		SelectGameLayer();
	}
	else
	{
		m_aFileName[0] = 0;
		Reset();
	}
	return result;
}

int CEditorMap::Load(class IStorage *pStorage, const char *pFileName, int StorageType)
{
	CDataFileReader DataFile;
	//DATAFILE *df = datafile_load(filename);
	if(!DataFile.Open(pStorage, pFileName, StorageType))
		return 0;

	Clean();

	// check version
	CMapItemVersion *pItem = (CMapItemVersion *)DataFile.FindItem(MAPITEMTYPE_VERSION, 0);
	if(!pItem)
	{
		// import old map
		/*MAP old_mapstuff;
		editor->reset();
		editor_load_old(df, this);
		*/
		return 0;
	}
	else if(pItem->m_Version == 1)
	{
		//editor.reset(false);

		// load map info
		{
			int Start, Num;
			DataFile.GetType(MAPITEMTYPE_INFO, &Start, &Num);
			for(int i = Start; i < Start + Num; i++)
			{
				int ItemSize = DataFile.GetItemSize(Start);
				int ItemID;
				CMapItemInfoSettings *pItem = (CMapItemInfoSettings *)DataFile.GetItem(i, 0, &ItemID);
				if(!pItem || ItemID != 0)
					continue;

				if(pItem->m_Author > -1)
					str_copy(m_MapInfo.m_aAuthor, (char *)DataFile.GetData(pItem->m_Author), sizeof(m_MapInfo.m_aAuthor));
				if(pItem->m_MapVersion > -1)
					str_copy(m_MapInfo.m_aVersion, (char *)DataFile.GetData(pItem->m_MapVersion), sizeof(m_MapInfo.m_aVersion));
				if(pItem->m_Credits > -1)
					str_copy(m_MapInfo.m_aCredits, (char *)DataFile.GetData(pItem->m_Credits), sizeof(m_MapInfo.m_aCredits));
				if(pItem->m_License > -1)
					str_copy(m_MapInfo.m_aLicense, (char *)DataFile.GetData(pItem->m_License), sizeof(m_MapInfo.m_aLicense));

				if(pItem->m_Version != 1 || ItemSize < (int)sizeof(CMapItemInfoSettings))
					break;

				if(!(pItem->m_Settings > -1))
					break;

				const unsigned Size = DataFile.GetDataSize(pItem->m_Settings);
				char *pSettings = (char *)DataFile.GetData(pItem->m_Settings);
				char *pNext = pSettings;
				while(pNext < pSettings + Size)
				{
					int StrSize = str_length(pNext) + 1;
					CSetting Setting;
					str_copy(Setting.m_aCommand, pNext, sizeof(Setting.m_aCommand));
					m_lSettings.add(Setting);
					pNext += StrSize;
				}
			}
		}

		// load images
		{
			int Start, Num;
			DataFile.GetType(MAPITEMTYPE_IMAGE, &Start, &Num);
			for(int i = 0; i < Num; i++)
			{
				CMapItemImage *pItem = (CMapItemImage *)DataFile.GetItem(Start + i, 0, 0);
				char *pName = (char *)DataFile.GetData(pItem->m_ImageName);

				// copy base info
				CEditorImage *pImg = new CEditorImage(m_pEditor);
				pImg->m_External = pItem->m_External;

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "mapres/%s.png", pName);
				dbg_msg("chiller", "skipping png load %s", aBuf);

				// copy image name
				if(pName)
					str_copy(pImg->m_aName, pName, 128);

				// load auto mapper file
				pImg->m_AutoMapper.Load(pImg->m_aName);

				m_lImages.add(pImg);

				// unload image
				DataFile.UnloadData(pItem->m_ImageData);
				DataFile.UnloadData(pItem->m_ImageName);
			}
		}

		// load sounds
		{
			int Start, Num;
			DataFile.GetType(MAPITEMTYPE_SOUND, &Start, &Num);
			for(int i = 0; i < Num; i++)
			{
				CMapItemSound *pItem = (CMapItemSound *)DataFile.GetItem(Start + i, 0, 0);
				char *pName = (char *)DataFile.GetData(pItem->m_SoundName);

				// copy base info
				CEditorSound *pSound = new CEditorSound(m_pEditor);

				if(pItem->m_External)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "mapres/%s.opus", pName);

					// load external
					IOHANDLE SoundFile = pStorage->OpenFile(pName, IOFLAG_READ, IStorage::TYPE_ALL);
					if(SoundFile)
					{
						// read the whole file into memory
						pSound->m_DataSize = io_length(SoundFile);

						if(pSound->m_DataSize > 0)
						{
							pSound->m_pData = malloc(pSound->m_DataSize);
							io_read(SoundFile, pSound->m_pData, pSound->m_DataSize);
						}
						io_close(SoundFile);
						if(pSound->m_DataSize > 0)
						{
							pSound->m_SoundID = m_pEditor->Sound()->LoadOpusFromMem(pSound->m_pData, pSound->m_DataSize, true);
						}
					}
				}
				else
				{
					pSound->m_DataSize = pItem->m_SoundDataSize;

					// copy sample data
					void *pData = DataFile.GetData(pItem->m_SoundData);
					pSound->m_pData = malloc(pSound->m_DataSize);
					mem_copy(pSound->m_pData, pData, pSound->m_DataSize);
					pSound->m_SoundID = m_pEditor->Sound()->LoadOpusFromMem(pSound->m_pData, pSound->m_DataSize, true);
				}

				// copy image name
				if(pName)
					str_copy(pSound->m_aName, pName, sizeof(pSound->m_aName));

				m_lSounds.add(pSound);

				// unload image
				DataFile.UnloadData(pItem->m_SoundData);
				DataFile.UnloadData(pItem->m_SoundName);
			}
		}

		// load groups
		{
			int LayersStart, LayersNum;
			DataFile.GetType(MAPITEMTYPE_LAYER, &LayersStart, &LayersNum);

			int Start, Num;
			DataFile.GetType(MAPITEMTYPE_GROUP, &Start, &Num);
			for(int g = 0; g < Num; g++)
			{
				CMapItemGroup *pGItem = (CMapItemGroup *)DataFile.GetItem(Start + g, 0, 0);

				if(pGItem->m_Version < 1 || pGItem->m_Version > CMapItemGroup::CURRENT_VERSION)
					continue;

				CLayerGroup *pGroup = NewGroup();
				pGroup->m_ParallaxX = pGItem->m_ParallaxX;
				pGroup->m_ParallaxY = pGItem->m_ParallaxY;
				pGroup->m_OffsetX = pGItem->m_OffsetX;
				pGroup->m_OffsetY = pGItem->m_OffsetY;

				if(pGItem->m_Version >= 2)
				{
					pGroup->m_UseClipping = pGItem->m_UseClipping;
					pGroup->m_ClipX = pGItem->m_ClipX;
					pGroup->m_ClipY = pGItem->m_ClipY;
					pGroup->m_ClipW = pGItem->m_ClipW;
					pGroup->m_ClipH = pGItem->m_ClipH;
				}

				// load group name
				if(pGItem->m_Version >= 3)
					IntsToStr(pGItem->m_aName, sizeof(pGroup->m_aName) / sizeof(int), pGroup->m_aName);

				for(int l = 0; l < pGItem->m_NumLayers; l++)
				{
					CLayer *pLayer = 0;
					CMapItemLayer *pLayerItem = (CMapItemLayer *)DataFile.GetItem(LayersStart + pGItem->m_StartLayer + l, 0, 0);
					if(!pLayerItem)
						continue;

					if(pLayerItem->m_Type == LAYERTYPE_TILES)
					{
						CMapItemLayerTilemap *pTilemapItem = (CMapItemLayerTilemap *)pLayerItem;
						CLayerTiles *pTiles = 0;

						if(pTilemapItem->m_Flags & TILESLAYERFLAG_GAME)
						{
							pTiles = new CLayerGame(pTilemapItem->m_Width, pTilemapItem->m_Height);
							MakeGameLayer(pTiles);
							MakeGameGroup(pGroup);
						}
						else if(pTilemapItem->m_Flags & TILESLAYERFLAG_TELE)
						{
							if(pTilemapItem->m_Version <= 2)
								pTilemapItem->m_Tele = *((int *)(pTilemapItem) + 15);

							pTiles = new CLayerTele(pTilemapItem->m_Width, pTilemapItem->m_Height);
							MakeTeleLayer(pTiles);
						}
						else if(pTilemapItem->m_Flags & TILESLAYERFLAG_SPEEDUP)
						{
							if(pTilemapItem->m_Version <= 2)
								pTilemapItem->m_Speedup = *((int *)(pTilemapItem) + 16);

							pTiles = new CLayerSpeedup(pTilemapItem->m_Width, pTilemapItem->m_Height);
							MakeSpeedupLayer(pTiles);
						}
						else if(pTilemapItem->m_Flags & TILESLAYERFLAG_FRONT)
						{
							if(pTilemapItem->m_Version <= 2)
								pTilemapItem->m_Front = *((int *)(pTilemapItem) + 17);

							pTiles = new CLayerFront(pTilemapItem->m_Width, pTilemapItem->m_Height);
							MakeFrontLayer(pTiles);
						}
						else if(pTilemapItem->m_Flags & TILESLAYERFLAG_SWITCH)
						{
							if(pTilemapItem->m_Version <= 2)
								pTilemapItem->m_Switch = *((int *)(pTilemapItem) + 18);

							pTiles = new CLayerSwitch(pTilemapItem->m_Width, pTilemapItem->m_Height);
							MakeSwitchLayer(pTiles);
						}
						else if(pTilemapItem->m_Flags & TILESLAYERFLAG_TUNE)
						{
							if(pTilemapItem->m_Version <= 2)
								pTilemapItem->m_Tune = *((int *)(pTilemapItem) + 19);

							pTiles = new CLayerTune(pTilemapItem->m_Width, pTilemapItem->m_Height);
							MakeTuneLayer(pTiles);
						}
						else
						{
							pTiles = new CLayerTiles(pTilemapItem->m_Width, pTilemapItem->m_Height);
							pTiles->m_pEditor = m_pEditor;
							pTiles->m_Color = pTilemapItem->m_Color;
							pTiles->m_ColorEnv = pTilemapItem->m_ColorEnv;
							pTiles->m_ColorEnvOffset = pTilemapItem->m_ColorEnvOffset;
						}

						pLayer = pTiles;

						pGroup->AddLayer(pTiles);
						void *pData = DataFile.GetData(pTilemapItem->m_Data);
						unsigned int Size = DataFile.GetDataSize(pTilemapItem->m_Data);
						pTiles->m_Image = pTilemapItem->m_Image;
						pTiles->m_Game = pTilemapItem->m_Flags & TILESLAYERFLAG_GAME;

						// load layer name
						if(pTilemapItem->m_Version >= 3)
							IntsToStr(pTilemapItem->m_aName, sizeof(pTiles->m_aName) / sizeof(int), pTiles->m_aName);

						if(pTiles->m_Tele)
						{
							void *pTeleData = DataFile.GetData(pTilemapItem->m_Tele);
							unsigned int Size = DataFile.GetDataSize(pTilemapItem->m_Tele);
							if(Size >= (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTeleTile))
							{
								static const int s_aTilesRep[] = {
									TILE_TELEIN,
									TILE_TELEINEVIL,
									TILE_TELEOUT,
									TILE_TELECHECK,
									TILE_TELECHECKIN,
									TILE_TELECHECKINEVIL,
									TILE_TELECHECKOUT,
									TILE_TELEINWEAPON,
									TILE_TELEINHOOK};
								mem_copy(((CLayerTele *)pTiles)->m_pTeleTile, pTeleData, (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTeleTile));

								for(int i = 0; i < pTiles->m_Width * pTiles->m_Height; i++)
								{
									pTiles->m_pTiles[i].m_Index = 0;
									for(int TilesRep : s_aTilesRep)
									{
										if(((CLayerTele *)pTiles)->m_pTeleTile[i].m_Type == TilesRep)
											pTiles->m_pTiles[i].m_Index = TilesRep;
									}
								}
							}
							DataFile.UnloadData(pTilemapItem->m_Tele);
						}
						else if(pTiles->m_Speedup)
						{
							void *pSpeedupData = DataFile.GetData(pTilemapItem->m_Speedup);
							unsigned int Size = DataFile.GetDataSize(pTilemapItem->m_Speedup);

							if(Size >= (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CSpeedupTile))
							{
								mem_copy(((CLayerSpeedup *)pTiles)->m_pSpeedupTile, pSpeedupData, (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CSpeedupTile));

								for(int i = 0; i < pTiles->m_Width * pTiles->m_Height; i++)
								{
									if(((CLayerSpeedup *)pTiles)->m_pSpeedupTile[i].m_Force > 0)
										pTiles->m_pTiles[i].m_Index = TILE_BOOST;
									else
										pTiles->m_pTiles[i].m_Index = 0;
								}
							}

							DataFile.UnloadData(pTilemapItem->m_Speedup);
						}
						else if(pTiles->m_Front)
						{
							void *pFrontData = DataFile.GetData(pTilemapItem->m_Front);
							unsigned int Size = DataFile.GetDataSize(pTilemapItem->m_Front);
							if(Size >= (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTile))
								mem_copy(((CLayerFront *)pTiles)->m_pTiles, pFrontData, (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTile));

							DataFile.UnloadData(pTilemapItem->m_Front);
						}
						else if(pTiles->m_Switch)
						{
							void *pSwitchData = DataFile.GetData(pTilemapItem->m_Switch);
							unsigned int Size = DataFile.GetDataSize(pTilemapItem->m_Switch);
							if(Size >= (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CSwitchTile))
							{
								const int s_aTilesComp[] = {
									TILE_SWITCHTIMEDOPEN,
									TILE_SWITCHTIMEDCLOSE,
									TILE_SWITCHOPEN,
									TILE_SWITCHCLOSE,
									TILE_FREEZE,
									TILE_DFREEZE,
									TILE_DUNFREEZE,
									TILE_HIT_ENABLE,
									TILE_HIT_DISABLE,
									TILE_JUMP,
									TILE_ADD_TIME,
									TILE_SUBTRACT_TIME,
									TILE_ALLOW_TELE_GUN,
									TILE_ALLOW_BLUE_TELE_GUN};
								CSwitchTile *pLayerSwitchTiles = ((CLayerSwitch *)pTiles)->m_pSwitchTile;
								mem_copy(((CLayerSwitch *)pTiles)->m_pSwitchTile, pSwitchData, (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CSwitchTile));

								for(int i = 0; i < pTiles->m_Width * pTiles->m_Height; i++)
								{
									if(((pLayerSwitchTiles[i].m_Type > (ENTITY_CRAZY_SHOTGUN + ENTITY_OFFSET) && ((CLayerSwitch *)pTiles)->m_pSwitchTile[i].m_Type < (ENTITY_DRAGGER_WEAK + ENTITY_OFFSET)) || ((CLayerSwitch *)pTiles)->m_pSwitchTile[i].m_Type == (ENTITY_LASER_O_FAST + 1 + ENTITY_OFFSET)))
										continue;
									else if(pLayerSwitchTiles[i].m_Type >= (ENTITY_ARMOR_1 + ENTITY_OFFSET) && pLayerSwitchTiles[i].m_Type <= (ENTITY_DOOR + ENTITY_OFFSET))
									{
										pTiles->m_pTiles[i].m_Index = pLayerSwitchTiles[i].m_Type;
										pTiles->m_pTiles[i].m_Flags = pLayerSwitchTiles[i].m_Flags;
										continue;
									}

									for(int TilesComp : s_aTilesComp)
									{
										if(pLayerSwitchTiles[i].m_Type == TilesComp)
										{
											pTiles->m_pTiles[i].m_Index = TilesComp;
											pTiles->m_pTiles[i].m_Flags = pLayerSwitchTiles[i].m_Flags;
										}
									}
								}
							}
							DataFile.UnloadData(pTilemapItem->m_Switch);
						}
						else if(pTiles->m_Tune)
						{
							void *pTuneData = DataFile.GetData(pTilemapItem->m_Tune);
							unsigned int Size = DataFile.GetDataSize(pTilemapItem->m_Tune);
							if(Size >= (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTuneTile))
							{
								CTuneTile *pLayerTuneTiles = ((CLayerTune *)pTiles)->m_pTuneTile;
								mem_copy(((CLayerTune *)pTiles)->m_pTuneTile, pTuneData, (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTuneTile));

								for(int i = 0; i < pTiles->m_Width * pTiles->m_Height; i++)
								{
									if(pLayerTuneTiles[i].m_Type == TILE_TUNE)
										pTiles->m_pTiles[i].m_Index = TILE_TUNE;
									else
										pTiles->m_pTiles[i].m_Index = 0;
								}
							}
							DataFile.UnloadData(pTilemapItem->m_Tune);
						}
						else // regular tile layer or game layer
						{
							if(Size >= (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTile))
							{
								mem_copy(pTiles->m_pTiles, pData, (size_t)pTiles->m_Width * pTiles->m_Height * sizeof(CTile));

								if(pTiles->m_Game && pTilemapItem->m_Version == MakeVersion(1, *pTilemapItem))
								{
									for(int i = 0; i < pTiles->m_Width * pTiles->m_Height; i++)
									{
										if(pTiles->m_pTiles[i].m_Index)
											pTiles->m_pTiles[i].m_Index += ENTITY_OFFSET;
									}
								}
							}
						}

						DataFile.UnloadData(pTilemapItem->m_Data);

						// Remove unused tiles on game and front layers
						/*if(pTiles->m_Game)
						{
							for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
							{
								if(!IsValidGameTile(pTiles->m_pTiles[i].m_Index))
								{
									if(pTiles->m_pTiles[i].m_Index) {
										char aBuf[256];
										str_format(aBuf, sizeof(aBuf), "game layer, tile %d", pTiles->m_pTiles[i].m_Index);
										dbg_msg("editor", aBuf);
										Changed = true;
									}
									pTiles->m_pTiles[i].m_Index = 0;
									pTiles->m_pTiles[i].m_Flags = 0;
								}
							}
						}
						else if(pTiles->m_Front)
						{
							for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
							{
								if(!IsValidFrontTile(pTiles->m_pTiles[i].m_Index))
								{
									if(pTiles->m_pTiles[i].m_Index) {
										char aBuf[256];
										str_format(aBuf, sizeof(aBuf), "front layer, tile %d", pTiles->m_pTiles[i].m_Index);
										dbg_msg("editor", aBuf);
										Changed = true;
									}
									pTiles->m_pTiles[i].m_Index = 0;
									pTiles->m_pTiles[i].m_Flags = 0;
								}
							}
						}
						else if(pTiles->m_Tele)
						{
							for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
							{
								if(!IsValidTeleTile(pTiles->m_pTiles[i].m_Index))
								{
									if(pTiles->m_pTiles[i].m_Index) {
										char aBuf[256];
										str_format(aBuf, sizeof(aBuf), "tele layer, tile %d", pTiles->m_pTiles[i].m_Index);
										dbg_msg("editor", aBuf);
										Changed = true;
									}
									pTiles->m_pTiles[i].m_Index = 0;
									pTiles->m_pTiles[i].m_Flags = 0;
								}
							}
						}
						else if(pTiles->m_Speedup)
						{
							for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
							{
								if(!IsValidSpeedupTile(pTiles->m_pTiles[i].m_Index))
								{
									if(pTiles->m_pTiles[i].m_Index) {
										char aBuf[256];
										str_format(aBuf, sizeof(aBuf), "speedup layer, tile %d", pTiles->m_pTiles[i].m_Index);
										dbg_msg("editor", aBuf);
										Changed = true;
									}
									pTiles->m_pTiles[i].m_Index = 0;
									pTiles->m_pTiles[i].m_Flags = 0;
								}
							}
						}
						else if(pTiles->m_Switch)
						{
							for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
							{
								if(!IsValidSwitchTile(pTiles->m_pTiles[i].m_Index))
								{
									if(pTiles->m_pTiles[i].m_Index) {
										char aBuf[256];
										str_format(aBuf, sizeof(aBuf), "switch layer, tile %d", pTiles->m_pTiles[i].m_Index);
										dbg_msg("editor", aBuf);
										Changed = true;
									}
									pTiles->m_pTiles[i].m_Index = 0;
									pTiles->m_pTiles[i].m_Flags = 0;
								}
							}
						}*/

						// Convert race stoppers to ddrace stoppers
						/*if(pTiles->m_Game)
						{
							for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
							{
								if(pTiles->m_pTiles[i].m_Index == 29)
								{
									pTiles->m_pTiles[i].m_Index = 60;
									pTiles->m_pTiles[i].m_Flags = TILEFLAG_HFLIP|TILEFLAG_VFLIP|TILEFLAG_ROTATE;
								}
								else if(pTiles->m_pTiles[i].m_Index == 30)
								{
									pTiles->m_pTiles[i].m_Index = 60;
									pTiles->m_pTiles[i].m_Flags = TILEFLAG_ROTATE;
								}
								else if(pTiles->m_pTiles[i].m_Index == 31)
								{
									pTiles->m_pTiles[i].m_Index = 60;
									pTiles->m_pTiles[i].m_Flags = TILEFLAG_HFLIP|TILEFLAG_VFLIP;
								}
								else if(pTiles->m_pTiles[i].m_Index == 32)
								{
									pTiles->m_pTiles[i].m_Index = 60;
									pTiles->m_pTiles[i].m_Flags = 0;
								}
							}
						}*/
					}
					else if(pLayerItem->m_Type == LAYERTYPE_QUADS)
					{
						CMapItemLayerQuads *pQuadsItem = (CMapItemLayerQuads *)pLayerItem;
						CLayerQuads *pQuads = new CLayerQuads;
						pQuads->m_pEditor = m_pEditor;
						pLayer = pQuads;
						pQuads->m_Image = pQuadsItem->m_Image;
						if(pQuads->m_Image < -1 || pQuads->m_Image >= m_lImages.size())
							pQuads->m_Image = -1;

						// load layer name
						if(pQuadsItem->m_Version >= 2)
							IntsToStr(pQuadsItem->m_aName, sizeof(pQuads->m_aName) / sizeof(int), pQuads->m_aName);

						void *pData = DataFile.GetDataSwapped(pQuadsItem->m_Data);
						pGroup->AddLayer(pQuads);
						pQuads->m_lQuads.set_size(pQuadsItem->m_NumQuads);
						mem_copy(pQuads->m_lQuads.base_ptr(), pData, sizeof(CQuad) * pQuadsItem->m_NumQuads);
						DataFile.UnloadData(pQuadsItem->m_Data);
					}
					else if(pLayerItem->m_Type == LAYERTYPE_SOUNDS)
					{
						CMapItemLayerSounds *pSoundsItem = (CMapItemLayerSounds *)pLayerItem;
						if(pSoundsItem->m_Version < 1 || pSoundsItem->m_Version > CMapItemLayerSounds::CURRENT_VERSION)
							continue;

						CLayerSounds *pSounds = new CLayerSounds;
						pSounds->m_pEditor = m_pEditor;
						pLayer = pSounds;
						pSounds->m_Sound = pSoundsItem->m_Sound;

						// validate m_Sound
						if(pSounds->m_Sound < -1 || pSounds->m_Sound >= m_lSounds.size())
							pSounds->m_Sound = -1;

						// load layer name
						if(pSoundsItem->m_Version >= 1)
							IntsToStr(pSoundsItem->m_aName, sizeof(pSounds->m_aName) / sizeof(int), pSounds->m_aName);

						// load data
						void *pData = DataFile.GetDataSwapped(pSoundsItem->m_Data);
						pGroup->AddLayer(pSounds);
						pSounds->m_lSources.set_size(pSoundsItem->m_NumSources);
						mem_copy(pSounds->m_lSources.base_ptr(), pData, sizeof(CSoundSource) * pSoundsItem->m_NumSources);
						DataFile.UnloadData(pSoundsItem->m_Data);
					}

					if(pLayer)
						pLayer->m_Flags = pLayerItem->m_Flags;
				}
			}
		}

		// load envelopes
		{
			CEnvPoint *pPoints = 0;

			{
				int Start, Num;
				DataFile.GetType(MAPITEMTYPE_ENVPOINTS, &Start, &Num);
				if(Num)
					pPoints = (CEnvPoint *)DataFile.GetItem(Start, 0, 0);
			}

			int Start, Num;
			DataFile.GetType(MAPITEMTYPE_ENVELOPE, &Start, &Num);
			for(int e = 0; e < Num; e++)
			{
				CMapItemEnvelope *pItem = (CMapItemEnvelope *)DataFile.GetItem(Start + e, 0, 0);
				CEnvelope *pEnv = new CEnvelope(pItem->m_Channels);
				pEnv->m_lPoints.set_size(pItem->m_NumPoints);
				mem_copy(pEnv->m_lPoints.base_ptr(), &pPoints[pItem->m_StartPoint], sizeof(CEnvPoint) * pItem->m_NumPoints);
				if(pItem->m_aName[0] != -1) // compatibility with old maps
					IntsToStr(pItem->m_aName, sizeof(pItem->m_aName) / sizeof(int), pEnv->m_aName);
				m_lEnvelopes.add(pEnv);
				if(pItem->m_Version >= 2)
					pEnv->m_Synchronized = pItem->m_Synchronized;
			}
		}

		{
			int Start, Num;
			DataFile.GetType(MAPITEMTYPE_AUTOMAPPER_CONFIG, &Start, &Num);
			for(int i = 0; i < Num; i++)
			{
				CMapItemAutoMapperConfig *pItem = (CMapItemAutoMapperConfig *)DataFile.GetItem(Start + i, 0, 0);
				if(pItem->m_Version == CMapItemAutoMapperConfig::CURRENT_VERSION)
				{
					if(pItem->m_GroupId >= 0 && pItem->m_GroupId < m_lGroups.size() &&
						pItem->m_LayerId >= 0 && pItem->m_LayerId < m_lGroups[pItem->m_GroupId]->m_lLayers.size())
					{
						CLayer *pLayer = m_lGroups[pItem->m_GroupId]->m_lLayers[pItem->m_LayerId];
						if(pLayer->m_Type == LAYERTYPE_TILES)
						{
							CLayerTiles *pLayer = (CLayerTiles *)m_lGroups[pItem->m_GroupId]->m_lLayers[pItem->m_LayerId];
							// only load auto mappers for tile layers (not physics layers)
							if(!(pLayer->m_Game || pLayer->m_Tele || pLayer->m_Speedup ||
								   pLayer->m_Front || pLayer->m_Switch || pLayer->m_Tune))
							{
								pLayer->m_AutoMapperConfig = pItem->m_AutomapperConfig;
								pLayer->m_Seed = pItem->m_AutomapperSeed;
								pLayer->m_AutoAutoMap = !!(pItem->m_Flags & CMapItemAutoMapperConfig::FLAG_AUTOMATIC);
							}
						}
					}
				}
			}
		}
	}
	else
		return 0;

	m_Modified = false;
	return 1;
}

// ui.cpp

CUI::CUI()
{
	m_pHotItem = 0;
	m_pActiveItem = 0;
	m_pLastActiveItem = 0;
	m_pBecommingHotItem = 0;

	m_MouseX = 0;
	m_MouseY = 0;
	m_MouseWorldX = 0;
	m_MouseWorldY = 0;
	m_MouseButtons = 0;
	m_LastMouseButtons = 0;

	m_Screen.x = 0;
	m_Screen.y = 0;
	m_Screen.w = 848.0f;
	m_Screen.h = 480.0f;
}

CUI::~CUI()
{
	for(CUIElement *&pEl : m_OwnUIElements)
	{
		delete pEl;
	}
	m_OwnUIElements.clear();
}

#include <game/editor/auto_map.cpp>
#include <game/editor/layer_game.cpp>

int main(int argc, const char **argv)
{
	dbg_logger_stdout();


	// if(argc < 2 || argc > 3)
	// {
	// 	dbg_msg("map_convert_07", "Invalid arguments");
	// 	dbg_msg("map_convert_07", "Usage: map_convert_07 <source map filepath> [<dest map filepath>]");
	// 	return -1;
	// }

	gs_pStorage = CreateStorage("Teeworlds", IStorage::STORAGETYPE_BASIC, argc, argv);

	if(!gs_pStorage)
	{
		dbg_msg("map_convert_07", "error loading storage");
		return -1;
	}

	CEditor m_Editor;
	m_Editor.Init();
	CEditorMap m_Map;
	m_Map.m_pEditor = &m_Editor;
	m_Map.Load(gs_pStorage, "data/maps/ctf1.map", IStorage::TYPE_ALL);

	return 0;
}
