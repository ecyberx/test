// $Id: simpleutil.hpp,v 1.1 2006/04/15 11:16:36 ander Exp $
/**
	@brief	���[�e�B���e�B�֐��w�b�_�t�@�C��
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2006/04/15 11:16:36 $
	@note	�{�w�b�_�t�@�C���Ɋ܂܂��e�֐���simpleutil.cpp�ɂĒ�`����Ă���B
 */

#ifndef	_SIMPLE_UTIL_HPP
#define	_SIMPLE_UTIL_HPP

#include "./base.hpp"
#include "./simpleplugin.hpp"
#include "./simplescript.hpp"


namespace simple
{

/**
	@brief		�X�N���v�g��͂̃o�C���h�e�[�u���\����
 */
struct	script_parse_bind
{
	const char*	szKey;			/// �L�[
	size_t		tOffset;		/// �Ώۂ̃I�t�Z�b�g�A�h���X
	const char*	szProcess;		/// �p�����[�^�擾�p�v���Z�X
	size_t		tSize;			/// �Ώۊi�[�̈�̊m�ۃT�C�Y
};
#define	SCRIPT_PARSE_BIND(_KEY,_CLASS,_VALUE,_PROCESS, _TYPE )	{_KEY, offsetof(_CLASS,_VALUE), _PROCESS, sizeof(_TYPE) }
/**
	@brief		�X�N���v�g����\���̃f�[�^���擾
	@param		vecScript		�X�N���v�g
	@param		parseBind		�o�C���h�e�[�u��
	@param		nCount			�o�C���h�e�[�u�������o��
	@param		lpDest			�o�͐�ϐ��擪�A�h���X
	@param		plugIn			�v���O�C���̎Q��
	@return		���������ꍇ�A�ݒ肵���ϐ��̐���Ԃ��B�擾�Ɏ��s�����ϐ�������΂O��Ԃ��B
 */
int	ScriptParse( script_vector& vecScript, const script_parse_bind* parseBind, int nCount, void* lpDest, CPlugIn& plugIn );

};	// namespace simple
#endif	//_SIMPLE_UTIL_HPP
