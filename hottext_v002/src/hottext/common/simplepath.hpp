// $Id: simplepath.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	パス名制御クラスヘッダファイル
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:55 $
 */


#ifndef	_SIMPLE_PATH_HPP
#define	_SIMPLE_PATH_HPP

namespace	simple {			/// namespace simple

////////////////////////////////////////////////////////////////////////////////
// simple_pass
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	パス管理クラス
			パスの移動・指定された位置からの相対ファイル指定など
			フォルダ文字(\\or/)を統一する。
 */
class	CPath {
private:
	tstring		m_tstrPath;					/// パスの文字列
	TCHAR		m_tchDelimiter;				/// ディレクトリのデリミタ('/' or '\')

	/**
		@brief	パスの末尾がデリミタ文字ならそれを削る
		@param	tstrPath	パス名
	 */
	static void	chompDelimiter( tstring& tstrPath )
	{
		if( tstrPath.length() == 0 )return;

		const tstring::reverse_iterator	it = tstrPath.rbegin();
		if( tstrPath.length() > 1
			&& (*it == _T('\\') || *it == _T('/') ) )
		{
			if( *(it+1) != _T(':') )
				tstrPath.resize( tstrPath.length()-1 );
		}
	}

	/**
		@brief	'\\'か'/'を検索し、見つかったキャラクタを返す。
				見つからなければ'\\0'を返す
		@param	tstrPath	検索するパス文字列
		@return	見つかったキャラクタ文字
				見つからなければ'\\0'を返す
	 */
	TCHAR	getDelimiter( const tstring& tstrPath )
	{
		if( tstrPath.find(_T('/')) != tstrPath.npos )
		{
			return  _T('/');
		} else if( tstrPath.find( _T('\\') ) != tstrPath.npos )
		{
			return _T('\\');
		}
		return _T('\0');
	}

	/**
		@brief	パスのデリミタキャラクタをデフォルトに統一する
				デフォルトのデリミタキャラクタが設定されていなければ何もしない
		@param	tstrPath	変更するパス文字列
		@retval	tstrPath	デリミタキャラクタを統一した結果を格納
	 */
	void	setDelimiter( tstring& tstrPath )
	{
		// デフォルトデリミタキャラクタが設定されている？
		if( m_tchDelimiter == _T('\0') )
		{
			return;
		}

		// デフォルトデリミタ以外のデリミタキャラクタを取得
		TCHAR	tchExceptDelimiter ;
		switch( m_tchDelimiter )
		{
		default:
			RuntimeError("デリミタ指定が異常です");
			// breakしない
		case _T('\\'):
			tchExceptDelimiter = _T('/');
			break;
		case _T('/'):
			tchExceptDelimiter = _T('\\');
			break;
		}

		// 異なるデリミタキャラクタをデフォルトキャラクタへ書き換え
		size_t	nPos = tstrPath.find( tchExceptDelimiter,0 );
		for( ; nPos != tstrPath.npos ; nPos = tstrPath.find( tchExceptDelimiter, nPos+1) )
		{
			tstrPath[nPos] = m_tchDelimiter;
		}
	}

public:
	/**
		@brief	コンストラクタ(初期値あり)
		@param	tszPath		格納するパス文字列
	 */
	CPath( const TCHAR* tszPath )
	{
		SetPath( tszPath );
	}
	/**
		@brief	コンストラクタ(初期値なし)
	 */
	CPath( void )
		: m_tstrPath(_T("")), m_tchDelimiter( _T('\0') )
	{}
	/**
		@brief	デストラクタ
	 */
	virtual ~CPath(void)
	{}

	/**
		@brief	パスを新たに設定する(文字列からの指定)
		@param	tszPath		設定するパス文字列
	*/
	void	SetPath( const TCHAR* tszPath )
	{
		// set new path
		m_tstrPath = tszPath;
		// get dir char
		m_tchDelimiter = getDelimiter( m_tstrPath );
		// chomp DirChar
		chompDelimiter( m_tstrPath );

	}
	/**
		@brief	パスを新たに設定する(stringからの指定)
		@param	tstrPath		設定するパス文字列
	 */
	void	SetPath( const tstring& tstrPath )
	{
		SetPath( tstrPath.c_str() );
	}

	/**
		@brief	現在のパスへパスを追加する。
				パスが相対パスの場合は現在のパスから追加
				絶対パスの場合は新しいパスへ変更
		@param	tstrAddPath		追加するパス文字列
	 */
	void	AddPath( tstring tstrAddPath )
	{
		// 絶対パス？
		if( tstrAddPath[0] == _T('\\') || tstrAddPath[0] == _T('/')
			|| tstrAddPath.find(_T(':')) != tstrAddPath.npos )
		{
			// absolute path
			SetPath( tstrAddPath );
			return ;
		}

		// connect path
		if( m_tchDelimiter == _T('\0') )
		{
			// get new delimiter
			m_tchDelimiter = getDelimiter( tstrAddPath );
		} else 
		{
			// set now-delimiter for add path
			setDelimiter( tstrAddPath );
		}
		// add path string
		if(m_tstrPath.length() != 0 && *(m_tstrPath.rbegin()) != m_tchDelimiter)
		{
			if( m_tchDelimiter == _T('\0') )
			{
				// デリミタ文字が設定されていない場合は、'\'を設定する
				m_tchDelimiter = _T('\\');
			}
			m_tstrPath += m_tchDelimiter;
		}
		m_tstrPath += tstrAddPath;

		// chomp delimiter
		chompDelimiter( m_tstrPath );
	}
	/**
		@brief	パス文字列の取得
		@return	取得したパス文字列
	 */
	const TCHAR* GetPath(void) const
	{
		return m_tstrPath.c_str();
	}

	/**
		@brief		ファイル名の取得
		@param		tstrDest		取得したファイル名の格納先
		@return		true:ファイル名の取得に成功／false:失敗
	 */
	const bool GetFilename( tstring& tstrDest ) const
	{
		CPath	path( this->GetPath() ) ;

		if( path.UpPath() == false )return false;

		tstring	tstrPath = path.GetPath();
		
		size_t tStart = tstrPath.length();
		if( m_tstrPath[ tStart ] == m_tchDelimiter )
		{
			tStart++;
		}
		tstrDest = m_tstrPath.substr( tStart, m_tstrPath.length() );
		chompDelimiter( tstrDest );

		return true;
	}

	/**
		@brief	ひとつ上のディレクトリへ移動する
		@return	true:ひとつ上へ移動した
				false:移動できなかった(最上位のフォルダにいる)
	 */
	bool	UpPath(void)
	{
		size_t		nPos ;

		// 変換前の文字数を保存
		size_t		tPreLength = m_tstrPath.length();

		nPos = m_tstrPath.rfind( m_tchDelimiter, m_tstrPath.length() );
		if( nPos == m_tstrPath.npos )
		{
			// デリミタ文字が見つからない
			// ドライブ情報がある？
			nPos = m_tstrPath.rfind( _T(':') );
			if( nPos == m_tstrPath.npos )
			{
				// ドライブ情報がなくデリミタもない場合("readme.txt")は、パスをクリア
				m_tstrPath = _T("");
				return ( tPreLength != m_tstrPath.length() );
			}
		}

		if( nPos == 0 )
		{
			// 文字列の先頭にデリミタ文字がある場合は、デリミタ文字のみとする
			m_tstrPath.resize( 1 );
		} else {
			if( m_tstrPath[nPos-1] == m_tchDelimiter
				|| m_tstrPath[nPos] == _T(':')
				|| m_tstrPath[nPos-1] == _T(':')
				)
			{
				// デリミタ文字の一文字前がデリミタ文字("//james")
				// または、デリミタ文字の一文字手前がドライブ文字("a:\")
				// または、デリミタ文字がドライブ文字("a:")
				// の場合は、デリミタ文字/ドライブ文字を残す
				m_tstrPath.resize( nPos+1 );
			} else {
				m_tstrPath.resize( nPos );
			}
		}

		return ( tPreLength != m_tstrPath.length() );
	}

	// operators
	/**
		@brief	"="演算子のオーバロード
				パス文字列の設定
		@param	tstrNewPath		新たなパス文字列
		@return	自分自身の参照
	 */
	const CPath& operator =( const tstring& tstrNewPath )
	{
		SetPath( tstrNewPath );
		return (*this);
	}
	/**
		@brief	"="演算子のオーバロード
				パス文字列の設定(文字列からの指定)
		@param	tszNewPath		新たなパス文字列
		@return	自分自身の参照
	 */
	const CPath& operator =(const TCHAR* tszNewPath )
	{
		SetPath( tszNewPath );
		return (*this);
	}
	/**
		@brief	"+="演算子のオーバロード
				パスを追加する
		@param	tszAddPath	追加するパス
		@return	自分自身の参照
	 */
	const CPath& operator +=(const TCHAR* tszAddPath )
	{
		AddPath( tszAddPath );
		return (*this);
	}
	/**
		@brief	"+="演算子のオーバロード(stringによる指定)
				パスを追加する
		@param	tstrAddPath	追加するパス
		@return	自分自身の参照
	 */
	const CPath& operator +=(const tstring& tstrAddPath )
	{
		AddPath( tstrAddPath );
		return (*this);
	}

	/**
		@brief	前置デクリメント演算子のオーバロード
				ひとつ上のディレクトリへ移動
		@return	自分自身の参照
	 */
	const CPath& operator --(void)
	{
		UpPath();
		return (*this);
	}
	/**
		@brief	後置デクリメント演算子のオーバロード
				ひとつ上のディレクトリへ移動
		@param	n		デクリメント回数？
		@return	自分自身の参照
	 */
	const CPath& operator --(int)
	{
		UpPath();
		return (*this);
	}
	// コピーコンストラクタ
	/**
		@brief	コピーコンストラクタ
				パス文字列をコピーする
		@param	path	コピー元のパス
	 */
	CPath( CPath& path )
	{
		SetPath( path.GetPath() );
		m_tchDelimiter = path.m_tchDelimiter;
	}

	/**
		@brief	ストリームへの出力処理
				パスを文字列として出力
		@param	ost		出力先のストリームの参照
		@param	path	出力するパス
		@return	出力先のストリームの参照
	 */
	friend std::ostream&   operator << (std::ostream& ost,CPath& path )
	{
		ost << path.GetPath();
		return ost;
	}
};


}			/// namespace simple

#endif // _SIMPLE_PATH_HPP