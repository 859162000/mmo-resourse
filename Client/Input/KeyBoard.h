#pragma once
#include "stdafx.h"
#define KEYBUFFERSIZE		64

class CKeyBoard  
{
public:
	// 构造函数。
	CKeyBoard();
	// 析构函数。
	virtual ~CKeyBoard();

public:
	LPDIRECTINPUT8       m_pDI;				// DirectInput设备
	LPDIRECTINPUTDEVICE8 m_pdidKeyboard;	// 键盘操作句柄

	struct KEYBOARDSTATE					// 键盘缓冲数据结构
	{
        BYTE key[256];						// DirectInput键盘状态缓冲数据
	};
	KEYBOARDSTATE ks;						// DirectInput键盘状态缓冲数据

	DWORD m_dwItems;						// 键盘缓冲(硬件)
	queue <long>		m_queueKey;			// 输入缓冲队列
	unsigned char		m_lFirstKey;

private:
	bool m_bNextQueryInvalid;					// 标识下次查询数据是否无效
	unsigned char KeyBuffer[KEYBUFFERSIZE];		// 键盘缓冲区
	DIDEVICEOBJECTDATA Object[KEYBUFFERSIZE];	// DirectInput缓冲输入缓冲区

	// 功能:询问键盘的输入。
	// 参数:ks-用于保存输入数据的缓冲。
	void QueryKeyboardState(KEYBOARDSTATE& ks);

public:

	// 功能:获取键盘输入数据。
	// 注释:在游戏循环中进行时时检测获取。
	void RecieveKeyboardInput();

	// 功能:初始化键盘硬件设备(DInput)。
	// 参数:hInst - 创建DirectX输入设备的窗口实例；
	//      hWnd - 创建DirectX输入设备的窗口句柄。
	// 返回:创建失败返回false，成功返回true。
	bool Create(HINSTANCE hInst, HWND hWnd);

	// 功能:获取当前输入。
	// 返回:返回键盘DIK码。
	unsigned char GetCurrentKey(void);

	// 功能:设置当前输入。
	// 参数:value-要插入的按键DIK码。
	// 注释:将参数DIK码插入到当前的键盘列表，此不影响取键盘压键属性操作。
	void SetCurrentKey(unsigned char value);

	// 功能:获取键盘缓冲(硬件)的大小。
	// 返回:返回缓冲大小。
	long GetItems(void) { return m_dwItems; }

	// 功能:检测某键是否被按下。
	// 参数:value-所要检测的按键DIK码。
	// 返回:返回true成立，否则为false。
	// 注释:DIK操作码参看补充资料部分。
	bool IsKeyDown(long value);

	// 功能:清除键盘数据(但不释放设备)
	void ClearUpData();

	// 功能:设定下次查询无效
	// 注释:控制从设备中取出数据,但不更新到缓冲中
	void SetNextQueryInvalid()		{ m_bNextQueryInvalid = true; }

	LPDIRECTINPUTDEVICE8 GetDevice() { return m_pdidKeyboard; }		// 获取DirectInput的句柄。
};

/*
补充资料：

// ****************************************************************************
   *
   *      DirectInput键盘扫描码
   *
// ****************************************************************************

#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_MINUS           0x0C    // - on main keyboard
#define DIK_EQUALS          0x0D
#define DIK_BACK            0x0E    // backspace
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_LBRACKET        0x1A
#define DIK_RBRACKET        0x1B
#define DIK_RETURN          0x1C    // Enter on main keyboard
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_SEMICOLON       0x27
#define DIK_APOSTROPHE      0x28
#define DIK_GRAVE           0x29    // accent grave
#define DIK_LSHIFT          0x2A
#define DIK_BACKSLASH       0x2B
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_COMMA           0x33
#define DIK_PERIOD          0x34    // . on main keyboard
#define DIK_SLASH           0x35    // / on main keyboard
#define DIK_RSHIFT          0x36
#define DIK_MULTIPLY        0x37    // * on numeric keypad
#define DIK_LMENU           0x38    // left Alt
#define DIK_SPACE           0x39
#define DIK_CAPITAL         0x3A
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMLOCK         0x45
#define DIK_SCROLL          0x46    // Scroll Lock
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_SUBTRACT        0x4A    // - on numeric keypad
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_ADD             0x4E    // + on numeric keypad
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_DECIMAL         0x53    // . on numeric keypad
#define DIK_OEM_102         0x56    // <> or \| on RT 102-key keyboard (Non-U.S.)
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_F13             0x64    //                     (NEC PC98)
#define DIK_F14             0x65    //                     (NEC PC98)
#define DIK_F15             0x66    //                     (NEC PC98)
#define DIK_KANA            0x70    // (Japanese keyboard)           
#define DIK_ABNT_C1         0x73    // /? on Brazilian keyboard
#define DIK_CONVERT         0x79    // (Japanese keyboard)           
#define DIK_NOCONVERT       0x7B    // (Japanese keyboard)           
#define DIK_YEN             0x7D    // (Japanese keyboard)            
#define DIK_ABNT_C2         0x7E    // Numpad . on Brazilian keyboard
#define DIK_NUMPADEQUALS    0x8D    // = on numeric keypad (NEC PC98)
#define DIK_PREVTRACK       0x90    // Previous Track (DIK_CIRCUMFLEX on Japanese keyboard)
#define DIK_AT              0x91    //                     (NEC PC98)
#define DIK_COLON           0x92    //                     (NEC PC98)
#define DIK_UNDERLINE       0x93    //                     (NEC PC98)
#define DIK_KANJI           0x94    // (Japanese keyboard)           
#define DIK_STOP            0x95    //                     (NEC PC98)
#define DIK_AX              0x96    //                     (Japan AX)
#define DIK_UNLABELED       0x97    //                        (J3100)
#define DIK_NEXTTRACK       0x99    // Next Track
#define DIK_NUMPADENTER     0x9C    // Enter on numeric keypad
#define DIK_RCONTROL        0x9D
#define DIK_MUTE            0xA0    // Mute
#define DIK_CALCULATOR      0xA1    // Calculator
#define DIK_PLAYPAUSE       0xA2    // Play / Pause
#define DIK_MEDIASTOP       0xA4    // Media Stop
#define DIK_VOLUMEDOWN      0xAE    // Volume -
#define DIK_VOLUMEUP        0xB0    // Volume +
#define DIK_WEBHOME         0xB2    // Web home
#define DIK_NUMPADCOMMA     0xB3    // , on numeric keypad (NEC PC98)
#define DIK_DIVIDE          0xB5    // / on numeric keypad
#define DIK_SYSRQ           0xB7
#define DIK_RMENU           0xB8    // right Alt
#define DIK_PAUSE           0xC5    // Pause
#define DIK_HOME            0xC7    // Home on arrow keypad
#define DIK_UP              0xC8    // UpArrow on arrow keypad
#define DIK_PRIOR           0xC9    // PgUp on arrow keypad
#define DIK_LEFT            0xCB    // LeftArrow on arrow keypad
#define DIK_RIGHT           0xCD    // RightArrow on arrow keypad
#define DIK_END             0xCF    // End on arrow keypad
#define DIK_DOWN            0xD0    // DownArrow on arrow keypad
#define DIK_NEXT            0xD1    // PgDn on arrow keypad
#define DIK_INSERT          0xD2    // Insert on arrow keypad
#define DIK_DELETE          0xD3    // Delete on arrow keypad
#define DIK_LWIN            0xDB    // Left Windows key
#define DIK_RWIN            0xDC    // Right Windows key
#define DIK_APPS            0xDD    // AppMenu key
#define DIK_POWER           0xDE    // System Power
#define DIK_SLEEP           0xDF    // System Sleep
#define DIK_WAKE            0xE3    // System Wake
#define DIK_WEBSEARCH       0xE5    // Web Search
#define DIK_WEBFAVORITES    0xE6    // Web Favorites
#define DIK_WEBREFRESH      0xE7    // Web Refresh
#define DIK_WEBSTOP         0xE8    // Web Stop
#define DIK_WEBFORWARD      0xE9    // Web Forward
#define DIK_WEBBACK         0xEA    // Web Back
#define DIK_MYCOMPUTER      0xEB    // My Computer
#define DIK_MAIL            0xEC    // Mail
#define DIK_MEDIASELECT     0xED    // Media Select

// *
   *  Alternate names for keys, to facilitate transition from DOS.
// *

#define DIK_BACKSPACE       DIK_BACK            // backspace
#define DIK_NUMPADSTAR      DIK_MULTIPLY        // * on numeric keypad
#define DIK_LALT            DIK_LMENU           // left Alt
#define DIK_CAPSLOCK        DIK_CAPITAL         // CapsLock
#define DIK_NUMPADMINUS     DIK_SUBTRACT        // - on numeric keypad
#define DIK_NUMPADPLUS      DIK_ADD             // + on numeric keypad
#define DIK_NUMPADPERIOD    DIK_DECIMAL         // . on numeric keypad
#define DIK_NUMPADSLASH     DIK_DIVIDE          // / on numeric keypad
#define DIK_RALT            DIK_RMENU           // right Alt
#define DIK_UPARROW         DIK_UP              // UpArrow on arrow keypad
#define DIK_PGUP            DIK_PRIOR           // PgUp on arrow keypad
#define DIK_LEFTARROW       DIK_LEFT            // LeftArrow on arrow keypad
#define DIK_RIGHTARROW      DIK_RIGHT           // RightArrow on arrow keypad
#define DIK_DOWNARROW       DIK_DOWN            // DownArrow on arrow keypad
#define DIK_PGDN            DIK_NEXT            // PgDn on arrow keypad

*/