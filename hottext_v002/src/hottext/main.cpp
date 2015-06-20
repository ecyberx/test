/**
	@brief		ホットキーテキスト太郎 メイン
	@version	0.03
 */
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>


#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <map>

#include "./common/simplescript.hpp"
#include "./common/simpleplugin.hpp"
#include "./common/simpleutil.hpp"
RuntimeErrorDefine;

using namespace std;
using namespace simple;

#include "resource.h"
#include "tasktray.hpp"
#include "hotkey.hpp"
#include "formatdatetime.hpp"

#define MWM_NOTIFYICON  (WM_APP + 0) 
#define ID_TASKTRAY     (0) 

#define	_APP_NAME		"ホットキーテキスト太郎"		/// アプリケーション名
#define	_APP_VERSION	"1.40"							/// バージョン
#define	_CONFIG_FILE	"hottext.cfg"					/// 設定ファイル名
#define	_CRLF			"\r\n"							/// 改行コード

/// タイトル
const char*	_TITLE =	_APP_NAME	" Ver."	_APP_VERSION _CRLF
						"Compiled on " __DATE__ " at " __TIME__ _CRLF
						;

/**
	@brief		ホットキー定義用のキー情報
 */
struct key_info
{
	UINT vk;		/// キーコード
	UINT mod;		/// 
};
/// オプション
struct	option
{
	bool	bHide;				/// ウインドウを隠した状態で起動するか？
	bool	bIMEChar;			/// メッセージをIME_CHARで送信するか？
};


int	getClipboardText( string& strDest )
{
	// get clipboard text
	if(OpenClipboard(NULL)==0)
	{
		return false;
	}

	HANDLE	hText = GetClipboardData( CF_TEXT );
	if( hText == NULL )
	{
		CloseClipboard();
		return false;
	}
	char*	pText = reinterpret_cast<char*>(GlobalLock(hText));
	if( pText == NULL )
	{
		CloseClipboard();
		return false;
	}

	strDest = pText;

	GlobalUnlock(hText);
	CloseClipboard();

	return true;
}

/**
	@brief		文字列中の日付/環境変数を展開する。
	@param		strSrc		展開元の文字列
	@param		strDest		展開結果の保存先
	@return		true:OK/false:NG
	@note		%～%を環境変数の文字列に置き換える。
				ただし、%$～%は指定形式の日付/時刻情報(yyyy.mm.dd→2008.12.14など)に置き換える。
 */
int	parseText( string strSrc, string& strDest )
{
	strDest.clear();

	// 日付の取得
	time_t	tTime;
	time(&tTime);
	CFormatDateTime	fmDate;

	size_t tPos = strSrc.find("%");
	size_t tPosStart = 0;

	for( ; tPos != strSrc.npos ; tPos = strSrc.find( "%", tPos+1 ) )
	{
		// %までの文字を取得
		strDest += strSrc.substr( tPosStart, tPos-tPosStart );

		// %～%までを取得
		size_t	tPosNext = strSrc.find( "%", tPos+1 );
		if( tPosNext == strSrc.npos )
		{
			break;
		}

		string	strEnv = strSrc.substr( tPos+1, tPosNext-tPos-1 );

		// 環境変数？
		if( !strEnv.empty() )
		{
			if( strEnv[0] == '$' )
			{
				// 日付
				fmDate.ParseDate( strEnv.c_str()+1, localtime(&tTime) );
				const char*	pDate = fmDate.GetDateTime();
				if( pDate )
				{
					strDest += pDate;
				}
			}else if( strEnv[0] == '!' )
			{
				// 拡張
				if( strEnv == "!clipboard" )
				{
					// クリップボードの内容に展開
					string strClip;
					if( getClipboardText( strClip ) )
					{
						strDest += strClip;
					}
				}

			}else
			{
				// 環境変数
				const char* pEnv = getenv( strEnv.c_str() );
				if( pEnv )strDest += pEnv;
			}
		}
		
		tPos = tPosNext;
		tPosStart = tPos+1;
	}

	// 最後の%より後ろの文字を取得
	strDest += strSrc.substr( tPosStart, tPos );

	
	return true;
}

/**
	@brief		ターゲットウインドウへのテキスト文字列の送信
	@param		szMsg		送信する文字列
	@note		現在アクティブなウインドウの入力フォーカスの当たっているウインドウハンドルへ
				指定の文字列をメッセージとして送信する。
	*/
void	sendString(  const char* szMsg, option& opt )
{
	HWND	hwndTarget = GetForegroundWindow();
	DWORD	dwTargetThread = GetWindowThreadProcessId( hwndTarget, NULL );
	DWORD	dwSelfThread = GetCurrentThreadId();
	AttachThreadInput( dwSelfThread, dwTargetThread, TRUE );
	HWND	hwndActive = GetFocus();

	for( ; *szMsg ; ++szMsg )
	{
		if( opt.bIMEChar )
		{
			// IME_CHAR形式で送信
			if( *szMsg < 0 )
			{
				// 2バイト文字は1文字として扱う
				unsigned short	usChar = ( ((*szMsg) & 0xff ) << 8) | ((*szMsg) & 0xff);
				PostMessage( hwndActive, WM_IME_CHAR, usChar, 1 );
			} else
			{
				PostMessage( hwndActive, WM_IME_CHAR, (*szMsg) & 0xff, 1 );
			}
		} else
		{
			// CHAR形式で送信
			PostMessage( hwndActive, WM_CHAR, (*szMsg) & 0xff, 1 );
		}
	}

	AttachThreadInput( dwSelfThread, dwTargetThread, FALSE );
}

/**
	@brief		指定ファイルを指定の拡張子に関連づけられたアプリケーションで開く
	@param		szExt		アプリケーションに関連づけられた拡張子
	@param		szFilename	開く対象のファイル名
	@return		true:OK/false:NG
 */
int	openAssociatedFile( const char* szExt, const char* szFilename )
{
	// テキストファイルに関連づけられたエディタで設定ファイルを開く
	char buf[_MAX_PATH+_MAX_FNAME];
	DWORD dwOut = sizeof(buf);
	HRESULT hr = AssocQueryString(ASSOCF_NOTRUNCATE, ASSOCSTR_EXECUTABLE, szExt, _T("open"), buf, &dwOut ); 
	if( FAILED(hr ) || dwOut == 0 )
	{
		// failed
		return false;
	} else
	{
		ShellExecute( NULL,NULL, buf, szFilename ,NULL, SW_SHOWNORMAL );
	}

	return true;
}


/**
	@brief		ホットキーによるアクション制御の基本クラス
	@note		各キーアクションは、このクラスを継承して作成する。
 */
class	CAction
{
public:
	virtual	~CAction(void)
	{}
	CAction(void)
	{}
	/**
		@brief		ホットキー押下時のアクション処理
		@return		TRUE:OK/FALSE:NG
		@param		opt		アプリケーションのオプション
	 */
	virtual int		ExecAction(option& opt)=0;

	/**
		@brief		設定ファイルの解析処理
		@param		vecScript		スクリプトコンテナ
		@param		plugin			プラグイン(スクリプト解析時に使用)
		@param		keyInfo			取得したキー情報(メソッド内で設定して返す)
		@return		TRUE:OK/FALSE:NG
	 */
	virtual int		ParseScript(script_vector& vecScript, CPlugIn& plugin, key_info& keyInfo )=0;
};

typedef	CAction*(create_action_func)(void);

/**
	@brief		テキスト挿入アクションクラス
	@note		あらかじめ指定したテキストを挿入する。
 */
class	CAction_Input : public CAction
{
protected:
	string		m_strFormat;

public:
	virtual ~CAction_Input(void)
	{
	}

	virtual int	ParseScript(script_vector& vecScript, CPlugIn& plugin, key_info& keyInfo )
	{
		struct	key_bind
		{
			UINT	vk;
			UINT	mod;
			string	strFormat;
		}	keyBind;

		// パラメータのデフォルト値の設定
		keyBind.vk = keyBind.mod= 0;

		// キー設定の読み込み
		script_parse_bind	parseBind[] = 
		{
			SCRIPT_PARSE_BIND( "key",			key_bind,		vk,	"app/ScriptParseInt", int ),
			SCRIPT_PARSE_BIND( "mod",			key_bind,		mod,"app/ScriptParseInt", int ),
			SCRIPT_PARSE_BIND( "format",		key_bind,		strFormat,	"app/ScriptParseBasicString", string ),
		};
		if( !ScriptParse( vecScript, parseBind, _NUMOF(parseBind), &keyBind, plugin ) )
		{
			return false;
		}

		keyInfo.vk = keyBind.vk;
		keyInfo.mod= keyBind.mod;
		m_strFormat= keyBind.strFormat;

		return true;
	}
	virtual int		ExecAction(option& opt)
	{
		string strDest;
		parseText( m_strFormat, strDest );
		sendString( strDest.c_str(), opt );

		return true;
	}


};

_EXPORT(CAction*)	action_input(void)
{
	CAction*	lpAction = new CAction_Input();
	return lpAction;
}


class	CAction_ClipText : public CAction
{
protected:
	string		m_strPath;
	UINT		m_isOpen;
	UINT		m_isExplorer;

public:
	virtual	~CAction_ClipText(void)
	{
	}
	CAction_ClipText(void)
	{
	}
	virtual int	ParseScript(script_vector& vecScript, CPlugIn& plugin, key_info& keyInfo )
	{
		struct	key_bind
		{
			UINT	vk;
			UINT	mod;
			string	path;
			UINT	open;
			UINT	explorer;
		}	keyBind;

		// パラメータのデフォルト値の設定
		keyBind.vk = keyBind.mod= 0;
		keyBind.open = keyBind.explorer = false;

		// キー設定の読み込み
		script_parse_bind	parseBind[] = 
		{
			SCRIPT_PARSE_BIND( "key",			key_bind,		vk,			"app/ScriptParseInt", int ),
			SCRIPT_PARSE_BIND( "mod",			key_bind,		mod,		"app/ScriptParseInt", int ),
			SCRIPT_PARSE_BIND( "path",			key_bind,		path,		"app/ScriptParseBasicString", string ),
			SCRIPT_PARSE_BIND( "open",			key_bind,		open,		"app/ScriptParseInt", int ),
			SCRIPT_PARSE_BIND( "explorer",		key_bind,		explorer,	"app/ScriptParseInt", int ),
		};
		if( !ScriptParse( vecScript, parseBind, _NUMOF(parseBind), &keyBind, plugin ) )
		{
			return false;
		}

		keyInfo.vk	= keyBind.vk;
		keyInfo.mod	= keyBind.mod;
		m_strPath	= keyBind.path;
		m_isOpen	= keyBind.open;
		m_isExplorer= keyBind.explorer;

		return true;
	}

	virtual int		ExecAction(option& opt)
	{
		// get clipboard text
		if(OpenClipboard(NULL)==0)return false;

		HANDLE	hText = GetClipboardData( CF_TEXT );
		if( hText == NULL )
		{
			CloseClipboard();
			return false;
		}
		char*	pText = reinterpret_cast<char*>(GlobalLock(hText));
		if( pText == NULL )
		{
			CloseClipboard();
			return false;
		}

		string strDest;
		parseText( m_strPath, strDest );
		ofstream	ofst( strDest.c_str() );
		if( ofst.is_open() )
		{
			for(;*pText;++pText)
			{
				if( *pText == '\r' && *(pText+1)=='\n' )
				{}
				else ofst << *pText;
			}
			ofst.close();
		}
		GlobalUnlock(hText);
		CloseClipboard();

		if( m_isOpen )
		{
			openAssociatedFile( ".txt", strDest.c_str() );
		}

		if( m_isExplorer )
		{
			string opt = "/select,";
			opt += strDest;
			ShellExecute( NULL, NULL, "explorer.exe", opt.c_str(), NULL, SW_SHOWNORMAL ); 
		}

		return true;
	}

protected:

};


_EXPORT(CAction*)	action_cliptext(void)
{
	CAction*	lpAction = new CAction_ClipText();
	return lpAction;
}



class	CAction_Shell : public CAction
{
protected:
	string	m_path;
	string	m_app;
	string	m_opt;

public:
	virtual ~CAction_Shell(void)
	{}
	CAction_Shell(void)
	{}
	/**
		@brief		ホットキー押下時のアクション処理
		@return		TRUE:OK/FALSE:NG
		@param		opt		アプリケーションのオプション
	 */
	int		ExecAction(option& )
	{
		string app, opt, path;
		if( !parseText( m_app, app )
			|| !parseText( m_path,path )
			|| !parseText( m_opt,opt ) )
		{
			return false;
		}

		ShellExecute( NULL, NULL, app.c_str(), opt.c_str(), path.c_str(), SW_NORMAL );
		return true;
	}
	/**
		@brief		設定ファイルの解析処理
		@param		vecScript		スクリプトコンテナ
		@param		plugin			プラグイン(スクリプト解析時に使用)
		@param		keyInfo			取得したキー情報(メソッド内で設定して返す)
		@return		TRUE:OK/FALSE:NG
	 */
	int		ParseScript(script_vector& vecScript, CPlugIn& plugin, key_info& keyInfo )
	{
		struct	key_bind
		{
			UINT	vk;
			UINT	mod;
			string	path;
			string	app;
			string	opt;
		} keyBind;

		keyBind.vk = keyBind.mod = 0;

		// キー設定の読み込み
		script_parse_bind	parseBind[] = 
		{
			SCRIPT_PARSE_BIND( "key",			key_bind,		vk,	"app/ScriptParseInt", int ),
			SCRIPT_PARSE_BIND( "mod",			key_bind,		mod,"app/ScriptParseInt", int ),

			SCRIPT_PARSE_BIND( "path",			key_bind,		path,	"app/ScriptParseBasicString", string ),
			SCRIPT_PARSE_BIND( "app",			key_bind,		app,	"app/ScriptParseBasicString", string ),
			SCRIPT_PARSE_BIND( "opt",			key_bind,		opt,	"app/ScriptParseBasicString", string ),
		};
		if( !ScriptParse( vecScript, parseBind, _NUMOF(parseBind), &keyBind, plugin ) )
		{
			return false;
		}

		keyInfo.vk = keyBind.vk;
		keyInfo.mod= keyBind.mod;
		m_path = keyBind.path;
		m_app  = keyBind.app;
		m_opt  = keyBind.opt;

		return true;
	}

protected:


};

_EXPORT(CAction*)	action_shell(void)
{
	CAction*	lpAction = new CAction_Shell();
	return lpAction;
}


/**
	@brief		アプリケーションクラス
 */
class CApplication
{
public:

	/// キー設定
	struct	action_info
	{
		CHotKey*	phkTrigger;		/// 起動するホットキー設定
		CAction*	lpAction;		/// 実行するアクション
	};
	typedef	map< LPARAM, action_info >		action_info_map;

private:
	HINSTANCE	m_hInstance;		/// アプリケーションのインスタンスハンドル
	HWND		m_hWnd;				/// メインダイアログのウインドウハンドル

	CTaskTray	m_TaskTray;			/// タスクトレイ
	option		m_Option;			/// オプション情報
	CPlugIn		m_Plugin;			/// プラグイン

	action_info_map	m_mapAction;	/// ホットキー動作

	string		m_strErrorMsg;		/// エラーメッセージ

	/**
		@brief		ダイアログのウインドウプロシージャエントリポイント
	 */
	static BOOL CALLBACK entryDialogProc(
	  HWND hwndDlg, 
	  UINT uMsg,
	  WPARAM wParam,
	  LPARAM lParam
	)
	{
		if( uMsg == WM_INITDIALOG ){
			SetWindowLong( hwndDlg, GWL_USERDATA, (LONG)lParam );
		}
		CApplication* lpProc = reinterpret_cast<CApplication*>(GetWindowLong( hwndDlg, GWL_USERDATA));
		if( lpProc )return lpProc->dlgProc(hwndDlg,uMsg,wParam,lParam);
		return TRUE;
	}


	/**
		@brief		ホットキーが押された
		@param		lParam		ホットキー押下メッセージのLPARAM
	 */
	void	onHotkey( LPARAM lParam )
	{
		// 押されたキーに対応するアクションを検索
		action_info_map::iterator	itAct = m_mapAction.find( lParam );
		if( itAct != m_mapAction.end() )
		{
			// アクションを実行
			itAct->second.lpAction->ExecAction(m_Option);
		}
	}
	/**
		@brief		タスクトレイのアイコンのメッセージ処理
	 */
	void onNotifyIcon(WPARAM wParam , LPARAM lParam)
	{
		// タスクトレイ以外のメッセージなら帰る
		if(wParam != ID_TASKTRAY)return ; 
		// メッセージ分岐
		switch (lParam) {   // lParam == mouse message ? 
		case WM_RBUTTONDOWN:	// 右クリックなら、ウインドウをアクティブにしてメニューを表示 
			{
				POINT	pts;
				GetCursorPos(&pts);

				HMENU	hMenu = LoadMenu( m_hInstance, MAKEINTRESOURCE(IDR_MENU_POPUP) );
				HMENU	hSubMenu = GetSubMenu( hMenu, 0 );

				TrackPopupMenu( hSubMenu, TPM_LEFTALIGN|TPM_TOPALIGN, pts.x, pts.y, 0, m_hWnd, NULL );
				DestroyMenu( hMenu );
			}
			return ;
		case WM_LBUTTONDOWN:	// 左クリックは何もしない
			break;
		case WM_LBUTTONDBLCLK:	// 左ダブルクリックなら、ウインドウを表示
			showDialog();
			break;
		} 
	}

	/**
		@brief		ダイアログを開く
	 */
	void	showDialog(void)
	{
		ShowWindow( m_hWnd, SW_SHOW );
		SetForegroundWindow(m_hWnd);
	}

	/**
		@brief		ダイアログのバージョン情報テキストを更新
	 */
	void	renewDialogText(void)
	{
		if(m_hWnd==NULL)return;

		HWND hwndVersion = GetDlgItem( m_hWnd, IDC_EDIT_VERSION );
		if( hwndVersion )
		{
			string strText = _TITLE;
			strText += _CRLF;
			char buf[512];

			time_t	tTime;
			time(&tTime);
			CFormatDateTime fmDate;
			fmDate.ParseDate( "yyyy.mm.dd hh:mm:ss", localtime(&tTime) );

			sprintf( buf,	"設定ファイル:%s" _CRLF 
							"取得日時:%s",
							_CONFIG_FILE,
							fmDate.GetDateTime() );
			strText += buf;
							
			SetWindowText( hwndVersion, strText.c_str() );
		}
	}

	/**
		@brief		メインダイアログのウインドウプロシージャ
	 */
	BOOL CALLBACK dlgProc(
	  HWND hwndDlg,
	  UINT uMsg,
	  WPARAM wParam,
	  LPARAM lParam
	)
	{
		switch( uMsg )
		{
		case	WM_HOTKEY:
			onHotkey(lParam);
			break;
		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case	IDCANCEL:		// "終了"ボタンが押下された
				closeDialog();
				break;
			case	IDOK:			// "OK"ボタンが押下された
				ShowWindow( m_hWnd, SW_HIDE );
				break;
			case	ID_POPUP_ABOUT:	// メニュー"ABOUT"が選択された
				showDialog();
				break;
			case	IDC_BUTTON_OPENCFG:
				{
					// テキストファイルに関連づけられたエディタで設定ファイルを開く
					openAssociatedFile( ".txt", "hottext.cfg" );
				}
				break;
			case	IDC_BUTTON_RELOADCFG:
				releaseAction();
				if( !loadConfig() )
				{
					string msg = m_strErrorMsg + "\nアプリケーションを終了します。";
					MessageBox( NULL, msg.c_str(), _APP_NAME, MB_OK );
					closeDialog();
				}
				
				renewDialogText();

				break;
			}
			break;
		case	WM_CLOSE:
			closeDialog();
			break;
		case MWM_NOTIFYICON:	// タスクトレイのアイコンに何かあったらしい
			onNotifyIcon(wParam,lParam);
			break;
		}

		return	FALSE;
	}


	/**
		@brief		ダイアログの作成
		@param		hInstance		アプリケーションのインスタンスハンドル
		@param		lpTemplate		ダイアログリソース
		@return		true:OK/false:NG
	 */
	int	openDialog(  HINSTANCE hInstance, LPCTSTR lpTemplate )
	{
		// ダイアログを開く
		m_hWnd = CreateDialogParam( hInstance, lpTemplate, NULL,
									reinterpret_cast<DLGPROC>(entryDialogProc),
									reinterpret_cast<LONG>(this)
									);

		if( m_hWnd == NULL )
		{
			// 失敗
			return false;
		}

		// テキストボックスへバージョン情報を設定
		renewDialogText();

		// ダイアログのタイトルをアプリケーション名に設定
		SetWindowText( m_hWnd, _APP_NAME );

		return true;
	}

	/**
		@brief		ダイアログを閉じる
		@return		true:ダイアログを閉じた/false:ダイアログは開かれていない
	 */
	bool	closeDialog(void)
	{
		if( m_hWnd )
		{
			releaseAction();
			m_TaskTray.Release();
			DestroyWindow( m_hWnd );
			m_hWnd = NULL;
			return true;
		}
		return false;
	}

	/**
		@brief		オプションの読み込み
		@return		true:OK/false:NG
	 */
	int	loadOption(script_vector& vecScript)
	{
		// find_key
		script_vector::iterator	it = ScriptFindKey( "option", vecScript.begin(), vecScript );
		if( it == vecScript.end() ){ return false; }
		++it;
		if( it == vecScript.end() ){ return false; }

		// パラメータのデフォルト値を設定
		m_Option.bHide = true;
		m_Option.bIMEChar=false;

		script_parse_bind	parseBind[] = 
		{
			SCRIPT_PARSE_BIND( "hide",			option,		bHide,	"app/ScriptParseBool", bool ),
			SCRIPT_PARSE_BIND( "ime_char",		option,		bIMEChar,"app/ScriptParseBool", bool ),
		};
		if( !ScriptParse( it->vecChild, parseBind, _NUMOF(parseBind), &m_Option, m_Plugin ) )
		{
			return false;
		}

		return true;
	}

	void	releaseAction( void )
	{
		action_info_map::iterator	it = m_mapAction.begin();
		for( ; it != m_mapAction.end() ; it++ )
		{
			delete it->second.phkTrigger;
			delete it->second.lpAction;
			it->second.phkTrigger = NULL;
		}
		m_mapAction.clear();
	}
	int	registAction( key_info keyInfo, CAction* lpAction )
	{
		action_info	actInfo;
		actInfo.phkTrigger = new CHotKey;
		if( !actInfo.phkTrigger->Create( _APP_NAME,  m_hWnd, keyInfo.mod, keyInfo.vk ) )
		{
			delete	actInfo.phkTrigger;
			return false;
		}
		actInfo.lpAction = lpAction;

		LPARAM	lParam = actInfo.phkTrigger->GetModifiersAndVK();
		action_info_map::iterator it = m_mapAction.find( lParam );
		if( it != m_mapAction.end() )
		{
			// 重複している
			delete actInfo.phkTrigger;
			return false;
		}
		m_mapAction[ lParam ] = actInfo;
		return true;
	}
	
	/**
		@brief		アクションの読み込み
		@return		true:OK/false:NG
	 */
	int	loadAction( script_vector& vecScript )
	{
		// find_key
		script_vector::iterator	it = ScriptFindKey( "action", vecScript.begin(), vecScript );
		if( it == vecScript.end() ){ return false; }
		++it;
		if( it == vecScript.end() ){ return false; }

		script_vector::iterator	itAct = it->vecChild.begin();

		for( ; itAct != it->vecChild.end() ; itAct++ )
		{
			if( itAct->strWord == "key" )
			{
				itAct++;
				if( itAct == vecScript.end() ){return false;}

				itAct->getStringWord();

				create_action_func* lpFunc = reinterpret_cast<create_action_func*>(m_Plugin.GetProcess( itAct->strWord ));

				if( lpFunc == NULL )
				{
					m_strErrorMsg += " * プラグインが見つかりません：";
					m_strErrorMsg += itAct->strWord + "\n";
					return false;
				}

				itAct++;
				if( itAct == vecScript.end() ){return false;}

				CAction*	lpAction = lpFunc();
				key_info	keyInfo;
				if( !lpAction->ParseScript( itAct->vecChild, m_Plugin, keyInfo ) )
				{
					m_strErrorMsg += "ホットキーの記述が不正です。\n";
					delete lpAction;
					return false;
				}
				if( !registAction( keyInfo, dynamic_cast<CAction*>(lpAction) ) )
				{
					m_strErrorMsg += "ホットキー情報の登録に失敗しました。\n"
									 "このキーはすでに使用されている可能性があります。\n → ";
					m_strErrorMsg += showKeyData(keyInfo);
					m_strErrorMsg += '\n';

					return false;
				}

			}
		}
		return true;
	}

	/**
		@brief		設定のファイルからの読み込み
		@return		true:OK/false:NG
	 */
	int	loadConfig(void)
	{

		script_vector	vecScript;
		if( !ScriptLoadFromFile( _CONFIG_FILE, vecScript ) )
		{
			string strMsg = "スクリプトファイルの読み込みに失敗しました" _CRLF;
			string strErr;
			if( RuntimeErrorGetMsg(strErr) )
			{
				strMsg += strErr;
			}

			m_strErrorMsg += strMsg;
			return false;
		}

		script_vector::iterator itKey = ScriptFindKey("hottext", vecScript.begin(), vecScript );
		if( itKey == vecScript.end() ){ m_strErrorMsg = "タグ'hottext'が見つかりません";return false; }
		++itKey;
		if( itKey == vecScript.end() ){ m_strErrorMsg = "タグ'hottext'の記述箇所がファイル終端です";return false; }

		if( !loadOption(itKey->vecChild) )
		{
			m_strErrorMsg += "オプション情報の読み込みに失敗しました";
			return false;
		}

		if( !loadAction(itKey->vecChild) )
		{
			m_strErrorMsg += "ホットキー設定の読み込みに失敗しました";
			return false;
		}

		return	true;
	}

	string	showKeyData( key_info keyInfo )
	{
		string mod,vk,result;

		// mod
		if( keyInfo.mod & MOD_ALT )mod += "MOD_ALT ";
		if( keyInfo.mod & MOD_CONTROL)mod += "MOD_CONTROL ";
		if( keyInfo.mod & MOD_SHIFT )mod += "MOD_SHIFT ";
		if( keyInfo.mod & MOD_WIN ) mod += "MOD_WIN ";

		// vk
		if( keyInfo.vk >= 0 && keyInfo.vk <= _NUMOF(KEY_CODE) )
		{
			vk = KEY_CODE[keyInfo.vk];
		}
		char buf[512];
		sprintf( buf, "(vk:%d)", keyInfo.vk);
		vk += buf;
		
		result = mod + "+ ";
		result += vk;

		return result;
	}

public:
	virtual	~CApplication( void )
	{
		Release();
	}

	void	Release(void)
	{
		closeDialog();
	}

	CApplication( HINSTANCE hInstance )
		:m_hInstance( hInstance )
		,m_hWnd(NULL)
	{
		m_Plugin.AddInstance( m_hInstance, "app", false );
	}
	int	Create( void )
	{
		UINT uiRetry;

		if( !openDialog( m_hInstance, MAKEINTRESOURCE( IDD_DLG_MAIN ) ) )
		{
			MessageBox( m_hWnd, "ダイアログの作成に失敗しました", _APP_NAME, MB_OK );
			return false;
		}

		// 各種設定の読み出し
		if( !loadConfig() )
		{
			string strMsg = string("設定ファイルの読み込みに失敗しました" _CRLF) + m_strErrorMsg;
			MessageBox( m_hWnd, strMsg.c_str() ,
								_APP_NAME, MB_OK );
			return false;
		}

		// デフォルトのウインドウ状態の設定
		if( m_Option.bHide )
		{
			ShowWindow(m_hWnd, SW_HIDE);
		} else
		{
			ShowWindow(m_hWnd, SW_SHOWDEFAULT);
		}

		// タスクトレイへアイコンを登録
		
		for( uiRetry = 3 ; uiRetry != 0 ; --uiRetry )
		{
			CTaskTray::TASKTRAYINFO	ttInfo;
			ttInfo.dwIDTaskTray = ID_TASKTRAY;
			ttInfo.uCallbackMessage = MWM_NOTIFYICON;
			ttInfo.hIcon = LoadIcon( m_hInstance, MAKEINTRESOURCE(IDI_ICON_TASKTRAY) );
			ttInfo.strAppName = _APP_NAME;
			ttInfo.hwndNotify = m_hWnd;

			if( m_TaskTray.Create( ttInfo ) )
			{
				break;
			}
		}
		

		if( uiRetry == 0 )
		{
			MessageBox( m_hWnd, "タスクトレイの登録に失敗しました", _APP_NAME, MB_OK );
			return	false;
		}

		return true;
	}

	/**
		@brief		メイン処理の実行
		@return		true:OK/false:NG
	 */
	int	Exec( void )
	{
		MSG	msg;

		while(GetMessage(&msg , NULL , 0 , 0))
		{
			if( m_hWnd )
			{
				if( !IsDialogMessage( m_hWnd, &msg ) )
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			if( m_hWnd == NULL )break;
		}

		return true;
	}

};

/**
	@brief		プログラムのメイン
	@return		0:OK/負数:NG
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	try
	{
		CApplication	app(hInstance);
		if( !app.Create() )
		{
			return -1L;
		}

		app.Exec();

		app.Release();
	} catch(...)
	{
		MessageBox( NULL, "例外が発生しました", _APP_NAME, MB_OK );
	}
	return 0L;
}