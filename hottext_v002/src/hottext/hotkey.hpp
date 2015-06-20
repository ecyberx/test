/**
	@brief		ホットキー制御クラスヘッダファイル
 */

#ifndef	INCLUDE_HOTKEY_HPP
#define	INCLUDE_HOTKEY_HPP
/**
	@brief		ホットキー制御クラス
 */
class CHotKey {
private:
	UINT	m_uiVk;			/// キーコード
	UINT	m_uiMod;		/// 同時押しキー
	ATOM	m_HotKey;		/// ホットキーコード(アプリケーションごとに一意)
	HWND	m_hWndNotify;	/// ホットキー通知先
	BOOL	m_bEnabled;		/// ホットキー登録状態(true:登録/false:未登録(無効))

	/**
		@brief		ホットキー有効化
		@return		true:OK/false:NG
	 */
	int	enableHotKey(void)
	{
		if(m_HotKey==0 || m_bEnabled == TRUE)return false;
		if(RegisterHotKey(m_hWndNotify,m_HotKey,m_uiMod,m_uiVk)==0)
		{
			Release();
			return false;
		}
		m_bEnabled=TRUE;
		return true;
	}
	int	disableHotKey(void)
	{
		if(m_HotKey==0)return false;
		UnregisterHotKey(m_hWndNotify,m_HotKey);
		m_bEnabled=FALSE;
		return true;
	}

public:
	virtual ~CHotKey(){Release();}
	CHotKey()
		:	m_HotKey(0)
		,	m_bEnabled( FALSE )
	{
	}

	/**
		@brief		ホットキーの作成
		@param		szAppName		アプリケーション名
		@param		hWndNotify		通知先ウインドウハンドル
		@param		fsMod			同時押しキーコード
		@param		vk				ホットキーのキーコード
		@return		結果(true:OK/false:NG)
	 */
	int Create(LPCSTR szAppName,HWND hWndNotify,UINT fsMod,UINT vk)
	{
		m_hWndNotify = hWndNotify;
		m_uiMod=fsMod;
		m_uiVk=vk;

		static long	cnt = 0;
		char	buf[512];
		sprintf( buf, "%08x", cnt++ );
		string	strAtom = szAppName;
		strAtom += buf;

		m_HotKey=GlobalAddAtom( strAtom.c_str() );
		if(m_HotKey==0)return false;

		if( !enableHotKey() )
		{
			return false;
		}
		return true;
	}
	/**
		@brief		ホットキーの解放
		@note		本メソッドはいつ呼び出されても安全なように設計する
	 */
	void Release(void)
	{
		if(m_bEnabled)
		{
			disableHotKey();
			m_bEnabled=FALSE;
		}
		if(m_HotKey)
		{
			GlobalDeleteAtom(m_HotKey);
			m_HotKey=0;
		}
	}

	/**
		@brief		ホットキーIDの取得
		@return		ホットキーのID(ATOM)
	 */
	ATOM	GetID(void){return m_HotKey;}
	/**
		@brief		ホットキーのキーコード取得
		@return		ホットキーのキーコード(LOWORD:同時押しキー,HIWORD:キーコード)
		@note		WM_HOTKEYメッセージのLPARAMとの比較用
	 */
	LPARAM	GetModifiersAndVK(void){return	(m_HotKey ? MAKELPARAM(m_uiMod,m_uiVk) : 0L);}

	/**
		@brief		LPARAMとの比較演算子
		@note		WM_HOTKEYメッセージのLPARAMとの比較用
	 */
	bool operator ==( LPARAM lParam )
	{
		return GetModifiersAndVK() == lParam;
	}

	/**
		@brief		ホットキーの有効/無効設定
		@param		bEnabled		ホットキーの有効/無効設定(true:有効/false:無効)
		@return		true:OK/false:NG
		@note		デフォルトでは有効状態。
		@note		有効→無効設定で登録削除、無効→有効設定で再登録。状態が変化しない場合は何もしない。
	 */
	int SetEnable(BOOL bEnabled)
	{
		if(m_HotKey==0)return false;

		if(bEnabled!=m_bEnabled)
		{
			if(bEnabled)
			{
				enableHotKey();
			} else {
				disableHotKey();
			}
		}
		return true;
	}
};

const char* KEY_CODE[] = {
	"VK_$00",
	"VK_LBUTTON",
	"VK_RBUTTON",
	"VK_CANCEL",
	"VK_MBUTTON",
	"VK_XBUTTON1",
	"VK_XBUTTON2",
	"VK_$07",
	"VK_BACK",
	"VK_TAB",
	"VK_$0A",
	"VK_$0B",
	"VK_CLEAR",
	"VK_RETURN",
	"VK_$0E",
	"VK_$0F",
	"VK_SHIFT",
	"VK_CONTROL",
	"VK_MENU",
	"VK_PAUSE",
	"VK_CAPITAL",
	"VK_KANA",
	"VK_$16",
	"VK_JUNJA",
	"VK_FINAL",
	"VK_KANJI",
	"VK_$1A",
	"VK_ESCAPE",
	"VK_CONVERT",
	"VK_NONCONVERT",
	"VK_ACCEPT",
	"VK_MODECHANGE",
	"VK_SPACE",
	"VK_PRIOR",
	"VK_NEXT",
	"VK_END",
	"VK_HOME",
	"VK_LEFT",
	"VK_UP",
	"VK_RIGHT",
	"VK_DOWN",
	"VK_SELECT",
	"VK_PRINT",
	"VK_EXECUTE",
	"VK_SNAPSHOT",
	"VK_INSERT",
	"VK_DELETE",
	"VK_HELP",
	"VK_0",
	"VK_1",
	"VK_2",
	"VK_3",
	"VK_4",
	"VK_5",
	"VK_6",
	"VK_7",
	"VK_8",
	"VK_9",
	"VK_$3A",
	"VK_$3B",
	"VK_$3C",
	"VK_$3D",
	"VK_$3E",
	"VK_$3F",
	"VK_$40",
	"VK_A",
	"VK_B",
	"VK_C",
	"VK_D",
	"VK_E",
	"VK_F",
	"VK_G",
	"VK_H",
	"VK_I",
	"VK_J",
	"VK_K",
	"VK_L",
	"VK_M",
	"VK_N",
	"VK_O",
	"VK_P",
	"VK_Q",
	"VK_R",
	"VK_S",
	"VK_T",
	"VK_U",
	"VK_V",
	"VK_W",
	"VK_X",
	"VK_Y",
	"VK_Z",
	"VK_LWIN",
	"VK_RWIN",
	"VK_APPS",
	"VK_$5E",
	"VK_SLEEP",
	"VK_NUMPAD0",
	"VK_NUMPAD1",
	"VK_NUMPAD2",
	"VK_NUMPAD3",
	"VK_NUMPAD4",
	"VK_NUMPAD5",
	"VK_NUMPAD6",
	"VK_NUMPAD7",
	"VK_NUMPAD8",
	"VK_NUMPAD9",
	"VK_MULTIPLY",
	"VK_ADD",
	"VK_SEPARATOR",
	"VK_SUBTRACT",
	"VK_DECIMAL",
	"VK_DIVIDE",
	"VK_F1",
	"VK_F2",
	"VK_F3",
	"VK_F4",
	"VK_F5",
	"VK_F6",
	"VK_F7",
	"VK_F8",
	"VK_F9",
	"VK_F10",
	"VK_F11",
	"VK_F12",
	"VK_F13",
	"VK_F14",
	"VK_F15",
	"VK_F16",
	"VK_F17",
	"VK_F18",
	"VK_F19",
	"VK_F20",
	"VK_F21",
	"VK_F22",
	"VK_F23",
	"VK_F24",
	"VK_$88",
	"VK_$89",
	"VK_$8A",
	"VK_$8B",
	"VK_$8C",
	"VK_$8D",
	"VK_$8E",
	"VK_$8F",
	"VK_NUMLOCK",
	"VK_SCROLL",
	"VK_$92",
	"VK_$93",
	"VK_$94",
	"VK_$95",
	"VK_$96",
	"VK_$97",
	"VK_$98",
	"VK_$99",
	"VK_$9A",
	"VK_$9B",
	"VK_$9C",
	"VK_$9D",
	"VK_$9E",
	"VK_$9F",
	"VK_LSHIFT",
	"VK_RSHIFT",
	"VK_LCONTROL",
	"VK_RCONTROL",
	"VK_LMENU",
	"VK_RMENU",
	"VK_BROWSER_BACK",
	"VK_BROWSER_FORWARD",
	"VK_BROWSER_REFRESH",
	"VK_BROWSER_STOP",
	"VK_BROWSER_SEARCH",
	"VK_BROWSER_FAVORITES",
	"VK_BROWSER_HOME",
	"VK_VOLUME_MUTE",
	"VK_VOLUME_DOWN",
	"VK_VOLUME_UP",
	"VK_MEDIA_NEXT_TRACK",
	"VK_MEDIA_PREV_TRACK",
	"VK_MEDIA_STOP",
	"VK_MEDIA_PLAY_PAUSE",
	"VK_LAUNCH_MAIL",
	"VK_LAUNCH_MEDIA_SELECT",
	"VK_LAUNCH_APP1",
	"VK_LAUNCH_APP2",
	"VK_$B8",
	"VK_$B9",
	"VK_OEM_1",
	"VK_OEM_PLUS",
	"VK_OEM_COMMA",
	"VK_OEM_MINUS",
	"VK_OEM_PERIOD",
	"VK_OEM_2",
	"VK_OEM_3",
	"VK_$C1",
	"VK_$C2",
	"VK_$C3",
	"VK_$C4",
	"VK_$C5",
	"VK_$C6",
	"VK_$C7",
	"VK_$C8",
	"VK_$C9",
	"VK_$CA",
	"VK_$CB",
	"VK_$CC",
	"VK_$CD",
	"VK_$CE",
	"VK_$CF",
	"VK_$D0",
	"VK_$D1",
	"VK_$D2",
	"VK_$D3",
	"VK_$D4",
	"VK_$D5",
	"VK_$D6",
	"VK_$D7",
	"VK_$D8",
	"VK_$D9",
	"VK_$DA",
	"VK_OEM_4",
	"VK_OEM_5",
	"VK_OEM_6",
	"VK_OEM_7",
	"VK_OEM_8",
	"VK_$E0",
	"VK_OEM_AX",
	"VK_OEM_102",
	"VK_ICO_HELP",
	"VK_ICO_00",
	"VK_PROCESSKEY",
	"VK_ICO_CLEAR",
	"VK_PACKET",
	"VK_$E8",
	"VK_OEM_RESET",
	"VK_OEM_JUMP",
	"VK_OEM_PA1",
	"VK_OEM_PA2",
	"VK_OEM_PA3",
	"VK_OEM_WSCTRL",
	"VK_OEM_CUSEL",
	"VK_OEM_ATTN",
	"VK_OEM_FINISH",
	"VK_OEM_COPY",
	"VK_OEM_AUTO",
	"VK_OEM_ENLW",
	"VK_OEM_BACKTAB",
	"VK_ATTN",
	"VK_CRSEL",
	"VK_EXSEL",
	"VK_EREOF",
	"VK_PLAY",
	"VK_ZOOM",
	"VK_NONAME",
	"VK_PA1",
	"VK_OEM_CLEAR",
	"VK_$FF"
};

#endif	// INCLUDE_HOTKEY_HPP
