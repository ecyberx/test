// $Id: simplefilesrch.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	ファイル検索クラスヘッダファイル
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:55 $
 */
#ifndef	_SIMPLE_FILESRCH_HPP
#define	_SIMPLE_FILESRCH_HPP

#include "./base.hpp"
#include "./simpletext.hpp"

#include <windows.h>

namespace	simple	{		/// namespace simple
/**
	@brief	フォルダ検索クラス
 */
class	CFileSearch {
public:
	/**
		@brief	パスのファイル名への追加
		@param	tstrPath	パス文字列
		@param	tszFName	追加するファイル名
		@retval	tstrPath	ファイル名を追加したパス
	 */
	static void	AddFilename( tstring& tstrPath, const _TCHAR* tszFName )
	{
		if( *tstrPath.rbegin() != '\\' )tstrPath += "\\";
		tstrPath += tszFName;
	}

private:
	CFileSearch(const CFileSearch&);
	CFileSearch& operator=(const CFileSearch&);

	string_vector&	m_vecFilelist;		/// 検索結果格納用
	bool			m_bSearchChild;		/// サブディレクトリを検索する？
	const _TCHAR*	m_tszExt;			/// 拡張子(;で区切って複数指定可能)

	/**
		@brief	ファイル名の重複調査
		@param	tstrFName	チェックするファイル名
		@param	nStart		リストの開始位置
		@return	true:すでに同名のファイルがリスト内にある
				／false:ファイル名がリストにない
	 */
	bool	isDuplication( const tstring& tstrFName, size_t nStart )
	{
		string_vector::iterator it = &m_vecFilelist[nStart];
		for( ; it != m_vecFilelist.end() ; it++ )
		{
			if( *it == tstrFName )
			{
				return true;
			}
		}
		return false;
	}

	/**
		@brief	サブディレクトリを検索
		@param	tszPath		検索するパス
		@note	サブディレクトリを検索し、リストにファイル名を追加する
	 */
	void	searchDirectory( const _TCHAR* tszPath )
	{	
		WIN32_FIND_DATA	ffd;

		tstring	tstrPath = tszPath;
		AddFilename( tstrPath, "*.*" );

		HANDLE	hFind = FindFirstFile( tstrPath.c_str(), &ffd );

		if( hFind == INVALID_HANDLE_VALUE )
		{

		} else {
			do {
				if( ffd.cFileName[0] == _T('.') )continue;

				if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					std::string	tstrNewPath = tszPath;
					AddFilename( tstrNewPath, ffd.cFileName );
					search( tstrNewPath.c_str() );
				}

			} while( FindNextFile( hFind, &ffd) != 0 );

			FindClose( hFind );
		}
	}

	// 
	/**
		@brief	拡張子ごとのファイル検索
		@param	tszPath	検索するパス
		@param	tszExt	拡張子フィルタ
		@param	nStart	リストの追加位置
	 */
	void	searchFileExt( const _TCHAR* tszPath, const _TCHAR* tszExt, size_t nStart )
	{
		WIN32_FIND_DATA	ffd;
		tstring	tstrPathExt = tszPath;
		AddFilename( tstrPathExt, tszExt );

		HANDLE	hFind = FindFirstFile( tstrPathExt.c_str(), &ffd );

		if( hFind == INVALID_HANDLE_VALUE )
		{

		} else {
			do {
				if( ~ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					tstring	tstrFName = tszPath;
					AddFilename( tstrFName, ffd.cFileName );

					if( !isDuplication( tstrFName, nStart ) )
					{
						m_vecFilelist.push_back( tstrFName );
					}
				}

			} while( FindNextFile( hFind, &ffd) != 0 );

			FindClose( hFind );
		}


	}

	/**
		@brief	フォルダ内のファイルを検索
		@param	tszPath		検索するパス
	 */
	void	searchFile( const _TCHAR* tszPath )
	{
		size_t nStart = m_vecFilelist.size();

		CStringSearch	srch(m_tszExt);
		tstring	tstrExt;

		while( srch.GetNext( tstrExt ) )
		{
			searchFileExt( tszPath, tstrExt.c_str(), nStart );
			
		}
	}

	/**
		@brief	フォルダ内のファイルおよびサブディレクトリを検索
		@param	tszPath		検索するパス
	 */
	void	search( const _TCHAR* tszPath )
	{
		if( m_bSearchChild )
		{
			searchDirectory( tszPath );
		}
		searchFile( tszPath );
	}

public:
	/**
		@brief	デストラクタ
	 */
	virtual	~CFileSearch(void){}
	/**
		@brief	コンストラクタ
				／フォルダから拡張子に該当するファイルを検索しリストを作成する
		@param	vecFilelist		作成したファイル名リストの出力先
		@param	tszPath			検索の起点パス
		@param	tszExt			拡張子(;で区切って複数指定可能)
		@param	bSearchChild	サブディレクトリ検索の有無
		@retval	vecFilelist		作成したファイル名リスト
	 */
	CFileSearch( string_vector& vecFilelist,		// 検索結果のファイル名リスト
						const _TCHAR* tszPath,			// 検索の起点パス
						const _TCHAR* tszExt,			// 拡張子(;で区切って複数指定可能)
						bool bSearchChild				// サブディレクトリ検索の有無
						)
		:	m_bSearchChild(bSearchChild),
			m_tszExt(tszExt),
			m_vecFilelist(vecFilelist)
	{
		search( tszPath );
	}

};

} /// namespace simple
#endif	//	_SIMPLE_FILESRCH_HPP
