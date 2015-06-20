// $Id: simpletext.hpp,v 1.2 2005/07/07 23:35:56 Ander Exp $
/**
	@brief	テキスト管理クラス関連ヘッダファイル
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:56 $
 */

#ifndef	_CText_HPP
#define	_CText_HPP

#include "./base.hpp"

namespace simple {		// namespace simple

#include <sstream>

/// テキストクラスで使用する予約文字定義
#define	_CTextEX_RESERVED__TCHAR	_T("{}(),;+-*/\"^|&~!=[]")
/// テキストクラスで使用する予約語定義
#define _CTextEX_RESERVED_STR		_T("++;--;==")

/// 一桁のHEX文字を数値に変換
#define	_HTOI(x)	(	( (x)>=_T('0') && (x)<=_T('9') ) ? ((x) - _T('0'))				\
						 : ( (x)>=_T('a') && (x)<=_T('f')) ? ((x)-_T('a')+0x0a)			\
						 : ( (x)>=_T('A') && (x)<=_T('F')) ? ((x)-_T('A')+0x0a) : -1 )
/// 一桁の数字を数値に変換
#define	_ATOI(x)	( ( (x)>=_T('0') && (x)<=_T('9') ) ? ((x)-_T('0')) : -1 )
/// 一桁の8進数数字を数値に変換
#define	_OTOI(x)	( ( (x)>=_T('0') && (x)<=_T('7') ) ? ((x)-_T('0')) : -1 )

#ifdef _UNICODE
/// 漢字コード判別
#define	_ISKANJI(x)	(false)
#else
/// 漢字コード判別
#define	_ISKANJI(x)	( (signed)(x)<-1 )
#endif

////////////////////////////////////////////////////////////////////////////////
// simple_search_string
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	デリミタで区切られた文字列の検索などを行う
 */
class	CStringSearch {
public:
	/**
		@brief	文字列src1の先頭がsrc2かチェック
		@param	src1	チェック対象の文字列
		@param	src2	先頭にあるかチェックする文字列
		@return	true:src1の先頭にsrc2が見つかった
				false:src1の先頭はsrc2ではない
	 */
	static bool	IsStrTop( const _TCHAR* src1,const _TCHAR* src2 )
	{
		for( ; *src1 && *src2 && (*src1 == *src2) ; src1++,src2++ );
		return ( *src2 == NULL );
	}

	/**
		@brief	strSrcのnPosStart～nPosEndまでをstrDestへコピー
		@param	strSrc		コピー元の文字列
		@param	strDest		コピーデータの格納先
		@param	nPosStart	コピー開始位置
		@param	nPosEnd		コピー終端位置
							文字列の最後まで取得する場合はnPosEndにnposを設定
		@retval	strDest		コピーされた文字列
	 */
	static 	void SubstrEx( const tstring& strSrc,
							tstring& strDest,
							size_t nPosStart, size_t nPosEnd )
	{
		if( nPosEnd < nPosStart )
		{
			RuntimeError( "SubstrEx::引数が不正です", nPosStart, nPosEnd );
			return;
		}
		size_t nCnt = (nPosEnd == strSrc.npos ) ? nPosEnd : (nPosEnd - nPosStart) ;
		strDest = strSrc.substr( nPosStart, nCnt );
	}

	/**
		@brief	文字列をデリミタで区切られた文字列から検索して発見された位置を返す(SRCの文字列指定)
		@param	it				検索対象の文字列(単語)
		@param	tstrSrc			検索対象のデリミタで区切られた文字列
		@param	tstrDest		見つかった単語の格納先
		@param	tchDelimiter	tstrSrcのデリミタ文字
		@return	文字列itが文字列tstrSrcから見つかった場所
		@retval	tstrDest		見つかった単語
		@note	"*.h"が"*.c;*.h;*.hpp"にあるかチェックする場合などに使用
	 */
	static int	GetCount( const _TCHAR* it,
							const _TCHAR* tstrSrc,
							tstring& tstrDest,
							const _TCHAR tchDelimiter = _T(';') )
	{
		return GetCount( it,tstring(tstrSrc),tstrDest,tchDelimiter );
	}
	/**
		@brief	文字列をデリミタで区切られた文字列から検索して発見された位置を返す
		@param	it				検索対象の文字列(単語)
		@param	tstrSrc			検索対象のデリミタで区切られた文字列
		@param	tstrDest		見つかった単語の格納先
		@param	tchDelimiter	tstrSrcのデリミタ文字
		@return	文字列itが文字列tstrSrcから見つかった場所
		@retval	tstrDest		見つかった単語
		@note	"*.h"が"*.c;*.h;*.hpp"にあるかチェックする場合などに使用
	 */
	static int	GetCount( const _TCHAR* it,
							const tstring& tstrSrc,
							tstring& tstrDest,
							const _TCHAR tchDelimiter = _T(';') )
	{
		size_t nPosStart = 0;
		size_t nPos = tstrSrc.find( tchDelimiter ,0 );

		for(int i=0 ; nPosStart != tstring::npos ; i++ )
		{
			SubstrEx( tstrSrc, tstrDest, nPosStart, nPos );
			if( tstrDest == it )return i;
			nPosStart = ( nPos == tstring::npos ) ? nPos : nPos+1;
			nPos = tstrSrc.find( tchDelimiter ,nPosStart );
		}
		return -1;
	}
private:
	const tstring	m_tstrSrc;					/// 検索対象の文字列
	tstring			m_tstrDest;					/// 検索結果の格納先
	const _TCHAR	m_tchDelimiter;				/// デリミタ文字
	size_t			m_nPosStart;				/// 検索開始位置
	CStringSearch( const CStringSearch& );
	CStringSearch& operator =( const CStringSearch& );
public:
	/**
		@brief	デストラクタ
	 */
	virtual	~CStringSearch(void){}
	/**
		@brief	コンストラクタ
		@param	tstrSrc			デリミタで区切られた文字列
		@param	tchDelimiter	デリミタ文字(デフォルトは';')
	 */
	CStringSearch( const tstring& tstrSrc, _TCHAR tchDelimiter=_T(';') )
		: m_tstrSrc(tstrSrc), m_tchDelimiter(tchDelimiter)
	{
		Reset();
	}

	// 次の文字列を取得
	/**
		@brief	デリミタから次の文字列を取得
		@param	tstrDest	文字列の格納先
		@return	true:次の文字列が見つかった
				false:次の文字列が見つからなかった(デリミタ文字列の最後にいる)
		@retval	tstrDest	次の文字列
	 */
	bool GetNext(tstring& tstrDest )
	{
		// 文字列の検索が完了しているならEOFを投げる
		if( m_nPosStart == tstring::npos )
		{
			return false;
		}

		// 文字列を取得
		size_t nPos = m_tstrSrc.find( m_tchDelimiter ,m_nPosStart );
		SubstrEx( m_tstrSrc, tstrDest, m_nPosStart, nPos );

		// 次の文字列の開始位置を設定
		m_nPosStart = ( nPos == tstring::npos ) ? nPos : nPos+1;

		return	true;
	}
	/**
		@brief	現在のデリミタ文字列検索位置をリセットする
				(先頭から検索できるように設定する)
	 */
	void	Reset(void)
	{
		m_nPosStart = 0;
	}

};


////////////////////////////////////////////////////////////////////////////////
// CTextLine
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	文字列解析クラス
			単語・行単位での文字列取得
 */
class	CTextLine {
protected:
	
	unsigned int			m_uiLine;			/// 現在の行番号

	tistream&			m_ist ;					/// 読込先のストリーム
	tstring				m_strLine;				// 現在読み込んでいる行
	tstring::iterator	m_itLine;				// 現在読み込んでいる行の参照位置
	
	tstring _WHITE_SPACE ;						// ホワイトスペース文字の集合

	// 次の行を取得
	/**
		@brief	新しい行をストリームから読み込む
		@param	bConnect	行を連結する？(行端の'\'による行の連結時にtrue)
		@return	true:行の読み込みに成功／false:EOFを検出(行を読み込めない)
		@retval	m_strLine	読み込んだ行の内容を格納
		@retval	m_itLine	読み込んだ行の先頭位置を参照
	 */
	bool	getNewLine( bool bConnect = false )
	{
		// ファイルがEOFならEOF例外
		if( m_ist.eof() )
		{
			return false;
		}
		
		// 行をクリア
		m_strLine = _T("");
		
		++m_uiLine;		// 行番号を加算

		int c;
		for(;;) {
			c = m_ist.get();
			if( c == EOF || m_ist.eof() ){
				if( m_strLine.empty() )
				{
					return false;
				}
				 else break;
			}
			if( c == _T('\n') || c == _T('\r') ){
				break;
			}
			m_strLine += (_TCHAR)c;//c&0xff;
		}
		
		// 改行のみの行の場合は次の行を読む
		if( m_strLine.empty() && bConnect==false){
			return getNewLine();
		}
		// 行の連接をチェック
		if( m_strLine[ m_strLine.length()-1 ] == _T('\\') )
		{
			// 末尾が'\'なら次の行と連接する
			tstring	line2 = m_strLine;			// 現在の行の内容を保存
			line2.resize( line2.length()-1 );		// 末尾の'\'を削除
			getNewLine(true);						// 連接ありで次の行を読み出す
			m_strLine.insert(0,line2);				// 現在の行と次の行を連結
		}
		m_itLine = m_strLine.begin();

		return true;
	}

	/**
		@brief	指定した文字がホワイトスペースかチェックする
		@param	c	チェックする文字
		@return	true:文字がホワイトスペースに該当／false:文字はホワイトスペースではない
	 */
	bool	isWhiteSpace( const _TCHAR c )
	{
		return (_WHITE_SPACE.find(c) != tstring::npos) ;
	}
private:
	CTextLine(const CTextLine&);
	CTextLine& operator=(const CTextLine&);
	
public:
	/**
		@brief	デストラクタ
	 */
	virtual	~CTextLine(){}
	
	/**
		@brief	コンストラクタ
		@param	ist	入力するストリーム
	 */
	CTextLine( tistream& ist )
	 : m_ist(ist), _WHITE_SPACE( _T(" \r\n\t") ), m_uiLine(0)
	{
		m_itLine = m_strLine.end();
	}
	
	/**
		@brief	単語を取得
		@param	tstrWord	単語の格納先
		@return	true:文字列あり／false:EOF
		@retval	tstrWord	取得した単語を格納
	 */
	bool	GetWord(tstring& tstrWord )
	{
		tstrWord = "";	// DESTをクリア

		// 行末なら新しい行を読む
		if( m_itLine == m_strLine.end() ){
			if( getNewLine() == false )
			{
				return false;
			}
		}
		// ホワイトスペースをスキップ
		for( ; isWhiteSpace(*m_itLine) ; m_itLine++ ){
			if( m_itLine == m_strLine.end() ){
				if( getNewLine() == false )
				{
					return false;
				}
			}
		}
		// 次のホワイトスペースまたは行端までを単語として取得
		for( ; !isWhiteSpace(*m_itLine) && m_itLine != m_strLine.end() ; m_itLine++ )
		{
			tstrWord += *m_itLine ;
		}
		
		// 空の文字列ならもう一回読み直す
		if( tstrWord.empty() )return GetWord(tstrWord);
		return true;
	}
	
	// 
	/**
		@brief	読み込み中の行の現在位置から行末までを取得
		@param	tstrLine	取得した文字列の格納先
		@return	true:文字列あり／false:EOF
		@retval	tstrLine	取得した文字列を格納
	 */
	bool 	GetLine(tstring& tstrLine )
	{
		if( m_itLine == m_strLine.end() ){
			if( getNewLine() == false )
			{
				return false;
			}
		}
		if( m_itLine == 0 )
		{
			tstrLine = _T("");
			return true;
		}

		tstrLine = &(*m_itLine);
		m_itLine = m_strLine.end();
		
		return true;
	}

	/**
		@brief	現在読み込んでいる位置の文字を取得
		@return	現在位置の文字
	 */
	_TCHAR GetPosCharactor(void)
	{
		return *m_itLine;
	}

	/**
		@brief	現在読み込んでいる行の行番号の取得
		@return	現在読み込んでいる行番号
	 */
	unsigned int	GetLineNumber(void)
	{
		return m_uiLine;
	}

	/**
		@brief	文字列にストリームから一文字追加する
				(漢字であれば2バイト追加)
		@param	strWord		追加される文字列
		@retval	strWord		文字列の追加された結果を格納
	 */
	void AddWithKanji( tstring& strWord )
	{
		strWord += *m_itLine;
		if( _ISKANJI( *m_itLine ) ){
			m_itLine++;
			strWord += *m_itLine;
		}
	}


};


////////////////////////////////////////////////////////////////////////////////
// CTextLineEx
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	プリプロセッサ用単語単位での文字列取得拡張版
			コメント、文字列(" ～ ")に対応
 */
class	CTextLineEx : public CTextLine {
public:
	enum	{
		ERROR_NONE	= 0,
		ERROR_BAD_ESCSEQENCE,
		ERROR_STRING_TERM_NOTFOUND,
		ERROR_COMMENT_TERM_NOTFOUND,
		ERROR_BAD_NUMERIC,
		ERROR_UNKNOWN,
	};
	/**
		@brief	テキスト読み込み時のエラーを取得する
	 */
	unsigned int	GetLastError( void )
	{
		return m_uiLastError;
	}
protected:
	unsigned int	m_uiLastError;
	void	setError( unsigned int uiErrorCode )
	{
		m_uiLastError = uiErrorCode;
	}
public:
	/**
		@brief	文字列が" ～ "形式なら中身を抽出してTRUEを返す。通常文字列ならFALSE。
		@param	strWord		解析する文字列
		@return	true:文字列が”で区切られた形式だった／false:文字列が”で区切られていない
		@retval	strWord		戻り値がtrueの場合”～”で区切られた文字列の中身が格納される
	 */
	static bool	IsStringWord( tstring& strWord )
	{
		if( strWord.empty() || (strWord[0] != _T('\"')
			&& strWord[ strWord.length() -1 ] != _T('\"') ) ){

			// " ～ "形式でない
			return false;

		} else {
			
			// 行端を削除
			strWord.erase( strWord.begin(), strWord.begin()+1 );
			strWord.resize( strWord.length() -1 );
		}

		return true;
	}

	/**
		@brief	HEX文字を数値に変換する
				HEX文字でなければ例外
		@param	c	HEX文字
		@return 変換後の数値
		@exception	CSimpleError	指定した文字がHEX文字でない
	 */
	static _TCHAR HtoiEx( _TCHAR c )
	{
		c = _HTOI(c);
		if( c==-1 )
		{
			return -1;
		}
		return c;
	}
	/**
		@brief	OCT文字を数値に変換する
				OCT文字でなければ例外
		@param	c	OCT文字
		@return 変換後の数値
		@exception	CSimpleError	指定した文字がOCT文字でない
	 */
	static _TCHAR OtoiEx( _TCHAR c )
	{
		c = _ATOI(c);
		if( c==-1 || c>= 8)
		{
			return c;
		}
		return c;
	}

protected:
	tstring	_SPECIAL__TCHAR;					/// 特殊な文字群(単独で意味をもつ文字)
	tstring	_SPECIAL_STR;						/// 特殊な文字列群(単独で意味を持つ文字列)

	/**
		@brief	エスケープシーケンスから文字へ変換
		@param	c	エスケープ文字'\'の次の文字
		@return	変換された文字
		@exception	CSimpleError	不正なエスケープシーケンス
	 */
	bool	changeESC( _TCHAR& dest, _TCHAR c ) 
///	_TCHAR	changeESC( _TCHAR c ) 
	{
		// エスケープシーケンスから変換
		switch( c ){
		 case _T('\\'):
		 case _T('\"'):
		 case _T('\''):
			dest = c;
			break;
		 case _T('t'):
			dest = _T('\t');
			break;
		 case _T('n'):
			dest = _T('\n');
			break;
		 case _T('r'):
			 dest = _T('\r');
			 break;
		 case _T('a'):
			 dest = _T('\a');
			 break;
		 case _T('b'):
			 dest =  _T('\b');
			 break;
		 case _T('v'):
			 dest =  _T('\v');
			 break;
		 case _T('f'):
			 dest =  _T('\f');
			 break;
		 case _T('x'):
			 m_itLine++;
			 c = HtoiEx( *m_itLine ); 
			 m_itLine++;
			 while( _HTOI(*m_itLine) != -1 )
			 {
				 c = (c<<4) | _HTOI( *m_itLine ); 
				 m_itLine++;
			 }
			 m_itLine--;
			 dest = c;
			 break;
		 case _T('0'):
		 case _T('1'):
		 case _T('2'):
		 case _T('3'):
		 case _T('4'):
		 case _T('5'):
		 case _T('6'):
		 case _T('7'):
			 c = OtoiEx( *m_itLine ); 
			 m_itLine++;
			 while( _OTOI(*m_itLine) != -1 )
			 {
				 c = (c<<3) | _OTOI( *m_itLine ); 
				 m_itLine++;
			 }
			 m_itLine--;
			 dest = c;
			 break;
		 default:
			 setError( ERROR_BAD_ESCSEQENCE );
			 return false;
		}
		return true;
	}
	/**
		@brief	特殊文字か判別する
		@param	c	判別する文字
		@return	true:特殊文字である／false:特殊文字でない
	 */
	bool	isSpecial_TCHAR( const _TCHAR c )
	{
		return (_SPECIAL__TCHAR.find_first_of(c) != tstring::npos) ;
	}

	// 
	//	
	//	
	/**
		@brief	文字列の先頭に予約語があるかチェックする
				予約語を発見した場合、予約語文字列中の該当する予約語の先頭位置を返す。
				発見できない場合は-1を返す
		@param	it		検索対象の文字列
		@param	str		予約語の記述された文字列(';'で区切って複数指定可能)
		@return	予約語文字列中の該当する語の先頭位置
				／予約語が発見できなければ-1を返す
	 */
	int	searchSpecialStr( const _TCHAR* it, tstring& str )
	{
		CStringSearch	srch( _SPECIAL_STR );
		for( int i=0;;i++)
		{
			tstring tstrSrc;
			if( srch.GetNext(tstrSrc) == false )
			{
				return -1;
			}
			if( CStringSearch::IsStrTop( it, tstrSrc.c_str() ) )
			{
				str = tstrSrc; //srch.getWord();
				return i;
			}
		}
	}


	/**
		@brief	文字列が行コメント開始文字かチェック
		@param	it	チェックする文字列
		@return	true:文字列の先頭が行コメント／false:文字列が行コメントでない
	 */
	bool	isCommentLine( const _TCHAR* it )
	{
		return CStringSearch::IsStrTop( it, _T("//") );
	}

	/**
		@brief	文字列の先頭が”／＊”かチェック
		@param	it	チェックする文字列
		@return	true:文字列の先頭が”／＊”
				／false:文字列が”／＊”でない
	 */
	bool	isCommentBlockStart( const _TCHAR* it )
	{
		return CStringSearch::IsStrTop( it, _T("/*") );
	}
	/**
		@brief	文字列の先頭が”＊／”かチェック
		@param	it	チェックする文字列
		@return	true:文字列の先頭が”＊／”
				／false:文字列が”＊／”でない
	 */
	bool	isCommentBlockEnd( const _TCHAR* it )
	{
		return CStringSearch::IsStrTop( it, _T("*/") );
	}

	/**
		@brief	次のホワイトスペースまたは行端までを単語として取得
		@param	strWord		単語の格納先
		@retval	strWord		取得した単語
		@return	true:正常終了/false:エラー
	 */
	bool	addNormalWord( tstring& strWord )
	{
		bool	bESC = false;

		// 次のホワイトスペースまたは行端までを単語として取得
		for( ; !isWhiteSpace(*m_itLine) && m_itLine != m_strLine.end() ; m_itLine++ )
		{
			tstring	tstrEmpty;
			if( *m_itLine == _T('\\') ){
				// エスケープシーケンスを発見した場合は、ESCフラグを立ててループ終了
				bESC = true;
				continue;
			}
			if( isCommentLine( &(*m_itLine) ) || isCommentBlockStart( &(*m_itLine) ) ){
				// コメントを発見
				break;
			} else if( searchSpecialStr( &(*m_itLine), tstrEmpty ) >=0 ){
				break;	
			} else 	if( !bESC && isSpecial_TCHAR( *m_itLine ) ){
				// 特殊文字を発見した場合は、単語を打ち切る
				break;
			}		

			if( bESC == true ){
				// エスケープシーケンス
				bESC = false;
				_TCHAR	c;
				if( changeESC(c, *m_itLine) == false )
				{
					return false;
				}
				strWord += c;
			} else {

				AddWithKanji( strWord );
 
			}
		}

		// エスケープ文字が未解決(最後の文字が"\")の場合はエラー
		if( bESC )
		{
			setError( ERROR_BAD_ESCSEQENCE );
			return false;
 		}

		return true;
	}

	/**
		@brief	”～”で区切られた文字列の取得
		@param	strWord		取得した文字列の格納先
		@retval	strWord		取得した文字列
	 */
	bool	addStringWord( tstring& strWord )
	{
		// ”～”で区切られた文字列
		m_itLine++;		// ”をスキップ
		if( strWord.empty() )
		{
			// 文字列の連結でない場合は、先頭へ文字列開始文字を追加
			strWord = _T('\"');
		}

		bool	bESC = false;		/// エスケープ文字フラグ

		for( ; m_itLine != m_strLine.end() ; m_itLine++ ){
			
			if( bESC )
			{
				_TCHAR	c;
				if( changeESC( c, *m_itLine ) == false )
				{
					return false;
				}
				strWord += c;
				bESC = false;
			} else {

				if( *m_itLine == _T('\"') ){
					break;
				}

				if( *m_itLine == _T('\\') ){
					bESC = true;
					continue;
				}

				AddWithKanji( strWord );
			}
		}
		if( (*m_itLine) != _T('\"') )
		{
			// 文字列の終了文字が見つからない
			setError( ERROR_STRING_TERM_NOTFOUND );
			return false;
		}

		m_itLine++;		// ”をスキップ

		// 文字列の連結チェック
		// 次の文字の一文字目が文字列開始であれば、文字列を連結する
		if( skipSpaceAndComment() && (*m_itLine) == _T('\"') )
		{
			// 次の文字列を連結
			return addStringWord( strWord );
		}

		// 文字列の終了文字の追加
		strWord += _T('\"');

		return true;
	}

	/**
		@brief	コメントブロック(／＊～＊／)をスキップする
		@exception	CSimpleError	”＊／”が見つからない
	 */
	bool	skipCommentBlock(void)
	{
		// コメント/* ～ */までをスキップ
		m_itLine += 2;		// 先頭の"/*"をスキップ

		for( ; ; m_itLine++ ){
			if( m_itLine == m_strLine.end() )
			{
				if( getNewLine() == false )
				{
					setError( ERROR_COMMENT_TERM_NOTFOUND );
					return false;
				}
			}
			
			if( _ISKANJI( *m_itLine ) ){
				m_itLine++;
				continue;
			}
			if( isCommentBlockEnd( &(*m_itLine) ) ){
				// */が見つかった
				break;
			}
		}

		m_itLine += 2;		// 末尾の"*/"をスキップ

		return true;
	}
	/**
		@brief	 次の語が予約語かチェックし、予約語なら抜き出しtrueを返す
		@param	strWord		取得した文字列の格納先
		@return	true:次の語が予約語である／false:予約語ではない
		@retval	strWord		戻り値がtrueの場合、予約語を格納
	 */
	bool	setSpecialStr( tstring& strWord )
	{
		tstring tstr;
		if( searchSpecialStr( &(*m_itLine), tstr ) < 0 )return false;

		strWord = tstr;
		m_itLine += strWord.length();
		return true;
	}

	/**
		@brief	次の語が文字が数字かどうかチェックし、
				数字であれば数値と考えられる範囲を取得する
		@param	tstrWord	取得した数字列の格納先
		@return	true:次の語が数字／false:次の語が数字でない
		@retval	tstrWord	戻り値がtrueの場合、数字が続く範囲を取得する
		@exception	CSimpleError	数字列の最後が"."だった
	 */
	bool	getNumStr( tstring& tstrWord ) 
	{
		// 文字が数字または、＋または－で次の文字が数字なら数値とする
		if( _ATOI(*m_itLine) >= 0 
			|| ( (*m_itLine == '-' || *m_itLine=='+') && _ATOI( *(m_itLine+1) ) >= 0 ) )
		
		{
			// 数値を取得
			do {
				tstrWord += *m_itLine;
				m_itLine++;
			} while( (*m_itLine >= '0' && *m_itLine <= '9') || (*m_itLine == '.') ); 

			// HEXチェック
			if( (*m_itLine == 'x' || *m_itLine == 'X') && tstrWord == "0" )
			{
				tstrWord += *m_itLine;
				m_itLine++;
				if( _HTOI( *m_itLine ) < 0 )
				{
					// "0x"の後がHEXでない
					setError( ERROR_BAD_NUMERIC );
					return false;
				}

				// 16進数の取得
				while( *m_itLine ) 
				{
					tstrWord += *m_itLine;
					m_itLine++;
					if( _HTOI( *m_itLine ) < 0 )break;
				}
				// HEX取得OK
				return true;
			}

			if( *tstrWord.rbegin() == '.' )
			{
				// 数値文字列の末尾が'.'ならエラー
				setError( ERROR_BAD_NUMERIC );
				return false;
			}
			// 数値の取得OK
			return true;
		}
		// 数値ではなかった
		return false;
	}

	/**
		@brief		スペースおよびコメントをスキップする
	 */
	bool	skipSpaceAndComment( void )
	{
		// スペース＆コメント以外の文字が来るまでループ
		//  EOFまたはエラー発生時はfalseを返す
		for(;;)
		{
			// 行末なら新しい行を読む
			if( m_itLine == m_strLine.end() ){
				if( getNewLine() == false )
				{
					return false;
				}
			}

			// ホワイトスペースをスキップ
			for( ; isWhiteSpace(*m_itLine) ; m_itLine++ ){
				if( m_itLine == m_strLine.end() ){
					if( getNewLine() == false )
					{
						return false;
					}
				}
			}


			if( isCommentBlockStart( &(*m_itLine) ) ){
				// コメント"/*"なら次の"*/"まで破棄
				if( skipCommentBlock() == false )
				{
					return false;
				}
			} else if( isCommentLine( &(*m_itLine) ) )
			{
				// コメント"//"なら現在の行を破棄
				m_itLine = m_strLine.end();
			} else 
			{
				break;
			}
		}
		return true;
	}


public:
	/**
		@brief	デストラクタ
	 */
	virtual ~CTextLineEx(void){}
	/**
		@brief	コンストラクタ
		@param	ist		テキストの入力ストリーム
	 */
	CTextLineEx(  tistream& ist )
		: CTextLine( ist ),
			 _SPECIAL__TCHAR(_CTextEX_RESERVED__TCHAR),
			 _SPECIAL_STR(_CTextEX_RESERVED_STR),
			 m_uiLastError( ERROR_NONE )
	{}

	/**
		@brief	行の文字数を指定して一行取得(日本語対応)
		@param	tstrLine	文字列の格納先
		@param	nMaxSize	1行の長さ
		@param	nTAB		TABサイズ
		@return	true:文字列を取得／false:EOFを検出(文字列を取得しない)
		@retval	tstrLine	取得した文字列
	 */
	bool GetLineEx( tstring& tstrLine, int nMaxSize=-1, int nTAB=8 )
	{
		tstrLine = "";

		if( m_itLine == m_strLine.end() ){
			if( getNewLine() == false )
			{
				return false;
			}
		}
		if( m_itLine == NULL )
		{
			return GetLineEx(tstrLine,nMaxSize,nTAB);
		}

		int nSize = 0;

		tstring::iterator it = m_itLine ;
		for(	; it != m_strLine.end() && (nMaxSize>0 ? (nSize < nMaxSize) : true)
				; nSize++, it++ )
		{
			if( _ISKANJI(*it) ){
				it++;
				nSize++;
			} else if( *it == _T('\t') ){
				nSize+=nTAB;
			}
		}

		tstrLine = m_strLine.substr( m_itLine-m_strLine.begin(), it-m_itLine );
		m_itLine = it;

		return true;
	}

	/**
		@brief	予約語・エスケープシーケンスを含めた単語取得
		@param	tstrWord	単語の格納先
		@return	true:文字列を取得／false:EOFを検出(単語を取得しない)
		@retval	tstrWord	取得した単語
	 */
	bool GetWordEx(tstring& tstrWord	)
	{
		tstrWord = "";		// DESTをクリア

		// ホワイトスペースおよびコメントを読み飛ばし
		if( skipSpaceAndComment() == false )
		{
			// EOFまたはエラーなど
			return false;
		}

		if( setSpecialStr( tstrWord ) ){
			// 予約語を検出
		} else if( *m_itLine == _T('\"') ){
			// 文字列？
			if( addStringWord( tstrWord ) == false )
			{
				// 文字列形式が異常
				return false;
			}
		} else  if( getNumStr( tstrWord ) ){
		
		} else 	if( isSpecial_TCHAR( *m_itLine ) ){
			// 特殊文字なら１文字のみの文字列とする
			tstrWord = *m_itLine;
			m_itLine++;
		} else {
			// 通常の文字列
			if( addNormalWord( tstrWord ) == false )
			{
				// エラー
				return false;
			}
		}
		
		// 空の文字列ならもう一回読み直す
		if( tstrWord.empty() )
		{
			return GetWordEx(tstrWord);
		}
		return true;	
	}

};
////////////////////////////////////////////////////////////////////////////////
// CText
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	簡易スクリプタ
			単語単位でParserへ出力。コメント(';' , '#')対応。		
 */
template< class PARSER >
class	CSimpleScript {
private:
	
	
public:
	/**
		@brief	デストラクタ
	 */
	virtual	~CSimpleScript(){}
	
	/**
		@brief	デフォルトコンストラクタ
	 */
	CSimpleScript(){}
	/**
		@brief	ファイル指定のコンストラクタ
		@param	ist		入力するストリーム
		@param	parser	解析クラス
	 */
	CSimpleScript( tistream& ist, PARSER& parser )
	{
		load( ist, parser );
	}
	/**
		@brief	スクリプトの解析
		@param	ist		入力するストリーム
		@param	parser	解析クラス
	 */
	void	operator()(tistream& ist, PARSER& parser )
	{
		load( ist, parser );
	}
	/**
		@brief	スクリプトの解析
		@param	ist		入力するストリーム
		@param	parser	解析クラス
		@exception	CSimpleError	':'のみの行を発見(不正なラベル)
	 */
	bool	Load( tistream& ist, PARSER& parser ) 
	{
		CTextLine	buf(ist);
		
		tstring	str;
		
		while( buf.getWord(str) ){
			
			if( str.empty() )continue ;
			
			if( str[0] == _T('#') || str[0] == _T(';') ){
				// Comment
				buf.getLine();
				continue;
			}
			
			if( str[ str.length()-1 ] == _T(':') ){
				// Label
				if( str.length() == 1 )
				{
					// ラベルエラー
					return false;
				}
				
				str.resize( str.length()-1 );	// cut ':'
				
				parser.label( str );
			} else {
				
				parser.data( str );
				
			}
		} 
		return true;
	}
};

/**
	@brief	16進数の文字列を数値に変換
	@param	data	変換する文字列
	@return	変換した数値
	@exception	CSimpleError	文字列が16進数文字ではない
 */
inline int GetHex( tstring data ) 
{
	int n = 0;
	tstring::iterator	it = data.begin();
	for( ; it != data.end() ; it++ ){
		
		int r = _HTOI(*it);
		if( r == -1 )
		{
///			// 変換できない文字を読み込んだ場合は、一つ前の文字までを変換
			return n;
		}
		n <<= 4;
		n |= r;
	}
	return n;
}

/**
	@brief	10進数の文字列を数値に変換(例外つき)
	@param	data	変換する文字列
	@return	変換した数値
	@exception	CSimpleError	文字列が10進数文字ではない
 */
inline int GetDec( tstring data ) 
{
	int n = 0;
	tstring::iterator	it = data.begin();
	for( ; it != data.end() ; it++ ){
		
		int r = _ATOI(*it);
		if( r == -1 )
		{
			// 変換できない文字が見つかった場合は、直前の文字までの変換結果を返す
			return n;
		}
		n *= 10;
		n |= r;
	}

	return n;
}

/**
	@brief	文字列中に指定の文字が存在するかチェック
	@param	src		検索する文字列
	@param	c		検索する文字
	@return	文字列中に指定の文字があればsrc文字列中の見つかった場所のポインタを返す。
			見つからなければNULLを返す
 */
inline _TCHAR* StrSearch( _TCHAR* src, _TCHAR c )
{
	for( ; *src ; src++ )
		if( *src == c )return src;
	return NULL;
}

}	// namespace simple
#endif	// _CText_HPP
