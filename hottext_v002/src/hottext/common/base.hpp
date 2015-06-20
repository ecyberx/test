// $Id: base.hpp,v 1.2 2005/07/07 23:35:54 Ander Exp $
/**
	@file	base.hpp
	@brief	シンプルクラスライブラリの基本ヘッダファイル
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:54 $
 */

#ifndef _SIMPLE_BASE_HPP
#define	_SIMPLE_BASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <tchar.h>

namespace simple {
/// TCHARのstringクラス定義
typedef std::basic_string<_TCHAR>			tstring;
/// TCHARのstringstreamクラス定義
typedef std::basic_stringstream<_TCHAR>		tstringstream;
/// TCHARのistreamクラス定義
typedef std::basic_istream<_TCHAR>			tistream;
/// TCHARのostreamクラス定義
typedef std::basic_ostream<_TCHAR>			tostream;
/// TCHARのfstreamクラス定義
typedef std::basic_fstream<_TCHAR>			tfstream;
/// TCHAR文字列のコンテナクラス定義
typedef std::vector< tstring >				string_vector;

#ifdef _UNICODE
#define	tcout	wcout			/// TCHARの標準出力先定義
#define	tcin	wcin			/// TCHARの標準入力先定義
#else
#define	tcout	cout			/// TCHARの標準出力先定義
#define	tcin	cin				/// TCHARの標準入力先定義
#endif

}
#include "./simpleerr.hpp"

#endif	// _SIMPLE_BASE_HPP
