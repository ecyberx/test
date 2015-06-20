// $Id: simpleplugin.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	�v���O�C������N���X�֘A
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:55 $
 */

#ifndef	_SIMPLE_PLUGIN_HPP
#define	_SIMPLE_PLUGIN_HPP

#include <algorithm>
#include "./simplefilesrch.hpp"

namespace simple {		/// namespace

#ifndef _NUMOF
#define	_NUMOF(x)		(sizeof(x) / sizeof(*(x)))
#endif

/******************************************************************************
	@brief	�f�o�b�K�ւ̏o��
 ******************************************************************************/
inline void	_Trace( LPCSTR strMsg, ... )
{
	char	buf[512];
	va_list	args;
	va_start( args, strMsg );
	_vsnprintf( buf, _NUMOF(buf), strMsg, args );
	buf[_NUMOF(buf)-1] ='\0';

	OutputDebugString( buf );
}
/******************************************************************************
	@brief	�f�o�b�K�ւ̏o��(���Ԃ�ǉ�)
 ******************************************************************************/
inline void	_TraceLog( LPCSTR strMsg, ... )
{
	char	buf[512];
	va_list	args;
	va_start( args, strMsg );
	_vsnprintf( buf, _NUMOF(buf), strMsg, args );
	buf[_NUMOF(buf)-1] ='\0';

	char	buf2[512];
	sprintf( buf2, "%d: %s\n", timeGetTime(), buf );
	OutputDebugString( buf2 );
}

/******************************************************************************
	@brief	�f�o�b�O�p��`
 ******************************************************************************/
#ifdef	_DEBUG
#ifndef _MY_TRACE
#define	_MY_TRACE			simple::_Trace
#endif

#ifndef _MY_TRACE_LOG
#define	_MY_TRACE_LOG		simple::_TraceLog
#endif

#ifndef _MY_ASSERT
#define	_MY_ASSERT(x)		if( (x) ){}else {_TRACE( "[ASSERT]:%s,%l\n", __FILE__, __LINE__ );exit(EXIT_FAILURE);}
#endif

#else

#ifndef	_MY_TRACE
#define	_MY_TRACE			if(1);else
#endif

#ifndef	_MY_TRACE_LOG
#define	_MY_TRACE_LOG		if(1);else
#endif

#ifndef	_MY_ASSERT
#define	_MY_ASSERT			if(1);else
#endif

#endif


/// �֐����G�N�X�|�[�g����ꍇ�̒�`
///  ex)
///		_EXPORT(CFrameObject*)	CreateFrameObject( script_vector& vecScript, void*, long lWaitTime )
#define  _EXPORT(x)	extern "C" _declspec(dllexport) x _cdecl
/// �C���^�t�F�[�X�֐��̒�`
#define  _INTERFACE(x)	extern "C"  x _cdecl

/**
	@brief	�v���O�C������N���X
			�w�肵���t�H���_�̃v���O�C�����������A���ׂēǂݍ���
 */
class	CPlugIn {
public:
	/**
		@brief	�v���O�C���t�@�C���P�̂̏��\����
	 */
	struct plugin_object {
		HINSTANCE		hDLL;				/// �v���O�C��DLL�C���X�^���X
		tstring			tstrName;			/// �v���O�C����(�t�@�C����)
		bool			bInstance;			/// �v���O�C������N���X���v���O�C���̃C���X�^���X�������Ă��邩
	};
	/// ������̃R���e�i��`
	typedef std::vector< tstring >			string_vector;
	/// �v���O�C���t�@�C���̃R���e�i��`
	typedef std::vector< plugin_object >	plugin_vector;
private:
	plugin_vector	m_vecPI;				/// �v���O�C���t�@�C���̃R���e�i
public:
	/**
		@brief	�f�X�g���N�^
				�ǂݍ��񂾂��ׂẴv���O�C���̉��
	 */
	virtual ~CPlugIn(void)
	{
		// DLL�̉��
		plugin_vector::iterator it = m_vecPI.begin();

		for( ; it != m_vecPI.end() ; it++ )
		{
			if( it->bInstance )
			{
				FreeLibrary( it->hDLL );
			}
		}
	}
	/**
		@brief	�f�t�H���g�R���X�g���N�^
	 */
	CPlugIn(void)
	{}
	/**
		@brief	�R���X�g���N�^
				�v���O�C�����������ǂݍ���
		@param	tszPath			��������p�X
		@param	tszExt			��������g���q(*.dll;*.exe�Ȃ�';'�ŋ�؂蕡���w��\)
		@param	bSearchChild	�T�u�f�B���N�g�����������邩�H
	 */
	CPlugIn(const _TCHAR* tszPath, const _TCHAR* tszExt, bool bSearchChild=true )
	{
		SearchPlugin( tszPath, tszExt, bSearchChild );
	}
	/**
		@brief	�v���O�C�����t�H���_���猟�����ǂݍ���
		@param	tszPath			��������p�X
		@param	tszExt			��������g���q(*.dll;*.exe�Ȃ�';'�ŋ�؂蕡���w��\)
		@param	bSearchChild	�T�u�f�B���N�g�����������邩�H
	 */
	void	SearchPlugin(const _TCHAR* tszPath, const _TCHAR* tszExt, bool bSearchChild=true )
	{
		string_vector	vecFilelist;
		CFileSearch( vecFilelist, tszPath, tszExt, bSearchChild );
		
		string_vector::iterator it = vecFilelist.begin();
		for( ; it != vecFilelist.end() ; it++ )
		{
			HINSTANCE	hDLL = LoadLibrary( it->c_str() );
			if( hDLL )
			{
				// DLL�̃��[�h�ɐ���
				AddInstance( hDLL, it->c_str(), true );
			}
		}
	}
	/**
		@brief	�C���X�^���X��ǉ�
		@param	hInstance		�ǉ�����v���O�C���̃C���X�^���X
		@param	tszName			�v���O�C����
		@param	bInstance		�ǉ�����v���O�C���̃C���X�^���X���L��
								�v���O�C������N���X������ɃC���X�^���X��������邩
	 */
	void	AddInstance( HINSTANCE hInstance, const _TCHAR* tszName, bool bInstance )
	{
		plugin_object	obj;
		obj.hDLL = hInstance;
		obj.tstrName = tszName;
		obj.bInstance = bInstance ;
		m_vecPI.push_back( obj );
	}

	/**
		@brief	�p�X���Ŏw�肵��DLL�̃C���X�^���X���擾
		@param	tstrPath	�p�X����DLL��
		@return	�擾�����v���O�C���̃C���X�^���X
				������Ȃ����NULL��Ԃ�
	*/
	HINSTANCE GetInstanceWithPath( const tstring& tstrPath )
	{
		plugin_vector::iterator	it = m_vecPI.begin();
		for( ; it != m_vecPI.end() ; it++ )
		{
			if( it->tstrName == tstrPath )return it->hDLL;
		}
		return NULL;
	}
	/**
		@brief	�w�肵��DLL�̃C���X�^���X���擾
		@param	tstrPath	�p�X����DLL��
		@return	�擾�����v���O�C���̃C���X�^���X
				������Ȃ����NULL��Ԃ�
	 */
	HINSTANCE GetInstance( const tstring& tstrPath )
	{

		if( tstrPath.find_last_of( "\\" ) != tstring::npos )
		{
			// �p�X���v���O�C�����̎w��Ȃ�p�X���������s��
			return GetInstanceWithPath( tstrPath );
		}

		// �p�X����t�@�C�������擾
		size_t nPos;

		plugin_vector::iterator	it = m_vecPI.begin();
		for( ; it != m_vecPI.end() ; it++ )
		{
			nPos = it->tstrName.find_last_of( "\\" );
			if( nPos == tstring::npos )
			{
				// �����v���O�C�����Ƀp�X���Ȃ��ꍇ�͂��̂܂ܔ�r
				if( it->tstrName == tstrPath )return it->hDLL;
			} else {
				// �����v���O�C�����Ƀp�X������ꍇ�̓p�X�������Ĕ�r
				if( it->tstrName.substr(nPos+1,tstring::npos) == tstrPath )
				{
					return it->hDLL;
				}
			}

		}
		return NULL;
	}

	/**
		@brief	�w�肵���v���Z�X�̎擾
				"plugin.dll/Proc"�̂悤��"/"�ŋ�؂��Ďw�肵���v���Z�X���擾����
				��L��ł̓t�@�C��plugin.dll����֐�Proc�̃A�h���X���擾����
		@param	tstrPath	�v���Z�X�̃p�X
		@return	�擾�����v���Z�X�̃A�h���X
		@exception	simple_error	�v���Z�X�̎擾�Ɏ��s
	 */
	FARPROC	GetProcess( tstring tstrPath )
	{
		// �v���Z�X���ƃp�X�̕���
		size_t n = tstrPath.find_last_of( "/" );
		if( n == tstrPath.npos )
		{
			return NULL;
		}
		tstring tstrProcName = tstrPath.substr( n+1, tstrPath.npos );
		tstring _tstrPath = tstrPath.substr( 0, n );
///		tstrPath.resize( n );//, tstrPath.length() );

		// �p�X�̌���
		HINSTANCE hDLL = GetInstance( _tstrPath );
		if( hDLL == NULL )
		{
			return NULL;
		}

		// �v���Z�X�̎擾
		FARPROC  pProc = GetProcAddress( hDLL, tstrProcName.c_str() );
		
		if( pProc == NULL )
		{
			return NULL;
		}
		return pProc;
	}

};
/**
	@brief	�N���X�v���O�C���p�̊�{�N���X��(simple_frame�p)
 */
class	simple_plugin_object {
public:
	/**
		@brief	�f�X�g���N�^
	 */
	virtual ~simple_plugin_object(void){}
	/**
		@brief	�C���X�^���X�̎擾
		@return	�C���X�^���X
	 */
	virtual	HINSTANCE GetInstance(void)=0;
	/**
		@brief	�v���O�C���̍X�V(simple_frame�p)
		@param	nFrame	�t���[���J�E���^
		@return	true:�t���[���I�u�W�F�N�g�̏������s
				false:�t���[���I�u�W�F�N�g�̉���v��
	 */
	virtual bool	Update( int nFrame )=0;
};
/// �N���X�v���O�C���I�u�W�F�N�g���쐬����֐��̌^��`
typedef simple_plugin_object*(*simple_plugin_interface)(void*);

/**
	@brief	�N���X�v���O�C���Ǘ��N���X
			�d�����Ȃ��N���X�v���O�C���I�u�W�F�N�g�̊Ǘ����s��
			addClass�Ńv���Z�X����N���X�v���O�C���I�u�W�F�N�gTYPE���쐬�E�擾�B
			�N���X�v���O�C���I�u�W�F�N�g�̍쐬�v���Z�X�ɂ�interface_proc���g�p����
			�V�[���ɓo�ꂷ��G�L�����̎�ނ��ƂɂЂƂ��쐬����I�u�W�F�N�g�Ȃǂɗ��p�ł���A
	@template	TYPE	�N���X�v���O�C���̃N���X�^�C�v
						ex) CEnemyType
						    CC3DObject
	@template	interface_proc		�N���X�v���O�C���쐬�p�̃C���^�t�F�[�X�֐��^�C�v
									ex)  create_c3dobject_func
										( typedef CC3DObject*(create_c3dobject_func)( const TCHAR* tszID, void* ) )
	@template	t					�N���X�v���O�C���쐬���̃p�����[�^�^�C�v
									ex) void*
										
 */
template< class TYPE, class interface_proc, class t >
class	CClassPlugIn {
private:
	/// ������R���e�i�̒�`
	typedef std::vector< tstring >				string_vector;

	CPlugIn*	m_lpPlugIn;					/// �v���O�C���Ǘ��N���X�̎Q��
//	typedef TYPE*(*interface_proc)(void);

	/**
		@brief	�N���X�v���O�C�����
	 */
	struct type_info 
	{
		unsigned int		uiReference;	/// ���t�@�����X�J�E���^
		TYPE*				pType;			/// �v���O�C���N���X�ւ̃|�C���^
		tstring				tstrProc;		/// �v���Z�X��
		tstring				tstrID;			/// ID

		bool	operator ==( const TYPE* _pType )
		{
			return pType == _pType;
		}
		bool	operator ==( const type_info& info )
		{
			return (tstrProc == info.tstrProc)
					&& (tstrID == info.tstrID);
		}
	};
	/// �N���X�v���O�C�����R���e�i�̒�`
	typedef std::vector< type_info >				type_vector;
	type_vector		m_vecType;				/// �N���X�v���O�C���̃R���e�i


	/**
		@brief	�N���X�v���O�C���I�u�W�F�N�g�̍쐬
		@param	tszProc		�쐬�Ɏg�p����v���Z�X��
		@param	param		�쐬�ɕK�v�ȃp�����[�^
		@return	�쐬�����N���X�v���O�C���I�u�W�F�N�g�ւ̃|�C���^
				�쐬�Ɏ��s�����ꍇ��NULL��Ԃ�
	 */
	TYPE*	createClass(	const _TCHAR* tszProc, 
							const _TCHAR* tszID, 
							t param )
	{
		// create type
		interface_proc*	pFunc = (interface_proc*)m_lpPlugIn->GetProcess( tszProc );

		if( pFunc == NULL )
		{
			return NULL;
		}

		TYPE* pType = pFunc( tszID, param );
		if( pType )
		{
			// �I�u�W�F�N�g�̍쐬�ɐ���
			// �I�u�W�F�N�g�����X�g�ɒǉ�����
			type_info	info;
			info.uiReference = 0;
			info.pType = pType;
			info.tstrProc = tszProc;
			info.tstrID = tszID;
			m_vecType.push_back( info );

			// �I�u�W�F�N�g�̃|�C���^��Ԃ�

			_MY_TRACE( "simple::CClassPlugIn::createClass:%s, %s\n", tszProc, tszID );
			//RuntimeError( "simple::CClassPlugIn::createClass:%s, %s\n", tszProc, tszID );
			return pType;
		}

		// �I�u�W�F�N�g�̍쐬�Ɏ��s�����^�v���Z�X��������Ȃ�����
		return NULL;
	}

public:

	/**
		@brief	�f�X�g���N�^
				�쐬�����N���X�v���O�C�������ׂĉ��
	 */
	virtual ~CClassPlugIn(void)
	{
		Release();
	}

	/**
		@brief	���ׂẴI�u�W�F�N�g�����
	 */
	void	Release( void )
	{
		// �N���X�v���O�C���I�u�W�F�N�g�̉��
		type_vector::iterator	it = m_vecType.begin();
		for( ; it != m_vecType.end() ; it++ )
		{
			_MY_TRACE( "simple::CClassPlugIn::Release:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );
			//RuntimeError( "simple::CClassPlugIn::Release:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );

			delete	it->pType;
			it->pType = NULL;
		}

		m_vecType.clear();

		m_lpPlugIn = NULL;
	}
	/**
		@brief	�R���X�g���N�^
		@param	pi	�Q�Ƃ���v���O�C���Ǘ��N���X
	 */
	CClassPlugIn( void )
		: m_lpPlugIn( NULL )
	{}

	/**
	 */
	int	Create( CPlugIn& pi )
	{
		m_lpPlugIn = &pi;
		m_vecType.clear();
		return true;
	}

	CClassPlugIn (CPlugIn& pi)
		: m_lpPlugIn(&pi)
	{}


	// �N���X�v���O�C���I�u�W�F�N�g�̒ǉ�
	//	tszProc�Ŏw�肵���v���Z�X����N���X�v���O�C���I�u�W�F�N�g���쐬����
	//	�쐬����param�������Ƃ��ăv���Z�X�֓n��
	//	�쐬��A�N���X�v���O�C���I�u�W�F�N�g�̃|�C���^��Ԃ�
	//	���łɓ����̃v���Z�X�ō쐬�ς݂̏ꍇ�́A���̃|�C���^��Ԃ�
	/**
		@brief	�N���X�v���O�C���I�u�W�F�N�g�̒ǉ�
				tszProc�Ŏw�肵���v���Z�X����N���X�v���O�C���I�u�W�F�N�g���쐬����
				�쐬����param�������Ƃ��ăv���Z�X�֓n��
				�쐬��A�N���X�v���O�C���I�u�W�F�N�g�̃|�C���^��Ԃ�
				���łɓ����̃v���Z�X�ō쐬�ς݂̏ꍇ�́A���̃|�C���^��Ԃ�
		@param	tszProc		�擾����N���X�v���O�C���̍쐬�v���Z�X��
		@param	param		�쐬���Ɏg�p����p�����[�^
		@return	�擾�����N���X�v���O�C���I�u�W�F�N�g�̃|�C���^
				���s�����ꍇ��NULL��Ԃ�
	 */
	TYPE*	LoadClass(	const _TCHAR* tszProc, 
						const _TCHAR* tszID, 
						t param )
	{
		// search duplication
		type_vector::iterator	it = m_vecType.begin();
		for( ; it != m_vecType.end() ; it++ )
		{
			// �d������I�u�W�F�N�g������΁A���̃|�C���^��Ԃ�
			if( it->tstrProc == tszProc 
				&& it->tstrID == tszID	)
			{
				++it->uiReference;		// ���t�@�����X�J�E���^���C���N�������g
				return it->pType;
			}
		}

		TYPE* pType = createClass( tszProc, tszID, param );
		return pType;
	}

	/**
		@brief		�I�u�W�F�N�g�̉��
		@return		true:�I�u�W�F�N�g����������^false:�������I�u�W�F�N�g��������Ȃ�
	 */
	int		ReleaseClass(	TYPE* pType )
	{
		// ��v����I�u�W�F�N�g������
		type_vector::iterator	it = std::find( m_vecType.begin(), m_vecType.end(), pType );

		if( it == m_vecType.end() )
		{
			// ������Ȃ�
			return false;
		}


		if( it->uiReference > 0 )
		{
			// �Q�Ƃ��܂��c���Ă���
			--(it->uiReference);
		} else
		{
			_MY_TRACE( "simple::CClassPlugIn::ReleaseClass:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );
			//RuntimeError( "simple::CClassPlugIn::ReleaseClass:%s, %s\n", it->tstrProc.c_str(), it->tstrID.c_str() );

			// �Q�Ƃ��Ȃ��Ȃ����̂ŁA�I�u�W�F�N�g���폜����
			delete it->pType;
			it->pType = NULL;
			m_vecType.erase( it );
		}

		return true;
	}
};


}		/// namespace


#endif	// _SIMPLE_PLUGIN_HPP
