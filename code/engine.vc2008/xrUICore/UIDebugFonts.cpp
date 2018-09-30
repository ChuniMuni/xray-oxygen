#include "StdAfx.h"
#include "UIDebugFonts.h"
#include "UIDialogHolder.h"
#include "dinput.h"


CUIDebugFonts::CUIDebugFonts()
{
	AttachChild(&m_background);
	InitDebugFonts(Frect().set(0.0f, 0.0f, UI_BASE_WIDTH, UI_BASE_HEIGHT));
}

CUIDebugFonts::~CUIDebugFonts()
{
}

void CUIDebugFonts::InitDebugFonts(Frect r)
{
	CUIDialogWnd::SetWndRect(r);

	FillUpList();

	m_background.SetWndRect	(r);
	m_background.InitTexture("ui\\ui_debug_font");
}

bool CUIDebugFonts::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	switch (dik)
	{
	case DIK_ESCAPE: HideDialog();	break;
	case DIK_F12: return false;		break;
	}

    return true;
}
#include "../xrEngine/string_table.h"

void CUIDebugFonts::FillUpList()
{
	Fvector2 pos, sz;
	pos.set(0.0f, 0.0f);
	sz.set(UI_BASE_WIDTH, UI_BASE_HEIGHT);
	string256 str;
	for (CGameFont** ppFont : UI().Font().m_all_fonts)
	{
		if (!ppFont || !*ppFont)
			continue;

		xr_sprintf(str, "%s:%s", (*ppFont)->m_font_name.c_str(), CStringTable().translate("Test_Font_String").c_str());

		CUITextWnd* pItem = xr_new<CUITextWnd>();
		pItem->SetWndPos			(pos);
		pItem->SetWndSize			(sz);
		pItem->SetFont				(*ppFont);
		pItem->SetText				(str);
		pItem->SetTextComplexMode	(false);
		pItem->SetVTextAlignment	(valCenter);
		pItem->SetTextAlignment		(CGameFont::alCenter);
		pItem->AdjustHeightToText	();
		pItem->SetAutoDelete		(true);

		pos.y += pItem->GetHeight() + 20.0f;

		AttachChild(pItem);
	}
}
