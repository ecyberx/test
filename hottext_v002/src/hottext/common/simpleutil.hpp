// $Id: simpleutil.hpp,v 1.1 2006/04/15 11:16:36 ander Exp $
/**
	@brief	ユーティリティ関数ヘッダファイル
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2006/04/15 11:16:36 $
	@note	本ヘッダファイルに含まれる各関数はsimpleutil.cppにて定義されている。
 */

#ifndef	_SIMPLE_UTIL_HPP
#define	_SIMPLE_UTIL_HPP

#include "./base.hpp"
#include "./simpleplugin.hpp"
#include "./simplescript.hpp"


namespace simple
{

/**
	@brief		スクリプト解析のバインドテーブル構造体
 */
struct	script_parse_bind
{
	const char*	szKey;			/// キー
	size_t		tOffset;		/// 対象のオフセットアドレス
	const char*	szProcess;		/// パラメータ取得用プロセス
	size_t		tSize;			/// 対象格納領域の確保サイズ
};
#define	SCRIPT_PARSE_BIND(_KEY,_CLASS,_VALUE,_PROCESS, _TYPE )	{_KEY, offsetof(_CLASS,_VALUE), _PROCESS, sizeof(_TYPE) }
/**
	@brief		スクリプトから構造体データを取得
	@param		vecScript		スクリプト
	@param		parseBind		バインドテーブル
	@param		nCount			バインドテーブルメンバ数
	@param		lpDest			出力先変数先頭アドレス
	@param		plugIn			プラグインの参照
	@return		成功した場合、設定した変数の数を返す。取得に失敗した変数があれば０を返す。
 */
int	ScriptParse( script_vector& vecScript, const script_parse_bind* parseBind, int nCount, void* lpDest, CPlugIn& plugIn );

};	// namespace simple
#endif	//_SIMPLE_UTIL_HPP
