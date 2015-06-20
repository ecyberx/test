// $Id: simplepath.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	�p�X������N���X�w�b�_�t�@�C��
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
	@brief	�p�X�Ǘ��N���X
			�p�X�̈ړ��E�w�肳�ꂽ�ʒu����̑��΃t�@�C���w��Ȃ�
			�t�H���_����(\\or/)�𓝈ꂷ��B
 */
class	CPath {
private:
	tstring		m_tstrPath;					/// �p�X�̕�����
	TCHAR		m_tchDelimiter;				/// �f�B���N�g���̃f���~�^('/' or '\')

	/**
		@brief	�p�X�̖������f���~�^�����Ȃ炻������
		@param	tstrPath	�p�X��
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
		@brief	'\\'��'/'���������A���������L�����N�^��Ԃ��B
				������Ȃ����'\\0'��Ԃ�
		@param	tstrPath	��������p�X������
		@return	���������L�����N�^����
				������Ȃ����'\\0'��Ԃ�
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
		@brief	�p�X�̃f���~�^�L�����N�^���f�t�H���g�ɓ��ꂷ��
				�f�t�H���g�̃f���~�^�L�����N�^���ݒ肳��Ă��Ȃ���Ή������Ȃ�
		@param	tstrPath	�ύX����p�X������
		@retval	tstrPath	�f���~�^�L�����N�^�𓝈ꂵ�����ʂ��i�[
	 */
	void	setDelimiter( tstring& tstrPath )
	{
		// �f�t�H���g�f���~�^�L�����N�^���ݒ肳��Ă���H
		if( m_tchDelimiter == _T('\0') )
		{
			return;
		}

		// �f�t�H���g�f���~�^�ȊO�̃f���~�^�L�����N�^���擾
		TCHAR	tchExceptDelimiter ;
		switch( m_tchDelimiter )
		{
		default:
			RuntimeError("�f���~�^�w�肪�ُ�ł�");
			// break���Ȃ�
		case _T('\\'):
			tchExceptDelimiter = _T('/');
			break;
		case _T('/'):
			tchExceptDelimiter = _T('\\');
			break;
		}

		// �قȂ�f���~�^�L�����N�^���f�t�H���g�L�����N�^�֏�������
		size_t	nPos = tstrPath.find( tchExceptDelimiter,0 );
		for( ; nPos != tstrPath.npos ; nPos = tstrPath.find( tchExceptDelimiter, nPos+1) )
		{
			tstrPath[nPos] = m_tchDelimiter;
		}
	}

public:
	/**
		@brief	�R���X�g���N�^(�����l����)
		@param	tszPath		�i�[����p�X������
	 */
	CPath( const TCHAR* tszPath )
	{
		SetPath( tszPath );
	}
	/**
		@brief	�R���X�g���N�^(�����l�Ȃ�)
	 */
	CPath( void )
		: m_tstrPath(_T("")), m_tchDelimiter( _T('\0') )
	{}
	/**
		@brief	�f�X�g���N�^
	 */
	virtual ~CPath(void)
	{}

	/**
		@brief	�p�X��V���ɐݒ肷��(�����񂩂�̎w��)
		@param	tszPath		�ݒ肷��p�X������
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
		@brief	�p�X��V���ɐݒ肷��(string����̎w��)
		@param	tstrPath		�ݒ肷��p�X������
	 */
	void	SetPath( const tstring& tstrPath )
	{
		SetPath( tstrPath.c_str() );
	}

	/**
		@brief	���݂̃p�X�փp�X��ǉ�����B
				�p�X�����΃p�X�̏ꍇ�͌��݂̃p�X����ǉ�
				��΃p�X�̏ꍇ�͐V�����p�X�֕ύX
		@param	tstrAddPath		�ǉ�����p�X������
	 */
	void	AddPath( tstring tstrAddPath )
	{
		// ��΃p�X�H
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
				// �f���~�^�������ݒ肳��Ă��Ȃ��ꍇ�́A'\'��ݒ肷��
				m_tchDelimiter = _T('\\');
			}
			m_tstrPath += m_tchDelimiter;
		}
		m_tstrPath += tstrAddPath;

		// chomp delimiter
		chompDelimiter( m_tstrPath );
	}
	/**
		@brief	�p�X������̎擾
		@return	�擾�����p�X������
	 */
	const TCHAR* GetPath(void) const
	{
		return m_tstrPath.c_str();
	}

	/**
		@brief		�t�@�C�����̎擾
		@param		tstrDest		�擾�����t�@�C�����̊i�[��
		@return		true:�t�@�C�����̎擾�ɐ����^false:���s
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
		@brief	�ЂƂ�̃f�B���N�g���ֈړ�����
		@return	true:�ЂƂ�ֈړ�����
				false:�ړ��ł��Ȃ�����(�ŏ�ʂ̃t�H���_�ɂ���)
	 */
	bool	UpPath(void)
	{
		size_t		nPos ;

		// �ϊ��O�̕�������ۑ�
		size_t		tPreLength = m_tstrPath.length();

		nPos = m_tstrPath.rfind( m_tchDelimiter, m_tstrPath.length() );
		if( nPos == m_tstrPath.npos )
		{
			// �f���~�^������������Ȃ�
			// �h���C�u��񂪂���H
			nPos = m_tstrPath.rfind( _T(':') );
			if( nPos == m_tstrPath.npos )
			{
				// �h���C�u��񂪂Ȃ��f���~�^���Ȃ��ꍇ("readme.txt")�́A�p�X���N���A
				m_tstrPath = _T("");
				return ( tPreLength != m_tstrPath.length() );
			}
		}

		if( nPos == 0 )
		{
			// ������̐擪�Ƀf���~�^����������ꍇ�́A�f���~�^�����݂̂Ƃ���
			m_tstrPath.resize( 1 );
		} else {
			if( m_tstrPath[nPos-1] == m_tchDelimiter
				|| m_tstrPath[nPos] == _T(':')
				|| m_tstrPath[nPos-1] == _T(':')
				)
			{
				// �f���~�^�����̈ꕶ���O���f���~�^����("//james")
				// �܂��́A�f���~�^�����̈ꕶ����O���h���C�u����("a:\")
				// �܂��́A�f���~�^�������h���C�u����("a:")
				// �̏ꍇ�́A�f���~�^����/�h���C�u�������c��
				m_tstrPath.resize( nPos+1 );
			} else {
				m_tstrPath.resize( nPos );
			}
		}

		return ( tPreLength != m_tstrPath.length() );
	}

	// operators
	/**
		@brief	"="���Z�q�̃I�[�o���[�h
				�p�X������̐ݒ�
		@param	tstrNewPath		�V���ȃp�X������
		@return	�������g�̎Q��
	 */
	const CPath& operator =( const tstring& tstrNewPath )
	{
		SetPath( tstrNewPath );
		return (*this);
	}
	/**
		@brief	"="���Z�q�̃I�[�o���[�h
				�p�X������̐ݒ�(�����񂩂�̎w��)
		@param	tszNewPath		�V���ȃp�X������
		@return	�������g�̎Q��
	 */
	const CPath& operator =(const TCHAR* tszNewPath )
	{
		SetPath( tszNewPath );
		return (*this);
	}
	/**
		@brief	"+="���Z�q�̃I�[�o���[�h
				�p�X��ǉ�����
		@param	tszAddPath	�ǉ�����p�X
		@return	�������g�̎Q��
	 */
	const CPath& operator +=(const TCHAR* tszAddPath )
	{
		AddPath( tszAddPath );
		return (*this);
	}
	/**
		@brief	"+="���Z�q�̃I�[�o���[�h(string�ɂ��w��)
				�p�X��ǉ�����
		@param	tstrAddPath	�ǉ�����p�X
		@return	�������g�̎Q��
	 */
	const CPath& operator +=(const tstring& tstrAddPath )
	{
		AddPath( tstrAddPath );
		return (*this);
	}

	/**
		@brief	�O�u�f�N�������g���Z�q�̃I�[�o���[�h
				�ЂƂ�̃f�B���N�g���ֈړ�
		@return	�������g�̎Q��
	 */
	const CPath& operator --(void)
	{
		UpPath();
		return (*this);
	}
	/**
		@brief	��u�f�N�������g���Z�q�̃I�[�o���[�h
				�ЂƂ�̃f�B���N�g���ֈړ�
		@param	n		�f�N�������g�񐔁H
		@return	�������g�̎Q��
	 */
	const CPath& operator --(int)
	{
		UpPath();
		return (*this);
	}
	// �R�s�[�R���X�g���N�^
	/**
		@brief	�R�s�[�R���X�g���N�^
				�p�X��������R�s�[����
		@param	path	�R�s�[���̃p�X
	 */
	CPath( CPath& path )
	{
		SetPath( path.GetPath() );
		m_tchDelimiter = path.m_tchDelimiter;
	}

	/**
		@brief	�X�g���[���ւ̏o�͏���
				�p�X�𕶎���Ƃ��ďo��
		@param	ost		�o�͐�̃X�g���[���̎Q��
		@param	path	�o�͂���p�X
		@return	�o�͐�̃X�g���[���̎Q��
	 */
	friend std::ostream&   operator << (std::ostream& ost,CPath& path )
	{
		ost << path.GetPath();
		return ost;
	}
};


}			/// namespace simple

#endif // _SIMPLE_PATH_HPP