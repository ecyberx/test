// $Id: simplescript.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	スクリプトヘッダファイル
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:55 $
 */

#ifndef _SIMPLE_SCRIPT_HPP
#define	_SIMPLE_SCRIPT_HPP

#include "./base.hpp"
#include "./simpletext.hpp"
#include "./simplepath.hpp"

#include <stack>
#include <algorithm>
#include <windows.h>

namespace simple {		/// namespace simple



////////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	スクリプトオブジェクト構造体
 */
struct SCRIPTOBJECT_INFO {
	tstring			tstrFNameSrc;		/// ワードが含まれるスクリプトファイル(ヘッダファイルに記載されている場合はヘッダファイル名を格納)
	tstring			tstrFNameBase;		/// 読み込んだスクリプトファイル
	unsigned long	ulLine;				/// 行番号
	void*			lpParam;			/// スクリプトに付与される任意のパラメータ(アプリ層で付与)

	SCRIPTOBJECT_INFO(void)
		: lpParam( NULL )
	{}
};


/**
	@brief	スクリプトオブジェクト(単語単位)
 */
struct CScriptObject 
{
	typedef std::vector< CScriptObject >		script_vector;
	tstring			strWord;
	script_vector	vecChild;
	
	SCRIPTOBJECT_INFO	scrInfo;

	void	getStringLocate( tstring& strDest )
	{
		char buf[512];
		sprintf( buf, "%s(%d)", scrInfo.tstrFNameSrc.c_str(), scrInfo.ulLine );
		strDest = buf;
	}
	tstring	getStringLocate( void )
	{
		tstring	tstr;
		getStringLocate(tstr);
		return tstr;
	}

	void	getStringWord( const _TCHAR* tszExcept )
	{
		if( !CTextLineEx::IsStringWord( strWord ) )
		{
			throw( CSimpleError( tstring(tszExcept)) );
		}
	}
	/**
		@brief		文字列の取得
		@return		true:文字列を取得できた/false:単語は文字列ではなかった
		@remarks	成功した場合、単語の前後にあるダブルクォーテーションは削除される
	 */
	int		getStringWord( void )
	{
		return	CTextLineEx::IsStringWord( strWord );
	}

	bool	operator ==( const TCHAR* tsz ) const
	{
		return strWord == tsz;
	}
};

typedef std::vector< CScriptObject >	script_vector;



////////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	スクリプト演算処理
 */
template< class T = long >
class	CScriptCalculator 
{
public:
	/**
		@brief		エラー情報
	 */
	struct error 
	{
		unsigned char	ucCode;			/// エラーコード
		tstring			tstrMsg;		/// エラーメッセージ

		enum {
			CODE_NONE = 0,				/// 予約
			CODE_VALUE_NOT_MACTCH,		/// スクリプトを値へ変換できない
			CODE_OPE_NOT_MATCH,			/// スクリプトを演算子へ変換できない
			CODE_PARENT_FAIL,			/// 括弧の解析に失敗
		};

		error( void )
		{}
		error( unsigned char _ucCode, const _TCHAR* tchMsg )
			: ucCode( _ucCode )
			, tstrMsg( tchMsg )
		{}
	};
	typedef std::vector< error >	error_vector;
	/// 演算子
	struct	ope_func
	{
		unsigned int	uiLevel;		/// 演算レベル

		/**
			@brief		文字列が演算子に一致するかチェック
			@return		true:一致／false:不一致
			@param		tch		チェックする文字列
		 */
		virtual bool	isMatch( const TCHAR* tch ) = 0;

		/**
			@brief		演算の実行
			@return		true:正常終了／false:エラー
			@param		dest	結果の格納先
			@param		s1		ソース1
			@param		s2		ソース2
		 */
		virtual bool	calculate( T& dest, const T& s1, const T& s2 ) = 0;
		virtual ~ope_func(void){}
		ope_func(void){}

		// 演算子優先順位(ガイドライン) 
		enum 
		{
			PRIORITY_NONE = 0,		/// 何もしない
			PRIORITY_CONDITION,		/// 条件
			PRIORITY_SHIFT,			/// シフト演算
			PRIORITY_PLUSMINUS,		/// 加減算
			PRIORITY_MULTIDIV,		/// 乗除／剰余算
			PRIORITY_BIT,			/// ビット演算
		};

	};
	typedef	std::vector< ope_func* >		ope_vector;			/// 演算子情報コンテナタイプ
	typedef int (calc_convert)( T& dest, const TCHAR* tsz );	/// 文字列→値変換関数タイプ

	/// 設定
	struct config
	{
		ope_vector		vecOperator;		/// サポートする演算子
		calc_convert*	lpValueConvert;		/// 文字列から値へ変換する関数
		T				zero;				/// 0を示す値
	};
	/// 演算要素(値/演算子)の基本クラス
	struct	calc_element
	{
		enum 
		{
			TYPE_UNKNOWN = 0,		/// 未定義
			TYPE_VALUE,				/// 値
			TYPE_OPERATOR,			/// 演算子
		};
		CScriptObject*	lpScriptObject;
		virtual unsigned char	getType( void ) = 0;
	};

	struct	ope : public calc_element
	{
		ope_func*	lpOpe;
		virtual unsigned char	getType( void )
		{
			return TYPE_OPERATOR;
		}
	};

	struct	value : public calc_element
	{
		// 暫定的に整数型のみ
		T	_v;
		virtual unsigned char	getType( void )
		{
			return TYPE_VALUE;
		}
	};

	typedef std::vector< calc_element* >	element_vector;


private:
	error_vector	m_vecErrors;

	/// Phase
	enum
	{
		PHASE_VALUE = 0,		/// 値
		PHASE_OPERATOR,			/// 演算子
	};

	unsigned char	m_ucPhase;	/// スクリプト解析時のフェーズ
	config			m_Config;		/// 設定情報
	element_vector	m_vecElement;
	unsigned int	m_uiMaxLevel;		/// 演算レベルの最高レベル数

	/**
		@brief		要素をすべて解放
	 */
	void	releaseElement( void )
	{
		for( element_vector::iterator it = m_vecElement.begin()
			 ; it != m_vecElement.end()
			 ; it++ )
		{
			delete *it;
			*it = NULL;
		}
		m_vecElement.clear();
	}
	
	T		m_lResult;			/// 結果の格納用


	int		getPhaseValue( script_vector::iterator& it, script_vector& vecScript )
	{
		if( it == vecScript.end() )
		{
			RuntimeError("getPhaseValue::引数が不正です");
			return false;
		}
		// 単語が括弧か？
		if( it->strWord == "(" )
		{
			// カッコ内の演算式を解析
			CScriptCalculator<T>	calc( it->vecChild, m_Config );
			if( calc.GetErrorInfo() != NULL )
			{
				error	err;
				m_vecErrors.push_back( err );
				return false;
			}

			// 結果を値とする
			value*	pValue = new value;
			pValue->_v = calc.GetResult();
			m_vecElement.push_back( pValue );

			// 括弧の次の単語が")"か？
			it++;
			if( it == vecScript.end() )
			{
				error	err;
				m_vecErrors.push_back( err );
				return false;
			}
			if( it->strWord != ")" )
			{
				error	err;
				m_vecErrors.push_back( err );
				return false;
			}
		} else
		{
			// 値の取得
			value*	pValue = new value;
			if( !(m_Config.lpValueConvert)( pValue->_v, it->strWord.c_str() ) )
			{
				// 値の取得エラー
				return false;
			}
			m_vecElement.push_back( pValue );
		}
		return true;
	}

	/**
	 */
	int		isMatchOperator( const TCHAR* tsz )
	{
		for( ope_vector::iterator itOpe = m_Config.vecOperator.begin()
			 ; itOpe != m_Config.vecOperator.end()
			 ; itOpe++ )
		{
			if( (*itOpe)->isMatch( tsz ) )
			{
				// 演算子の作成
				ope*	lpOpe = new ope;
				lpOpe->lpOpe = *itOpe;
				m_vecElement.push_back( lpOpe );

				// 優先レベルの更新
				if( lpOpe->lpOpe->uiLevel > m_uiMaxLevel )
				{
					m_uiMaxLevel = lpOpe->lpOpe->uiLevel;
				}
				return true;
			}
		}

		return false;
	}

	/**
		@brief	演算子の解析
	 */
	int		getPhaseOperator( const TCHAR* tsz )//script_vector::iterator& it, script_vector& vecScript )
	{
		if( !isMatchOperator( tsz )  )
		{
			TCHAR	tch[2];
			tch[0] = tsz[0];
			tch[1] = NULL;
			if( isMatchOperator( tch ) )
			{
				// 残りを値とする
				// 値の取得
				value*	pValue = new value;
				if( !(m_Config.lpValueConvert)( pValue->_v, &tsz[1] ) )
				{
					// 値の取得エラー
					return false;
				}
				m_vecElement.push_back( pValue );
				return true;
			}
			// 演算子でない(値が連続)
			return false;
		}

		// フェーズ移行
		m_ucPhase = PHASE_VALUE;
		return true;
	}

	/**
		@brief	スクリプトを解析し、演算子と値の要素リストを作成
		@note	演算対象は、現在位置から","または";"までの区間。
	 */
	int		createElement( script_vector& vecScript, script_vector::iterator& it )
	{
		m_ucPhase = PHASE_VALUE;
		m_uiMaxLevel = 0;

		for( ; it != vecScript.end() && it->strWord != ";" && it->strWord != ","
			 ; it++ )
		{
			// フェーズをチェック
			switch( m_ucPhase )
			{
			case PHASE_VALUE:
				if( !getPhaseValue( it, vecScript ) )
				{
					// エラー
					return false;
				}
				// フェーズ移行
				m_ucPhase = PHASE_OPERATOR;
				break;
			case PHASE_OPERATOR:
				if( !getPhaseOperator( it->strWord.c_str() ) )
				{
					// エラー
					return false;
				}
				break;
			default:
				;
			}
		}

		return true;
	}

	/**
		@brief		演算要素情報の表示
		@note		デバッグ用
	 */
	void	showElement( calc_element* lpElement )
	{
		switch( lpElement->getType() )
		{
		case calc_element::TYPE_VALUE:
			{
				value*	lpValue = (value*)(lpElement);
				std::cout << "[value]" << lpValue->_v <<  std::endl;
			}
			break;
		case calc_element::TYPE_OPERATOR:
			std::cout << "[ope]" << std::endl;
			break;
		}
	}

	/**
		@brief		演算情報の表示
		@note		デバッグ用
	 */
	void	showElements( void )
	{
		std::cout << "show elements:" << std::endl;
		for( unsigned int i=0 ; i<m_vecElement.size() ; i++ )
		{
			showElement( m_vecElement[i] );
		}
	}

	/**
		@brief	演算の実行
	 */
	int		execCalculate( void )
	{
		m_lResult = m_Config.zero;

		for( unsigned int uiLevel = m_uiMaxLevel ; uiLevel > 0 || m_vecElement.size() > 1; uiLevel-- )
		{
			// レベルごとの演算(先頭から順に演算する)
			element_vector::iterator	itValue1 = NULL;		// 値1の参照を保存
			element_vector::iterator	itOpe = NULL;			// 最後に取得した演算子情報を保存

			for( element_vector::iterator itElement = m_vecElement.begin()
				 ; itElement != m_vecElement.end()
				 ; itElement++ )
			{
//				showElements();

				switch( (*itElement)->getType() )
				{
				case calc_element::TYPE_VALUE:
					if( (itOpe != NULL) && (itValue1 != NULL) )
					{
						ope_func*	lpOpe = ((ope*)(*itOpe))->lpOpe;
						// 演算レベルチェック(優先度の低い演算子はまだ演算しない)
						if( lpOpe->uiLevel == uiLevel )
						{
							value* lpValue1 = (value*)*itValue1;

							// 演算実行
							T tResult;
							if( !lpOpe->calculate(		tResult, 
														lpValue1->_v, 
														((value*)(*itElement))->_v ) )
							{
								// 演算に失敗
								return false;
							}

							lpValue1->_v = tResult;

							// 演算子～値2までの間を削除
							m_vecElement.erase( itOpe, itElement+1 );

							//  オペレータに従って演算を実行し、演算結果を一つ目の値の場所へ保存・演算子と二つ目の値を削除する
							itElement = itValue1;
						}

						// 演算子情報をクリア
						itOpe = NULL;
					}

					// 値1を現在の位置に設定
					itValue1 = itElement;
					break;
				case calc_element::TYPE_OPERATOR:
					// 演算子の処理
					itOpe = itElement;
					break;
				default:
					return false;
				}
			}
		}

		// 演算結果のチェック
		if( m_vecElement.size() != 1 )
		{
			// 最後に残った演算結果がひとつではない
			return false;
		}
		if( m_vecElement[0]->getType() != calc_element::TYPE_VALUE )
		{
			// 最後に残ったのが値ではない
			return false;
		}
		value*	lpVal = (value*)m_vecElement[0];
		m_lResult = lpVal->_v;

		return true;
	}

	void	execCalculate(	script_vector::iterator it,
							script_vector& vecScript )
	{
		// オブジェクトを演算子と値に分ける
		if( !createElement( vecScript,it ) )
		{
			// スクリプトが異常
			error err;
			m_vecErrors.push_back( err );
			return;
		}

		// 演算を実行
		if( !execCalculate() )
		{
			// 演算できない
			error err;
			m_vecErrors.push_back( err );
			return;
		}
	}

public:
	virtual ~CScriptCalculator( void )
	{
		releaseElement();
	}

	CScriptCalculator( script_vector& vecScript, const config& cfg, script_vector::iterator it = NULL )
		: m_Config( cfg )
	{
		if( it == NULL )
		{
			it = vecScript.begin();
		}
		execCalculate( it, vecScript );
	}

	/**
		@brief		演算結果の取得
	 */
	T	GetResult( void )
	{
		return m_lResult;
	}
	/**
		@brief		エラー情報の取得
	 */
	error_vector*	GetErrorInfo( void )
	{
		if( m_vecErrors.empty() )return NULL;
		return &m_vecErrors;
	}
};

/**
	@brief		文字列から整数型数値への変換
 */
inline int	CalcConvertLong( long& dest, const TCHAR* tsz )
{
	// HEXチェック
	if( strlen( tsz ) >= 3 )
	{
		if( tsz[0] == '0' && (tsz[1] == 'X' || tsz[1]=='x' ) )
		{
			// HEX
			dest = 0;
			tsz += 2;
			for( ; *tsz ; tsz++ )
			{
				int n = _HTOI( *tsz );
				if( n < 0 )
				{
					// 変換エラー
					return false;
				}
				dest <<= 4;
				dest |= n;
			}
			
			return true;
		}
	}
	// 文字チェック
	if( tsz[0] == '\'' )
	{
		tsz++;
		if( *tsz == NULL )
		{
			// 変換エラー
			return false;
		}
		dest = *tsz;
		tsz++;
		if( *tsz != '\'' )
		{
			return false;
		}
		return true;
	}
	dest = atoi( tsz );
	return true;
}
/**
	@brief		文字列から浮動小数点数値への変換
 */
inline int	CalcConvertFloat( float& dest, const TCHAR* tsz )
{
	dest = (float)atof( tsz );
	return true;
}

/**
	@brief	加算処理
 */
template< class T >
struct ope_func_long_plus : public CScriptCalculator< T >::ope_func
{
	virtual bool	isMatch( const TCHAR* tch )
	{
		return ( tstring( tch ) == "+" );
	}
	virtual bool	calculate( T& dest, const T& s1, const T& s2 )
	{
		dest = s1 + s2;
		return true;
	}

	ope_func_long_plus(void)
	{
		uiLevel = PRIORITY_PLUSMINUS;
	}
};
/**
	@brief	整数型の減算処理
 */
template< class T >
struct ope_func_long_minus : public CScriptCalculator< T >::ope_func
{
	virtual bool	isMatch( const TCHAR* tch )
	{
		return ( tstring( tch ) == "-" );
	}
	virtual bool	calculate( T& dest, const T& s1, const T& s2 )
	{
		dest = s1 - s2;
		return true;
	}

	ope_func_long_minus(void)
	{
		uiLevel = PRIORITY_PLUSMINUS;
	}
};
/**
	@brief	整数型の乗算処理
 */
template< class T >
struct ope_func_long_multi : public CScriptCalculator< T >::ope_func
{
	virtual bool	isMatch( const TCHAR* tch )
	{
		return ( tstring( tch ) == "*" );
	}
	virtual bool	calculate( T& dest, const T& s1, const T& s2 )
	{
		dest = s1 * s2;
		return true;
	}

	ope_func_long_multi(void)
	{
		uiLevel = PRIORITY_MULTIDIV;
	}
};
/**
	@brief	乗算処理
 */
template< class T >
struct ope_func_long_div : public CScriptCalculator< T >::ope_func
{
	virtual bool	isMatch( const TCHAR* tch )
	{
		return ( tstring( tch ) == "/" );
	}
	virtual bool	calculate( T& dest, const T& s1, const T& s2 )
	{
		dest = s1 / s2;
		return true;
	}

	ope_func_long_div(void)
	{
		uiLevel = PRIORITY_MULTIDIV;
	}
};
/**
	@brief	剰余算処理
 */
template< class T >
struct ope_func_long_mod : public CScriptCalculator< T >::ope_func
{
	virtual bool	isMatch( const TCHAR* tch )
	{
		return ( tstring( tch ) == "%" );
	}
	virtual bool	calculate( T& dest, const T& s1, const T& s2 )
	{
		dest = s1 % s2;
		return true;
	}

	ope_func_long_mod(void)
	{
		uiLevel = PRIORITY_MULTIDIV;
	}
};
/**
	@brief	比較処理
 */
template< class T >
struct ope_func_long_equal : public CScriptCalculator< T >::ope_func
{
	virtual bool	isMatch( const TCHAR* tch )
	{
		return ( tstring( tch ) == "==" );
	}
	virtual bool	calculate( T& dest, const T& s1, const T& s2 )
	{
		dest = s1 == s2;
		return true;
	}

	ope_func_long_equal(void)
	{
		uiLevel = PRIORITY_CONDITION;
	}
};
#define	_DEFINE_OPE_FUNC( _TYPE, _OPE, _STR, _EXEC, _PRIORITY )						\
	template< class T >																\
	struct ope_func_##_TYPE##_##_OPE : public CScriptCalculator< T >::ope_func	    \
	{	                                                                            \
		virtual bool	isMatch( const TCHAR* tch )	                                \
		{	                                                                        \
			return ( tstring( tch ) == _STR );	                                    \
		}	                                                                        \
		virtual bool	calculate( T& dest, const T& s1, const T& s2 )	            \
		{	                                                                        \
			dest = s1 _EXEC s2;	                                                    \
			return true;	                                                        \
		}	                                                                        \
	                                                                                \
		ope_func_##_TYPE##_##_OPE(void)	                                            \
		{	                                                                        \
			uiLevel = _PRIORITY;	                                                \
		}	                                                                        \
	};

_DEFINE_OPE_FUNC( long, and,	"&", &, PRIORITY_BIT );
_DEFINE_OPE_FUNC( long, or,		"|", |, PRIORITY_BIT );
_DEFINE_OPE_FUNC( long, xor,	"^", ^, PRIORITY_BIT );

/**
	@brief		整数演算セット
 */
struct	calc_long_set : public CScriptCalculator<long>::config
{
	ope_func_long_plus<long>	opePlus;
	ope_func_long_minus<long>	opeMinus;
	ope_func_long_multi<long>	opeMulti;
	ope_func_long_div<long>		opeDiv;
	ope_func_long_mod<long>		opeMod;
	ope_func_long_equal<long>	opeEqual;
	ope_func_long_and<long>		opeAnd;
	ope_func_long_or<long>		opeOr;
	ope_func_long_xor<long>		opeXor;

	virtual	~calc_long_set( void )
	{
	}

	calc_long_set( void )
	{
		zero = 0;
		lpValueConvert = CalcConvertLong;

		vecOperator.push_back( &opePlus );
		vecOperator.push_back( &opeMinus );
		vecOperator.push_back( &opeMulti );
		vecOperator.push_back( &opeDiv );
		vecOperator.push_back( &opeMod );
		vecOperator.push_back( &opeEqual );
		vecOperator.push_back( &opeAnd );
		vecOperator.push_back( &opeOr );
		vecOperator.push_back( &opeXor );
	}
};


/**
	@brief		浮動小数点演算セット
 */
struct	calc_float_set : public CScriptCalculator<float>::config
{
	ope_func_long_plus<float>	opePlus;
	ope_func_long_minus<float>	opeMinus;
	ope_func_long_multi<float>	opeMulti;
	ope_func_long_div<float>	opeDiv;

	virtual	~calc_float_set( void )
	{
	}

	calc_float_set( void )
	{
		zero = 0.0f;
		lpValueConvert = CalcConvertFloat;

		vecOperator.push_back( &opePlus );
		vecOperator.push_back( &opeMinus );
		vecOperator.push_back( &opeMulti );
		vecOperator.push_back( &opeDiv );
	}
};


// 演算コンフィグレーションセット
static const calc_long_set	calc_cfg_int;		/// 整数演算セット
static const calc_float_set	calc_cfg_float;		/// 浮動小数点演算セット


////////////////////////////////////////////////////////////////////////////////
// CScriptParser
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	スクリプト解析クラスの基本クラス
	@note	レイヤ１から単語単位でこのクラスのparseメソッドを呼び出す。
 */
class	CScriptParser {
public:
	/**
		@brief	文字列の解析仮想関数
		@note	この関数を継承してそれぞれの解析クラスを作成する
		@param	strWord		単語の文字列
		@param	scrInfo		単語の情報構造体
	 */
	virtual bool parse( tstring& strWord, SCRIPTOBJECT_INFO& scrInfo ) = 0;
};
////////////////////////////////////////////////////////////////////////////////
// ScriptError
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	スクリプトエラー
	@note	スクリプトから呼び出される、エラーの発生する各処理へ渡すこと
 */
class	CScriptError
{
public:
	/**
		@brief	スクリプトエラー情報構造体
	 */
	struct error_object 
	{
		tstring			tstrFilename;	/// ファイル名
		long			lLine;			/// エラー発生の行番号
		unsigned short	wErrorCode;		/// エラーコード
		tstring			tstrErrorMsg;	///	エラーメッセージ

		/// エラーコード
		enum {
			ERRORCODE_NONE = 0,
			ERRORCODE_FILE_CANT_OPEN,					/// ファイルのオープンに失敗
			ERRORCODE_CANT_FOUND_CLOSE,					/// 子階層が閉じられていない
			ERRORCODE_ILLEGAL_CLOSE,					/// 不正な子階層クローズ(開始が見つかっていないのに閉じられた)
			ERRORCODE_PARSE_FAIL,						/// Parseに失敗
			ERRORCODE_NOT_PARE,							/// 子階層の開始と完了が対でない
			ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT,	/// パラメータつきマクロ展開時にパラメータ指定がない
			ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM,		/// パラメータつきマクロ展開時にパラメータ指定が異常
			ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM_NUM,	/// パラメータつきマクロ展開時のパラメータ個数が異常
			ERRORCODE_MACRO_EXTRACT_FAIL,				/// マクロ展開中にエラー
			ERRORCODE_MACRO_DEFINE_FAIL,				/// マクロ定義中にエラー
			ERRORCODE_INCLUDE_DEPTH_OVER,				/// インクルード深度がオーバー
			ERRORCODE_INCLUDE_ILLEGAL_FILE,				/// インクルードのファイル指定が異常
			ERRORCODE_INCLUDE_FILE_CANT_OPEN,			/// インクルードファイルのオープンに失敗
			ERRORCODE_INCLUDE_FAIL,						/// インクルードファイルの解析でエラー
			ERRORCODE_DEFINE_CANT_FOUND_KEY,			/// マクロ定義でマクロキーが見つからない
			ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,		/// マクロ定義でパラメータ指定が異常
			ERRORCODE_DEFINE_FAIL,						/// マクロ定義命令でエラー
			ERRORCODE_IF_CANT_FOUND_ENDIF,				/// 条件命令で指定した完了コード(#ENDIF/#ELSE/#ELIF)が見つからないままEOFまできてしまった
			ERRORCODE_IFDEF_CANT_FOUND_CONDITION,		/// IFDEF文で条件式が見つからない
			ERRORCODE_IF_FAIL,							/// 条件命令でエラー
			ERRORCODE_ILLEGAL_ENDIF,					/// 不正な条件命令終端
			ERRORCODE_COMMAND_ILLEGAL,					/// 不正なプリプロセッサコマンド
			ERRORCODE_COMMAND_FAIL,						/// プリプロセッサコマンドエラー

		};

		/**
			@brief	何もしないコンストラクタ
		 */
		error_object( void )
		{}

		/**
			@brief	パラメータ設定
		 */
		error_object(	tstring			_tstrFilename,
						long			_lLine,
						unsigned short	_wErrorCode,
						tstring			_tstrErrorMsg )
			: tstrFilename( _tstrFilename )
			, lLine( _lLine )
			, wErrorCode( _wErrorCode )
			, tstrErrorMsg( _tstrErrorMsg )
		{}

	};
	
	typedef	std::vector< error_object >		error_vector;
private:
	error_vector	m_vecErrors;		/// エラー情報
public:
	/**
		@brief	何もしないコンストラクタ
	 */
	CScriptError( void )
	{}
	/**
		@brief	何もしないデストラクタ
	 */
	virtual ~CScriptError( void )
	{}
	/**
		@brief	発生したエラー情報を取得する
	 */
	const error_vector&	GetErrors( void )
	{
		return m_vecErrors;
	}
	/**
		@brief	エラーが発生したか確認
		@return	true:エラーあり/false:エラーなし
	 */
	bool	FoundError( void )
	{
		return !m_vecErrors.empty();
	}
	/**
		@brief	エラー情報の追加
	 */
	const CScriptError& operator += ( const CScriptError& s )
	{
		m_vecErrors.insert( m_vecErrors.begin(), s.m_vecErrors.begin(), s.m_vecErrors.end()  );
		return *this;
	}
	/**
		@brief	エラーの追加
	 */
	void	AddError( const error_object& errorObject )
	{
		m_vecErrors.push_back( errorObject );
	}

	/**
		@brief	エラー情報の出力
	 */
	void	OutErrorInfo( tostream& ost )
	{
		for( error_vector::iterator it = m_vecErrors.begin()
			 ; it != m_vecErrors.end()
			 ; it++ )
		{
			ost << "file:" << it->tstrFilename
				<< " line:" << it->lLine
				<< " code:" << it->wErrorCode
				<< " msg:"  << it->tstrErrorMsg
				<< std::endl;
		}
	}
	void	OutErrorInfoString( tstring& tstr )
	{
		for( error_vector::iterator it = m_vecErrors.begin()
			 ; it != m_vecErrors.end()
			 ; it++ )
		{
			char buf[2048];
			sprintf( buf, "file:%s line:%d code:%d msg:%s\r\n",
							 it->tstrFilename.c_str(),
							 it->lLine,
							it->wErrorCode,
							it->tstrErrorMsg.c_str() );
			tstr += buf;
			
		}
	}

};

////////////////////////////////////////////////////////////////////////////////
// CMacroObject
////////////////////////////////////////////////////////////////////////////////
/**
	@brief		スクリプトのマクロクラス
	@note		マクロのキー、パラメータ、展開文の設定・展開を行う。
 */
class	CMacroObject {
public:
	/// 文字列のコンテナタイプ定義
	typedef	std::vector< tstring >			string_vector;
	/// 文字列コンテナのコンテナタイプ定義
	typedef std::vector< string_vector >	string_string_vector;

	/**
		@brief	スクリプト結果を文字列のコンテナへ追加するクラス
	 */
	class	param_parser : public CScriptParser {
	protected:
		string_vector&		m_vecParam;			/// 結果の格納先コンテナの参照
	private:
		param_parser(const param_parser&);
		param_parser& operator=(const param_parser&);

	public:
		/**
			@brief	何もしないデストラクタ
		 */
		virtual ~param_parser(void)
		{}
		/**
			@brief	結果格納先コンテナの参照設定
			@param	vecParam	結果格納先コンテナ
		 */
		param_parser( string_vector& vecParam )
			: m_vecParam( vecParam )
		{}
		/**
			@brief	単語の解析・コンテナへの格納
			@param	strWord		追加する文字列
		 */
		virtual bool	parse( tstring& strWord, SCRIPTOBJECT_INFO& )
		{
			m_vecParam.push_back( strWord );
			// 常に成功を返す
			return true;
		}
	};

protected:
	tstring				m_strKey;			/// マクロキー
	string_vector		m_vecParam;			/// マクロパラメータ
	bool				m_bParamType;		/// 関数型マクロかどうか(関数型:TRUE/非関数型:FALSE)
	string_vector		m_vecContents;		/// マクロの変換後データ


	// メソッドgetExtractParamObjectの戻り値定義
	enum {
		RET_CONTINUE=0,	// 正常取得(続きあり)
		RET_TERMINAL,	// 正常取得(これで終わり)
		RET_ERR,		// エラー
	};
	/**
		@brief		展開するマクロパラメータのカンマ区切りで取得
		@return		RET_CONTINUE:正常取得(次のパラメータがある)
					RET_TERMINAL:正常取得(')'を検出)
					RET_ERR:エラー
		@param		vecStr		取得結果の保存先
		@param		line		行データ
		@param		Layer1		Scriptレイヤ１
	 */
	template< class LAYER1 >
	int	getExtractParamObject(	string_vector&	vecStr,
								CTextLineEx& line,
								LAYER1& Layer1 )
	{
		// 括弧の開始/終了文字
		//  ※ 開始文字と終了文字は対応して記述すること
		std::string	strBegin = "({[";	// 括弧開始文字
		std::string	strEnd =   ")}]";	// 括弧終了文字

		tstring strParam;				// 取得単語の一時格納用
		tstringstream sst;				// 展開するマクロパラメータの一時保存用
		bool bEmpty = true;				// マクロパラメータが空か?(空のマクロパラメータチェック用)
		std::stack<size_t>	stkPare;	// 括弧の対応チェック

		// 先頭1単語の取得
		if( line.GetWordEx(strParam) == false )
		{
			// マクロパラメータの途中でEOF
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
								_T("macro parameter illegal") );
			return RET_ERR;
		}
		for( ;; )
		{
			if( strParam.find('\"') == strParam.npos )
			{
				// ダブルクォーテーションで区切られた文字列以外なら括弧のチェックを行う

				// 括弧開始のチェック
				size_t	tPos = strBegin.find_first_of(strParam);
				if( tPos != strBegin.npos )
				{
					// 括弧開始が見つかった
					stkPare.push( tPos );
				} else
				{
					// 括弧閉じのチェック
					size_t	tPosEnd = strEnd.find_first_of(strParam);
					if( tPosEnd != strEnd.npos )
					{
						// 括弧閉じが見つかった
						if( stkPare.size() == 0 )
						{
							// パラメータ中に括弧開始がない状態での括弧閉じの検出
							if( tPosEnd != 0 )
							{
								// ')'以外で終了しようとした?
								Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
													_T("macro parameter illegal(illegal parent end charactor)") );
								return RET_ERR;
							}
							// マクロパラメータの終了
							break;
						}

						// 括弧開始との対応チェック
						size_t tLastBegin = stkPare.top();
						if( tPosEnd != tLastBegin )
						{
							// 括弧の対応が違う
							Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
												_T("macro parameter illegal(parent charactor err)") );
							return RET_ERR;
						}
						stkPare.pop();
					}
				}
			}

			if( strParam == _T(",") && stkPare.size() == 0 )
			{
				// パラメータの区切りを発見
				break;
			}

			// パラメータを設定
			sst << strParam << _T(" ");
			bEmpty = false;
			
			if( line.GetWordEx(strParam ) == false )
			{
				// マクロパラメータの途中でEOF
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
									_T("macro parameter illegal(EOF)") );
				return RET_ERR;
			}
		}
		if( bEmpty )
		{
			// 空のマクロパラメータ
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
								_T("macro parameter illegal(empty param)") );
			return RET_ERR;
		}
		
		// パラメータ解析
		param_parser	parser( vecStr );
		// パラメータ中のプリプロセッサ展開
		LAYER1( sst, parser, Layer1.GetMacro() );
		
		if( strParam == _T(")") )
		{
			return RET_TERMINAL;
		}
		return RET_CONTINUE;
	}
									

	/**
		@brief	関数型マクロのパラメータ取得
		@param	vecParam	関数型マクロのパラメータの格納先
		@param	line		入力するストリーム
		@param	Layer1		スクリプトの解析クラス
		@retval	vecParam	取得した関数マクロの展開用パラメータ
		@note	パラメータの解析のためにスクリプトレイヤ１を使用している
		@return	true:エラーなし／false:エラーあり
	 */
	template< class LAYER1 >
	bool	getExtractParam(	string_string_vector& vecParam, 
								CTextLineEx& line, 
								LAYER1& Layer1 )
	{
		tstring strParam ;
		
		// 次の'('を取得
		if( line.GetWordEx(strParam) == false )
		{
			// EOF
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT, 
								_T("function type macro, but '(' is not found(EOF)") );
			return false;
		}
		if( strParam != _T("(") )
		{
			// 関数型マクロなのに次の文字が'('ではない
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT, 
								_T("function type macro, but '(' is not found") );
			return false;
		}
		
		// パラメータの取得
		int nRet;
		do {
			// パラメータをカンマ区切りで取得
			string_vector	vecStr;
			nRet = getExtractParamObject<LAYER1>( vecStr, line, Layer1 );
			if( nRet == RET_ERR )
			{
				// パラメータの取得に失敗
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT, 
									_T("illegal macro parameter") );
				return false;
			}

			// パラメータを格納
			vecParam.push_back( vecStr );

		} while(nRet != RET_TERMINAL );

		// パラメータサイズチェック
		if( m_vecParam.size() != vecParam.size() )
		{
			// マクロパラメータの個数が一致しない
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
								_T("macro parameter illegal(Param count err)") );
			return false;
		}

		// 正常終了
		return true;
	}

public:
	/**
		@brief	何もしないデストラクタ
	 */
	virtual	~CMacroObject(void)
	{}
	/**
		@brief	何もしないコンストラクタ
	 */
	CMacroObject(void)
		: m_bParamType(false)
	{}

	/**
		@brief	マクロキーの設定
		@param	strKey	マクロキー文字列
	 */
	void	SetKey( tstring& strKey )
	{
		// キーの設定
		m_strKey = strKey;

	}
	/**
		@brief	関数型マクロのパラメータの追加
		@param	strParam	追加するパラメータ文字列
		@note	マクロ定義でのパラメータとして置き換えられる文字列を指定する。
	 */
	void	AddParam( tstring& strParam )
	{
		m_bParamType = true;				// マクロタイプ＝関数型マクロを設定

		m_vecParam.push_back( strParam );	// パラメータ追加
	}

	/**
		@brief	マクロの展開内容の定義
		@param	Layer1	スクリプトレイヤ１
		@param	ist		入力ストリーム
		@note	マクロの展開される内容をストリームから取得する。
		@note	取得したパラメータはスクリプトレイヤ１に従って解析された単語単位で格納される
		@return	true:エラーなし／false:エラーあり
	 */
	template< class LAYER1 >
	bool	setContents( LAYER1& Layer1, tistream& ist )
	{
		param_parser	parser( m_vecContents );
		LAYER1	_Layer1( ist, parser, Layer1.GetMacro() );

		CScriptError* lpError =  _Layer1.GetErrorInfo();
		if( lpError != NULL )
		{
			// マクロ作成中にEOF
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_DEFINE_FAIL, 
								_T("macro define failed.") );
			return false;
		}


		return true;
	}

	/**
		@brief	単語がマクロキーかチェック
		@param	strWord		チェックする文字列
		@return	true:文字列はこのマクロのキー
				／false:文字列はこのマクロのキーではない
	 */
	bool	IsMacroKey( tstring& strWord )
	{
		return strWord == m_strKey;
	}
	
	/**
		@brief	マクロを展開
		@param	Parser	展開データの出力先
		@param	Layer1	スクリプトレイヤ１
		@param	line	入力するパラメータのソース
		@exception	CSimpleError	マクロエラーによる例外
		@return	true:エラーなし／false:エラーあり
	 */
	template< class PARSER, class LAYER1 >
	bool	extractMacro(	PARSER& Parser,
							LAYER1& Layer1,
							CTextLineEx& line ) 
	{
		string_string_vector	vecParam;
		if( m_bParamType )
		{
			// 関数マクロのパラメータ取得
			if( !getExtractParam( vecParam, line, Layer1 ) )
			{
				// パラメータ取得に失敗
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
									_T("Macro Extract Error") );
				return false;
			}

			if( vecParam.size() != m_vecParam.size() )
			{
				// マクロのパラメータ個数異常
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM_NUM,
									_T("macro parameter size is wrong") );
				return false;
			}
		}
		// 単語オブジェクトの詳細を取得
		SCRIPTOBJECT_INFO	scrInfo;
		scrInfo.tstrFNameSrc = Layer1.GetFilename();
		scrInfo.ulLine = Layer1.GetLineNumber();

		string_vector::iterator it = m_vecContents.begin();
		for( ; it != m_vecContents.end() ; it++ )
		{
			if( m_bParamType )
			{
				// マクロパラメータのいずれかに該当するか？
				string_vector::iterator	itCnt = m_vecParam.begin();
				string_string_vector::iterator itParam = vecParam.begin();

				for( ; itCnt != m_vecParam.end() && *it != *itCnt ; itCnt++,itParam++ )
					;

				if( itCnt != m_vecParam.end() )
				{
					// パラメータ展開
					string_vector::iterator itExt = itParam->begin();
					for( ; itExt != itParam->end() ; itExt++ )
					{
						if( !Parser.parse( *itExt, scrInfo ) )
						{
							// パラメータ展開中にエラー
							Layer1.AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
												_T("Macro Extract Error") );
							return false;
						}
					}
					continue;
				}

			}
			if( !Parser.parse( *it, scrInfo ) )
			{
				// パラメータ展開中にエラー
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
									_T("Macro Extract Error") );
				return false;
			}
		}

		// 正常終了
		return true;
	}

	// 
	/**
		@brief	マクロの登録(簡易登録版)
		@param	szKey	マクロキー
		@param	szValue	マクロの展開内容
		@note	szValueには複数の単語を指定可能
	 */
	CMacroObject( const _TCHAR* szKey, const _TCHAR* szValue )
		: 	m_bParamType(false)
			, m_strKey(szKey)
	{
		tstringstream	sst( szValue );

		CTextLineEx	line( sst );
		tstring	strWord;
		while( line.GetWordEx( strWord ) )
		{
			m_vecContents.push_back( strWord );
		}
	}

	/**
		@brief	マクロ情報の出力
		@note	デバッグ用
	 */
	static void	ShowMacro( CMacroObject& macroObject, tostream& ost )
	{
		ost << "key:" << macroObject.m_strKey << std::endl;

		if( macroObject.m_bParamType )
		{
			// parameter type
			ost << " have parameter:" << std::endl;
			for( string_vector::iterator it = macroObject.m_vecParam.begin()
				 ; it != macroObject.m_vecParam.end()
				 ; it++ )
			{
				ost << "  " << *it << std::endl;
			}
		}
		
		if( macroObject.m_vecContents.empty() )
		{
			// key only
		} else 
		{
			// extract contents
			ost << " extract:" << std::endl;
			for( string_vector::iterator it = macroObject.m_vecContents.begin()
				 ; it != macroObject.m_vecContents.end()
				 ; it++ )
			{
				ost << "  " << *it << std::endl;
			}
		}
	}
};

typedef std::vector< CMacroObject >	macro_vector;


inline int ScriptCalculateInt( long& dest, const TCHAR* tsz, macro_vector& vecMacro );



////////////////////////////////////////////////////////////////////////////////
// CScriptLayer1
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	スクリプト解析レイヤ１(プリプロセッサ＋単語分割)
	@note	各種プリプロセッサコマンドの展開、単語単位の分割を行う
 */
class	CScriptLayer1 {
public:
	/// マクロコンテナタイプ
	typedef std::vector< CMacroObject >		macro_vector;
	/// テキストバッファのauto_ptrタイプ
	typedef std::auto_ptr< CTextLineEx >	ptr_simple_text_line_ex;

	/**
		@brief	現在読み込み中のファイル名の取得
		@return	ファイル名文字列のポインタ
	 */
	const TCHAR*	GetFilename(void)
	{
		return m_tszFName;
	}
	/**
		@brief	現在の行番号の取得
		@return	現在の行番号
	 */
	unsigned long	GetLineNumber(void)
	{
		return m_pLine->GetLineNumber();
	}
private:
	CScriptLayer1(const CScriptLayer1&);
	CScriptLayer1& operator=(const CScriptLayer1&);

protected:
	CPath						m_Path;						/// ファイルパス
	CScriptParser&				m_Parser;					/// 上位レイヤ
	int							m_nDepth;					/// インクルードの深度
	int							m_nMaxDepth;				/// インクルードの最大深度
	CTextLineEx*				m_pLine;					/// テキスト解析クラス
	macro_vector&				m_vecMacro;					/// マクロコンテナ
	tistream&					m_ist;						/// 入力ストリーム
	const TCHAR*				m_tszFName;					/// 入力ファイル名

	unsigned int				m_nEndStatus;				/// ファイル完了時の状態

	CScriptError				m_ScriptError;				/// スクリプトエラー情報
	
	/// ファイル完了時の状態
	enum _END_STATUS {
		/// EOF(ファイルの最後まで完了)
		_RET_EOF = 0,
		/// #endifでの復帰
		_RET_ENDIF,				
		/// #elseでの復帰
		_RET_ELSE,				
		/// #elifでの復帰
		_RET_ELIF,
		/// エラーで中断
		_RET_FAIL,
	};

	/// ファイル深度の設定
	enum {
		_MAX_DEPTH = 8										// デフォルトの最大深度
	};

	/**
		@brief	最後の読み込み後の状態取得
		@return ファイル読み込み完了時の状態(_END_STATUS参照)
	 */
	unsigned int	getEndStatus(void)
	{
		return m_nEndStatus;
	}

	/**
		@brief	#includeの解析
		@exception	CSimpleError	インクルードに関する例外
		@return	true:エラーなし／false:エラーあり
	 */
	bool	cmdInclude(void) 
	{
		// インクルードの深度チェック
		if( m_nDepth >= m_nMaxDepth )
		{
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_DEPTH_OVER,
						_T("include depth over") );
			return false;
		}

		tstring strFName;
		if( m_pLine->GetWord(strFName) == false )
		{
			// ファイル名の指定がおかしい(EOF)
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_ILLEGAL_FILE,
						_T("include file name is not descripted") );
			return false;
		}

		if( !CTextLineEx::IsStringWord( strFName ) )
		{
			// ファイル名の指定がおかしい
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_ILLEGAL_FILE,
						_T("include file name is illegal") );
			return false;
		}

		// File open
		// ファイル名を作成(path + fname)
		CPath	path( m_Path );

		path += strFName;
		tstring tstrFNameInclude = path.GetPath();
#ifdef _UNICODE
		// ファイル名取得のため、WCHAR文字列をCHAR文字列へ変換
		std::string	_strFNameInclude;
		for( tstring::iterator it = tstrFNameInclude.begin()
			; it != tstrFNameInclude.end() 
			; it++ )
		{
			char	c ;
			if( wctomb( &c, *it ) )	_strFNameInclude + c;
		}
		std::wfstream	ifst( _strFNameInclude.c_str() );
#else
		std::fstream	ifst( tstrFNameInclude.c_str() );
#endif
		if( !ifst.is_open() )
		{
			// ファイルのオープンに失敗
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_FILE_CANT_OPEN,
						tstrFNameInclude + _T(" couldn't open") );
			return false;
		}
		// インクルードファイルの読み込み
		CScriptLayer1	Layer1( ifst, m_Parser,
								m_vecMacro,
								tstrFNameInclude.c_str(),
								m_nDepth,
								m_nMaxDepth );
		// ファイルを閉じる
		ifst.close();

		// インクルードファイルの結果をチェック
		if( Layer1.GetErrorInfo() != NULL )
		{
			AddErrors( Layer1.GetErrorInfo() );
			// インクルードファイルでエラー発生
			AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
						tstrFNameInclude + _T(" parse fail.") );
			return false;
		}

		// 正常終了
		return true;
	}

	bool	macroGetKey(CMacroObject& macroObject, CTextLineEx& line )
	{
		// キーの取得
		tstring	strKey;
		if( line.GetWordEx(strKey) == false )
		{
			// キーがみつからない(EOF)
			AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_FOUND_KEY,
						_T("illegal #define(EOF)") );
			return false;
		}

		macroObject.SetKey( strKey );

		// 関数マクロ?
		if( line.GetPosCharactor() == '(' )
		{
			// 関数型マクロ
			// 引数を取得
			tstring		strParam ;
			// '('をスキップ
			if( line.GetWordEx(strParam) == false )
			{
				// EOF
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("illegal macro parameter(0)") );
				return false;
			}
			
			while(line.GetWordEx(strParam))
			{
				macroObject.AddParam( strParam );
				// カンマまたは')'を取得・スキップ
				if( line.GetWordEx( strParam ) == false )
				{
					// 行端が異常
					AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
								_T("illegal macro parameter(0)") );
					return false;
				}
				if( strParam == _T(")") )break;
				if( strParam != _T(",") )
				{
					// マクロパラメータの間にカンマがない
					AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
								_T("illegal macro parameter(not found ',')") );
					return false;
				}
			}
			if(  strParam != _T(")") )
			{
				// ')'がみつからないまま行端に到達
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("')' is not found") );
				return false;
			}
		}

		return true;
	}

	/**
		@brief	マクロ定義文(#define)の解析
		@return	true:エラーなし／false:エラーあり
	 */
	bool	cmdDefine(void) 
	{
		CMacroObject		macroObject;
		tstring	tstrLine;
		if( m_pLine->GetLine(tstrLine) == false )
		{
			// Keyがない(#define[EOF])
			AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_FOUND_KEY,
						_T("illegal #define(EOF)") );
			return false;
		}

		tstringstream	sst( tstrLine.c_str() );
		CTextLineEx	line(sst);

		if( !macroGetKey( macroObject, line ) )
		{
			return false;
		}

		// マクロの中身を取得
		tstring		strLine;
		if( line.GetLine(strLine) )
		{
			// パラメータ以降に文字列があれば、マクロ内容として取得
			tstringstream	sst( strLine.c_str() );
			if( !macroObject.setContents( *this, sst ) )
			{
				// パラメータの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("macro parameter error") );
				return false;
			}
		}

		m_vecMacro.push_back( macroObject );

		// 正常終了
		return true;
	}
	/**
		@brief	マクロ定義文(#define)の解析
		@return	true:エラーなし／false:エラーあり
	 */
	bool	cmdDefineBlock(void) 
	{
		CMacroObject		macroObject;
		tstring	tstrLine;
		if( m_pLine->GetLine(tstrLine) == false )
		{
			// Keyがない(#define[EOF])
			AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_FOUND_KEY,
						_T("illegal #define(EOF)") );
			return false;
		}

		tstringstream	sst( tstrLine.c_str() );
		CTextLineEx	line(sst);

		if( !macroGetKey( macroObject, line ) )
		{
			return false;
		}

		// マクロの中身を取得
		{
			tstring		strWord;
			tstringstream	sst;
			if( line.GetLine(strWord) )
			{
				sst << strWord << std::endl;
			}

			while( m_pLine->GetWordEx(strWord) )
			{
				if( strWord == "#enddef" )break;
				sst << strWord << std::endl;
			}
			if( strWord != "#enddef" )
			{
				// パラメータの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("macro parameter error") );
				return false;
			}

			// パラメータ以降に文字列があれば、マクロ内容として取得
			if( !macroObject.setContents( *this, sst ) )
			{
				// パラメータの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("macro parameter error") );
				return false;
			}
		}

		m_vecMacro.push_back( macroObject );

		// 正常終了
		return true;
	}

	/**
		@brief	読み飛ばすだけのスクリプト解析クラス
	 */
	class	skip_parser : public CScriptParser {
	public:
		/**
			@brief	何もしないデストラクタ
		 */
		virtual ~skip_parser(void)
		{}
		/**
			@brief	空の解析関数(単語の読み飛ばし)
			@param	strWord	取得した単語
		 */
		bool parse(tstring& , SCRIPTOBJECT_INFO& )
		{
			// 常に成功を返す
			return true;
		}
	};

	/**
		@brief	指定した完了コードまで読み飛ばす
		@param	uiUntil	完了コード(_END_STATUS参照)
		@exception	CSimpleError	指定した完了コード以外の完了コードが見つかった
		@note	#ifなどに対応するendif/elif/elseなどの完了コードまでを読み飛ばす。
				指定した以外の完了コードの場合は例外を投げる。
		@return	true:エラーあり／false:エラーなし
	 */
	bool	skipUntil( const unsigned int uiUntil ) 
	{
		// ENDIFまで読み飛ばす
		for(;;)
		{
			skip_parser		Parser;
			macro_vector	vecMacro;
			CScriptLayer1	l1( m_ist, Parser, vecMacro,
				m_Path.GetPath(), m_nDepth,
				m_nMaxDepth );
			if( l1.GetErrorInfo() == NULL )
			{
				if( l1.getEndStatus() != uiUntil )
				{
					// 指定した完了コード(#ENDIF/#ELSE/#ELIF)が見つからないままEOFまできてしまった
					AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
								_T("illegal endif/else/elif") );
					return false;
				}
				break;
			}
			// スキップ中のスクリプトのエラーは無視する

		}
		return true;
	}

	/**
		@brief	IF/IFDEFにてtrueの場合の処理
		@exception	CSimpleError	#ifに対応する#endifが見つからなかった。
		@return	true:エラーあり／false:エラーなし
	 */
	bool	getTrue(void) 
	{
		// ENDIForELSEまで取得
		CScriptLayer1	l1( m_ist, m_Parser, m_vecMacro,
			m_Path.GetPath(), m_nDepth,
			m_nMaxDepth );
		// エラーのチェック
		if( l1.GetErrorInfo() != NULL )
		{
			// エラーを発見
			AddErrors( l1.GetErrorInfo() );
			return false;
		}


		switch( l1.getEndStatus() )
		{
		case _RET_EOF:
			// 指定した終了コードが見つからなかった
			AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
						_T("illegal endif/else/elif") );
			return false;

		case _RET_ENDIF:
			break;
		case _RET_ELSE:
			if( !skipUntil( _RET_ENDIF ) )
			{
				// スキップ中にエラー発生
				AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
							_T("illegal endif/else/elif") );
				return false;
			}
			break;
		}

		return true;
	}
	/**
		@brief	IF/IFDEFにてfalseの場合の処理
		@exception	CSimpleError	対応するendifが見つからない、またはelseの後のelse。
	 */
	bool	getFalse(void) 
	{
		for(;;)
		{
			// elseまで読み飛ばす
			skip_parser	Parser;
			macro_vector	vecMacro;
			CScriptLayer1	l1( m_ist, Parser, vecMacro,
				m_Path.GetPath(), m_nDepth,
				m_nMaxDepth );

			// エラーのチェック
			if( l1.GetErrorInfo() == NULL )
			{
				// エラーなしの場合、スキップの終了がENDIF/ELSEかをチェック
				switch( l1.getEndStatus() )
				{
				case _RET_EOF:
					// 指定した終了コードが見つからなかった
					AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
								_T("endif is not found") );
					return false;
				case _RET_ENDIF:
					break;
				case _RET_ELIF:
					break;
				case _RET_ELSE:
					{
						// ELSE条件内のスクリプトを取得
						CScriptLayer1	l1( m_ist, m_Parser, m_vecMacro,
							m_Path.GetPath(), m_nDepth,
							m_nMaxDepth );
						// エラーのチェック
						if( l1.GetErrorInfo() != NULL )
						{
							// エラーを発見
							AddErrors( l1.GetErrorInfo() );
							return false;
						}
						
						switch( l1.getEndStatus() )
						{
						case _RET_EOF:
							// 指定した終了コードが見つからなかった
							AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
										_T("endif is not found") );
							return false;
						case _RET_ENDIF:
							break;
						default:
							// 指定した終了コードが見つからなかった
							AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
										_T("illegal else nesting") );
							return false;
						}
					}
					break;
				}
				break;
			}
		}

		// 正常終了
		return true;
	}

	/**
		@brief	IF命令
	 */
	int		cmdIf( void )
	{
		// 条件の取得
		tstring tstrLine;
		if( m_pLine->GetLine( tstrLine ) == false )
		{
			// 条件が記述されていない
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("if Conditions") );
			return false;
		}

		long lResult;
		if( !ScriptCalculateInt( lResult, tstrLine.c_str(), m_vecMacro ) )
		{
			// 条件式でエラー
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("if Condition error") );
			return false;
		}

		if( lResult )
		{
			// 条件＝true
			if( !getTrue() )
			{
				// スクリプトの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (true Condition) parse failed.") );
				return false;
			}
		} else {
			// 条件＝false
			if( !getFalse() )
			{
				// スクリプトの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (false Condition) parse failed.") );
				return false;
			}
		}

		return true;
	}

	/**
		@brief	IFDEFの解析
		@exception	CSimpleError	不正な#ifdef文
		@return	true:エラーあり／false:エラーなし
	 */
	bool	cmdIfdef(void) 
	{
		// 条件の取得
		tstring tstrLine;
		if( m_pLine->GetLine( tstrLine ) == false )
		{
			// 条件が記述されていない
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("ifdef Conditions") );
			return false;
		}
		tstringstream	sst( tstrLine.c_str() );
		CTextLineEx	line(sst);

		tstring strCnd ;
		if( line.GetWordEx(strCnd) == false )
		{
			// 条件が記述されていない
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("ifdef Conditions(1)") );
			return false;
		}
		// マクロキーが定義されている？
		macro_vector::iterator	it = m_vecMacro.begin();
		for( ; it != m_vecMacro.end() && !it->IsMacroKey(strCnd) ; it++ )
			;
		if( it != m_vecMacro.end() )
		{
			// 条件＝true
			if( !getTrue() )
			{
				// スクリプトの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (true Condition) parse failed.") );
				return false;
			}
		} else {
			// 条件＝false
			if( !getFalse() )
			{
				// スクリプトの取得に失敗
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (false Condition) parse failed.") );
				return false;
			}
		}
		// 正常終了
		return true;
	}

	/**
		@brief	プリプロセッサコマンド解析
		@param	strWord		取得した文字列(#～の文字列)
		@param	bContinue	true ファイル読み込みを継続／false:ファイル完了
		@return	true:エラーなし／false:エラーあり
		@exception	CSimpleError	プリプロセッサではない/形式が異常
	 */
	bool	analyzePreprocess( tstring& strWord, bool& bContinue ) 
	{
		// ファイル継続読み込みをtrueに設定
		bContinue = true;
		// 
		strWord.erase( strWord.begin(), strWord.begin()+1 );	// １文字目'#'をスキップ

		if( strWord.compare(_T("include"))==0 )
		{
			// ファイルのインクルード
			if( !cmdInclude() )
			{
				// ファイルのインクルードに失敗
				AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_FAIL,
							_T("include failed.") );
				return false;
			}
		} else if( strWord.compare(_T("define"))==0 )
		{
			// マクロ定義
			if( !cmdDefine() )
			{
				// マクロ定義に失敗
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_FAIL,
							_T("define failed.") );
				return false;
			}
		} else if( strWord.compare(_T("define_begin"))==0 )
		{
			// マクロ定義
			if( !cmdDefineBlock() )
			{
				// マクロ定義に失敗
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_FAIL,
							_T("define failed.") );
				return false;
			}
		} else if( strWord.compare(_T("ifdef"))==0 )
		{
			// ifdef
			if( !cmdIfdef() )
			{
				// 条件命令に失敗
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("ifdef failed.") );
				return false;
			}
		} else if( strWord == _T("endif") )
		{
			// ENDIFが見つかったので、読み込みを中断
			m_nEndStatus = _RET_ENDIF;
			bContinue = false;
			return true;
		} else if( strWord == _T("else") )
		{
			// #ELSEが見つかったので、読み込みを中断
			m_nEndStatus = _RET_ELSE;
			bContinue = false;
			return true;
		} else if( strWord == _T("elif") )
		{
			// #ELIFが見つかったので、読み込みを中断
			m_nEndStatus = _RET_ELIF;
			bContinue = false;
			return true;
		} else if( strWord == _T("if") )
		{
			// #IF
			if( !cmdIf() )
			{
				// 条件命令に失敗
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if failed.") );
				return false;
			}

		} else {
			// 未定義のプリプロセッサコマンド
			AddError(	CScriptError::error_object::ERRORCODE_COMMAND_ILLEGAL,
						_T("undefined preprocess command") );
			return false;
		}

		// 正常終了
		return true;
	}

	/**
		@brief	単語がマクロキーに該当するかチェックし、マクロであれば展開・trueを返す。
		@param	strWord	チェックする文字列
		@param	bHit	true:単語がマクロキーに該当(展開を完了)
						／false:単語はマクロキーではない
		@return	true:エラーなし／false:エラーあり
	 */
	bool	parseMacro( tstring& strWord, bool& bHit )
	{
		// マクロキー検索
		macro_vector::iterator	it = m_vecMacro.begin();
		for( ; it != m_vecMacro.end() ; it++ )
		{
			if( it->IsMacroKey( strWord ) )
			{
				// マクロキーが見つかったので、マクロを展開
				if( !(it->extractMacro)( m_Parser, *this, *m_pLine  ) )
				{
					// マクロ展開中にエラー
					AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_FAIL,
								_T("macro extract fail.") );
					return false;
				}
				bHit = true;
				return true;
			}
		}
		// マクロキーが見つからなかった
		bHit = false;
		return true;
	}

public:
	/**
		@brief	何もしないデストラクタ
	 */
	virtual	~CScriptLayer1(void)
	{}
	// コンストラクタ
	//	Parser :	上位レイヤ
	//	szFName :	ファイル名(インクルードパスの指定のため)
	//	nDepth :	インクルードの深度
	/**
		@brief	スクリプトレイヤ１
		@param	Parser		上位レイヤ
		@param	ist			入力ストリーム
		@param	vecMacro	マクロコンテナ
		@param	szFName		ファイル名(インクルードパスの指定などに使用)
		@param	nDepth		インクルードファイルの深度
		@param	nMaxDepth	インクルードファイルの最大深度
		@exception	CSimpleError	インクルードファイルの最大深度を超えた
										／スクリプトの読み出し異常
		@note	文字列を単語単位に分解し、上位レイヤ(parser)へ単語を渡す
	 */
	CScriptLayer1(	tistream& ist,
							CScriptParser& Parser,
							macro_vector& vecMacro,
							const _TCHAR* szFName=_T(""), 
							int nDepth=0,
							int nMaxDepth=_MAX_DEPTH ) 
		: m_Parser(Parser), m_Path( szFName ), m_nDepth( nDepth ), m_nMaxDepth(nMaxDepth)
		, m_vecMacro(vecMacro), m_ist(ist)
		, m_nEndStatus( _RET_EOF )
		, m_tszFName(szFName)
	{
		// ネスティングデプスのチェック
		++m_nDepth;
		if( m_nDepth >= m_nMaxDepth )
		{
			// プリプロセスコマンドエラー
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_DEPTH_OVER,
						_T("over Depth level") );
			return;
		}

		// フォルダパスの取得
		m_Path--;				// ファイル名をカット
		// ファイルバッファの作成
		ptr_simple_text_line_ex	ptrLine( new CTextLineEx(ist) );
		m_pLine = ptrLine.get();

		SCRIPTOBJECT_INFO	scrInfo;
		scrInfo.tstrFNameSrc = m_tszFName;
		scrInfo.ulLine = m_pLine->GetLineNumber();

		tstring strWord;
		// streamから全ての単語を取得するまでループ
		while( m_pLine->GetWordEx(strWord) )
		{
			// プリプロセッサコマンドのチェック
			if( strWord[0] == '#' )
			{
				// プリプロセッサコマンド
				bool	bContinue;	// ファイルの継続読み込みフラグ
				if( analyzePreprocess( strWord, bContinue ) == false )
				{
					// プリプロセスコマンドエラー
					AddError(	CScriptError::error_object::ERRORCODE_COMMAND_FAIL,
								_T("preprocess command fail.") );
					return;
				}
				if( bContinue == false )
				{
					// ファイル処理を完了
					break;
				}
			} else 
			{
				bool bHit = false;

				if( !parseMacro( strWord, bHit ) )
				{
					// マクロ展開中にエラー
					return;
				}
				if( bHit == false )
				{
					// マクロにもヒットしなかった＝通常の文字列
					// 行番号の取得
					scrInfo.ulLine = m_pLine->GetLineNumber();
					// 単語の追加
					if( !m_Parser.parse( strWord, scrInfo ) )
					{
						// 解析中にエラー発生
						m_ScriptError.AddError( CScriptError::error_object(	scrInfo.tstrFNameSrc,
												scrInfo.ulLine,
												CScriptError::error_object::ERRORCODE_PARSE_FAIL,
												_T("Parse failed.") ) );
						return;
					}
				}
			}
		}

		if( m_nDepth == 1 )
		{
			// デプスが最上位なら、末端をチェック
			switch( getEndStatus() )
			{
			case _RET_EOF:
				break;
			case _RET_ENDIF:
			case _RET_ELSE:
			case _RET_ELIF:
				// 不正な条件命令終端
				AddError(	CScriptError::error_object::ERRORCODE_ILLEGAL_ENDIF,
							_T("illegal endif/else/elif") );
				return;
			}
		}

	}
	// マクロの取得
	macro_vector&	GetMacro(void)
	{
		return	m_vecMacro;
	}
	/**
		@brief	エラー情報を取得する
		@return	エラーあり：エラー情報へのポインタ
				エラーなし：NULL
	 */
	CScriptError*	GetErrorInfo( void )
	{
		if( !m_ScriptError.FoundError() )
		{
			return NULL;
		}
		return &m_ScriptError;
	}

	/**
		@brief	エラー情報の追加
	 */
	void	AddError(	unsigned short		wErrorCode,
						tstring				tstrErrorMsg )
	{
		unsigned long lLine=0;
		if( m_pLine != NULL )
		{
			lLine = m_pLine->GetLineNumber();
		}
		m_ScriptError.AddError( CScriptError::error_object( m_tszFName, lLine,
															wErrorCode,
															tstrErrorMsg ) );
	}
	/**
	 */
	void	AddErrors(	CScriptError*	lpScriptError )
	{
		if( lpScriptError == NULL )
		{
			// エラーなし
		}
		// エラーを追加
		m_ScriptError += *lpScriptError;
	}
};

////////////////////////////////////////////////////////////////////////////////
// CScriptLayer2
//	スクリプト解析レイヤ２(ツリー構築)
//	{～}のツリー構造を構築する
////////////////////////////////////////////////////////////////////////////////
#define	__SCRIPT_PARENT_START	_T("{;(;[;begin")
#define	__SCRIPT_PARENT_END		_T("};);];end")

class	CScriptLayer2 : public CScriptParser 
{
public:

	typedef std::vector< CMacroObject >			macro_vector;
	typedef std::auto_ptr< macro_vector >		ptr_macro_vector;
	typedef std::vector< int >					int_vector;

protected:
	script_vector		m_vecChild;				/// 上位構造の保存用
	int_vector			m_vecLastParent;		/// 子階層開始情報の保存用

	tstring		m_tstrParentStart;				/// 子階層開始文字列
	tstring		m_tstrParentEnd;				/// 子階層終了文字列
	tstring		m_tstrFilenameBase;				/// スクリプトファイル名

	CScriptError		m_ScriptError;			/// エラー情報

public:
	virtual	~CScriptLayer2(void)
	{}

	/**
		@brief	スクリプトの読み込み(stream)
		@param	ist			ソースストリーム
		@param	vecScript	スクリプト格納先
		@param	vecMacro	マクロ情報
	 */
	CScriptLayer2(	tistream& ist,
					script_vector& vecScript,
					macro_vector& vecMacro,
					const _TCHAR*	szFName = _T("")
					) 
		:	m_tstrParentStart( __SCRIPT_PARENT_START ),
			m_tstrParentEnd( __SCRIPT_PARENT_END )
	{
		LoadScript( ist, vecScript, vecMacro, szFName );
	}

	/**
		@brief	スクリプトの読み込み(file)
	 */
	CScriptLayer2(	const TCHAR* tszFName,
					script_vector& vecScript,
					macro_vector& vecMacro  ) 
		:	m_tstrParentStart( __SCRIPT_PARENT_START ),
			m_tstrParentEnd( __SCRIPT_PARENT_END )
	{
		// ファイルを開く
		if( tszFName == NULL || tszFName[0] == NULL )
		{
			// ファイルのオープンに失敗
			CScriptError::error_object	errorObject(	_T("NULL"),
														0,
														CScriptError::error_object::ERRORCODE_FILE_CANT_OPEN,
														_T(" script file can't open") );
			m_ScriptError.AddError( errorObject );
			return;
		}
		tfstream	ifst( tszFName );
		if( ifst.is_open() == false )
		{
			// ファイルのオープンに失敗
			CScriptError::error_object	errorObject(	tszFName,
														0,
														CScriptError::error_object::ERRORCODE_FILE_CANT_OPEN,
														tstring(tszFName) + _T(" script file can't open") );
			m_ScriptError.AddError( errorObject );
			return;
		}

		LoadScript( ifst, vecScript, vecMacro, tszFName );

		// ファイルを閉じる
		ifst.close();
	}
	/**
		@brief	何もしないコンストラクタ
	 */
	CScriptLayer2(	const TCHAR*	tszParentStart = __SCRIPT_PARENT_START,
					const TCHAR*	tszParentEnd = __SCRIPT_PARENT_END )
		: m_tstrParentStart( tszParentStart )
		, m_tstrParentEnd( tszParentEnd )
	{
	}

	/**
		@brief	スクリプトの読み込み
		@param	ist			ソースストリーム
		@param	vecScript	スクリプト格納先
		@param	vecMacro	マクロ情報
	 */
	void	LoadScript(	tistream& ist,
						script_vector& vecScript,
						macro_vector& vecMacro,
						const _TCHAR* szFName=_T("")
						) 
	{
		// スクリプトファイル名の取得
		m_tstrFilenameBase = szFName;

		// 最上位構造の作成
		CScriptObject	obj;
		m_vecChild.push_back( obj );
		// 解析実行
		CScriptLayer1	Layer1( ist, *this, vecMacro, szFName );
		// エラー情報の取得
		CScriptError*	lpError = Layer1.GetErrorInfo();
		if( lpError != NULL )
		{
			// Layer1でエラーが見つかった場合は、エラー情報を追加する
			m_ScriptError += *lpError;
		}

		// すべての下位構造が完了しているか？
		if( m_vecChild.size() != 1 )
		{
			// 最上位まで復帰していない
			CScriptError::error_object	errorObject(	"[root]",
														0,
														CScriptError::error_object::ERRORCODE_CANT_FOUND_CLOSE,
														_T("'{' is too much") );
			m_ScriptError.AddError( errorObject );
			return;
		}
		// 構造をコピー
		vecScript = (m_vecChild.begin()->vecChild);

		// スクリプトを閉じる
		m_vecChild.clear();
	}

	/**
		@brief	単語を取得して階層分けを行う
		@return	true:エラーなし／false:エラーあり
		@param	strWord		下位レイヤから渡された単語
		@param	scrInfo		スクリプト情報
	 */
	virtual bool	parse( tstring& strWord, SCRIPTOBJECT_INFO& scrInfo ) 
	{
		int	n;
		CScriptObject	obj;
		obj.strWord = strWord;
		obj.scrInfo = scrInfo;
		obj.scrInfo.tstrFNameBase = m_tstrFilenameBase;		// ベースファイル名の設定

		tstring	tstrTmp;

		// 単語が階層の開始文字？
		n = CStringSearch::GetCount( strWord.c_str(), m_tstrParentStart, tstrTmp );
		if( n>=0)
		{
			// 子階層を作成
			m_vecLastParent.push_back( n );		// 開始文字を保存
			m_vecChild.push_back( obj );		// 現在の階層を保存
			return true;
		}
		// 単語が階層の完了文字？
		n = CStringSearch::GetCount( strWord.c_str(), m_tstrParentEnd, tstrTmp );
		if( n>=0)
		{
			script_vector::reverse_iterator		itChild = m_vecChild.rbegin();
			// 親階層へ復帰
			// 最上位階層でないことをチェック
			script_vector::reverse_iterator itParent = itChild+1;
			if( m_vecLastParent.empty() || itParent == m_vecChild.rend() )
			{
				// 親階層が存在しない
				CScriptError::error_object	errorObject(	"[root]",
															0,
															CScriptError::error_object::ERRORCODE_ILLEGAL_CLOSE,
															_T("'}' is too much") );
				m_ScriptError.AddError( errorObject );
				return false;
			}
			// 階層の開始文字と完了文字が対になっているか？
			int nLast = *m_vecLastParent.rbegin();
			m_vecLastParent.pop_back();
			if( n != nLast )
			{
				// 階層の開始文字・完了文字が対でない
				m_ScriptError.AddError( CScriptError::error_object(	"[root]",
															0,
															CScriptError::error_object::ERRORCODE_NOT_PARE,
															_T("parent type is not much") ));
				return false;
			}

			// 親階層に現在の階層を保存
			itParent->vecChild.push_back( *itChild );
			m_vecChild.pop_back();

		}
		// 単語を現在の階層へ追加
		script_vector::reverse_iterator		itChild = m_vecChild.rbegin();
		itChild->vecChild.push_back( obj );

		return true;
	}


	/**
		@brief	エラー情報を取得する
		@return	エラーあり：エラー情報へのポインタ
				エラーなし：NULL
	 */
	CScriptError*	GetErrorInfo( void )
	{
		if( !m_ScriptError.FoundError() )
		{
			return NULL;
		}
		return &m_ScriptError;
	}
};




////////////////////////////////////////////////////////////////////////////////
// スクリプト補助関数
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	スクリプト簡易ローダ
	@param	szFilename		ファイル名
	@param	vecScript		スクリプトの取得結果
	@return	true:正常終了／false:エラー
 */
inline int	ScriptLoadFromFile( const TCHAR* szFilename, script_vector& vecScript )
{
	macro_vector	vecMacro;

	// スクリプトの読み込み
	CScriptLayer2	Script( szFilename, vecScript, vecMacro );

	// ファイルを閉じる
//	ifst.close();

	CScriptError*	lpError = Script.GetErrorInfo();
	if( lpError != NULL )
	{
		// エラーあり
		tstring	tstr;
		lpError->OutErrorInfoString( tstr );
		RuntimeError( tstr.c_str() );

		return false;
	}

	return true;
}

/**
	@brief	文字列からスクリプトの読み込み
	@param	tszScript		スクリプトを記述した文字列
	@param	vecScript		スクリプトの取得結果
	@return	true:正常終了／false:エラー
 */
inline int	ScriptLoadFromString( const TCHAR* tszScript, script_vector& vecScript )
{
	std::istringstream	isst( tszScript );
	macro_vector	vecMacro;

	// スクリプトの読み込み
	CScriptLayer2	Script( isst, vecScript, vecMacro );

	if( Script.GetErrorInfo() != NULL )
	{
		// エラーあり
		return false;
	}

	return true;
}

/**
	@brief		リソースからスクリプトの取得
	@param		hInstance		リソースの格納されたインスタンス
	@param		lpName			リソース名
	@param		lpType			リソースタイプ
	@param		vecScript		スクリプト格納先
	@return		true:OK/false:NG
 */
inline int	ScriptLoadFromResource( HINSTANCE hInstance, 
									LPCTSTR lpName,
									LPCTSTR lpType,
									simple::script_vector& vecScript  )
{

	HRSRC hResInfo = FindResource( hInstance, lpName, lpType );
	if( hResInfo == NULL )
	{
		return false;
	}
	HGLOBAL hBuf = LoadResource( hInstance, hResInfo );
	if( hBuf == NULL )
	{
		return false;
	}

	const TCHAR* tszBuf = reinterpret_cast< const TCHAR* >(LockResource( hBuf ));

	simple::tstringstream	tsst( tszBuf );
	
	simple::macro_vector	vecMacro;
	simple::CScriptLayer2	script;
	script.LoadScript( tsst, vecScript, vecMacro );
	if( script.GetErrorInfo() != NULL )
	{
		std::ofstream	ofst( "script.log" );
		simple::CScriptError* lpErrorInfo = script.GetErrorInfo();
		lpErrorInfo->OutErrorInfo( ofst );
		ofst.close();

		FreeResource( hBuf );
		return false;
	}
	
	FreeResource( hBuf );
	return true;
}


/**
	@brief		"xxx=???;"の文を解析
	@note		使用例：
					script_vector	vecScript;
					;
					for( script_vector::iterator it = vecScript.begin() ; it != vecScript.end() ; it++ )
					{
						if( it->strWord == "name" )
						{
							CScriptObject*	lpValue = ScriptGetValueEqual( vecScript, it );
							if( lpValue == NULL )
							{
								// error
								return false;
							}
							strName = lpValue->strWord;
						}
					}
 */
inline CScriptObject*		ScriptGetValueEqual( script_vector& vecScript, script_vector::iterator& itVec )
{
	enum {
		PHASE_EQUAL = 0,
		PHASE_VALUE,
		PHASE_DELIMITER,
		PHASE_COMPLETE,
	};

	CScriptObject*	lpValue = NULL;

	for( unsigned char ucPhase = PHASE_EQUAL
		 ; ucPhase < PHASE_COMPLETE
		 ; ucPhase++ )
	{
		itVec++;
		if( itVec == vecScript.end() )
		{
			// EOF
			return NULL;
		}
		switch( ucPhase )
		{
		case PHASE_EQUAL:
			// 次の単語が"="か？
			if( itVec->strWord != "=" )
			{
				// "="でない
				return NULL;
			}
			break;
		case PHASE_VALUE:
			if( itVec->strWord == ";" )
			{
				// 値がない(行端";"だった)
				return NULL;
			}
			// 値を保存
			lpValue = &(*itVec);
			break;
		case PHASE_DELIMITER:
			if( itVec->strWord != ";" )
			{
				// 行端でない
				return NULL;
			}
			break;
		}
	}

	return lpValue;
}

/**
	@brief		整数型値取得
 */
inline int	ScriptCalculateInt( long& dest, 
								script_vector::iterator it,
								script_vector& vecScript )
{
	CScriptCalculator< long >		calc( vecScript, calc_cfg_int, it );
	if( calc.GetErrorInfo() != NULL )
	{
		return false;
	}
	
	dest = calc.GetResult();
	return true;
}

/**
	@brief		スクリプトの演算(整数型)
	@note		マクロを指定(スクリプト内の#if文などで使用)
 */
inline int ScriptCalculateInt( long& dest, const TCHAR* tsz, macro_vector& vecMacro )
{
	script_vector	vecScript;
	tstringstream	isst( tsz );

	CScriptLayer2	script( isst, vecScript, vecMacro );
	if( script.GetErrorInfo() != NULL )
	{
		return false;
	}

	CScriptCalculator< long >		calc( vecScript, calc_cfg_int );
	if( calc.GetErrorInfo() != NULL )
	{
		return false;
	}
	
	dest = calc.GetResult();
	return true;
}

/**
	@brief		スクリプトの演算(浮動小数点型)
 */
inline int ScriptCalculateFloat( float& dest, const TCHAR* tsz, macro_vector& vecMacro )
{
	script_vector	vecScript;
	tstringstream	isst( tsz );

	CScriptLayer2	script( isst, vecScript, vecMacro );
	if( script.GetErrorInfo() != NULL )
	{
		return false;
	}

	CScriptCalculator< float >		calc( vecScript, calc_cfg_float );
	if( calc.GetErrorInfo() != NULL )
	{
		return false;
	}
	
	dest = calc.GetResult();
	return true;
}
/**
	@brief		スクリプトの演算(浮動小数点型)
 */
inline int ScriptCalculateFloat(	float& dest, 
									script_vector::iterator it,
									script_vector& vecScript )
{
	CScriptCalculator< float >		calc( vecScript, calc_cfg_float, it );
	if( calc.GetErrorInfo() != NULL )
	{
		return false;
	}
	
	dest = calc.GetResult();
	return true;
}



/**
	@brief		スクリプトの演算(整数型)
 */
inline int ScriptCalculateInt( long& dest, const TCHAR* tsz )
{
	macro_vector	vecMacro;
	return ScriptCalculateInt( dest, tsz, vecMacro );
}

inline bool	ScriptIsWord(	const TCHAR* tsz,
							script_vector::iterator& it,
							script_vector& vecScript )
{
	if( it == vecScript.end() )
	{
		return false;
	}
	return ( it->strWord == tsz );
}

/**
	@brief		スクリプトの下層構造を取得
	@param		itScript		スクリプトの参照
	@param		vecScript		スクリプト
	@param		tszBegin		開始カッコ
	@param		tszEnd			終端カッコ
	@return		取得した下層構造のポインタ。失敗時はNULLを返す
	@note		iteratorは成功時は	()の次の位置を指す。
 */
inline script_vector* ScriptGetChild( script_vector::iterator& itScript, 
									  script_vector& vecScript, 
									  const TCHAR* tszBegin, 
									  const TCHAR* tszEnd )
{
	if( !ScriptIsWord( tszBegin, itScript, vecScript ) )
	{
		return NULL;
	}
	script_vector* lpChild = &itScript->vecChild;
	itScript++;
	if( !ScriptIsWord( tszEnd, itScript, vecScript ) )
	{
		return NULL;
	}
	itScript++;

	return lpChild;
}

/// スクリプト参照位置のコンテナタイプ
typedef std::vector< script_vector::iterator >	script_iterator_vector;
/**
	@brief		カンマ区切りされた複数の値を取得
	@param		vecString		結果の保存先
	@param		it				スクリプトの参照
	@param		vecScript		スクリプト
	@param		nNum			取得する値の個数(負数であれば";"まですべて取得する)
	@return		取得した値の数。失敗した場合は0を返す。
	@note		key = value1,value2,value3,...valuen; のような記述で、
				value1,...valuenまでの取得に使用する。
				値の間の区切りは','、値列の終了は';'とする。
 */
inline int	ScriptGetValues(	script_iterator_vector&	vecString, 
								script_vector::iterator& it,
								script_vector& vecScript,
								int nNum = -1
								)
{
	int nCount = 0;								/// 取得した値の個数
	script_vector::iterator	itLast = NULL;		/// 取得中の値の開始位置
	int	nValCount = 0;							/// 値に含まれる文字列の個数

	for( ; it != vecScript.end() && nCount != nNum ; it++ )
	{
		if( it->strWord == "," || it->strWord == ";")
		{
			// デリミタ文字が見つかった

			if( nValCount == 0 )
			{
				// デリミタが連続(値の文字列数が０)
				return 0;
			}
			// 値の参照位置を保存
			vecString.push_back( itLast );

			nValCount = 0;
			// 値の個数を加算
			nCount++;

			// ";"が見つかった、または値の個数が要求を満たした?
			if( it->strWord == ";" || nCount == nNum)
			{
				break;
			}
		} else 
		{
			// 値
			if( nValCount == 0 )
			{
				// 値の開始位置を保存
				itLast = it;
			}
			// 値の個数を加算
			nValCount++;
		}
	}

	// 最後がEOFの場合で、値の途中であれば、そこまでを値と判断する
	if( nValCount > 0 && it == vecScript.end() )
	{
		nCount++;
		vecString.push_back( itLast );
	}

	if( nCount < nNum )
	{
		// 値の個数が足りない
		return 0;
	}

	// 取得した値の個数を返す
	return nCount;
}

/**
	@brief	key={value1,...}形式の値取得処理
	@param	vecString		値の参照位置保存先
	@param	it				スクリプト解析の開始位置(keyの位置)
	@param	vecScript		スクリプト
	@param	ppvecChild		値のある子階層スクリプトのポインタの保存先(値計算時に使用するため)
	@param	nNum			値の要求個数(負数で";"まですべて取得)
	@return	取得した値の個数。0の場合はエラー。
 */
inline int	ScriptGetValuesEqualChild
							(	script_iterator_vector&	vecString, 
								script_vector::iterator& it,
								script_vector& vecScript,
								script_vector** ppvecChild,
								int nNum = -1
								)
{
	it++;
	if( !ScriptIsWord( _T("="), it, vecScript ) )
	{
		return 0;
	}
	it++;
	if( it == vecScript.end() )
	{
		return 0;
	}
	if( ppvecChild )
	{
		*ppvecChild = &it->vecChild;
	}

	script_vector::iterator	itChild = it->vecChild.begin();
	return ScriptGetValues( vecString, itChild, it->vecChild, nNum );
}

/**
	@brief	現在位置から指定したキーを検索
 */
inline script_vector::iterator		ScriptFindKey(	const TCHAR* tszKey, 
													script_vector::iterator it, 
													script_vector& vecScript )
{

	return std::find( it, vecScript.end(), tszKey );
}

}		/// namespace simple

#endif // _SIMPLE_SCRIPT_HPP