#pragma once


class EditWnd;


class ToolEnvEffectFlagEdit
{
	int m_iBrushSize;

	DWORD m_dwCurSelGrid;

public:
	ToolEnvEffectFlagEdit(void);
	~ToolEnvEffectFlagEdit(void);

	void MessageProc(BOOL bActive);
	void Render();
	void Create();
	void Destroy(void);
	void _ShowStatus(void);
	
	void DrawBrush(void);
	void PaintBrush(BOOL bVisible);

	DWORD GetSelectGrid(void);
};
