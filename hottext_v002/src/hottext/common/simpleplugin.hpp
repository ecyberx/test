// $Id: simpleplugin.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	プラグイン制御クラス関連
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:55 $
 */

#ifndef	_SIMPLE_PLUGIN_HPP
#define	_SIMPLE_PLUGIN_HPP

#include <algorithm>
#include "./simplefilesrch.hpp"

namespace simple {		/// namespace

#ifndef _NUMOF
#define	_NUMOF(x)		(sizeof(x) / sizeof(*(x)))
#endif

/******************************************************************************
	@brief	デバッガへの出力
 ******************************************************************************/
inline void	_Trace( LPCSTR strMsg, ... )
{
	char	buf[512];
	va_list	args;
	va_start( args, strMsg );
	_vsnprintf( buf, _NUMOF(buf), strMsg, args );
	buf[_NUMOF(buf)-1] ='\0';

	OutputDebugString( buf );
}
/******************************************************************************
	@brief	デバッガへの出力(時間を追加)
 ******************************************************************************/
inline void	_TraceLog( LPCSTR strMsg, ... )
{
	char	buf[512];
	va_list	args;
	va_start( args, strMsg );
	_vsnprintf( buf, _NUMOF(buf), strMsg, args );
	buf[_NUMOF(buf)-1] ='\0';

	char	buf2[512];
	sprintf( buf2, "%d: %s\n", timeGetTime(), buf );
	OutputDebugString( buf2 );
}

/******************************************************************************
	@brief	デバッグ用定義
 ******************************************************************************/
#ifdef	_DEBUG
#ifndef _MY_TRACE
#define	_MY_TRACE			simple::_Trace
#endif

#ifndef _MY_TRACE_LOG
#define	_MY_TRACE_LOG		simple::_TraceLog
#endif

#ifndef _MY_ASSERT
#define	_MY_ASSERT(x)		if( (x) ){}else {_TRACE( "[ASSERT]:%s,%l\n", __FILE__, __LINE__ );exit(EXIT_FAILURE);}
#endif

#else

#ifndef	_MY_TRACE
#define	_MY_TRACE			if(1);else
#endif

#ifndef	_MY_TRACE_LOG
#define	_MY_TRACE_LOG		if(1);else
#endif

#ifndef	_MY_ASSERT
#define	_MY_ASSERT			if(1);else
#endif

#endif


/// 関数をエクスポートする場合の定義
///  ex)
///		_EXPORT(CFrameObject*)	CreateFrameObject( script_vector& vecScript, void*, long lWaitTime )
#define  _EXPORT(x)	extern "C" _declspec(dllexport) x _cdecl
/// インタフェース関数の定義
#define  _INTERFACE(x)	extern "C"  x _cdecl

/**
	@brief	プラグイン制御クラス
			指定したフォルダのプラグインを検索し、すべて読み込む
 */
class	CPlugIn {
public:
	/**
		@brief	プラグインファイル単体の情報構造体
	 */
	struct plugin_object {
		HINSTANCE		hDLL;				/// プラグインDLLインスタンス
		tstring			tstrName;			/// プラグイン名(ファイル名)
		bool			bInstance;			/// プラグイン制御クラスがプラグインのインスタンスを持っているか
	};
	/// 文字列のコンテナ定義
	typedef std::vector< tstring >			string_vector;
	/// プラグインファイルのコンテナ定義
	typedef std::vector< plugin_object >	plugin_vector;
private:
	plugin_vector	m_vecPI;				/// プラグインファイルのコンテナ
public:
	/**
		@brief	デストラクタ
				読み込んだすべてのプラグインの解放
	 */
	virtual ~CPlugIn(void)
	{
		// DLLの解放
		plugin_vector::iterator it = m_vecPI.begin();

		for( ; it != m_vecPI.end() ; it++ )
		{
			if( it->bInstance )
			{
				FreeLibrary( it->hDLL );
			}
		}
	}
	/**
		@brief	デフォルトコンストラクタ
	 */
	CPlugIn(void)
	{}
	/**
		@brief	コンストラクタ
				プラグインを検索し読み込む
		@param	tszPath			検索するパス
		@param	tszExt			検索する拡張子(*.dll;*.exeなど';'で区切り複数指定可能)
		@param	bSearchChild	サブディレクトリを検索するか？
	 */
	CPlugIn(const _TCHAR* tszPath, const _TCHAR* tszExt, bool bSearchChild=true )
	{
		SearchPlugin( tszPath, tszExt, bSearchChild );
	}
	/**
		@brief	プラグインをフォルダから検索し読み込む
		@param	tszPath			検索するパス
		@param	tszExt			検索する拡張子(*.dll;*.exeなど';'で区切り複数指定可能)
		@param	bSearchChild	サブディレクトリを検索するか？
	 */
	void	SearchPlugin(const _TCHAR* tszPath, const _TCHAR* tszExt, bool bSearchChild=true )
	{
		string_vector	vecFilelist;
		CFileSearch( vecFilelist, tszPath, tszExt, bSearchChild );
		
		string_vector::iterator it = vecFilelist.begin();
		for( ; it != vecFilelist.end() ; it++ )
		{
			HINSTANCE	hDLL = LoadLibrary( it->c_str() );
			if( hDLL )
			{
				// DLLのロードに成功
				AddInstance( hDLL, it->c_str(), true );
			}
		}
	}
	/**
		@brief	インスタンスを追加
		@param	hInstance		追加するプラグインのインスタンス
		@param	tszName			プラグイン名
		@param	bInstance		追加するプラグインのインスタンス所有権
								プラグイン制御クラス解放時にインスタンスを解放するか
	 */
	void	AddInstance( HINSTANCE hInstance, const _TCHAR* tszName, bool bInstance )
	{
		plugin_object	obj;
		obj.hDLL = hInstance;
		obj.tstrName = tszName;
		obj.bInstance = bInstance ;
		m_vecPI.push_back( obj );
	}

	/**
		@brief	パスつきで指定したDLLのインスタンスを取得
		@param	tstrPath	パスつきのDLL名
		@return	取得したプラグインのインスタンス
				見つからなければNULLを返す
	*/
	HINSTANCE GetInstanceWithPath( const tstring& tstrPath )
	{
		plugin_vector::iterator	it = m_vecPI.begin();
		for( ; it != m_vecPI.end() ; it++ )
		{
			if( it->tstrName == tstrPath )return it->hDLL;
		}
		return NULL;
	}
	/**
		@brief	指定したDLLのインスタンスを取得
		@param	tstrPath	パスつきのDLL名
		@return	取得したプラグインのインスタンス
				見つからなければNULLを返す
	 */
	HINSTANCE GetInstance( const tstring& tstrPath )
	{

		if( tstrPath.find_last_of( "\\" ) != tstring::npos )
		{
			// パスつきプラグイン名の指定ならパスつき検索を行う
			return GetInstanceWithPath( tstrPath );
		}

		// パスからファイル名を取得
		size_t nPos;

		plugin_vector::iterator	it = m_vecPI.begin();
		for( ; it != m_vecPI.end() ; it++ )
		{
			nPos = it->tstrName.find_last_of( "\\" );
			if( nPos == tstring::npos )
			{
				// 検索プラグイン名にパスがない場合はそのまま比較
				if( it->tstrName == tstrPath )return it->hDLL;
			} else {
				// 検索プラグイン名にパスがある場合はパスを除いて比較
				if( it->tstrName.substr(nPos+1,tstring::npos) == tstrPath )
				{
					return it->hDLL;
				}
			}

		}
		return NULL;
	}

	/**
		@brief	指定したプロセスの取得
				"plugin.dll/Proc"のように"/"で区切って指定したプロセスを取得する
				上記例ではファイルplugin.dllから関数Procのアドレスを取得する
		@param	tstrPath	プロセスのパス
		@return	取得したプロセスのアドレス
		@exception	simple_error	プロセスの取得に失敗
	 */
	FARPROC	GetProcess( tstring tstrPath )
	{
		// プロセス名とパスの分離
		size_t n = tstrPath.find_last_of( "/" );
		if( n == tstrPath.npos )
		{
			return NULL;
		}
		tstring tstrProcName = tstrPath.substr( n+1, tstrPath.npos );
		tstring _tstrPath = tstrPath.substr( 0, n );
///		tstrPath.resize( n );//, tstrPath.length() );

		// パスの検索
		HINSTANCE hDLL = GetInstance( _tstrPath );
		if( hDLL == NULL )
		{
			return NULL;
		}

		// プロセスの取得
		FARPROC  pProc = GetProcAddress( hDLL, tstrProcName.c_str() );
		
		if( pProc == NULL )
		{
			return NULL;
		}
		return pProc;
	}

};
/**
	@brief	クラスプラグイン用の基本クラス例(simple_frame用)
 */
class	simple_plugin_object {
public:
	/**
		@brief	デストラクタ
	 */
	virtual ~simple_plugin_object(void){}
	/**
		@brief	インスタンスの取得
		@return	インスタンス
	 */
	virtual	HINSTANCE GetInstance(void)=0;
	/**
		@brief	プラグインの更新(simple_frame用)
		@param	nFrame	フレームカウンタ
		@return	true:フレームオブジェクトの処理続行
				false:フレームオブジェクトの解放要求
	 */
	virtual bool	Update( int nFrame )=0;
};
/// クラスプラグインオブジェクトを作成する関数の型定義
typedef simple_plugin_object*(*simple_plugin_interface)(void*);

/**
	@brief	クラスプラグイン管理クラス
			重複しないクラスプラグインオブジェクトの管理を行う
			addClassでプロセスからクラスプラグインオブジェクトTYPEを作成・取得。
			クラスプラグインオブジェクトの作成プロセスにはinterface_procを使用する
			シーンに登場する敵キャラの種類ごとにひとつずつ作成するオブジェクトなどに利用できる、
	@template	TYPE	クラスプラグインのクラスタイプ
						ex) CEnemyType
						    CC3DObject
	@template	interface_proc		クラスプラグイン作成用のインタフェース関数タイプ
									ex)  create_c3dobject_func
										( typedef CC3DObject*(create_c3dobject_func)( const TCHAR* tszID, void* ) )
	@template	t					クラスプラグイン作成時のパラメータタイプ
									ex) void*
										
 */
template< class TYPE, class interface_proc, class t >
class	CClassPlugIn {
private:
	/// 文字列コンテナの定義
	typedef std::vector< tstring >				string_vector;

	CPlugIn*	m_lpPlugIn;					/// プラグイン管理クラスの参照
//	typedef TYPE*(*interface_proc)(void);

	/**
		@brief	クラスプラグイン情報
	 */
	struct type_info 
	{
		unsigned int		uiReference;	/// リファレンスカウンタ
		TYPE*				pType;			/// プラグインクラスへのポインタ
		tstring				tstrProc;		/// プロセス名
		tstring				tstrID;			/// ID

		bool	operator ==( const TYPE* _pType )
		{
			return pType == _pType;
		}
		bool	operator ==( const type_info& info )
		{
			return (tstrProc == info.tstrProc)
					&& (tstrID == info.tstrID);
		}
	};
	/// クラスプラグイン情報コンテナの定義
	typedef std::vector< type_info >				type_vector;
	type_vector		m_vecType;				/// クラスプラグインのコンテナ


	/**
		@brief	クラスプラグインオブジェクトの作成
		@param	tszProc		作成に使用するプロセス名
		@param	param		作成に必要なパラメータ
		@return	作成したクラスプラグインオブジェクトへのポインタ
				作成に失敗した場合はNULLを返す
	 */
	TYPE*	createClass(	const _TCHAR* tszProc, 
							const _TCHAR* tszID, 
							t param )
	{
		// create type
		interface_proc*	pFunc = (interface_proc*)m_lpPlugIn->GetProcess( tszProc );

		if( pFunc == NULL )
		{
			return NULL;
		}

		TYPE* pType = pFunc( tszID, param );
		if( pType )
		{
			// オブジェクトの作成に成功
			// オブジェクトをリストに追加する
			type_info	info;
			info.uiReference = 0;
			info.pType = pType;
			info.tstrProc = tszProc;
			info.tstrID = tszID;
			m_vecType.push_back( info );

			// オブジェクトのポインタを返す

			_MY_TRACE( "simple::CClassPlugIn::createClass:%s, %s\n", tszProc, tszID );
			//RuntimeError( "simple::CClassPlugIn::createClass:%s, %s\n", tszProc, tszID );
			return pType;
		}

		// オブジェクトの作成に失敗した／プロセスが見つからなかった
		return NULL;
	}

public:

	/**
		@brief	デストラクタ
				作成したクラスプラグインをすべて解放
	 */
	virtual ~CClassPlugIn(void)
	{
		Release();
	}

	/**
		@brief	すべてのオブジェクトを解放
	 */
	void	Release( void )
	{
		// クラスプラグインオブジェクトの解放
		type_vector::iterator	it = m_vecType.begin();
		for( ; it != m_vecType.end() ; it++ )
		{
			_MY_TRACE( "simple::CClassPlugIn::Release:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );
			//RuntimeError( "simple::CClassPlugIn::Release:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );

			delete	it->pType;
			it->pType = NULL;
		}

		m_vecType.clear();

		m_lpPlugIn = NULL;
	}
	/**
		@brief	コンストラクタ
		@param	pi	参照するプラグイン管理クラス
	 */
	CClassPlugIn( void )
		: m_lpPlugIn( NULL )
	{}

	/**
	 */
	int	Create( CPlugIn& pi )
	{
		m_lpPlugIn = &pi;
		m_vecType.clear();
		return true;
	}

	CClassPlugIn (CPlugIn& pi)
		: m_lpPlugIn(&pi)
	{}


	// クラスプラグインオブジェクトの追加
	//	tszProcで指定したプロセスからクラスプラグインオブジェクトを作成する
	//	作成時にparamを引数としてプロセスへ渡す
	//	作成後、クラスプラグインオブジェクトのポインタを返す
	//	すでに同名のプロセスで作成済みの場合は、そのポインタを返す
	/**
		@brief	クラスプラグインオブジェクトの追加
				tszProcで指定したプロセスからクラスプラグインオブジェクトを作成する
				作成時にparamを引数としてプロセスへ渡す
				作成後、クラスプラグインオブジェクトのポインタを返す
				すでに同名のプロセスで作成済みの場合は、そのポインタを返す
		@param	tszProc		取得するクラスプラグインの作成プロセス名
		@param	param		作成時に使用するパラメータ
		@return	取得したクラスプラグインオブジェクトのポインタ
				失敗した場合はNULLを返す
	 */
	TYPE*	LoadClass(	const _TCHAR* tszProc, 
						const _TCHAR* tszID, 
						t param )
	{
		// search duplication
		type_vector::iterator	it = m_vecType.begin();
		for( ; it != m_vecType.end() ; it++ )
		{
			// 重複するオブジェクトがあれば、そのポインタを返す
			if( it->tstrProc == tszProc 
				&& it->tstrID == tszID	)
			{
				++it->uiReference;		// リファレンスカウンタをインクリメント
				return it->pType;
			}
		}

		TYPE* pType = createClass( tszProc, tszID, param );
		return pType;
	}

	/**
		@brief		オブジェクトの解放
		@return		true:オブジェクトを解放した／false:解放するオブジェクトが見つからない
	 */
	int		ReleaseClass(	TYPE* pType )
	{
		// 一致するオブジェクトを検索
		type_vector::iterator	it = std::find( m_vecType.begin(), m_vecType.end(), pType );

		if( it == m_vecType.end() )
		{
			// 見つからない
			return false;
		}


		if( it->uiReference > 0 )
		{
			// 参照がまだ残っている
			--(it->uiReference);
		} else
		{
			_MY_TRACE( "simple::CClassPlugIn::ReleaseClass:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );
			//RuntimeError( "simple::CClassPlugIn::ReleaseClass:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );

			// 参照がなくなったので、オブジェクトを削除する
			delete it->pType;
			it->pType = NULL;
			m_vecType.erase( it );
		}

		return true;
	}
};


}		/// namespace


#endif	// _SIMPLE_PLUGIN_HPP
