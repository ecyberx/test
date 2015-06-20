
#include "simpleutil.hpp"

namespace simple
{
/**
	@brief		�X�N���v�g����\���̃f�[�^���擾
	@param		vecScript		�X�N���v�g
	@param		parseBind		�o�C���h�e�[�u��
	@param		nCount			�o�C���h�e�[�u�������o��
	@param		lpDest			�o�͐�ϐ��擪�A�h���X
	@param		plugIn			�v���O�C���̎Q��
	@return		���������ꍇ�A�ݒ肵���ϐ��̐���Ԃ��B�擾�Ɏ��s�����ϐ�������΂O��Ԃ��B
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
				RuntimeError( "�X�N���v�g�̋L�q�~�X�ł�" );
				return 0;
			}
			if( it->strWord != "=" )
			{
				RuntimeError( "%s �X�N���v�g�̋L�q�~�X�ł�", it->getStringLocate().c_str() );
				return 0;
			}
			it++;
			if( it == vecScript.end() )
			{
				RuntimeError( "�X�N���v�g�̋L�q�~�X�ł�" );
				return 0;
			}
			
			// get value
			nHit++;
			func*	lpFunc = (func*)plugIn.GetProcess( parseBind[i].szProcess );
			if( lpFunc == NULL )
			{
				RuntimeError( "%s ScriptParser�̃v���Z�X'%s'��������܂���", it->getStringLocate().c_str(), parseBind[i].szProcess );
				return 0;
			}
			if( !lpFunc( it, vecScript, ( (char*)lpDest+parseBind[i].tOffset), parseBind[i].tSize, plugIn ) )
			{
				RuntimeError( "%s ScriptParser�̃v���Z�X'%s'�Ő����G���[���������܂���", it->getStringLocate().c_str(), parseBind[i].szProcess );
				return 0;
			}
		}
	}

	return true;
}


/**
	@brief		ScriptParser INT�^�C�v���̓v���Z�X
	@param		it			�X�N���v�g�̎Q�ƊJ�n�ʒu
	@param		vecScript	�X�N���v�g
	@param		lpDest		���ʂ̕ۑ���
	@param		tSize		���ʕۑ���̈�̃T�C�Y
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
		RuntimeError( "%s int���̋L�q�G���[�ł�", it->getStringLocate().c_str() );
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
	@brief		ScriptParser Bool�^�C�v���̓v���Z�X
	@param		it			�X�N���v�g�̎Q�ƊJ�n�ʒu
	@param		vecScript	�X�N���v�g
	@param		lpDest		���ʂ̕ۑ���
	@param		tSize		���ʕۑ���̈�̃T�C�Y
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
		RuntimeError( "%s bool�l�̋L�q�G���[�ł�", it->getStringLocate().c_str() );
		return false;
	}

	*(bool*)(lpDest) = lResult ? true : false;

	return true;
}

/**
	@brief		ScriptParser INT�^�C�v���̓v���Z�X
	@param		it			�X�N���v�g�̎Q�ƊJ�n�ʒu
	@param		vecScript	�X�N���v�g
	@param		lpDest		���ʂ̕ۑ���
	@param		tSize		���ʕۑ���̈�̃T�C�Y
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
		RuntimeError( "%s float�l�̋L�q�G���[�ł�", it->getStringLocate().c_str() );
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
		RuntimeError( "%s float���Z�̃p�����[�^�G���[(size)�ł�", it->getStringLocate().c_str() );
		return false;
	}
	return true;
}


/**
	@brief		ScriptParser basic_string�^�C�v���̓v���Z�X
	@param		it			�X�N���v�g�̎Q�ƊJ�n�ʒu
	@param		vecScript	�X�N���v�g
	@param		lpDest		���ʂ̕ۑ���
	@param		tSize		���ʕۑ���̈�̃T�C�Y
	@return		true:OK/false:NG
 */
_EXPORT(int)	ScriptParseBasicString(	script_vector::iterator it, 
										script_vector& vecScript, 
										void* lpDest, size_t tSize, 
										CPlugIn& )
{
	if( tSize != sizeof(std::string) )
	{
		RuntimeError("string�擾�̃T�C�Y�G���[�ł�");
		return false;
	}
	if( it == vecScript.end() )
	{
		RuntimeError("string�擾�̋L�q�G���[�ł�");
		return false;
	}
	std::string*	lpString = reinterpret_cast< std::string* >( lpDest );
	it->getStringWord();		// ������̃_�u���N�H�[�e�[�V����������΍폜
	*lpString = it->strWord;
	return true;
}
/**
	@brief		ScriptParser basic_string�^�C�v���̓v���Z�X(�t�@�C���p�X�w���)
	@param		it			�X�N���v�g�̎Q�ƊJ�n�ʒu
	@param		vecScript	�X�N���v�g
	@param		lpDest		���ʂ̕ۑ���
	@param		tSize		���ʕۑ���̈�̃T�C�Y
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
		RuntimeError("string�擾�̃T�C�Y�G���[�ł�");
		return false;
	}
	if( it == vecScript.end() )
	{
		RuntimeError("string�擾�̋L�q�G���[�ł�");
		return false;
	}

	std::string*	lpString = reinterpret_cast< std::string* >( lpDest );
	it->getStringWord();		// ������̃_�u���N�H�[�e�[�V����������΍폜
	CPath	path( it->scrInfo.tstrFNameSrc.c_str() );
	path--;
	path += it->strWord.c_str();
	*lpString = path.GetPath();
	return true;
}

/**
	@brief		ScriptParser basic_string�^�C�v���̓v���Z�X
	@param		it			�X�N���v�g�̎Q�ƊJ�n�ʒu
	@param		vecScript	�X�N���v�g
	@param		lpDest		���ʂ̕ۑ���
	@param		tSize		���ʕۑ���̈�̃T�C�Y
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
		RuntimeError( "%s StringList�T�C�Y�G���[", it->getStringLocate() );
		return false;
	}

	string_vector*	lpVecString = reinterpret_cast< string_vector* >( lpDest );

	if( !ScriptIsWord( "{", it, vecScript ) )
	{
		RuntimeError( "%s StringList�擾�Ɏ��s���܂���", it->getStringLocate() );
		return false;
	}

	script_vector::iterator itChild = it->vecChild.begin();
	for( ; itChild != it->vecChild.end() ; itChild++ )
	{
		itChild->getStringWord();		// ������̃_�u���N�H�[�e�[�V����������΍폜
		lpVecString->push_back( itChild->strWord );

		itChild++;
		if( itChild == it->vecChild.end() )break;
		if( itChild->strWord != ","  )
		{
			RuntimeError( "%s StringList�̃��[�h�Ԃ�','������܂���", it->getStringLocate() );
			return false;
		}
	}

	return true;
}


};	//namespace simple
