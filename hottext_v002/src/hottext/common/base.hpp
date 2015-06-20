// $Id: base.hpp,v 1.2 2005/07/07 23:35:54 Ander Exp $
/**
	@file	base.hpp
	@brief	�V���v���N���X���C�u�����̊�{�w�b�_�t�@�C��
	@author	Ander/Echigo-ya koubou
	@date	$Date: 2005/07/07 23:35:54 $
 */

#ifndef _SIMPLE_BASE_HPP
#define	_SIMPLE_BASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <tchar.h>

namespace simple {
/// TCHAR��string�N���X��`
typedef std::basic_string<_TCHAR>			tstring;
/// TCHAR��stringstream�N���X��`
typedef std::basic_stringstream<_TCHAR>		tstringstream;
/// TCHAR��istream�N���X��`
typedef std::basic_istream<_TCHAR>			tistream;
/// TCHAR��ostream�N���X��`
typedef std::basic_ostream<_TCHAR>			tostream;
/// TCHAR��fstream�N���X��`
typedef std::basic_fstream<_TCHAR>			tfstream;
/// TCHAR������̃R���e�i�N���X��`
typedef std::vector< tstring >				string_vector;

#ifdef _UNICODE
#define	tcout	wcout			/// TCHAR�̕W���o�͐��`
#define	tcin	wcin			/// TCHAR�̕W�����͐��`
#else
#define	tcout	cout			/// TCHAR�̕W���o�͐��`
#define	tcin	cin				/// TCHAR�̕W�����͐��`
#endif

}
#include "./simpleerr.hpp"

#endif	// _SIMPLE_BASE_HPP
