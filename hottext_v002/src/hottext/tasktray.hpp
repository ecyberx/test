/**
	@brief		タスクトレイ制御クラスヘッダファイル
 */

#ifndef	INCLUDE_TASKTRAY_HPP
#define	INCLUDE_TASKTRAY_HPP

/**
	@brief		タスクトレイへアイコンを追加するクラス
 */
class CTaskTray 
{
public:
	struct TASKTRAYINFO 
	{
		HICON	hIcon;
		DWORD	dwIDTaskTray;
		UINT	uCallbackMessage;
		HWND	hwndNotify;
		HWND	hwndTray;
		string	strAppName;
	};

private:
	TASKTRAYINFO	m_Info;
	DWORD	m_dwtIDTray;
	DWORD	m_dwpIDTray;
public:
	virtual	~CTaskTray(void)
	{
		Release();
	}
	CTaskTray(void)
	{
		m_Info.hwndTray = NULL;
	}
	void Release(void)
	{
		if(m_Info.hwndTray)
		{
			NOTIFYICONDATA tnid; 
			
			tnid.cbSize = sizeof tnid; 
			tnid.hWnd = m_Info.hwndNotify; 
			tnid.uID = m_Info.dwIDTaskTray; 
			m_dwtIDTray = m_dwpIDTray = 0; 
			Shell_NotifyIcon(NIM_DELETE, &tnid);
			m_Info.hwndTray=NULL;
		}
	}
	int Create(TASKTRAYINFO& info)
	{
		m_Info = info;
		return SetTaskTray();
	}
	static BOOL CALLBACK EnumChildWndProc(HWND hwnd, LPARAM lParam)
	{
		static const TCHAR szNotifyWindow[] = ("TrayNotifyWnd"); 
		TCHAR szString[50]; 
	   
		GetClassName(hwnd, szString, sizeof(szString) / sizeof(TCHAR)); 

		if (lstrcmp(szString, szNotifyWindow) == 0) 
		{
			CTaskTray* self = reinterpret_cast<CTaskTray*>( lParam ); 
			self->m_Info.hwndTray = hwnd;
			return FALSE;
		} 
		return TRUE; 
	}
	int SetTaskTray(void)
	{
		//	タスクトレイのハンドルを探そう
		HWND hwndTray = FindWindow(("Shell_TrayWnd"), NULL);
		if(hwndTray==NULL)return false;

		EnumChildWindows(hwndTray, reinterpret_cast<WNDENUMPROC>( EnumChildWndProc ), reinterpret_cast<LPARAM>(this) );
		if(hwndTray==NULL)return false;
		m_Info.hwndTray=hwndTray;

		m_dwtIDTray = GetWindowThreadProcessId(m_Info.hwndTray,&m_dwpIDTray);
		if(!SetTaskTrayIcon(NIM_ADD) )
		{
			return false;
		}
		return true;
	}
	BOOL SetTaskTrayIcon(DWORD dwOpe)
	{
		// Register my icon 
		NOTIFYICONDATA tnid; 
	   
		tnid.cbSize = sizeof tnid; 
		tnid.hWnd = m_Info.hwndNotify; 
		tnid.uID = m_Info.dwIDTaskTray; 
		tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
		tnid.uCallbackMessage = m_Info.uCallbackMessage; 
		tnid.hIcon = m_Info.hIcon; 
		strncpy( tnid.szTip, m_Info.strAppName.c_str(), sizeof(tnid.szTip)-1);
	   
		return Shell_NotifyIcon(dwOpe, &tnid); 
	}
};

#endif	// INCLUDE_TASKTRAY_HPP
