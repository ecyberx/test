/**
	@brief		�z�b�g�L�[�e�L�X�g���Y ���C��
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

#define	_APP_NAME		"�z�b�g�L�[�e�L�X�g���Y"		/// �A�v���P�[�V������
#define	_APP_VERSION	"1.40"							/// �o�[�W����
#define	_CONFIG_FILE	"hottext.cfg"					/// �ݒ�t�@�C����
#define	_CRLF			"\r\n"							/// ���s�R�[�h

/// �^�C�g��
const char*	_TITLE =	_APP_NAME	" Ver."	_APP_VERSION _CRLF
						"Compiled on " __DATE__ " at " __TIME__ _CRLF
						;

/**
	@brief		�z�b�g�L�[��`�p�̃L�[���
 */
struct key_info
{
	UINT vk;		/// �L�[�R�[�h
	UINT mod;		/// 
};
/// �I�v�V����
struct	option
{
	bool	bHide;				/// �E�C���h�E���B������ԂŋN�����邩�H
	bool	bIMEChar;			/// ���b�Z�[�W��IME_CHAR�ő��M���邩�H
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
	@brief		�����񒆂̓��t/���ϐ���W�J����B
	@param		strSrc		�W�J���̕�����
	@param		strDest		�W�J���ʂ̕ۑ���
	@return		true:OK/false:NG
	@note		%�`%�����ϐ��̕�����ɒu��������B
				�������A%$�`%�͎w��`���̓��t/�������(yyyy.mm.dd��2008.12.14�Ȃ�)�ɒu��������B
 */
int	parseText( string strSrc, string& strDest )
{
	strDest.clear();

	// ���t�̎擾
	time_t	tTime;
	time(&tTime);
	CFormatDateTime	fmDate;

	size_t tPos = strSrc.find("%");
	size_t tPosStart = 0;

	for( ; tPos != strSrc.npos ; tPos = strSrc.find( "%", tPos+1 ) )
	{
		// %�܂ł̕������擾
		strDest += strSrc.substr( tPosStart, tPos-tPosStart );

		// %�`%�܂ł��擾
		size_t	tPosNext = strSrc.find( "%", tPos+1 );
		if( tPosNext == strSrc.npos )
		{
			break;
		}

		string	strEnv = strSrc.substr( tPos+1, tPosNext-tPos-1 );

		// ���ϐ��H
		if( !strEnv.empty() )
		{
			if( strEnv[0] == '$' )
			{
				// ���t
				fmDate.ParseDate( strEnv.c_str()+1, localtime(&tTime) );
				const char*	pDate = fmDate.GetDateTime();
				if( pDate )
				{
					strDest += pDate;
				}
			}else if( strEnv[0] == '!' )
			{
				// �g��
				if( strEnv == "!clipboard" )
				{
					// �N���b�v�{�[�h�̓��e�ɓW�J
					string strClip;
					if( getClipboardText( strClip ) )
					{
						strDest += strClip;
					}
				}

			}else
			{
				// ���ϐ�
				const char* pEnv = getenv( strEnv.c_str() );
				if( pEnv )strDest += pEnv;
			}
		}
		
		tPos = tPosNext;
		tPosStart = tPos+1;
	}

	// �Ō��%�����̕������擾
	strDest += strSrc.substr( tPosStart, tPos );

	
	return true;
}

/**
	@brief		�^�[�Q�b�g�E�C���h�E�ւ̃e�L�X�g������̑��M
	@param		szMsg		���M���镶����
	@note		���݃A�N�e�B�u�ȃE�C���h�E�̓��̓t�H�[�J�X�̓������Ă���E�C���h�E�n���h����
				�w��̕���������b�Z�[�W�Ƃ��đ��M����B
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
			// IME_CHAR�`���ő��M
			if( *szMsg < 0 )
			{
				// 2�o�C�g������1�����Ƃ��Ĉ���
				unsigned short	usChar = ( ((*szMsg) & 0xff ) << 8) | ((*szMsg) & 0xff);
				PostMessage( hwndActive, WM_IME_CHAR, usChar, 1 );
			} else
			{
				PostMessage( hwndActive, WM_IME_CHAR, (*szMsg) & 0xff, 1 );
			}
		} else
		{
			// CHAR�`���ő��M
			PostMessage( hwndActive, WM_CHAR, (*szMsg) & 0xff, 1 );
		}
	}

	AttachThreadInput( dwSelfThread, dwTargetThread, FALSE );
}

/**
	@brief		�w��t�@�C�����w��̊g���q�Ɋ֘A�Â���ꂽ�A�v���P�[�V�����ŊJ��
	@param		szExt		�A�v���P�[�V�����Ɋ֘A�Â���ꂽ�g���q
	@param		szFilename	�J���Ώۂ̃t�@�C����
	@return		true:OK/false:NG
 */
int	openAssociatedFile( const char* szExt, const char* szFilename )
{
	// �e�L�X�g�t�@�C���Ɋ֘A�Â���ꂽ�G�f�B�^�Őݒ�t�@�C�����J��
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
	@brief		�z�b�g�L�[�ɂ��A�N�V��������̊�{�N���X
	@note		�e�L�[�A�N�V�����́A���̃N���X���p�����č쐬����B
 */
class	CAction
{
public:
	virtual	~CAction(void)
	{}
	CAction(void)
	{}
	/**
		@brief		�z�b�g�L�[�������̃A�N�V��������
		@return		TRUE:OK/FALSE:NG
		@param		opt		�A�v���P�[�V�����̃I�v�V����
	 */
	virtual int		ExecAction(option& opt)=0;

	/**
		@brief		�ݒ�t�@�C���̉�͏���
		@param		vecScript		�X�N���v�g�R���e�i
		@param		plugin			�v���O�C��(�X�N���v�g��͎��Ɏg�p)
		@param		keyInfo			�擾�����L�[���(���\�b�h���Őݒ肵�ĕԂ�)
		@return		TRUE:OK/FALSE:NG
	 */
	virtual int		ParseScript(script_vector& vecScript, CPlugIn& plugin, key_info& keyInfo )=0;
};

typedef	CAction*(create_action_func)(void);

/**
	@brief		�e�L�X�g�}���A�N�V�����N���X
	@note		���炩���ߎw�肵���e�L�X�g��}������B
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

		// �p�����[�^�̃f�t�H���g�l�̐ݒ�
		keyBind.vk = keyBind.mod= 0;

		// �L�[�ݒ�̓ǂݍ���
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

		// �p�����[�^�̃f�t�H���g�l�̐ݒ�
		keyBind.vk = keyBind.mod= 0;
		keyBind.open = keyBind.explorer = false;

		// �L�[�ݒ�̓ǂݍ���
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
		@brief		�z�b�g�L�[�������̃A�N�V��������
		@return		TRUE:OK/FALSE:NG
		@param		opt		�A�v���P�[�V�����̃I�v�V����
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
		@brief		�ݒ�t�@�C���̉�͏���
		@param		vecScript		�X�N���v�g�R���e�i
		@param		plugin			�v���O�C��(�X�N���v�g��͎��Ɏg�p)
		@param		keyInfo			�擾�����L�[���(���\�b�h���Őݒ肵�ĕԂ�)
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

		// �L�[�ݒ�̓ǂݍ���
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
	@brief		�A�v���P�[�V�����N���X
 */
class CApplication
{
public:

	/// �L�[�ݒ�
	struct	action_info
	{
		CHotKey*	phkTrigger;		/// �N������z�b�g�L�[�ݒ�
		CAction*	lpAction;		/// ���s����A�N�V����
	};
	typedef	map< LPARAM, action_info >		action_info_map;

private:
	HINSTANCE	m_hInstance;		/// �A�v���P�[�V�����̃C���X�^���X�n���h��
	HWND		m_hWnd;				/// ���C���_�C�A���O�̃E�C���h�E�n���h��

	CTaskTray	m_TaskTray;			/// �^�X�N�g���C
	option		m_Option;			/// �I�v�V�������
	CPlugIn		m_Plugin;			/// �v���O�C��

	action_info_map	m_mapAction;	/// �z�b�g�L�[����

	string		m_strErrorMsg;		/// �G���[���b�Z�[�W

	/**
		@brief		�_�C�A���O�̃E�C���h�E�v���V�[�W���G���g���|�C���g
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
		@brief		�z�b�g�L�[�������ꂽ
		@param		lParam		�z�b�g�L�[�������b�Z�[�W��LPARAM
	 */
	void	onHotkey( LPARAM lParam )
	{
		// �����ꂽ�L�[�ɑΉ�����A�N�V����������
		action_info_map::iterator	itAct = m_mapAction.find( lParam );
		if( itAct != m_mapAction.end() )
		{
			// �A�N�V���������s
			itAct->second.lpAction->ExecAction(m_Option);
		}
	}
	/**
		@brief		�^�X�N�g���C�̃A�C�R���̃��b�Z�[�W����
	 */
	void onNotifyIcon(WPARAM wParam , LPARAM lParam)
	{
		// �^�X�N�g���C�ȊO�̃��b�Z�[�W�Ȃ�A��
		if(wParam != ID_TASKTRAY)return ; 
		// ���b�Z�[�W����
		switch (lParam) {   // lParam == mouse message ? 
		case WM_RBUTTONDOWN:	// �E�N���b�N�Ȃ�A�E�C���h�E���A�N�e�B�u�ɂ��ă��j���[��\�� 
			{
				POINT	pts;
				GetCursorPos(&pts);

				HMENU	hMenu = LoadMenu( m_hInstance, MAKEINTRESOURCE(IDR_MENU_POPUP) );
				HMENU	hSubMenu = GetSubMenu( hMenu, 0 );

				TrackPopupMenu( hSubMenu, TPM_LEFTALIGN|TPM_TOPALIGN, pts.x, pts.y, 0, m_hWnd, NULL );
				DestroyMenu( hMenu );
			}
			return ;
		case WM_LBUTTONDOWN:	// ���N���b�N�͉������Ȃ�
			break;
		case WM_LBUTTONDBLCLK:	// ���_�u���N���b�N�Ȃ�A�E�C���h�E��\��
			showDialog();
			break;
		} 
	}

	/**
		@brief		�_�C�A���O���J��
	 */
	void	showDialog(void)
	{
		ShowWindow( m_hWnd, SW_SHOW );
		SetForegroundWindow(m_hWnd);
	}

	/**
		@brief		�_�C�A���O�̃o�[�W�������e�L�X�g���X�V
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

			sprintf( buf,	"�ݒ�t�@�C��:%s" _CRLF 
							"�擾����:%s",
							_CONFIG_FILE,
							fmDate.GetDateTime() );
			strText += buf;
							
			SetWindowText( hwndVersion, strText.c_str() );
		}
	}

	/**
		@brief		���C���_�C�A���O�̃E�C���h�E�v���V�[�W��
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
			case	IDCANCEL:		// "�I��"�{�^�����������ꂽ
				closeDialog();
				break;
			case	IDOK:			// "OK"�{�^�����������ꂽ
				ShowWindow( m_hWnd, SW_HIDE );
				break;
			case	ID_POPUP_ABOUT:	// ���j���["ABOUT"���I�����ꂽ
				showDialog();
				break;
			case	IDC_BUTTON_OPENCFG:
				{
					// �e�L�X�g�t�@�C���Ɋ֘A�Â���ꂽ�G�f�B�^�Őݒ�t�@�C�����J��
					openAssociatedFile( ".txt", "hottext.cfg" );
				}
				break;
			case	IDC_BUTTON_RELOADCFG:
				releaseAction();
				if( !loadConfig() )
				{
					string msg = m_strErrorMsg + "\n�A�v���P�[�V�������I�����܂��B";
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
		case MWM_NOTIFYICON:	// �^�X�N�g���C�̃A�C�R���ɉ����������炵��
			onNotifyIcon(wParam,lParam);
			break;
		}

		return	FALSE;
	}


	/**
		@brief		�_�C�A���O�̍쐬
		@param		hInstance		�A�v���P�[�V�����̃C���X�^���X�n���h��
		@param		lpTemplate		�_�C�A���O���\�[�X
		@return		true:OK/false:NG
	 */
	int	openDialog(  HINSTANCE hInstance, LPCTSTR lpTemplate )
	{
		// �_�C�A���O���J��
		m_hWnd = CreateDialogParam( hInstance, lpTemplate, NULL,
									reinterpret_cast<DLGPROC>(entryDialogProc),
									reinterpret_cast<LONG>(this)
									);

		if( m_hWnd == NULL )
		{
			// ���s
			return false;
		}

		// �e�L�X�g�{�b�N�X�փo�[�W��������ݒ�
		renewDialogText();

		// �_�C�A���O�̃^�C�g�����A�v���P�[�V�������ɐݒ�
		SetWindowText( m_hWnd, _APP_NAME );

		return true;
	}

	/**
		@brief		�_�C�A���O�����
		@return		true:�_�C�A���O�����/false:�_�C�A���O�͊J����Ă��Ȃ�
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
		@brief		�I�v�V�����̓ǂݍ���
		@return		true:OK/false:NG
	 */
	int	loadOption(script_vector& vecScript)
	{
		// find_key
		script_vector::iterator	it = ScriptFindKey( "option", vecScript.begin(), vecScript );
		if( it == vecScript.end() ){ return false; }
		++it;
		if( it == vecScript.end() ){ return false; }

		// �p�����[�^�̃f�t�H���g�l��ݒ�
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
			// �d�����Ă���
			delete actInfo.phkTrigger;
			return false;
		}
		m_mapAction[ lParam ] = actInfo;
		return true;
	}
	
	/**
		@brief		�A�N�V�����̓ǂݍ���
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
					m_strErrorMsg += " * �v���O�C����������܂���F";
					m_strErrorMsg += itAct->strWord + "\n";
					return false;
				}

				itAct++;
				if( itAct == vecScript.end() ){return false;}

				CAction*	lpAction = lpFunc();
				key_info	keyInfo;
				if( !lpAction->ParseScript( itAct->vecChild, m_Plugin, keyInfo ) )
				{
					m_strErrorMsg += "�z�b�g�L�[�̋L�q���s���ł��B\n";
					delete lpAction;
					return false;
				}
				if( !registAction( keyInfo, dynamic_cast<CAction*>(lpAction) ) )
				{
					m_strErrorMsg += "�z�b�g�L�[���̓o�^�Ɏ��s���܂����B\n"
									 "���̃L�[�͂��łɎg�p����Ă���\��������܂��B\n �� ";
					m_strErrorMsg += showKeyData(keyInfo);
					m_strErrorMsg += '\n';

					return false;
				}

			}
		}
		return true;
	}

	/**
		@brief		�ݒ�̃t�@�C������̓ǂݍ���
		@return		true:OK/false:NG
	 */
	int	loadConfig(void)
	{

		script_vector	vecScript;
		if( !ScriptLoadFromFile( _CONFIG_FILE, vecScript ) )
		{
			string strMsg = "�X�N���v�g�t�@�C���̓ǂݍ��݂Ɏ��s���܂���" _CRLF;
			string strErr;
			if( RuntimeErrorGetMsg(strErr) )
			{
				strMsg += strErr;
			}

			m_strErrorMsg += strMsg;
			return false;
		}

		script_vector::iterator itKey = ScriptFindKey("hottext", vecScript.begin(), vecScript );
		if( itKey == vecScript.end() ){ m_strErrorMsg = "�^�O'hottext'��������܂���";return false; }
		++itKey;
		if( itKey == vecScript.end() ){ m_strErrorMsg = "�^�O'hottext'�̋L�q�ӏ����t�@�C���I�[�ł�";return false; }

		if( !loadOption(itKey->vecChild) )
		{
			m_strErrorMsg += "�I�v�V�������̓ǂݍ��݂Ɏ��s���܂���";
			return false;
		}

		if( !loadAction(itKey->vecChild) )
		{
			m_strErrorMsg += "�z�b�g�L�[�ݒ�̓ǂݍ��݂Ɏ��s���܂���";
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
			MessageBox( m_hWnd, "�_�C�A���O�̍쐬�Ɏ��s���܂���", _APP_NAME, MB_OK );
			return false;
		}

		// �e��ݒ�̓ǂݏo��
		if( !loadConfig() )
		{
			string strMsg = string("�ݒ�t�@�C���̓ǂݍ��݂Ɏ��s���܂���" _CRLF) + m_strErrorMsg;
			MessageBox( m_hWnd, strMsg.c_str() ,
								_APP_NAME, MB_OK );
			return false;
		}

		// �f�t�H���g�̃E�C���h�E��Ԃ̐ݒ�
		if( m_Option.bHide )
		{
			ShowWindow(m_hWnd, SW_HIDE);
		} else
		{
			ShowWindow(m_hWnd, SW_SHOWDEFAULT);
		}

		// �^�X�N�g���C�փA�C�R����o�^
		
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
			MessageBox( m_hWnd, "�^�X�N�g���C�̓o�^�Ɏ��s���܂���", _APP_NAME, MB_OK );
			return	false;
		}

		return true;
	}

	/**
		@brief		���C�������̎��s
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
	@brief		�v���O�����̃��C��
	@return		0:OK/����:NG
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
		MessageBox( NULL, "��O���������܂���", _APP_NAME, MB_OK );
	}
	return 0L;
}