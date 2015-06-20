/**
	@file	simpleerr.hpp
	@brief	�V���v���N���X���C�u�����Ŏg�p����G���[��O
	@author	Ander/Echigo-ya koubou
	@date	2004.7.17
 */
#ifndef	_SIMPLE_ERR_HPP
#define	_SIMPLE_ERR_HPP

#include <cstdarg>

namespace	simple {	// name space

/**
	@brief	�V���v���N���X���C�u�����Ŏg�p����G���[��O�̊�{�N���X
 */
class	CSimpleError {
protected:
	tstring	msg;						/// �G���[���b�Z�[�W������
public:
	/**
		@brief	�G���[��`(string����̐ݒ�)
		@param	s	�G���[���b�Z�[�W
	 */
	CSimpleError( tstring	s )
	 : msg(s){}

	/**
		@brief	�G���[��`(char*����̐ݒ�)
		@param	s	�G���[���b�Z�[�W
	 */
	CSimpleError( const _TCHAR* s )
	 : msg(s){}

	 /**
		@brief	�G���[���b�Z�[�W�̎擾
		@return	�G���[���b�Z�[�W������
	 */
	virtual const _TCHAR*	getMsg(void){return msg.c_str();}
};
#include <vector>
#include <string>

/**
	@brief		�����^�C���G���[���N���X
 */
class	CRuntimeError 
{
public:
	typedef std::vector< std::string >		string_vector;

protected:
	string_vector		s_vecErrorLog;		/// �G���[���O
public:
	virtual	~CRuntimeError( void )
	{
	}
	CRuntimeError( void )
	{
	}

	/**
		@brief		�G���[�ǉ�
	 */
	void	AddError( const char* szFmt,... )
	{
		char buf[1024];
		va_list args;
		va_start(args, szFmt);

		_vsnprintf( buf, sizeof(buf), szFmt, args);

		s_vecErrorLog.push_back( buf );
	}
	
	/**
		@brief		�G���[�擾
		@note		�Â����ɃG���[�����ЂƂ擾����
		@return		true:�G���[����^false:�G���[�Ȃ�
	 */
	int	GetError( std::string& strDest )
	{
		if( !IsError() )
		{
			return false;
		}

		strDest = *(s_vecErrorLog.begin());
		s_vecErrorLog.erase(s_vecErrorLog.begin() );

		return true;
	}
	/**
		@brief		�G���[�L���̃`�F�b�N
		@return		true:�G���[����^false:�G���[�Ȃ�
	 */
	int	IsError(void)
	{
		return	!s_vecErrorLog.empty();
	}
	/**
		@brief		�S�ẴG���[���擾
		@return		true:�G���[����^false:�G���[�Ȃ�
	 */
	int	GetErrorAll( std::string& strDest )
	{
		if( !IsError() )
		{
			return false;
		}
		std::string str;
		while( CRuntimeError::GetError( str ) )
		{
			strDest += str;
			strDest += "\r\n";
		}

		return true;
	}

};
/**
	@brief		�A�v���P�[�V�����S�̂̃G���[���N���X
	@note		�A�v���P�[�V�����S�̂ł̃G���[����ۑ�����B
	@note		Simple��C3D�Ȃǂł̃G���[����ۑ��^�o�͂��鏈�����A�v���P�[�V�������Ƃ�
				�쐬����������ʂ̃G���[��񏈗����쐬������������葁���̂ō쐬�B
				��O�͕֗������ǂ��̌�̕��A�������s���Ȃ̂�...
 */
class	CRuntimeErrorApp
{
public:
	static	CRuntimeError	s_Error;
};

// �����^�C���G���[�֘A��`
#define	RuntimeErrorClass		simple::CRuntimeErrorApp::s_Error				/// �����^�C���G���[�ۑ��N���X��`
#define	RuntimeError			RuntimeErrorClass.AddError				/// �����^�C���G���[�̒ǉ�
#define	RuntimeErrorGetMsg( X )	RuntimeErrorClass.GetErrorAll( X )		/// �����^�C���G���[�̎擾
#define	RuntimeErrorDefine		simple::CRuntimeError	RuntimeErrorClass		/// �����^�C���G���[�̒�`

}	// namespace

#endif	// _SIMPLE_ERR_HPP

