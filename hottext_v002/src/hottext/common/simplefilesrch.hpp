// $Id: simplefilesrch.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	�t�@�C�������N���X�w�b�_�t�@�C��
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
	@brief	�t�H���_�����N���X
 */
class	CFileSearch {
public:
	/**
		@brief	�p�X�̃t�@�C�����ւ̒ǉ�
		@param	tstrPath	�p�X������
		@param	tszFName	�ǉ�����t�@�C����
		@retval	tstrPath	�t�@�C������ǉ������p�X
	 */
	static void	AddFilename( tstring& tstrPath, const _TCHAR* tszFName )
	{
		if( *tstrPath.rbegin() != '\\' )tstrPath += "\\";
		tstrPath += tszFName;
	}

private:
	CFileSearch(const CFileSearch&);
	CFileSearch& operator=(const CFileSearch&);

	string_vector&	m_vecFilelist;		/// �������ʊi�[�p
	bool			m_bSearchChild;		/// �T�u�f�B���N�g������������H
	const _TCHAR*	m_tszExt;			/// �g���q(;�ŋ�؂��ĕ����w��\)

	/**
		@brief	�t�@�C�����̏d������
		@param	tstrFName	�`�F�b�N����t�@�C����
		@param	nStart		���X�g�̊J�n�ʒu
		@return	true:���łɓ����̃t�@�C�������X�g���ɂ���
				�^false:�t�@�C���������X�g�ɂȂ�
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
		@brief	�T�u�f�B���N�g��������
		@param	tszPath		��������p�X
		@note	�T�u�f�B���N�g�����������A���X�g�Ƀt�@�C������ǉ�����
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
		@brief	�g���q���Ƃ̃t�@�C������
		@param	tszPath	��������p�X
		@param	tszExt	�g���q�t�B���^
		@param	nStart	���X�g�̒ǉ��ʒu
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
		@brief	�t�H���_���̃t�@�C��������
		@param	tszPath		��������p�X
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
		@brief	�t�H���_���̃t�@�C������уT�u�f�B���N�g��������
		@param	tszPath		��������p�X
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
		@brief	�f�X�g���N�^
	 */
	virtual	~CFileSearch(void){}
	/**
		@brief	�R���X�g���N�^
				�^�t�H���_����g���q�ɊY������t�@�C�������������X�g���쐬����
		@param	vecFilelist		�쐬�����t�@�C�������X�g�̏o�͐�
		@param	tszPath			�����̋N�_�p�X
		@param	tszExt			�g���q(;�ŋ�؂��ĕ����w��\)
		@param	bSearchChild	�T�u�f�B���N�g�������̗L��
		@retval	vecFilelist		�쐬�����t�@�C�������X�g
	 */
	CFileSearch( string_vector& vecFilelist,		// �������ʂ̃t�@�C�������X�g
						const _TCHAR* tszPath,			// �����̋N�_�p�X
						const _TCHAR* tszExt,			// �g���q(;�ŋ�؂��ĕ����w��\)
						bool bSearchChild				// �T�u�f�B���N�g�������̗L��
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
