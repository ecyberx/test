// $Id: simpletext.hpp,v 1.2 2005/07/07 23:35:56 Ander Exp $
/**
	@brief	�e�L�X�g�Ǘ��N���X�֘A�w�b�_�t�@�C��
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:56 $
 */

#ifndef	_CText_HPP
#define	_CText_HPP

#include "./base.hpp"

namespace simple {		// namespace simple

#include <sstream>

/// �e�L�X�g�N���X�Ŏg�p����\�񕶎���`
#define	_CTextEX_RESERVED__TCHAR	_T("{}(),;+-*/\"^|&~!=[]")
/// �e�L�X�g�N���X�Ŏg�p����\����`
#define _CTextEX_RESERVED_STR		_T("++;--;==")

/// �ꌅ��HEX�����𐔒l�ɕϊ�
#define	_HTOI(x)	(	( (x)>=_T('0') && (x)<=_T('9') ) ? ((x) - _T('0'))				\
						 : ( (x)>=_T('a') && (x)<=_T('f')) ? ((x)-_T('a')+0x0a)			\
						 : ( (x)>=_T('A') && (x)<=_T('F')) ? ((x)-_T('A')+0x0a) : -1 )
/// �ꌅ�̐����𐔒l�ɕϊ�
#define	_ATOI(x)	( ( (x)>=_T('0') && (x)<=_T('9') ) ? ((x)-_T('0')) : -1 )
/// �ꌅ��8�i�������𐔒l�ɕϊ�
#define	_OTOI(x)	( ( (x)>=_T('0') && (x)<=_T('7') ) ? ((x)-_T('0')) : -1 )

#ifdef _UNICODE
/// �����R�[�h����
#define	_ISKANJI(x)	(false)
#else
/// �����R�[�h����
#define	_ISKANJI(x)	( (signed)(x)<-1 )
#endif

////////////////////////////////////////////////////////////////////////////////
// simple_search_string
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	�f���~�^�ŋ�؂�ꂽ������̌����Ȃǂ��s��
 */
class	CStringSearch {
public:
	/**
		@brief	������src1�̐擪��src2���`�F�b�N
		@param	src1	�`�F�b�N�Ώۂ̕�����
		@param	src2	�擪�ɂ��邩�`�F�b�N���镶����
		@return	true:src1�̐擪��src2����������
				false:src1�̐擪��src2�ł͂Ȃ�
	 */
	static bool	IsStrTop( const _TCHAR* src1,const _TCHAR* src2 )
	{
		for( ; *src1 && *src2 && (*src1 == *src2) ; src1++,src2++ );
		return ( *src2 == NULL );
	}

	/**
		@brief	strSrc��nPosStart�`nPosEnd�܂ł�strDest�փR�s�[
		@param	strSrc		�R�s�[���̕�����
		@param	strDest		�R�s�[�f�[�^�̊i�[��
		@param	nPosStart	�R�s�[�J�n�ʒu
		@param	nPosEnd		�R�s�[�I�[�ʒu
							������̍Ō�܂Ŏ擾����ꍇ��nPosEnd��npos��ݒ�
		@retval	strDest		�R�s�[���ꂽ������
	 */
	static 	void SubstrEx( const tstring& strSrc,
							tstring& strDest,
							size_t nPosStart, size_t nPosEnd )
	{
		if( nPosEnd < nPosStart )
		{
			RuntimeError( "SubstrEx::�������s���ł�", nPosStart, nPosEnd );
			return;
		}
		size_t nCnt = (nPosEnd == strSrc.npos ) ? nPosEnd : (nPosEnd - nPosStart) ;
		strDest = strSrc.substr( nPosStart, nCnt );
	}

	/**
		@brief	��������f���~�^�ŋ�؂�ꂽ�����񂩂猟�����Ĕ������ꂽ�ʒu��Ԃ�(SRC�̕�����w��)
		@param	it				�����Ώۂ̕�����(�P��)
		@param	tstrSrc			�����Ώۂ̃f���~�^�ŋ�؂�ꂽ������
		@param	tstrDest		���������P��̊i�[��
		@param	tchDelimiter	tstrSrc�̃f���~�^����
		@return	������it��������tstrSrc���猩�������ꏊ
		@retval	tstrDest		���������P��
		@note	"*.h"��"*.c;*.h;*.hpp"�ɂ��邩�`�F�b�N����ꍇ�ȂǂɎg�p
	 */
	static int	GetCount( const _TCHAR* it,
							const _TCHAR* tstrSrc,
							tstring& tstrDest,
							const _TCHAR tchDelimiter = _T(';') )
	{
		return GetCount( it,tstring(tstrSrc),tstrDest,tchDelimiter );
	}
	/**
		@brief	��������f���~�^�ŋ�؂�ꂽ�����񂩂猟�����Ĕ������ꂽ�ʒu��Ԃ�
		@param	it				�����Ώۂ̕�����(�P��)
		@param	tstrSrc			�����Ώۂ̃f���~�^�ŋ�؂�ꂽ������
		@param	tstrDest		���������P��̊i�[��
		@param	tchDelimiter	tstrSrc�̃f���~�^����
		@return	������it��������tstrSrc���猩�������ꏊ
		@retval	tstrDest		���������P��
		@note	"*.h"��"*.c;*.h;*.hpp"�ɂ��邩�`�F�b�N����ꍇ�ȂǂɎg�p
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
	const tstring	m_tstrSrc;					/// �����Ώۂ̕�����
	tstring			m_tstrDest;					/// �������ʂ̊i�[��
	const _TCHAR	m_tchDelimiter;				/// �f���~�^����
	size_t			m_nPosStart;				/// �����J�n�ʒu
	CStringSearch( const CStringSearch& );
	CStringSearch& operator =( const CStringSearch& );
public:
	/**
		@brief	�f�X�g���N�^
	 */
	virtual	~CStringSearch(void){}
	/**
		@brief	�R���X�g���N�^
		@param	tstrSrc			�f���~�^�ŋ�؂�ꂽ������
		@param	tchDelimiter	�f���~�^����(�f�t�H���g��';')
	 */
	CStringSearch( const tstring& tstrSrc, _TCHAR tchDelimiter=_T(';') )
		: m_tstrSrc(tstrSrc), m_tchDelimiter(tchDelimiter)
	{
		Reset();
	}

	// ���̕�������擾
	/**
		@brief	�f���~�^���玟�̕�������擾
		@param	tstrDest	������̊i�[��
		@return	true:���̕����񂪌�������
				false:���̕����񂪌�����Ȃ�����(�f���~�^������̍Ō�ɂ���)
		@retval	tstrDest	���̕�����
	 */
	bool GetNext(tstring& tstrDest )
	{
		// ������̌������������Ă���Ȃ�EOF�𓊂���
		if( m_nPosStart == tstring::npos )
		{
			return false;
		}

		// ��������擾
		size_t nPos = m_tstrSrc.find( m_tchDelimiter ,m_nPosStart );
		SubstrEx( m_tstrSrc, tstrDest, m_nPosStart, nPos );

		// ���̕�����̊J�n�ʒu��ݒ�
		m_nPosStart = ( nPos == tstring::npos ) ? nPos : nPos+1;

		return	true;
	}
	/**
		@brief	���݂̃f���~�^�����񌟍��ʒu�����Z�b�g����
				(�擪���猟���ł���悤�ɐݒ肷��)
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
	@brief	�������̓N���X
			�P��E�s�P�ʂł̕�����擾
 */
class	CTextLine {
protected:
	
	unsigned int			m_uiLine;			/// ���݂̍s�ԍ�

	tistream&			m_ist ;					/// �Ǎ���̃X�g���[��
	tstring				m_strLine;				// ���ݓǂݍ���ł���s
	tstring::iterator	m_itLine;				// ���ݓǂݍ���ł���s�̎Q�ƈʒu
	
	tstring _WHITE_SPACE ;						// �z���C�g�X�y�[�X�����̏W��

	// ���̍s���擾
	/**
		@brief	�V�����s���X�g���[������ǂݍ���
		@param	bConnect	�s��A������H(�s�[��'\'�ɂ��s�̘A������true)
		@return	true:�s�̓ǂݍ��݂ɐ����^false:EOF�����o(�s��ǂݍ��߂Ȃ�)
		@retval	m_strLine	�ǂݍ��񂾍s�̓��e���i�[
		@retval	m_itLine	�ǂݍ��񂾍s�̐擪�ʒu���Q��
	 */
	bool	getNewLine( bool bConnect = false )
	{
		// �t�@�C����EOF�Ȃ�EOF��O
		if( m_ist.eof() )
		{
			return false;
		}
		
		// �s���N���A
		m_strLine = _T("");
		
		++m_uiLine;		// �s�ԍ������Z

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
		
		// ���s�݂̂̍s�̏ꍇ�͎��̍s��ǂ�
		if( m_strLine.empty() && bConnect==false){
			return getNewLine();
		}
		// �s�̘A�ڂ��`�F�b�N
		if( m_strLine[ m_strLine.length()-1 ] == _T('\\') )
		{
			// ������'\'�Ȃ玟�̍s�ƘA�ڂ���
			tstring	line2 = m_strLine;			// ���݂̍s�̓��e��ۑ�
			line2.resize( line2.length()-1 );		// ������'\'���폜
			getNewLine(true);						// �A�ڂ���Ŏ��̍s��ǂݏo��
			m_strLine.insert(0,line2);				// ���݂̍s�Ǝ��̍s��A��
		}
		m_itLine = m_strLine.begin();

		return true;
	}

	/**
		@brief	�w�肵���������z���C�g�X�y�[�X���`�F�b�N����
		@param	c	�`�F�b�N���镶��
		@return	true:�������z���C�g�X�y�[�X�ɊY���^false:�����̓z���C�g�X�y�[�X�ł͂Ȃ�
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
		@brief	�f�X�g���N�^
	 */
	virtual	~CTextLine(){}
	
	/**
		@brief	�R���X�g���N�^
		@param	ist	���͂���X�g���[��
	 */
	CTextLine( tistream& ist )
	 : m_ist(ist), _WHITE_SPACE( _T(" \r\n\t") ), m_uiLine(0)
	{
		m_itLine = m_strLine.end();
	}
	
	/**
		@brief	�P����擾
		@param	tstrWord	�P��̊i�[��
		@return	true:�����񂠂�^false:EOF
		@retval	tstrWord	�擾�����P����i�[
	 */
	bool	GetWord(tstring& tstrWord )
	{
		tstrWord = "";	// DEST���N���A

		// �s���Ȃ�V�����s��ǂ�
		if( m_itLine == m_strLine.end() ){
			if( getNewLine() == false )
			{
				return false;
			}
		}
		// �z���C�g�X�y�[�X���X�L�b�v
		for( ; isWhiteSpace(*m_itLine) ; m_itLine++ ){
			if( m_itLine == m_strLine.end() ){
				if( getNewLine() == false )
				{
					return false;
				}
			}
		}
		// ���̃z���C�g�X�y�[�X�܂��͍s�[�܂ł�P��Ƃ��Ď擾
		for( ; !isWhiteSpace(*m_itLine) && m_itLine != m_strLine.end() ; m_itLine++ )
		{
			tstrWord += *m_itLine ;
		}
		
		// ��̕�����Ȃ�������ǂݒ���
		if( tstrWord.empty() )return GetWord(tstrWord);
		return true;
	}
	
	// 
	/**
		@brief	�ǂݍ��ݒ��̍s�̌��݈ʒu����s���܂ł��擾
		@param	tstrLine	�擾����������̊i�[��
		@return	true:�����񂠂�^false:EOF
		@retval	tstrLine	�擾������������i�[
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
		@brief	���ݓǂݍ���ł���ʒu�̕������擾
		@return	���݈ʒu�̕���
	 */
	_TCHAR GetPosCharactor(void)
	{
		return *m_itLine;
	}

	/**
		@brief	���ݓǂݍ���ł���s�̍s�ԍ��̎擾
		@return	���ݓǂݍ���ł���s�ԍ�
	 */
	unsigned int	GetLineNumber(void)
	{
		return m_uiLine;
	}

	/**
		@brief	������ɃX�g���[������ꕶ���ǉ�����
				(�����ł����2�o�C�g�ǉ�)
		@param	strWord		�ǉ�����镶����
		@retval	strWord		������̒ǉ����ꂽ���ʂ��i�[
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
	@brief	�v���v���Z�b�T�p�P��P�ʂł̕�����擾�g����
			�R�����g�A������(" �` ")�ɑΉ�
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
		@brief	�e�L�X�g�ǂݍ��ݎ��̃G���[���擾����
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
		@brief	������" �` "�`���Ȃ璆�g�𒊏o����TRUE��Ԃ��B�ʏ핶����Ȃ�FALSE�B
		@param	strWord		��͂��镶����
		@return	true:�����񂪁h�ŋ�؂�ꂽ�`���������^false:�����񂪁h�ŋ�؂��Ă��Ȃ�
		@retval	strWord		�߂�l��true�̏ꍇ�h�`�h�ŋ�؂�ꂽ������̒��g���i�[�����
	 */
	static bool	IsStringWord( tstring& strWord )
	{
		if( strWord.empty() || (strWord[0] != _T('\"')
			&& strWord[ strWord.length() -1 ] != _T('\"') ) ){

			// " �` "�`���łȂ�
			return false;

		} else {
			
			// �s�[���폜
			strWord.erase( strWord.begin(), strWord.begin()+1 );
			strWord.resize( strWord.length() -1 );
		}

		return true;
	}

	/**
		@brief	HEX�����𐔒l�ɕϊ�����
				HEX�����łȂ���Η�O
		@param	c	HEX����
		@return �ϊ���̐��l
		@exception	CSimpleError	�w�肵��������HEX�����łȂ�
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
		@brief	OCT�����𐔒l�ɕϊ�����
				OCT�����łȂ���Η�O
		@param	c	OCT����
		@return �ϊ���̐��l
		@exception	CSimpleError	�w�肵��������OCT�����łȂ�
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
	tstring	_SPECIAL__TCHAR;					/// ����ȕ����Q(�P�ƂňӖ���������)
	tstring	_SPECIAL_STR;						/// ����ȕ�����Q(�P�ƂňӖ�����������)

	/**
		@brief	�G�X�P�[�v�V�[�P���X���當���֕ϊ�
		@param	c	�G�X�P�[�v����'\'�̎��̕���
		@return	�ϊ����ꂽ����
		@exception	CSimpleError	�s���ȃG�X�P�[�v�V�[�P���X
	 */
	bool	changeESC( _TCHAR& dest, _TCHAR c ) 
///	_TCHAR	changeESC( _TCHAR c ) 
	{
		// �G�X�P�[�v�V�[�P���X����ϊ�
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
		@brief	���ꕶ�������ʂ���
		@param	c	���ʂ��镶��
		@return	true:���ꕶ���ł���^false:���ꕶ���łȂ�
	 */
	bool	isSpecial_TCHAR( const _TCHAR c )
	{
		return (_SPECIAL__TCHAR.find_first_of(c) != tstring::npos) ;
	}

	// 
	//	
	//	
	/**
		@brief	������̐擪�ɗ\��ꂪ���邩�`�F�b�N����
				�\���𔭌������ꍇ�A�\��ꕶ���񒆂̊Y������\���̐擪�ʒu��Ԃ��B
				�����ł��Ȃ��ꍇ��-1��Ԃ�
		@param	it		�����Ώۂ̕�����
		@param	str		�\���̋L�q���ꂽ������(';'�ŋ�؂��ĕ����w��\)
		@return	�\��ꕶ���񒆂̊Y�������̐擪�ʒu
				�^�\��ꂪ�����ł��Ȃ����-1��Ԃ�
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
		@brief	�����񂪍s�R�����g�J�n�������`�F�b�N
		@param	it	�`�F�b�N���镶����
		@return	true:������̐擪���s�R�����g�^false:�����񂪍s�R�����g�łȂ�
	 */
	bool	isCommentLine( const _TCHAR* it )
	{
		return CStringSearch::IsStrTop( it, _T("//") );
	}

	/**
		@brief	������̐擪���h�^���h���`�F�b�N
		@param	it	�`�F�b�N���镶����
		@return	true:������̐擪���h�^���h
				�^false:�����񂪁h�^���h�łȂ�
	 */
	bool	isCommentBlockStart( const _TCHAR* it )
	{
		return CStringSearch::IsStrTop( it, _T("/*") );
	}
	/**
		@brief	������̐擪���h���^�h���`�F�b�N
		@param	it	�`�F�b�N���镶����
		@return	true:������̐擪���h���^�h
				�^false:�����񂪁h���^�h�łȂ�
	 */
	bool	isCommentBlockEnd( const _TCHAR* it )
	{
		return CStringSearch::IsStrTop( it, _T("*/") );
	}

	/**
		@brief	���̃z���C�g�X�y�[�X�܂��͍s�[�܂ł�P��Ƃ��Ď擾
		@param	strWord		�P��̊i�[��
		@retval	strWord		�擾�����P��
		@return	true:����I��/false:�G���[
	 */
	bool	addNormalWord( tstring& strWord )
	{
		bool	bESC = false;

		// ���̃z���C�g�X�y�[�X�܂��͍s�[�܂ł�P��Ƃ��Ď擾
		for( ; !isWhiteSpace(*m_itLine) && m_itLine != m_strLine.end() ; m_itLine++ )
		{
			tstring	tstrEmpty;
			if( *m_itLine == _T('\\') ){
				// �G�X�P�[�v�V�[�P���X�𔭌������ꍇ�́AESC�t���O�𗧂Ăă��[�v�I��
				bESC = true;
				continue;
			}
			if( isCommentLine( &(*m_itLine) ) || isCommentBlockStart( &(*m_itLine) ) ){
				// �R�����g�𔭌�
				break;
			} else if( searchSpecialStr( &(*m_itLine), tstrEmpty ) >=0 ){
				break;	
			} else 	if( !bESC && isSpecial_TCHAR( *m_itLine ) ){
				// ���ꕶ���𔭌������ꍇ�́A�P���ł��؂�
				break;
			}		

			if( bESC == true ){
				// �G�X�P�[�v�V�[�P���X
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

		// �G�X�P�[�v������������(�Ō�̕�����"\")�̏ꍇ�̓G���[
		if( bESC )
		{
			setError( ERROR_BAD_ESCSEQENCE );
			return false;
 		}

		return true;
	}

	/**
		@brief	�h�`�h�ŋ�؂�ꂽ������̎擾
		@param	strWord		�擾����������̊i�[��
		@retval	strWord		�擾����������
	 */
	bool	addStringWord( tstring& strWord )
	{
		// �h�`�h�ŋ�؂�ꂽ������
		m_itLine++;		// �h���X�L�b�v
		if( strWord.empty() )
		{
			// ������̘A���łȂ��ꍇ�́A�擪�֕�����J�n������ǉ�
			strWord = _T('\"');
		}

		bool	bESC = false;		/// �G�X�P�[�v�����t���O

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
			// ������̏I��������������Ȃ�
			setError( ERROR_STRING_TERM_NOTFOUND );
			return false;
		}

		m_itLine++;		// �h���X�L�b�v

		// ������̘A���`�F�b�N
		// ���̕����̈ꕶ���ڂ�������J�n�ł���΁A�������A������
		if( skipSpaceAndComment() && (*m_itLine) == _T('\"') )
		{
			// ���̕������A��
			return addStringWord( strWord );
		}

		// ������̏I�������̒ǉ�
		strWord += _T('\"');

		return true;
	}

	/**
		@brief	�R�����g�u���b�N(�^���`���^)���X�L�b�v����
		@exception	CSimpleError	�h���^�h��������Ȃ�
	 */
	bool	skipCommentBlock(void)
	{
		// �R�����g/* �` */�܂ł��X�L�b�v
		m_itLine += 2;		// �擪��"/*"���X�L�b�v

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
				// */����������
				break;
			}
		}

		m_itLine += 2;		// ������"*/"���X�L�b�v

		return true;
	}
	/**
		@brief	 ���̌ꂪ�\��ꂩ�`�F�b�N���A�\���Ȃ甲���o��true��Ԃ�
		@param	strWord		�擾����������̊i�[��
		@return	true:���̌ꂪ�\���ł���^false:�\���ł͂Ȃ�
		@retval	strWord		�߂�l��true�̏ꍇ�A�\�����i�[
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
		@brief	���̌ꂪ�������������ǂ����`�F�b�N���A
				�����ł���ΐ��l�ƍl������͈͂��擾����
		@param	tstrWord	�擾����������̊i�[��
		@return	true:���̌ꂪ�����^false:���̌ꂪ�����łȂ�
		@retval	tstrWord	�߂�l��true�̏ꍇ�A�����������͈͂��擾����
		@exception	CSimpleError	������̍Ōオ"."������
	 */
	bool	getNumStr( tstring& tstrWord ) 
	{
		// �����������܂��́A�{�܂��́|�Ŏ��̕����������Ȃ琔�l�Ƃ���
		if( _ATOI(*m_itLine) >= 0 
			|| ( (*m_itLine == '-' || *m_itLine=='+') && _ATOI( *(m_itLine+1) ) >= 0 ) )
		
		{
			// ���l���擾
			do {
				tstrWord += *m_itLine;
				m_itLine++;
			} while( (*m_itLine >= '0' && *m_itLine <= '9') || (*m_itLine == '.') ); 

			// HEX�`�F�b�N
			if( (*m_itLine == 'x' || *m_itLine == 'X') && tstrWord == "0" )
			{
				tstrWord += *m_itLine;
				m_itLine++;
				if( _HTOI( *m_itLine ) < 0 )
				{
					// "0x"�̌オHEX�łȂ�
					setError( ERROR_BAD_NUMERIC );
					return false;
				}

				// 16�i���̎擾
				while( *m_itLine ) 
				{
					tstrWord += *m_itLine;
					m_itLine++;
					if( _HTOI( *m_itLine ) < 0 )break;
				}
				// HEX�擾OK
				return true;
			}

			if( *tstrWord.rbegin() == '.' )
			{
				// ���l������̖�����'.'�Ȃ�G���[
				setError( ERROR_BAD_NUMERIC );
				return false;
			}
			// ���l�̎擾OK
			return true;
		}
		// ���l�ł͂Ȃ�����
		return false;
	}

	/**
		@brief		�X�y�[�X����уR�����g���X�L�b�v����
	 */
	bool	skipSpaceAndComment( void )
	{
		// �X�y�[�X���R�����g�ȊO�̕���������܂Ń��[�v
		//  EOF�܂��̓G���[��������false��Ԃ�
		for(;;)
		{
			// �s���Ȃ�V�����s��ǂ�
			if( m_itLine == m_strLine.end() ){
				if( getNewLine() == false )
				{
					return false;
				}
			}

			// �z���C�g�X�y�[�X���X�L�b�v
			for( ; isWhiteSpace(*m_itLine) ; m_itLine++ ){
				if( m_itLine == m_strLine.end() ){
					if( getNewLine() == false )
					{
						return false;
					}
				}
			}


			if( isCommentBlockStart( &(*m_itLine) ) ){
				// �R�����g"/*"�Ȃ玟��"*/"�܂Ŕj��
				if( skipCommentBlock() == false )
				{
					return false;
				}
			} else if( isCommentLine( &(*m_itLine) ) )
			{
				// �R�����g"//"�Ȃ猻�݂̍s��j��
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
		@brief	�f�X�g���N�^
	 */
	virtual ~CTextLineEx(void){}
	/**
		@brief	�R���X�g���N�^
		@param	ist		�e�L�X�g�̓��̓X�g���[��
	 */
	CTextLineEx(  tistream& ist )
		: CTextLine( ist ),
			 _SPECIAL__TCHAR(_CTextEX_RESERVED__TCHAR),
			 _SPECIAL_STR(_CTextEX_RESERVED_STR),
			 m_uiLastError( ERROR_NONE )
	{}

	/**
		@brief	�s�̕��������w�肵�Ĉ�s�擾(���{��Ή�)
		@param	tstrLine	������̊i�[��
		@param	nMaxSize	1�s�̒���
		@param	nTAB		TAB�T�C�Y
		@return	true:��������擾�^false:EOF�����o(��������擾���Ȃ�)
		@retval	tstrLine	�擾����������
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
		@brief	�\���E�G�X�P�[�v�V�[�P���X���܂߂��P��擾
		@param	tstrWord	�P��̊i�[��
		@return	true:��������擾�^false:EOF�����o(�P����擾���Ȃ�)
		@retval	tstrWord	�擾�����P��
	 */
	bool GetWordEx(tstring& tstrWord	)
	{
		tstrWord = "";		// DEST���N���A

		// �z���C�g�X�y�[�X����уR�����g��ǂݔ�΂�
		if( skipSpaceAndComment() == false )
		{
			// EOF�܂��̓G���[�Ȃ�
			return false;
		}

		if( setSpecialStr( tstrWord ) ){
			// �\�������o
		} else if( *m_itLine == _T('\"') ){
			// ������H
			if( addStringWord( tstrWord ) == false )
			{
				// ������`�����ُ�
				return false;
			}
		} else  if( getNumStr( tstrWord ) ){
		
		} else 	if( isSpecial_TCHAR( *m_itLine ) ){
			// ���ꕶ���Ȃ�P�����݂̂̕�����Ƃ���
			tstrWord = *m_itLine;
			m_itLine++;
		} else {
			// �ʏ�̕�����
			if( addNormalWord( tstrWord ) == false )
			{
				// �G���[
				return false;
			}
		}
		
		// ��̕�����Ȃ�������ǂݒ���
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
	@brief	�ȈՃX�N���v�^
			�P��P�ʂ�Parser�֏o�́B�R�����g(';' , '#')�Ή��B		
 */
template< class PARSER >
class	CSimpleScript {
private:
	
	
public:
	/**
		@brief	�f�X�g���N�^
	 */
	virtual	~CSimpleScript(){}
	
	/**
		@brief	�f�t�H���g�R���X�g���N�^
	 */
	CSimpleScript(){}
	/**
		@brief	�t�@�C���w��̃R���X�g���N�^
		@param	ist		���͂���X�g���[��
		@param	parser	��̓N���X
	 */
	CSimpleScript( tistream& ist, PARSER& parser )
	{
		load( ist, parser );
	}
	/**
		@brief	�X�N���v�g�̉��
		@param	ist		���͂���X�g���[��
		@param	parser	��̓N���X
	 */
	void	operator()(tistream& ist, PARSER& parser )
	{
		load( ist, parser );
	}
	/**
		@brief	�X�N���v�g�̉��
		@param	ist		���͂���X�g���[��
		@param	parser	��̓N���X
		@exception	CSimpleError	':'�݂̂̍s�𔭌�(�s���ȃ��x��)
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
					// ���x���G���[
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
	@brief	16�i���̕�����𐔒l�ɕϊ�
	@param	data	�ϊ����镶����
	@return	�ϊ��������l
	@exception	CSimpleError	������16�i�������ł͂Ȃ�
 */
inline int GetHex( tstring data ) 
{
	int n = 0;
	tstring::iterator	it = data.begin();
	for( ; it != data.end() ; it++ ){
		
		int r = _HTOI(*it);
		if( r == -1 )
		{
///			// �ϊ��ł��Ȃ�������ǂݍ��񂾏ꍇ�́A��O�̕����܂ł�ϊ�
			return n;
		}
		n <<= 4;
		n |= r;
	}
	return n;
}

/**
	@brief	10�i���̕�����𐔒l�ɕϊ�(��O��)
	@param	data	�ϊ����镶����
	@return	�ϊ��������l
	@exception	CSimpleError	������10�i�������ł͂Ȃ�
 */
inline int GetDec( tstring data ) 
{
	int n = 0;
	tstring::iterator	it = data.begin();
	for( ; it != data.end() ; it++ ){
		
		int r = _ATOI(*it);
		if( r == -1 )
		{
			// �ϊ��ł��Ȃ����������������ꍇ�́A���O�̕����܂ł̕ϊ����ʂ�Ԃ�
			return n;
		}
		n *= 10;
		n |= r;
	}

	return n;
}

/**
	@brief	�����񒆂Ɏw��̕��������݂��邩�`�F�b�N
	@param	src		�������镶����
	@param	c		�������镶��
	@return	�����񒆂Ɏw��̕����������src�����񒆂̌��������ꏊ�̃|�C���^��Ԃ��B
			������Ȃ����NULL��Ԃ�
 */
inline _TCHAR* StrSearch( _TCHAR* src, _TCHAR c )
{
	for( ; *src ; src++ )
		if( *src == c )return src;
	return NULL;
}

}	// namespace simple
#endif	// _CText_HPP
