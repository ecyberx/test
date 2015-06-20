
#include "simpleutil.hpp"

namespace simple
{
/**
	@brief		スクリプトから構造体データを取得
	@param		vecScript		スクリプト
	@param		parseBind		バインドテーブル
	@param		nCount			バインドテーブルメンバ数
	@param		lpDest			出力先変数先頭アドレス
	@param		plugIn			プラグインの参照
	@return		成功した場合、設定した変数の数を返す。取得に失敗した変数があれば０を返す。
 */
int	ScriptParse( script_vector& vecScript, const script_parse_bind* parseBind, int nCount, void* lpDest, CPlugIn& plugIn )
{
	typedef int(func)( script_vector::iterator, script_vector&, void* lpDest, size_t tSize, CPlugIn& plugIn  );
	int	nHit = 0;
	for( int i=0 ; i<nCount ; i++ )
	{
		script_vector::iterator	it = simple::ScriptFindKey( parseBind[i].szKey, vecScript.begin(), vecScript );
		if( it != vecScript.end() )
		{
			// check format:"xxx=yyy"?
			it++;
			if( it == vecScript.end() )
			{
				RuntimeError( "スクリプトの記述ミスです" );
				return 0;
			}
			if( it->strWord != "=" )
			{
				RuntimeError( "%s スクリプトの記述ミスです", it->getStringLocate().c_str() );
				return 0;
			}
			it++;
			if( it == vecScript.end() )
			{
				RuntimeError( "スクリプトの記述ミスです" );
				return 0;
			}
			
			// get value
			nHit++;
			func*	lpFunc = (func*)plugIn.GetProcess( parseBind[i].szProcess );
			if( lpFunc == NULL )
			{
				RuntimeError( "%s ScriptParserのプロセス'%s'が見つかりません", it->getStringLocate().c_str(), parseBind[i].szProcess );
				return 0;
			}
			if( !lpFunc( it, vecScript, ( (char*)lpDest+parseBind[i].tOffset), parseBind[i].tSize, plugIn ) )
			{
				RuntimeError( "%s ScriptParserのプロセス'%s'で生成エラーが発生しました", it->getStringLocate().c_str(), parseBind[i].szProcess );
				return 0;
			}
		}
	}

	return true;
}


/**
	@brief		ScriptParser INTタイプ入力プロセス
	@param		it			スクリプトの参照開始位置
	@param		vecScript	スクリプト
	@param		lpDest		結果の保存先
	@param		tSize		結果保存先領域のサイズ
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseInt(	script_vector::iterator it, 
								script_vector& vecScript, 
								void* lpDest, 
								size_t tSize, 
								CPlugIn& )
{
	long	lResult;
	if( !simple::ScriptCalculateInt(  lResult,  it, vecScript ) )
	{
		RuntimeError( "%s int数の記述エラーです", it->getStringLocate().c_str() );
		return false;
	}

	if( tSize >= sizeof(long) )
	{
		*(long*)lpDest = lResult;
	} else
	{
		switch( tSize )
		{
		default:
		case sizeof(int):
			*(int*)(lpDest) = static_cast<int>(lResult);
			break;
		case sizeof(short):
			*(short*)(lpDest) = static_cast<short>(lResult);
			break;
		case sizeof(char):
			*(char*)(lpDest) = static_cast<char>(lResult);
			break;
		}
	}

	return true;
}



/**
	@brief		ScriptParser Boolタイプ入力プロセス
	@param		it			スクリプトの参照開始位置
	@param		vecScript	スクリプト
	@param		lpDest		結果の保存先
	@param		tSize		結果保存先領域のサイズ
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseBool(script_vector::iterator it, 
								script_vector& vecScript, 
								void* lpDest, 
								size_t tSize, 
								CPlugIn& )
{
	long	lResult;
	if( !simple::ScriptCalculateInt(  lResult,  it, vecScript ) )
	{
		RuntimeError( "%s bool値の記述エラーです", it->getStringLocate().c_str() );
		return false;
	}

	*(bool*)(lpDest) = lResult ? true : false;

	return true;
}

/**
	@brief		ScriptParser INTタイプ入力プロセス
	@param		it			スクリプトの参照開始位置
	@param		vecScript	スクリプト
	@param		lpDest		結果の保存先
	@param		tSize		結果保存先領域のサイズ
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseFloat(	script_vector::iterator it, 
									script_vector& vecScript, 
									void* lpDest, 
									size_t tSize, 
									CPlugIn&  )
{
	float	fResult;
	if( !simple::ScriptCalculateFloat(  fResult,  it, vecScript ) )
	{
		RuntimeError( "%s float値の記述エラーです", it->getStringLocate().c_str() );
		return false;
	}

	switch( tSize )
	{
	case sizeof(float):
		*(float*)lpDest = fResult;
		break;
	case sizeof(double):
		*(double*)lpDest = static_cast<double>( fResult );
		break;
	default:
		RuntimeError( "%s float演算のパラメータエラー(size)です", it->getStringLocate().c_str() );
		return false;
	}
	return true;
}


/**
	@brief		ScriptParser basic_stringタイプ入力プロセス
	@param		it			スクリプトの参照開始位置
	@param		vecScript	スクリプト
	@param		lpDest		結果の保存先
	@param		tSize		結果保存先領域のサイズ
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseBasicString(	script_vector::iterator it, 
										script_vector& vecScript, 
										void* lpDest, size_t tSize, 
										CPlugIn& )
{
	if( tSize != sizeof(std::string) )
	{
		RuntimeError("string取得のサイズエラーです");
		return false;
	}
	if( it == vecScript.end() )
	{
		RuntimeError("string取得の記述エラーです");
		return false;
	}
	std::string*	lpString = reinterpret_cast< std::string* >( lpDest );
	it->getStringWord();		// 文字列のダブルクォーテーションがあれば削除
	*lpString = it->strWord;
	return true;
}
/**
	@brief		ScriptParser basic_stringタイプ入力プロセス(ファイルパス指定つき)
	@param		it			スクリプトの参照開始位置
	@param		vecScript	スクリプト
	@param		lpDest		結果の保存先
	@param		tSize		結果保存先領域のサイズ
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseBasicStringWithPath( script_vector::iterator it, 
												script_vector& vecScript, 
												void* lpDest, 
												size_t tSize, 
												CPlugIn& )
{
	if( tSize != sizeof(std::string) )
	{
		RuntimeError("string取得のサイズエラーです");
		return false;
	}
	if( it == vecScript.end() )
	{
		RuntimeError("string取得の記述エラーです");
		return false;
	}

	std::string*	lpString = reinterpret_cast< std::string* >( lpDest );
	it->getStringWord();		// 文字列のダブルクォーテーションがあれば削除
	CPath	path( it->scrInfo.tstrFNameSrc.c_str() );
	path--;
	path += it->strWord.c_str();
	*lpString = path.GetPath();
	return true;
}

/**
	@brief		ScriptParser basic_stringタイプ入力プロセス
	@param		it			スクリプトの参照開始位置
	@param		vecScript	スクリプト
	@param		lpDest		結果の保存先
	@param		tSize		結果保存先領域のサイズ
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseBasicStringList( script_vector::iterator it, 
											script_vector& vecScript, 
											void* lpDest, 
											size_t tSize, 
											CPlugIn&  )
{
	if( tSize != sizeof(string_vector) )
	{
		RuntimeError( "%s StringListサイズエラー", it->getStringLocate() );
		return false;
	}

	string_vector*	lpVecString = reinterpret_cast< string_vector* >( lpDest );

	if( !ScriptIsWord( "{", it, vecScript ) )
	{
		RuntimeError( "%s StringList取得に失敗しました", it->getStringLocate() );
		return false;
	}

	script_vector::iterator itChild = it->vecChild.begin();
	for( ; itChild != it->vecChild.end() ; itChild++ )
	{
		itChild->getStringWord();		// 文字列のダブルクォーテーションがあれば削除
		lpVecString->push_back( itChild->strWord );

		itChild++;
		if( itChild == it->vecChild.end() )break;
		if( itChild->strWord != ","  )
		{
			RuntimeError( "%s StringListのワード間に','がありません", it->getStringLocate() );
			return false;
		}
	}

	return true;
}


};	//namespace simple
