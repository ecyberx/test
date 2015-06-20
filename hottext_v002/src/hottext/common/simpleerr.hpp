/**
	@file	simpleerr.hpp
	@brief	シンプルクラスライブラリで使用するエラー例外
	@author	Ander/Echigo-ya koubou
	@date	2004.7.17
 */
#ifndef	_SIMPLE_ERR_HPP
#define	_SIMPLE_ERR_HPP

#include <cstdarg>

namespace	simple {	// name space

/**
	@brief	シンプルクラスライブラリで使用するエラー例外の基本クラス
 */
class	CSimpleError {
protected:
	tstring	msg;						/// エラーメッセージ文字列
public:
	/**
		@brief	エラー定義(stringからの設定)
		@param	s	エラーメッセージ
	 */
	CSimpleError( tstring	s )
	 : msg(s){}

	/**
		@brief	エラー定義(char*からの設定)
		@param	s	エラーメッセージ
	 */
	CSimpleError( const _TCHAR* s )
	 : msg(s){}

	 /**
		@brief	エラーメッセージの取得
		@return	エラーメッセージ文字列
	 */
	virtual const _TCHAR*	getMsg(void){return msg.c_str();}
};
#include <vector>
#include <string>

/**
	@brief		ランタイムエラー情報クラス
 */
class	CRuntimeError 
{
public:
	typedef std::vector< std::string >		string_vector;

protected:
	string_vector		s_vecErrorLog;		/// エラーログ
public:
	virtual	~CRuntimeError( void )
	{
	}
	CRuntimeError( void )
	{
	}

	/**
		@brief		エラー追加
	 */
	void	AddError( const char* szFmt,... )
	{
		char buf[1024];
		va_list args;
		va_start(args, szFmt);

		_vsnprintf( buf, sizeof(buf), szFmt, args);

		s_vecErrorLog.push_back( buf );
	}
	
	/**
		@brief		エラー取得
		@note		古い順にエラー情報をひとつ取得する
		@return		true:エラーあり／false:エラーなし
	 */
	int	GetError( std::string& strDest )
	{
		if( !IsError() )
		{
			return false;
		}

		strDest = *(s_vecErrorLog.begin());
		s_vecErrorLog.erase(s_vecErrorLog.begin() );

		return true;
	}
	/**
		@brief		エラー有無のチェック
		@return		true:エラーあり／false:エラーなし
	 */
	int	IsError(void)
	{
		return	!s_vecErrorLog.empty();
	}
	/**
		@brief		全てのエラー情報取得
		@return		true:エラーあり／false:エラーなし
	 */
	int	GetErrorAll( std::string& strDest )
	{
		if( !IsError() )
		{
			return false;
		}
		std::string str;
		while( CRuntimeError::GetError( str ) )
		{
			strDest += str;
			strDest += "\r\n";
		}

		return true;
	}

};
/**
	@brief		アプリケーション全体のエラー情報クラス
	@note		アプリケーション全体でのエラー情報を保存する。
	@note		SimpleやC3Dなどでのエラー情報を保存／出力する処理をアプリケーションごとに
				作成するよりも共通のエラー情報処理を作成する方が手っ取り早いので作成。
				例外は便利だけどその後の復帰処理が不安なので...
 */
class	CRuntimeErrorApp
{
public:
	static	CRuntimeError	s_Error;
};

// ランタイムエラー関連定義
#define	RuntimeErrorClass		simple::CRuntimeErrorApp::s_Error				/// ランタイムエラー保存クラス定義
#define	RuntimeError			RuntimeErrorClass.AddError				/// ランタイムエラーの追加
#define	RuntimeErrorGetMsg( X )	RuntimeErrorClass.GetErrorAll( X )		/// ランタイムエラーの取得
#define	RuntimeErrorDefine		simple::CRuntimeError	RuntimeErrorClass		/// ランタイムエラーの定義

}	// namespace

#endif	// _SIMPLE_ERR_HPP

