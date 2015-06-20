// $Id: simplescript.hpp,v 1.2 2005/07/07 23:35:55 Ander Exp $
/**
	@brief	�X�N���v�g�w�b�_�t�@�C��
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
	@brief	�X�N���v�g�I�u�W�F�N�g�\����
 */
struct SCRIPTOBJECT_INFO {
	tstring			tstrFNameSrc;		/// ���[�h���܂܂��X�N���v�g�t�@�C��(�w�b�_�t�@�C���ɋL�ڂ���Ă���ꍇ�̓w�b�_�t�@�C�������i�[)
	tstring			tstrFNameBase;		/// �ǂݍ��񂾃X�N���v�g�t�@�C��
	unsigned long	ulLine;				/// �s�ԍ�
	void*			lpParam;			/// �X�N���v�g�ɕt�^�����C�ӂ̃p�����[�^(�A�v���w�ŕt�^)

	SCRIPTOBJECT_INFO(void)
		: lpParam( NULL )
	{}
};


/**
	@brief	�X�N���v�g�I�u�W�F�N�g(�P��P��)
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
		@brief		������̎擾
		@return		true:��������擾�ł���/false:�P��͕�����ł͂Ȃ�����
		@remarks	���������ꍇ�A�P��̑O��ɂ���_�u���N�H�[�e�[�V�����͍폜�����
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
	@brief	�X�N���v�g���Z����
 */
template< class T = long >
class	CScriptCalculator 
{
public:
	/**
		@brief		�G���[���
	 */
	struct error 
	{
		unsigned char	ucCode;			/// �G���[�R�[�h
		tstring			tstrMsg;		/// �G���[���b�Z�[�W

		enum {
			CODE_NONE = 0,				/// �\��
			CODE_VALUE_NOT_MACTCH,		/// �X�N���v�g��l�֕ϊ��ł��Ȃ�
			CODE_OPE_NOT_MATCH,			/// �X�N���v�g�����Z�q�֕ϊ��ł��Ȃ�
			CODE_PARENT_FAIL,			/// ���ʂ̉�͂Ɏ��s
		};

		error( void )
		{}
		error( unsigned char _ucCode, const _TCHAR* tchMsg )
			: ucCode( _ucCode )
			, tstrMsg( tchMsg )
		{}
	};
	typedef std::vector< error >	error_vector;
	/// ���Z�q
	struct	ope_func
	{
		unsigned int	uiLevel;		/// ���Z���x��

		/**
			@brief		�����񂪉��Z�q�Ɉ�v���邩�`�F�b�N
			@return		true:��v�^false:�s��v
			@param		tch		�`�F�b�N���镶����
		 */
		virtual bool	isMatch( const TCHAR* tch ) = 0;

		/**
			@brief		���Z�̎��s
			@return		true:����I���^false:�G���[
			@param		dest	���ʂ̊i�[��
			@param		s1		�\�[�X1
			@param		s2		�\�[�X2
		 */
		virtual bool	calculate( T& dest, const T& s1, const T& s2 ) = 0;
		virtual ~ope_func(void){}
		ope_func(void){}

		// ���Z�q�D�揇��(�K�C�h���C��) 
		enum 
		{
			PRIORITY_NONE = 0,		/// �������Ȃ�
			PRIORITY_CONDITION,		/// ����
			PRIORITY_SHIFT,			/// �V�t�g���Z
			PRIORITY_PLUSMINUS,		/// �����Z
			PRIORITY_MULTIDIV,		/// �揜�^��]�Z
			PRIORITY_BIT,			/// �r�b�g���Z
		};

	};
	typedef	std::vector< ope_func* >		ope_vector;			/// ���Z�q���R���e�i�^�C�v
	typedef int (calc_convert)( T& dest, const TCHAR* tsz );	/// �����񁨒l�ϊ��֐��^�C�v

	/// �ݒ�
	struct config
	{
		ope_vector		vecOperator;		/// �T�|�[�g���鉉�Z�q
		calc_convert*	lpValueConvert;		/// �����񂩂�l�֕ϊ�����֐�
		T				zero;				/// 0�������l
	};
	/// ���Z�v�f(�l/���Z�q)�̊�{�N���X
	struct	calc_element
	{
		enum 
		{
			TYPE_UNKNOWN = 0,		/// ����`
			TYPE_VALUE,				/// �l
			TYPE_OPERATOR,			/// ���Z�q
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
		// �b��I�ɐ����^�̂�
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
		PHASE_VALUE = 0,		/// �l
		PHASE_OPERATOR,			/// ���Z�q
	};

	unsigned char	m_ucPhase;	/// �X�N���v�g��͎��̃t�F�[�Y
	config			m_Config;		/// �ݒ���
	element_vector	m_vecElement;
	unsigned int	m_uiMaxLevel;		/// ���Z���x���̍ō����x����

	/**
		@brief		�v�f�����ׂĉ��
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
	
	T		m_lResult;			/// ���ʂ̊i�[�p


	int		getPhaseValue( script_vector::iterator& it, script_vector& vecScript )
	{
		if( it == vecScript.end() )
		{
			RuntimeError("getPhaseValue::�������s���ł�");
			return false;
		}
		// �P�ꂪ���ʂ��H
		if( it->strWord == "(" )
		{
			// �J�b�R���̉��Z�������
			CScriptCalculator<T>	calc( it->vecChild, m_Config );
			if( calc.GetErrorInfo() != NULL )
			{
				error	err;
				m_vecErrors.push_back( err );
				return false;
			}

			// ���ʂ�l�Ƃ���
			value*	pValue = new value;
			pValue->_v = calc.GetResult();
			m_vecElement.push_back( pValue );

			// ���ʂ̎��̒P�ꂪ")"���H
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
			// �l�̎擾
			value*	pValue = new value;
			if( !(m_Config.lpValueConvert)( pValue->_v, it->strWord.c_str() ) )
			{
				// �l�̎擾�G���[
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
				// ���Z�q�̍쐬
				ope*	lpOpe = new ope;
				lpOpe->lpOpe = *itOpe;
				m_vecElement.push_back( lpOpe );

				// �D�惌�x���̍X�V
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
		@brief	���Z�q�̉��
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
				// �c���l�Ƃ���
				// �l�̎擾
				value*	pValue = new value;
				if( !(m_Config.lpValueConvert)( pValue->_v, &tsz[1] ) )
				{
					// �l�̎擾�G���[
					return false;
				}
				m_vecElement.push_back( pValue );
				return true;
			}
			// ���Z�q�łȂ�(�l���A��)
			return false;
		}

		// �t�F�[�Y�ڍs
		m_ucPhase = PHASE_VALUE;
		return true;
	}

	/**
		@brief	�X�N���v�g����͂��A���Z�q�ƒl�̗v�f���X�g���쐬
		@note	���Z�Ώۂ́A���݈ʒu����","�܂���";"�܂ł̋�ԁB
	 */
	int		createElement( script_vector& vecScript, script_vector::iterator& it )
	{
		m_ucPhase = PHASE_VALUE;
		m_uiMaxLevel = 0;

		for( ; it != vecScript.end() && it->strWord != ";" && it->strWord != ","
			 ; it++ )
		{
			// �t�F�[�Y���`�F�b�N
			switch( m_ucPhase )
			{
			case PHASE_VALUE:
				if( !getPhaseValue( it, vecScript ) )
				{
					// �G���[
					return false;
				}
				// �t�F�[�Y�ڍs
				m_ucPhase = PHASE_OPERATOR;
				break;
			case PHASE_OPERATOR:
				if( !getPhaseOperator( it->strWord.c_str() ) )
				{
					// �G���[
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
		@brief		���Z�v�f���̕\��
		@note		�f�o�b�O�p
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
		@brief		���Z���̕\��
		@note		�f�o�b�O�p
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
		@brief	���Z�̎��s
	 */
	int		execCalculate( void )
	{
		m_lResult = m_Config.zero;

		for( unsigned int uiLevel = m_uiMaxLevel ; uiLevel > 0 || m_vecElement.size() > 1; uiLevel-- )
		{
			// ���x�����Ƃ̉��Z(�擪���珇�ɉ��Z����)
			element_vector::iterator	itValue1 = NULL;		// �l1�̎Q�Ƃ�ۑ�
			element_vector::iterator	itOpe = NULL;			// �Ō�Ɏ擾�������Z�q����ۑ�

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
						// ���Z���x���`�F�b�N(�D��x�̒Ⴂ���Z�q�͂܂����Z���Ȃ�)
						if( lpOpe->uiLevel == uiLevel )
						{
							value* lpValue1 = (value*)*itValue1;

							// ���Z���s
							T tResult;
							if( !lpOpe->calculate(		tResult, 
														lpValue1->_v, 
														((value*)(*itElement))->_v ) )
							{
								// ���Z�Ɏ��s
								return false;
							}

							lpValue1->_v = tResult;

							// ���Z�q�`�l2�܂ł̊Ԃ��폜
							m_vecElement.erase( itOpe, itElement+1 );

							//  �I�y���[�^�ɏ]���ĉ��Z�����s���A���Z���ʂ���ڂ̒l�̏ꏊ�֕ۑ��E���Z�q�Ɠ�ڂ̒l���폜����
							itElement = itValue1;
						}

						// ���Z�q�����N���A
						itOpe = NULL;
					}

					// �l1�����݂̈ʒu�ɐݒ�
					itValue1 = itElement;
					break;
				case calc_element::TYPE_OPERATOR:
					// ���Z�q�̏���
					itOpe = itElement;
					break;
				default:
					return false;
				}
			}
		}

		// ���Z���ʂ̃`�F�b�N
		if( m_vecElement.size() != 1 )
		{
			// �Ō�Ɏc�������Z���ʂ��ЂƂł͂Ȃ�
			return false;
		}
		if( m_vecElement[0]->getType() != calc_element::TYPE_VALUE )
		{
			// �Ō�Ɏc�����̂��l�ł͂Ȃ�
			return false;
		}
		value*	lpVal = (value*)m_vecElement[0];
		m_lResult = lpVal->_v;

		return true;
	}

	void	execCalculate(	script_vector::iterator it,
							script_vector& vecScript )
	{
		// �I�u�W�F�N�g�����Z�q�ƒl�ɕ�����
		if( !createElement( vecScript,it ) )
		{
			// �X�N���v�g���ُ�
			error err;
			m_vecErrors.push_back( err );
			return;
		}

		// ���Z�����s
		if( !execCalculate() )
		{
			// ���Z�ł��Ȃ�
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
		@brief		���Z���ʂ̎擾
	 */
	T	GetResult( void )
	{
		return m_lResult;
	}
	/**
		@brief		�G���[���̎擾
	 */
	error_vector*	GetErrorInfo( void )
	{
		if( m_vecErrors.empty() )return NULL;
		return &m_vecErrors;
	}
};

/**
	@brief		�����񂩂琮���^���l�ւ̕ϊ�
 */
inline int	CalcConvertLong( long& dest, const TCHAR* tsz )
{
	// HEX�`�F�b�N
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
					// �ϊ��G���[
					return false;
				}
				dest <<= 4;
				dest |= n;
			}
			
			return true;
		}
	}
	// �����`�F�b�N
	if( tsz[0] == '\'' )
	{
		tsz++;
		if( *tsz == NULL )
		{
			// �ϊ��G���[
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
	@brief		�����񂩂畂�������_���l�ւ̕ϊ�
 */
inline int	CalcConvertFloat( float& dest, const TCHAR* tsz )
{
	dest = (float)atof( tsz );
	return true;
}

/**
	@brief	���Z����
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
	@brief	�����^�̌��Z����
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
	@brief	�����^�̏�Z����
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
	@brief	��Z����
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
	@brief	��]�Z����
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
	@brief	��r����
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
	@brief		�������Z�Z�b�g
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
	@brief		���������_���Z�Z�b�g
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


// ���Z�R���t�B�O���[�V�����Z�b�g
static const calc_long_set	calc_cfg_int;		/// �������Z�Z�b�g
static const calc_float_set	calc_cfg_float;		/// ���������_���Z�Z�b�g


////////////////////////////////////////////////////////////////////////////////
// CScriptParser
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	�X�N���v�g��̓N���X�̊�{�N���X
	@note	���C���P����P��P�ʂł��̃N���X��parse���\�b�h���Ăяo���B
 */
class	CScriptParser {
public:
	/**
		@brief	������̉�͉��z�֐�
		@note	���̊֐����p�����Ă��ꂼ��̉�̓N���X���쐬����
		@param	strWord		�P��̕�����
		@param	scrInfo		�P��̏��\����
	 */
	virtual bool parse( tstring& strWord, SCRIPTOBJECT_INFO& scrInfo ) = 0;
};
////////////////////////////////////////////////////////////////////////////////
// ScriptError
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	�X�N���v�g�G���[
	@note	�X�N���v�g����Ăяo�����A�G���[�̔�������e�����֓n������
 */
class	CScriptError
{
public:
	/**
		@brief	�X�N���v�g�G���[���\����
	 */
	struct error_object 
	{
		tstring			tstrFilename;	/// �t�@�C����
		long			lLine;			/// �G���[�����̍s�ԍ�
		unsigned short	wErrorCode;		/// �G���[�R�[�h
		tstring			tstrErrorMsg;	///	�G���[���b�Z�[�W

		/// �G���[�R�[�h
		enum {
			ERRORCODE_NONE = 0,
			ERRORCODE_FILE_CANT_OPEN,					/// �t�@�C���̃I�[�v���Ɏ��s
			ERRORCODE_CANT_FOUND_CLOSE,					/// �q�K�w�������Ă��Ȃ�
			ERRORCODE_ILLEGAL_CLOSE,					/// �s���Ȏq�K�w�N���[�Y(�J�n���������Ă��Ȃ��̂ɕ���ꂽ)
			ERRORCODE_PARSE_FAIL,						/// Parse�Ɏ��s
			ERRORCODE_NOT_PARE,							/// �q�K�w�̊J�n�Ɗ������΂łȂ�
			ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT,	/// �p�����[�^���}�N���W�J���Ƀp�����[�^�w�肪�Ȃ�
			ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM,		/// �p�����[�^���}�N���W�J���Ƀp�����[�^�w�肪�ُ�
			ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM_NUM,	/// �p�����[�^���}�N���W�J���̃p�����[�^�����ُ�
			ERRORCODE_MACRO_EXTRACT_FAIL,				/// �}�N���W�J���ɃG���[
			ERRORCODE_MACRO_DEFINE_FAIL,				/// �}�N����`���ɃG���[
			ERRORCODE_INCLUDE_DEPTH_OVER,				/// �C���N���[�h�[�x���I�[�o�[
			ERRORCODE_INCLUDE_ILLEGAL_FILE,				/// �C���N���[�h�̃t�@�C���w�肪�ُ�
			ERRORCODE_INCLUDE_FILE_CANT_OPEN,			/// �C���N���[�h�t�@�C���̃I�[�v���Ɏ��s
			ERRORCODE_INCLUDE_FAIL,						/// �C���N���[�h�t�@�C���̉�͂ŃG���[
			ERRORCODE_DEFINE_CANT_FOUND_KEY,			/// �}�N����`�Ń}�N���L�[��������Ȃ�
			ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,		/// �}�N����`�Ńp�����[�^�w�肪�ُ�
			ERRORCODE_DEFINE_FAIL,						/// �}�N����`���߂ŃG���[
			ERRORCODE_IF_CANT_FOUND_ENDIF,				/// �������߂Ŏw�肵�������R�[�h(#ENDIF/#ELSE/#ELIF)��������Ȃ��܂�EOF�܂ł��Ă��܂���
			ERRORCODE_IFDEF_CANT_FOUND_CONDITION,		/// IFDEF���ŏ�������������Ȃ�
			ERRORCODE_IF_FAIL,							/// �������߂ŃG���[
			ERRORCODE_ILLEGAL_ENDIF,					/// �s���ȏ������ߏI�[
			ERRORCODE_COMMAND_ILLEGAL,					/// �s���ȃv���v���Z�b�T�R�}���h
			ERRORCODE_COMMAND_FAIL,						/// �v���v���Z�b�T�R�}���h�G���[

		};

		/**
			@brief	�������Ȃ��R���X�g���N�^
		 */
		error_object( void )
		{}

		/**
			@brief	�p�����[�^�ݒ�
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
	error_vector	m_vecErrors;		/// �G���[���
public:
	/**
		@brief	�������Ȃ��R���X�g���N�^
	 */
	CScriptError( void )
	{}
	/**
		@brief	�������Ȃ��f�X�g���N�^
	 */
	virtual ~CScriptError( void )
	{}
	/**
		@brief	���������G���[�����擾����
	 */
	const error_vector&	GetErrors( void )
	{
		return m_vecErrors;
	}
	/**
		@brief	�G���[�������������m�F
		@return	true:�G���[����/false:�G���[�Ȃ�
	 */
	bool	FoundError( void )
	{
		return !m_vecErrors.empty();
	}
	/**
		@brief	�G���[���̒ǉ�
	 */
	const CScriptError& operator += ( const CScriptError& s )
	{
		m_vecErrors.insert( m_vecErrors.begin(), s.m_vecErrors.begin(), s.m_vecErrors.end()  );
		return *this;
	}
	/**
		@brief	�G���[�̒ǉ�
	 */
	void	AddError( const error_object& errorObject )
	{
		m_vecErrors.push_back( errorObject );
	}

	/**
		@brief	�G���[���̏o��
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
	@brief		�X�N���v�g�̃}�N���N���X
	@note		�}�N���̃L�[�A�p�����[�^�A�W�J���̐ݒ�E�W�J���s���B
 */
class	CMacroObject {
public:
	/// ������̃R���e�i�^�C�v��`
	typedef	std::vector< tstring >			string_vector;
	/// ������R���e�i�̃R���e�i�^�C�v��`
	typedef std::vector< string_vector >	string_string_vector;

	/**
		@brief	�X�N���v�g���ʂ𕶎���̃R���e�i�֒ǉ�����N���X
	 */
	class	param_parser : public CScriptParser {
	protected:
		string_vector&		m_vecParam;			/// ���ʂ̊i�[��R���e�i�̎Q��
	private:
		param_parser(const param_parser&);
		param_parser& operator=(const param_parser&);

	public:
		/**
			@brief	�������Ȃ��f�X�g���N�^
		 */
		virtual ~param_parser(void)
		{}
		/**
			@brief	���ʊi�[��R���e�i�̎Q�Ɛݒ�
			@param	vecParam	���ʊi�[��R���e�i
		 */
		param_parser( string_vector& vecParam )
			: m_vecParam( vecParam )
		{}
		/**
			@brief	�P��̉�́E�R���e�i�ւ̊i�[
			@param	strWord		�ǉ����镶����
		 */
		virtual bool	parse( tstring& strWord, SCRIPTOBJECT_INFO& )
		{
			m_vecParam.push_back( strWord );
			// ��ɐ�����Ԃ�
			return true;
		}
	};

protected:
	tstring				m_strKey;			/// �}�N���L�[
	string_vector		m_vecParam;			/// �}�N���p�����[�^
	bool				m_bParamType;		/// �֐��^�}�N�����ǂ���(�֐��^:TRUE/��֐��^:FALSE)
	string_vector		m_vecContents;		/// �}�N���̕ϊ���f�[�^


	// ���\�b�hgetExtractParamObject�̖߂�l��`
	enum {
		RET_CONTINUE=0,	// ����擾(��������)
		RET_TERMINAL,	// ����擾(����ŏI���)
		RET_ERR,		// �G���[
	};
	/**
		@brief		�W�J����}�N���p�����[�^�̃J���}��؂�Ŏ擾
		@return		RET_CONTINUE:����擾(���̃p�����[�^������)
					RET_TERMINAL:����擾(')'�����o)
					RET_ERR:�G���[
		@param		vecStr		�擾���ʂ̕ۑ���
		@param		line		�s�f�[�^
		@param		Layer1		Script���C���P
	 */
	template< class LAYER1 >
	int	getExtractParamObject(	string_vector&	vecStr,
								CTextLineEx& line,
								LAYER1& Layer1 )
	{
		// ���ʂ̊J�n/�I������
		//  �� �J�n�����ƏI�������͑Ή����ċL�q���邱��
		std::string	strBegin = "({[";	// ���ʊJ�n����
		std::string	strEnd =   ")}]";	// ���ʏI������

		tstring strParam;				// �擾�P��̈ꎞ�i�[�p
		tstringstream sst;				// �W�J����}�N���p�����[�^�̈ꎞ�ۑ��p
		bool bEmpty = true;				// �}�N���p�����[�^����?(��̃}�N���p�����[�^�`�F�b�N�p)
		std::stack<size_t>	stkPare;	// ���ʂ̑Ή��`�F�b�N

		// �擪1�P��̎擾
		if( line.GetWordEx(strParam) == false )
		{
			// �}�N���p�����[�^�̓r����EOF
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
								_T("macro parameter illegal") );
			return RET_ERR;
		}
		for( ;; )
		{
			if( strParam.find('\"') == strParam.npos )
			{
				// �_�u���N�H�[�e�[�V�����ŋ�؂�ꂽ������ȊO�Ȃ犇�ʂ̃`�F�b�N���s��

				// ���ʊJ�n�̃`�F�b�N
				size_t	tPos = strBegin.find_first_of(strParam);
				if( tPos != strBegin.npos )
				{
					// ���ʊJ�n����������
					stkPare.push( tPos );
				} else
				{
					// ���ʕ��̃`�F�b�N
					size_t	tPosEnd = strEnd.find_first_of(strParam);
					if( tPosEnd != strEnd.npos )
					{
						// ���ʕ�����������
						if( stkPare.size() == 0 )
						{
							// �p�����[�^���Ɋ��ʊJ�n���Ȃ���Ԃł̊��ʕ��̌��o
							if( tPosEnd != 0 )
							{
								// ')'�ȊO�ŏI�����悤�Ƃ���?
								Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
													_T("macro parameter illegal(illegal parent end charactor)") );
								return RET_ERR;
							}
							// �}�N���p�����[�^�̏I��
							break;
						}

						// ���ʊJ�n�Ƃ̑Ή��`�F�b�N
						size_t tLastBegin = stkPare.top();
						if( tPosEnd != tLastBegin )
						{
							// ���ʂ̑Ή����Ⴄ
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
				// �p�����[�^�̋�؂�𔭌�
				break;
			}

			// �p�����[�^��ݒ�
			sst << strParam << _T(" ");
			bEmpty = false;
			
			if( line.GetWordEx(strParam ) == false )
			{
				// �}�N���p�����[�^�̓r����EOF
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
									_T("macro parameter illegal(EOF)") );
				return RET_ERR;
			}
		}
		if( bEmpty )
		{
			// ��̃}�N���p�����[�^
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
								_T("macro parameter illegal(empty param)") );
			return RET_ERR;
		}
		
		// �p�����[�^���
		param_parser	parser( vecStr );
		// �p�����[�^���̃v���v���Z�b�T�W�J
		LAYER1( sst, parser, Layer1.GetMacro() );
		
		if( strParam == _T(")") )
		{
			return RET_TERMINAL;
		}
		return RET_CONTINUE;
	}
									

	/**
		@brief	�֐��^�}�N���̃p�����[�^�擾
		@param	vecParam	�֐��^�}�N���̃p�����[�^�̊i�[��
		@param	line		���͂���X�g���[��
		@param	Layer1		�X�N���v�g�̉�̓N���X
		@retval	vecParam	�擾�����֐��}�N���̓W�J�p�p�����[�^
		@note	�p�����[�^�̉�͂̂��߂ɃX�N���v�g���C���P���g�p���Ă���
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	template< class LAYER1 >
	bool	getExtractParam(	string_string_vector& vecParam, 
								CTextLineEx& line, 
								LAYER1& Layer1 )
	{
		tstring strParam ;
		
		// ����'('���擾
		if( line.GetWordEx(strParam) == false )
		{
			// EOF
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT, 
								_T("function type macro, but '(' is not found(EOF)") );
			return false;
		}
		if( strParam != _T("(") )
		{
			// �֐��^�}�N���Ȃ̂Ɏ��̕�����'('�ł͂Ȃ�
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT, 
								_T("function type macro, but '(' is not found") );
			return false;
		}
		
		// �p�����[�^�̎擾
		int nRet;
		do {
			// �p�����[�^���J���}��؂�Ŏ擾
			string_vector	vecStr;
			nRet = getExtractParamObject<LAYER1>( vecStr, line, Layer1 );
			if( nRet == RET_ERR )
			{
				// �p�����[�^�̎擾�Ɏ��s
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_PARAM_NOT_FOUNT, 
									_T("illegal macro parameter") );
				return false;
			}

			// �p�����[�^���i�[
			vecParam.push_back( vecStr );

		} while(nRet != RET_TERMINAL );

		// �p�����[�^�T�C�Y�`�F�b�N
		if( m_vecParam.size() != vecParam.size() )
		{
			// �}�N���p�����[�^�̌�����v���Ȃ�
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM, 
								_T("macro parameter illegal(Param count err)") );
			return false;
		}

		// ����I��
		return true;
	}

public:
	/**
		@brief	�������Ȃ��f�X�g���N�^
	 */
	virtual	~CMacroObject(void)
	{}
	/**
		@brief	�������Ȃ��R���X�g���N�^
	 */
	CMacroObject(void)
		: m_bParamType(false)
	{}

	/**
		@brief	�}�N���L�[�̐ݒ�
		@param	strKey	�}�N���L�[������
	 */
	void	SetKey( tstring& strKey )
	{
		// �L�[�̐ݒ�
		m_strKey = strKey;

	}
	/**
		@brief	�֐��^�}�N���̃p�����[�^�̒ǉ�
		@param	strParam	�ǉ�����p�����[�^������
		@note	�}�N����`�ł̃p�����[�^�Ƃ��Ēu���������镶������w�肷��B
	 */
	void	AddParam( tstring& strParam )
	{
		m_bParamType = true;				// �}�N���^�C�v���֐��^�}�N����ݒ�

		m_vecParam.push_back( strParam );	// �p�����[�^�ǉ�
	}

	/**
		@brief	�}�N���̓W�J���e�̒�`
		@param	Layer1	�X�N���v�g���C���P
		@param	ist		���̓X�g���[��
		@note	�}�N���̓W�J�������e���X�g���[������擾����B
		@note	�擾�����p�����[�^�̓X�N���v�g���C���P�ɏ]���ĉ�͂��ꂽ�P��P�ʂŊi�[�����
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	template< class LAYER1 >
	bool	setContents( LAYER1& Layer1, tistream& ist )
	{
		param_parser	parser( m_vecContents );
		LAYER1	_Layer1( ist, parser, Layer1.GetMacro() );

		CScriptError* lpError =  _Layer1.GetErrorInfo();
		if( lpError != NULL )
		{
			// �}�N���쐬����EOF
			Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_DEFINE_FAIL, 
								_T("macro define failed.") );
			return false;
		}


		return true;
	}

	/**
		@brief	�P�ꂪ�}�N���L�[���`�F�b�N
		@param	strWord		�`�F�b�N���镶����
		@return	true:������͂��̃}�N���̃L�[
				�^false:������͂��̃}�N���̃L�[�ł͂Ȃ�
	 */
	bool	IsMacroKey( tstring& strWord )
	{
		return strWord == m_strKey;
	}
	
	/**
		@brief	�}�N����W�J
		@param	Parser	�W�J�f�[�^�̏o�͐�
		@param	Layer1	�X�N���v�g���C���P
		@param	line	���͂���p�����[�^�̃\�[�X
		@exception	CSimpleError	�}�N���G���[�ɂ���O
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	template< class PARSER, class LAYER1 >
	bool	extractMacro(	PARSER& Parser,
							LAYER1& Layer1,
							CTextLineEx& line ) 
	{
		string_string_vector	vecParam;
		if( m_bParamType )
		{
			// �֐��}�N���̃p�����[�^�擾
			if( !getExtractParam( vecParam, line, Layer1 ) )
			{
				// �p�����[�^�擾�Ɏ��s
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
									_T("Macro Extract Error") );
				return false;
			}

			if( vecParam.size() != m_vecParam.size() )
			{
				// �}�N���̃p�����[�^���ُ�
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_ILLEGAL_PARAM_NUM,
									_T("macro parameter size is wrong") );
				return false;
			}
		}
		// �P��I�u�W�F�N�g�̏ڍׂ��擾
		SCRIPTOBJECT_INFO	scrInfo;
		scrInfo.tstrFNameSrc = Layer1.GetFilename();
		scrInfo.ulLine = Layer1.GetLineNumber();

		string_vector::iterator it = m_vecContents.begin();
		for( ; it != m_vecContents.end() ; it++ )
		{
			if( m_bParamType )
			{
				// �}�N���p�����[�^�̂����ꂩ�ɊY�����邩�H
				string_vector::iterator	itCnt = m_vecParam.begin();
				string_string_vector::iterator itParam = vecParam.begin();

				for( ; itCnt != m_vecParam.end() && *it != *itCnt ; itCnt++,itParam++ )
					;

				if( itCnt != m_vecParam.end() )
				{
					// �p�����[�^�W�J
					string_vector::iterator itExt = itParam->begin();
					for( ; itExt != itParam->end() ; itExt++ )
					{
						if( !Parser.parse( *itExt, scrInfo ) )
						{
							// �p�����[�^�W�J���ɃG���[
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
				// �p�����[�^�W�J���ɃG���[
				Layer1.AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
									_T("Macro Extract Error") );
				return false;
			}
		}

		// ����I��
		return true;
	}

	// 
	/**
		@brief	�}�N���̓o�^(�ȈՓo�^��)
		@param	szKey	�}�N���L�[
		@param	szValue	�}�N���̓W�J���e
		@note	szValue�ɂ͕����̒P����w��\
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
		@brief	�}�N�����̏o��
		@note	�f�o�b�O�p
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
	@brief	�X�N���v�g��̓��C���P(�v���v���Z�b�T�{�P�ꕪ��)
	@note	�e��v���v���Z�b�T�R�}���h�̓W�J�A�P��P�ʂ̕������s��
 */
class	CScriptLayer1 {
public:
	/// �}�N���R���e�i�^�C�v
	typedef std::vector< CMacroObject >		macro_vector;
	/// �e�L�X�g�o�b�t�@��auto_ptr�^�C�v
	typedef std::auto_ptr< CTextLineEx >	ptr_simple_text_line_ex;

	/**
		@brief	���ݓǂݍ��ݒ��̃t�@�C�����̎擾
		@return	�t�@�C����������̃|�C���^
	 */
	const TCHAR*	GetFilename(void)
	{
		return m_tszFName;
	}
	/**
		@brief	���݂̍s�ԍ��̎擾
		@return	���݂̍s�ԍ�
	 */
	unsigned long	GetLineNumber(void)
	{
		return m_pLine->GetLineNumber();
	}
private:
	CScriptLayer1(const CScriptLayer1&);
	CScriptLayer1& operator=(const CScriptLayer1&);

protected:
	CPath						m_Path;						/// �t�@�C���p�X
	CScriptParser&				m_Parser;					/// ��ʃ��C��
	int							m_nDepth;					/// �C���N���[�h�̐[�x
	int							m_nMaxDepth;				/// �C���N���[�h�̍ő�[�x
	CTextLineEx*				m_pLine;					/// �e�L�X�g��̓N���X
	macro_vector&				m_vecMacro;					/// �}�N���R���e�i
	tistream&					m_ist;						/// ���̓X�g���[��
	const TCHAR*				m_tszFName;					/// ���̓t�@�C����

	unsigned int				m_nEndStatus;				/// �t�@�C���������̏��

	CScriptError				m_ScriptError;				/// �X�N���v�g�G���[���
	
	/// �t�@�C���������̏��
	enum _END_STATUS {
		/// EOF(�t�@�C���̍Ō�܂Ŋ���)
		_RET_EOF = 0,
		/// #endif�ł̕��A
		_RET_ENDIF,				
		/// #else�ł̕��A
		_RET_ELSE,				
		/// #elif�ł̕��A
		_RET_ELIF,
		/// �G���[�Œ��f
		_RET_FAIL,
	};

	/// �t�@�C���[�x�̐ݒ�
	enum {
		_MAX_DEPTH = 8										// �f�t�H���g�̍ő�[�x
	};

	/**
		@brief	�Ō�̓ǂݍ��݌�̏�Ԏ擾
		@return �t�@�C���ǂݍ��݊������̏��(_END_STATUS�Q��)
	 */
	unsigned int	getEndStatus(void)
	{
		return m_nEndStatus;
	}

	/**
		@brief	#include�̉��
		@exception	CSimpleError	�C���N���[�h�Ɋւ����O
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	bool	cmdInclude(void) 
	{
		// �C���N���[�h�̐[�x�`�F�b�N
		if( m_nDepth >= m_nMaxDepth )
		{
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_DEPTH_OVER,
						_T("include depth over") );
			return false;
		}

		tstring strFName;
		if( m_pLine->GetWord(strFName) == false )
		{
			// �t�@�C�����̎w�肪��������(EOF)
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_ILLEGAL_FILE,
						_T("include file name is not descripted") );
			return false;
		}

		if( !CTextLineEx::IsStringWord( strFName ) )
		{
			// �t�@�C�����̎w�肪��������
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_ILLEGAL_FILE,
						_T("include file name is illegal") );
			return false;
		}

		// File open
		// �t�@�C�������쐬(path + fname)
		CPath	path( m_Path );

		path += strFName;
		tstring tstrFNameInclude = path.GetPath();
#ifdef _UNICODE
		// �t�@�C�����擾�̂��߁AWCHAR�������CHAR������֕ϊ�
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
			// �t�@�C���̃I�[�v���Ɏ��s
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_FILE_CANT_OPEN,
						tstrFNameInclude + _T(" couldn't open") );
			return false;
		}
		// �C���N���[�h�t�@�C���̓ǂݍ���
		CScriptLayer1	Layer1( ifst, m_Parser,
								m_vecMacro,
								tstrFNameInclude.c_str(),
								m_nDepth,
								m_nMaxDepth );
		// �t�@�C�������
		ifst.close();

		// �C���N���[�h�t�@�C���̌��ʂ��`�F�b�N
		if( Layer1.GetErrorInfo() != NULL )
		{
			AddErrors( Layer1.GetErrorInfo() );
			// �C���N���[�h�t�@�C���ŃG���[����
			AddError(	CScriptError::error_object::ERRORCODE_PARSE_FAIL,
						tstrFNameInclude + _T(" parse fail.") );
			return false;
		}

		// ����I��
		return true;
	}

	bool	macroGetKey(CMacroObject& macroObject, CTextLineEx& line )
	{
		// �L�[�̎擾
		tstring	strKey;
		if( line.GetWordEx(strKey) == false )
		{
			// �L�[���݂���Ȃ�(EOF)
			AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_FOUND_KEY,
						_T("illegal #define(EOF)") );
			return false;
		}

		macroObject.SetKey( strKey );

		// �֐��}�N��?
		if( line.GetPosCharactor() == '(' )
		{
			// �֐��^�}�N��
			// �������擾
			tstring		strParam ;
			// '('���X�L�b�v
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
				// �J���}�܂���')'���擾�E�X�L�b�v
				if( line.GetWordEx( strParam ) == false )
				{
					// �s�[���ُ�
					AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
								_T("illegal macro parameter(0)") );
					return false;
				}
				if( strParam == _T(")") )break;
				if( strParam != _T(",") )
				{
					// �}�N���p�����[�^�̊ԂɃJ���}���Ȃ�
					AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
								_T("illegal macro parameter(not found ',')") );
					return false;
				}
			}
			if(  strParam != _T(")") )
			{
				// ')'���݂���Ȃ��܂܍s�[�ɓ��B
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("')' is not found") );
				return false;
			}
		}

		return true;
	}

	/**
		@brief	�}�N����`��(#define)�̉��
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	bool	cmdDefine(void) 
	{
		CMacroObject		macroObject;
		tstring	tstrLine;
		if( m_pLine->GetLine(tstrLine) == false )
		{
			// Key���Ȃ�(#define[EOF])
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

		// �}�N���̒��g���擾
		tstring		strLine;
		if( line.GetLine(strLine) )
		{
			// �p�����[�^�ȍ~�ɕ����񂪂���΁A�}�N�����e�Ƃ��Ď擾
			tstringstream	sst( strLine.c_str() );
			if( !macroObject.setContents( *this, sst ) )
			{
				// �p�����[�^�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("macro parameter error") );
				return false;
			}
		}

		m_vecMacro.push_back( macroObject );

		// ����I��
		return true;
	}
	/**
		@brief	�}�N����`��(#define)�̉��
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	bool	cmdDefineBlock(void) 
	{
		CMacroObject		macroObject;
		tstring	tstrLine;
		if( m_pLine->GetLine(tstrLine) == false )
		{
			// Key���Ȃ�(#define[EOF])
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

		// �}�N���̒��g���擾
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
				// �p�����[�^�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("macro parameter error") );
				return false;
			}

			// �p�����[�^�ȍ~�ɕ����񂪂���΁A�}�N�����e�Ƃ��Ď擾
			if( !macroObject.setContents( *this, sst ) )
			{
				// �p�����[�^�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_CANT_ILLEGAL_PARAM,
							_T("macro parameter error") );
				return false;
			}
		}

		m_vecMacro.push_back( macroObject );

		// ����I��
		return true;
	}

	/**
		@brief	�ǂݔ�΂������̃X�N���v�g��̓N���X
	 */
	class	skip_parser : public CScriptParser {
	public:
		/**
			@brief	�������Ȃ��f�X�g���N�^
		 */
		virtual ~skip_parser(void)
		{}
		/**
			@brief	��̉�͊֐�(�P��̓ǂݔ�΂�)
			@param	strWord	�擾�����P��
		 */
		bool parse(tstring& , SCRIPTOBJECT_INFO& )
		{
			// ��ɐ�����Ԃ�
			return true;
		}
	};

	/**
		@brief	�w�肵�������R�[�h�܂œǂݔ�΂�
		@param	uiUntil	�����R�[�h(_END_STATUS�Q��)
		@exception	CSimpleError	�w�肵�������R�[�h�ȊO�̊����R�[�h����������
		@note	#if�ȂǂɑΉ�����endif/elif/else�Ȃǂ̊����R�[�h�܂ł�ǂݔ�΂��B
				�w�肵���ȊO�̊����R�[�h�̏ꍇ�͗�O�𓊂���B
		@return	true:�G���[����^false:�G���[�Ȃ�
	 */
	bool	skipUntil( const unsigned int uiUntil ) 
	{
		// ENDIF�܂œǂݔ�΂�
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
					// �w�肵�������R�[�h(#ENDIF/#ELSE/#ELIF)��������Ȃ��܂�EOF�܂ł��Ă��܂���
					AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
								_T("illegal endif/else/elif") );
					return false;
				}
				break;
			}
			// �X�L�b�v���̃X�N���v�g�̃G���[�͖�������

		}
		return true;
	}

	/**
		@brief	IF/IFDEF�ɂ�true�̏ꍇ�̏���
		@exception	CSimpleError	#if�ɑΉ�����#endif��������Ȃ������B
		@return	true:�G���[����^false:�G���[�Ȃ�
	 */
	bool	getTrue(void) 
	{
		// ENDIForELSE�܂Ŏ擾
		CScriptLayer1	l1( m_ist, m_Parser, m_vecMacro,
			m_Path.GetPath(), m_nDepth,
			m_nMaxDepth );
		// �G���[�̃`�F�b�N
		if( l1.GetErrorInfo() != NULL )
		{
			// �G���[�𔭌�
			AddErrors( l1.GetErrorInfo() );
			return false;
		}


		switch( l1.getEndStatus() )
		{
		case _RET_EOF:
			// �w�肵���I���R�[�h��������Ȃ�����
			AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
						_T("illegal endif/else/elif") );
			return false;

		case _RET_ENDIF:
			break;
		case _RET_ELSE:
			if( !skipUntil( _RET_ENDIF ) )
			{
				// �X�L�b�v���ɃG���[����
				AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
							_T("illegal endif/else/elif") );
				return false;
			}
			break;
		}

		return true;
	}
	/**
		@brief	IF/IFDEF�ɂ�false�̏ꍇ�̏���
		@exception	CSimpleError	�Ή�����endif��������Ȃ��A�܂���else�̌��else�B
	 */
	bool	getFalse(void) 
	{
		for(;;)
		{
			// else�܂œǂݔ�΂�
			skip_parser	Parser;
			macro_vector	vecMacro;
			CScriptLayer1	l1( m_ist, Parser, vecMacro,
				m_Path.GetPath(), m_nDepth,
				m_nMaxDepth );

			// �G���[�̃`�F�b�N
			if( l1.GetErrorInfo() == NULL )
			{
				// �G���[�Ȃ��̏ꍇ�A�X�L�b�v�̏I����ENDIF/ELSE�����`�F�b�N
				switch( l1.getEndStatus() )
				{
				case _RET_EOF:
					// �w�肵���I���R�[�h��������Ȃ�����
					AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
								_T("endif is not found") );
					return false;
				case _RET_ENDIF:
					break;
				case _RET_ELIF:
					break;
				case _RET_ELSE:
					{
						// ELSE�������̃X�N���v�g���擾
						CScriptLayer1	l1( m_ist, m_Parser, m_vecMacro,
							m_Path.GetPath(), m_nDepth,
							m_nMaxDepth );
						// �G���[�̃`�F�b�N
						if( l1.GetErrorInfo() != NULL )
						{
							// �G���[�𔭌�
							AddErrors( l1.GetErrorInfo() );
							return false;
						}
						
						switch( l1.getEndStatus() )
						{
						case _RET_EOF:
							// �w�肵���I���R�[�h��������Ȃ�����
							AddError(	CScriptError::error_object::ERRORCODE_IF_CANT_FOUND_ENDIF,
										_T("endif is not found") );
							return false;
						case _RET_ENDIF:
							break;
						default:
							// �w�肵���I���R�[�h��������Ȃ�����
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

		// ����I��
		return true;
	}

	/**
		@brief	IF����
	 */
	int		cmdIf( void )
	{
		// �����̎擾
		tstring tstrLine;
		if( m_pLine->GetLine( tstrLine ) == false )
		{
			// �������L�q����Ă��Ȃ�
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("if Conditions") );
			return false;
		}

		long lResult;
		if( !ScriptCalculateInt( lResult, tstrLine.c_str(), m_vecMacro ) )
		{
			// �������ŃG���[
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("if Condition error") );
			return false;
		}

		if( lResult )
		{
			// ������true
			if( !getTrue() )
			{
				// �X�N���v�g�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (true Condition) parse failed.") );
				return false;
			}
		} else {
			// ������false
			if( !getFalse() )
			{
				// �X�N���v�g�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (false Condition) parse failed.") );
				return false;
			}
		}

		return true;
	}

	/**
		@brief	IFDEF�̉��
		@exception	CSimpleError	�s����#ifdef��
		@return	true:�G���[����^false:�G���[�Ȃ�
	 */
	bool	cmdIfdef(void) 
	{
		// �����̎擾
		tstring tstrLine;
		if( m_pLine->GetLine( tstrLine ) == false )
		{
			// �������L�q����Ă��Ȃ�
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("ifdef Conditions") );
			return false;
		}
		tstringstream	sst( tstrLine.c_str() );
		CTextLineEx	line(sst);

		tstring strCnd ;
		if( line.GetWordEx(strCnd) == false )
		{
			// �������L�q����Ă��Ȃ�
			AddError(	CScriptError::error_object::ERRORCODE_IFDEF_CANT_FOUND_CONDITION,
						_T("ifdef Conditions(1)") );
			return false;
		}
		// �}�N���L�[����`����Ă���H
		macro_vector::iterator	it = m_vecMacro.begin();
		for( ; it != m_vecMacro.end() && !it->IsMacroKey(strCnd) ; it++ )
			;
		if( it != m_vecMacro.end() )
		{
			// ������true
			if( !getTrue() )
			{
				// �X�N���v�g�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (true Condition) parse failed.") );
				return false;
			}
		} else {
			// ������false
			if( !getFalse() )
			{
				// �X�N���v�g�̎擾�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if (false Condition) parse failed.") );
				return false;
			}
		}
		// ����I��
		return true;
	}

	/**
		@brief	�v���v���Z�b�T�R�}���h���
		@param	strWord		�擾����������(#�`�̕�����)
		@param	bContinue	true �t�@�C���ǂݍ��݂��p���^false:�t�@�C������
		@return	true:�G���[�Ȃ��^false:�G���[����
		@exception	CSimpleError	�v���v���Z�b�T�ł͂Ȃ�/�`�����ُ�
	 */
	bool	analyzePreprocess( tstring& strWord, bool& bContinue ) 
	{
		// �t�@�C���p���ǂݍ��݂�true�ɐݒ�
		bContinue = true;
		// 
		strWord.erase( strWord.begin(), strWord.begin()+1 );	// �P������'#'���X�L�b�v

		if( strWord.compare(_T("include"))==0 )
		{
			// �t�@�C���̃C���N���[�h
			if( !cmdInclude() )
			{
				// �t�@�C���̃C���N���[�h�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_FAIL,
							_T("include failed.") );
				return false;
			}
		} else if( strWord.compare(_T("define"))==0 )
		{
			// �}�N����`
			if( !cmdDefine() )
			{
				// �}�N����`�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_FAIL,
							_T("define failed.") );
				return false;
			}
		} else if( strWord.compare(_T("define_begin"))==0 )
		{
			// �}�N����`
			if( !cmdDefineBlock() )
			{
				// �}�N����`�Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_DEFINE_FAIL,
							_T("define failed.") );
				return false;
			}
		} else if( strWord.compare(_T("ifdef"))==0 )
		{
			// ifdef
			if( !cmdIfdef() )
			{
				// �������߂Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("ifdef failed.") );
				return false;
			}
		} else if( strWord == _T("endif") )
		{
			// ENDIF�����������̂ŁA�ǂݍ��݂𒆒f
			m_nEndStatus = _RET_ENDIF;
			bContinue = false;
			return true;
		} else if( strWord == _T("else") )
		{
			// #ELSE�����������̂ŁA�ǂݍ��݂𒆒f
			m_nEndStatus = _RET_ELSE;
			bContinue = false;
			return true;
		} else if( strWord == _T("elif") )
		{
			// #ELIF�����������̂ŁA�ǂݍ��݂𒆒f
			m_nEndStatus = _RET_ELIF;
			bContinue = false;
			return true;
		} else if( strWord == _T("if") )
		{
			// #IF
			if( !cmdIf() )
			{
				// �������߂Ɏ��s
				AddError(	CScriptError::error_object::ERRORCODE_IF_FAIL,
							_T("if failed.") );
				return false;
			}

		} else {
			// ����`�̃v���v���Z�b�T�R�}���h
			AddError(	CScriptError::error_object::ERRORCODE_COMMAND_ILLEGAL,
						_T("undefined preprocess command") );
			return false;
		}

		// ����I��
		return true;
	}

	/**
		@brief	�P�ꂪ�}�N���L�[�ɊY�����邩�`�F�b�N���A�}�N���ł���ΓW�J�Etrue��Ԃ��B
		@param	strWord	�`�F�b�N���镶����
		@param	bHit	true:�P�ꂪ�}�N���L�[�ɊY��(�W�J������)
						�^false:�P��̓}�N���L�[�ł͂Ȃ�
		@return	true:�G���[�Ȃ��^false:�G���[����
	 */
	bool	parseMacro( tstring& strWord, bool& bHit )
	{
		// �}�N���L�[����
		macro_vector::iterator	it = m_vecMacro.begin();
		for( ; it != m_vecMacro.end() ; it++ )
		{
			if( it->IsMacroKey( strWord ) )
			{
				// �}�N���L�[�����������̂ŁA�}�N����W�J
				if( !(it->extractMacro)( m_Parser, *this, *m_pLine  ) )
				{
					// �}�N���W�J���ɃG���[
					AddError(	CScriptError::error_object::ERRORCODE_MACRO_EXTRACT_FAIL,
								_T("macro extract fail.") );
					return false;
				}
				bHit = true;
				return true;
			}
		}
		// �}�N���L�[��������Ȃ�����
		bHit = false;
		return true;
	}

public:
	/**
		@brief	�������Ȃ��f�X�g���N�^
	 */
	virtual	~CScriptLayer1(void)
	{}
	// �R���X�g���N�^
	//	Parser :	��ʃ��C��
	//	szFName :	�t�@�C����(�C���N���[�h�p�X�̎w��̂���)
	//	nDepth :	�C���N���[�h�̐[�x
	/**
		@brief	�X�N���v�g���C���P
		@param	Parser		��ʃ��C��
		@param	ist			���̓X�g���[��
		@param	vecMacro	�}�N���R���e�i
		@param	szFName		�t�@�C����(�C���N���[�h�p�X�̎w��ȂǂɎg�p)
		@param	nDepth		�C���N���[�h�t�@�C���̐[�x
		@param	nMaxDepth	�C���N���[�h�t�@�C���̍ő�[�x
		@exception	CSimpleError	�C���N���[�h�t�@�C���̍ő�[�x�𒴂���
										�^�X�N���v�g�̓ǂݏo���ُ�
		@note	�������P��P�ʂɕ������A��ʃ��C��(parser)�֒P���n��
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
		// �l�X�e�B���O�f�v�X�̃`�F�b�N
		++m_nDepth;
		if( m_nDepth >= m_nMaxDepth )
		{
			// �v���v���Z�X�R�}���h�G���[
			AddError(	CScriptError::error_object::ERRORCODE_INCLUDE_DEPTH_OVER,
						_T("over Depth level") );
			return;
		}

		// �t�H���_�p�X�̎擾
		m_Path--;				// �t�@�C�������J�b�g
		// �t�@�C���o�b�t�@�̍쐬
		ptr_simple_text_line_ex	ptrLine( new CTextLineEx(ist) );
		m_pLine = ptrLine.get();

		SCRIPTOBJECT_INFO	scrInfo;
		scrInfo.tstrFNameSrc = m_tszFName;
		scrInfo.ulLine = m_pLine->GetLineNumber();

		tstring strWord;
		// stream����S�Ă̒P����擾����܂Ń��[�v
		while( m_pLine->GetWordEx(strWord) )
		{
			// �v���v���Z�b�T�R�}���h�̃`�F�b�N
			if( strWord[0] == '#' )
			{
				// �v���v���Z�b�T�R�}���h
				bool	bContinue;	// �t�@�C���̌p���ǂݍ��݃t���O
				if( analyzePreprocess( strWord, bContinue ) == false )
				{
					// �v���v���Z�X�R�}���h�G���[
					AddError(	CScriptError::error_object::ERRORCODE_COMMAND_FAIL,
								_T("preprocess command fail.") );
					return;
				}
				if( bContinue == false )
				{
					// �t�@�C������������
					break;
				}
			} else 
			{
				bool bHit = false;

				if( !parseMacro( strWord, bHit ) )
				{
					// �}�N���W�J���ɃG���[
					return;
				}
				if( bHit == false )
				{
					// �}�N���ɂ��q�b�g���Ȃ��������ʏ�̕�����
					// �s�ԍ��̎擾
					scrInfo.ulLine = m_pLine->GetLineNumber();
					// �P��̒ǉ�
					if( !m_Parser.parse( strWord, scrInfo ) )
					{
						// ��͒��ɃG���[����
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
			// �f�v�X���ŏ�ʂȂ�A���[���`�F�b�N
			switch( getEndStatus() )
			{
			case _RET_EOF:
				break;
			case _RET_ENDIF:
			case _RET_ELSE:
			case _RET_ELIF:
				// �s���ȏ������ߏI�[
				AddError(	CScriptError::error_object::ERRORCODE_ILLEGAL_ENDIF,
							_T("illegal endif/else/elif") );
				return;
			}
		}

	}
	// �}�N���̎擾
	macro_vector&	GetMacro(void)
	{
		return	m_vecMacro;
	}
	/**
		@brief	�G���[�����擾����
		@return	�G���[����F�G���[���ւ̃|�C���^
				�G���[�Ȃ��FNULL
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
		@brief	�G���[���̒ǉ�
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
			// �G���[�Ȃ�
		}
		// �G���[��ǉ�
		m_ScriptError += *lpScriptError;
	}
};

////////////////////////////////////////////////////////////////////////////////
// CScriptLayer2
//	�X�N���v�g��̓��C���Q(�c���[�\�z)
//	{�`}�̃c���[�\�����\�z����
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
	script_vector		m_vecChild;				/// ��ʍ\���̕ۑ��p
	int_vector			m_vecLastParent;		/// �q�K�w�J�n���̕ۑ��p

	tstring		m_tstrParentStart;				/// �q�K�w�J�n������
	tstring		m_tstrParentEnd;				/// �q�K�w�I��������
	tstring		m_tstrFilenameBase;				/// �X�N���v�g�t�@�C����

	CScriptError		m_ScriptError;			/// �G���[���

public:
	virtual	~CScriptLayer2(void)
	{}

	/**
		@brief	�X�N���v�g�̓ǂݍ���(stream)
		@param	ist			�\�[�X�X�g���[��
		@param	vecScript	�X�N���v�g�i�[��
		@param	vecMacro	�}�N�����
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
		@brief	�X�N���v�g�̓ǂݍ���(file)
	 */
	CScriptLayer2(	const TCHAR* tszFName,
					script_vector& vecScript,
					macro_vector& vecMacro  ) 
		:	m_tstrParentStart( __SCRIPT_PARENT_START ),
			m_tstrParentEnd( __SCRIPT_PARENT_END )
	{
		// �t�@�C�����J��
		if( tszFName == NULL || tszFName[0] == NULL )
		{
			// �t�@�C���̃I�[�v���Ɏ��s
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
			// �t�@�C���̃I�[�v���Ɏ��s
			CScriptError::error_object	errorObject(	tszFName,
														0,
														CScriptError::error_object::ERRORCODE_FILE_CANT_OPEN,
														tstring(tszFName) + _T(" script file can't open") );
			m_ScriptError.AddError( errorObject );
			return;
		}

		LoadScript( ifst, vecScript, vecMacro, tszFName );

		// �t�@�C�������
		ifst.close();
	}
	/**
		@brief	�������Ȃ��R���X�g���N�^
	 */
	CScriptLayer2(	const TCHAR*	tszParentStart = __SCRIPT_PARENT_START,
					const TCHAR*	tszParentEnd = __SCRIPT_PARENT_END )
		: m_tstrParentStart( tszParentStart )
		, m_tstrParentEnd( tszParentEnd )
	{
	}

	/**
		@brief	�X�N���v�g�̓ǂݍ���
		@param	ist			�\�[�X�X�g���[��
		@param	vecScript	�X�N���v�g�i�[��
		@param	vecMacro	�}�N�����
	 */
	void	LoadScript(	tistream& ist,
						script_vector& vecScript,
						macro_vector& vecMacro,
						const _TCHAR* szFName=_T("")
						) 
	{
		// �X�N���v�g�t�@�C�����̎擾
		m_tstrFilenameBase = szFName;

		// �ŏ�ʍ\���̍쐬
		CScriptObject	obj;
		m_vecChild.push_back( obj );
		// ��͎��s
		CScriptLayer1	Layer1( ist, *this, vecMacro, szFName );
		// �G���[���̎擾
		CScriptError*	lpError = Layer1.GetErrorInfo();
		if( lpError != NULL )
		{
			// Layer1�ŃG���[�����������ꍇ�́A�G���[����ǉ�����
			m_ScriptError += *lpError;
		}

		// ���ׂẲ��ʍ\�����������Ă��邩�H
		if( m_vecChild.size() != 1 )
		{
			// �ŏ�ʂ܂ŕ��A���Ă��Ȃ�
			CScriptError::error_object	errorObject(	"[root]",
														0,
														CScriptError::error_object::ERRORCODE_CANT_FOUND_CLOSE,
														_T("'{' is too much") );
			m_ScriptError.AddError( errorObject );
			return;
		}
		// �\�����R�s�[
		vecScript = (m_vecChild.begin()->vecChild);

		// �X�N���v�g�����
		m_vecChild.clear();
	}

	/**
		@brief	�P����擾���ĊK�w�������s��
		@return	true:�G���[�Ȃ��^false:�G���[����
		@param	strWord		���ʃ��C������n���ꂽ�P��
		@param	scrInfo		�X�N���v�g���
	 */
	virtual bool	parse( tstring& strWord, SCRIPTOBJECT_INFO& scrInfo ) 
	{
		int	n;
		CScriptObject	obj;
		obj.strWord = strWord;
		obj.scrInfo = scrInfo;
		obj.scrInfo.tstrFNameBase = m_tstrFilenameBase;		// �x�[�X�t�@�C�����̐ݒ�

		tstring	tstrTmp;

		// �P�ꂪ�K�w�̊J�n�����H
		n = CStringSearch::GetCount( strWord.c_str(), m_tstrParentStart, tstrTmp );
		if( n>=0)
		{
			// �q�K�w���쐬
			m_vecLastParent.push_back( n );		// �J�n������ۑ�
			m_vecChild.push_back( obj );		// ���݂̊K�w��ۑ�
			return true;
		}
		// �P�ꂪ�K�w�̊��������H
		n = CStringSearch::GetCount( strWord.c_str(), m_tstrParentEnd, tstrTmp );
		if( n>=0)
		{
			script_vector::reverse_iterator		itChild = m_vecChild.rbegin();
			// �e�K�w�֕��A
			// �ŏ�ʊK�w�łȂ����Ƃ��`�F�b�N
			script_vector::reverse_iterator itParent = itChild+1;
			if( m_vecLastParent.empty() || itParent == m_vecChild.rend() )
			{
				// �e�K�w�����݂��Ȃ�
				CScriptError::error_object	errorObject(	"[root]",
															0,
															CScriptError::error_object::ERRORCODE_ILLEGAL_CLOSE,
															_T("'}' is too much") );
				m_ScriptError.AddError( errorObject );
				return false;
			}
			// �K�w�̊J�n�����Ɗ����������΂ɂȂ��Ă��邩�H
			int nLast = *m_vecLastParent.rbegin();
			m_vecLastParent.pop_back();
			if( n != nLast )
			{
				// �K�w�̊J�n�����E�����������΂łȂ�
				m_ScriptError.AddError( CScriptError::error_object(	"[root]",
															0,
															CScriptError::error_object::ERRORCODE_NOT_PARE,
															_T("parent type is not much") ));
				return false;
			}

			// �e�K�w�Ɍ��݂̊K�w��ۑ�
			itParent->vecChild.push_back( *itChild );
			m_vecChild.pop_back();

		}
		// �P������݂̊K�w�֒ǉ�
		script_vector::reverse_iterator		itChild = m_vecChild.rbegin();
		itChild->vecChild.push_back( obj );

		return true;
	}


	/**
		@brief	�G���[�����擾����
		@return	�G���[����F�G���[���ւ̃|�C���^
				�G���[�Ȃ��FNULL
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
// �X�N���v�g�⏕�֐�
////////////////////////////////////////////////////////////////////////////////
/**
	@brief	�X�N���v�g�ȈՃ��[�_
	@param	szFilename		�t�@�C����
	@param	vecScript		�X�N���v�g�̎擾����
	@return	true:����I���^false:�G���[
 */
inline int	ScriptLoadFromFile( const TCHAR* szFilename, script_vector& vecScript )
{
	macro_vector	vecMacro;

	// �X�N���v�g�̓ǂݍ���
	CScriptLayer2	Script( szFilename, vecScript, vecMacro );

	// �t�@�C�������
//	ifst.close();

	CScriptError*	lpError = Script.GetErrorInfo();
	if( lpError != NULL )
	{
		// �G���[����
		tstring	tstr;
		lpError->OutErrorInfoString( tstr );
		RuntimeError( tstr.c_str() );

		return false;
	}

	return true;
}

/**
	@brief	�����񂩂�X�N���v�g�̓ǂݍ���
	@param	tszScript		�X�N���v�g���L�q����������
	@param	vecScript		�X�N���v�g�̎擾����
	@return	true:����I���^false:�G���[
 */
inline int	ScriptLoadFromString( const TCHAR* tszScript, script_vector& vecScript )
{
	std::istringstream	isst( tszScript );
	macro_vector	vecMacro;

	// �X�N���v�g�̓ǂݍ���
	CScriptLayer2	Script( isst, vecScript, vecMacro );

	if( Script.GetErrorInfo() != NULL )
	{
		// �G���[����
		return false;
	}

	return true;
}

/**
	@brief		���\�[�X����X�N���v�g�̎擾
	@param		hInstance		���\�[�X�̊i�[���ꂽ�C���X�^���X
	@param		lpName			���\�[�X��
	@param		lpType			���\�[�X�^�C�v
	@param		vecScript		�X�N���v�g�i�[��
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
	@brief		"xxx=???;"�̕������
	@note		�g�p��F
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
			// ���̒P�ꂪ"="���H
			if( itVec->strWord != "=" )
			{
				// "="�łȂ�
				return NULL;
			}
			break;
		case PHASE_VALUE:
			if( itVec->strWord == ";" )
			{
				// �l���Ȃ�(�s�[";"������)
				return NULL;
			}
			// �l��ۑ�
			lpValue = &(*itVec);
			break;
		case PHASE_DELIMITER:
			if( itVec->strWord != ";" )
			{
				// �s�[�łȂ�
				return NULL;
			}
			break;
		}
	}

	return lpValue;
}

/**
	@brief		�����^�l�擾
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
	@brief		�X�N���v�g�̉��Z(�����^)
	@note		�}�N�����w��(�X�N���v�g����#if���ȂǂŎg�p)
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
	@brief		�X�N���v�g�̉��Z(���������_�^)
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
	@brief		�X�N���v�g�̉��Z(���������_�^)
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
	@brief		�X�N���v�g�̉��Z(�����^)
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
	@brief		�X�N���v�g�̉��w�\�����擾
	@param		itScript		�X�N���v�g�̎Q��
	@param		vecScript		�X�N���v�g
	@param		tszBegin		�J�n�J�b�R
	@param		tszEnd			�I�[�J�b�R
	@return		�擾�������w�\���̃|�C���^�B���s����NULL��Ԃ�
	@note		iterator�͐�������	()�̎��̈ʒu���w���B
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

/// �X�N���v�g�Q�ƈʒu�̃R���e�i�^�C�v
typedef std::vector< script_vector::iterator >	script_iterator_vector;
/**
	@brief		�J���}��؂肳�ꂽ�����̒l���擾
	@param		vecString		���ʂ̕ۑ���
	@param		it				�X�N���v�g�̎Q��
	@param		vecScript		�X�N���v�g
	@param		nNum			�擾����l�̌�(�����ł����";"�܂ł��ׂĎ擾����)
	@return		�擾�����l�̐��B���s�����ꍇ��0��Ԃ��B
	@note		key = value1,value2,value3,...valuen; �̂悤�ȋL�q�ŁA
				value1,...valuen�܂ł̎擾�Ɏg�p����B
				�l�̊Ԃ̋�؂��','�A�l��̏I����';'�Ƃ���B
 */
inline int	ScriptGetValues(	script_iterator_vector&	vecString, 
								script_vector::iterator& it,
								script_vector& vecScript,
								int nNum = -1
								)
{
	int nCount = 0;								/// �擾�����l�̌�
	script_vector::iterator	itLast = NULL;		/// �擾���̒l�̊J�n�ʒu
	int	nValCount = 0;							/// �l�Ɋ܂܂�镶����̌�

	for( ; it != vecScript.end() && nCount != nNum ; it++ )
	{
		if( it->strWord == "," || it->strWord == ";")
		{
			// �f���~�^��������������

			if( nValCount == 0 )
			{
				// �f���~�^���A��(�l�̕����񐔂��O)
				return 0;
			}
			// �l�̎Q�ƈʒu��ۑ�
			vecString.push_back( itLast );

			nValCount = 0;
			// �l�̌������Z
			nCount++;

			// ";"�����������A�܂��͒l�̌����v���𖞂�����?
			if( it->strWord == ";" || nCount == nNum)
			{
				break;
			}
		} else 
		{
			// �l
			if( nValCount == 0 )
			{
				// �l�̊J�n�ʒu��ۑ�
				itLast = it;
			}
			// �l�̌������Z
			nValCount++;
		}
	}

	// �ŌオEOF�̏ꍇ�ŁA�l�̓r���ł���΁A�����܂ł�l�Ɣ��f����
	if( nValCount > 0 && it == vecScript.end() )
	{
		nCount++;
		vecString.push_back( itLast );
	}

	if( nCount < nNum )
	{
		// �l�̌�������Ȃ�
		return 0;
	}

	// �擾�����l�̌���Ԃ�
	return nCount;
}

/**
	@brief	key={value1,...}�`���̒l�擾����
	@param	vecString		�l�̎Q�ƈʒu�ۑ���
	@param	it				�X�N���v�g��͂̊J�n�ʒu(key�̈ʒu)
	@param	vecScript		�X�N���v�g
	@param	ppvecChild		�l�̂���q�K�w�X�N���v�g�̃|�C���^�̕ۑ���(�l�v�Z���Ɏg�p���邽��)
	@param	nNum			�l�̗v����(������";"�܂ł��ׂĎ擾)
	@return	�擾�����l�̌��B0�̏ꍇ�̓G���[�B
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
	@brief	���݈ʒu����w�肵���L�[������
 */
inline script_vector::iterator		ScriptFindKey(	const TCHAR* tszKey, 
													script_vector::iterator it, 
													script_vector& vecScript )
{

	return std::find( it, vecScript.end(), tszKey );
}

}		/// namespace simple

#endif // _SIMPLE_SCRIPT_HPP