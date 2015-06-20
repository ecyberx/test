#ifndef	INCLUDE_FORMATDATETIME_HPP
#define	INCLUDE_FORMATDATETIME_HPP

#include <ctime>

/// �ꕶ���̂ݏ������ϊ�
#define	_MYSTRLWR(x)	(( (x) >= 'A' && (x)<='Z') ? (x)-'A'+'a' : (x) )

/**
	@brief		�������w�肵��������̏o�͌`���ɒu��
	@note		��{�I��Excel�����B�������A���S�݊��ł͂Ȃ��̂Œ��ӁB
 */
class	CFormatDateTime 
{
private:
	tm*		m_pTime;		/// ���[�J������(�ϊ���)
	bool	m_bMinute;		/// 'm'�𕪂Ɣ��f����H
	string	m_strDest;		/// �o�͐�

	static const char*	_MONTH[];		/// ��
	static const char*	_WEEK[];		/// �j��
	static const char*	_WEEK_JA[];		/// �j��(���{��)


	/**
		@brief		�����������������̎擾
		@return		�擪�Ɠ���������������
		@param		pStr		�Ώە�����
	 */
	size_t		countChar( const char* pStr )
	{
		const char*	pStart = pStr;
		for( ; *pStr == *pStart ; ++pStr )
			;
		return	static_cast<size_t>( pStr - pStart );
	}

	/**
		@brief		�N(����)�̏o��
		@param		tLen		�w�茅��
	 */
	void	parseYear( size_t tLen )
	{
		char	buf[512];
		_snprintf( buf, sizeof(buf), "%04d",  m_pTime->tm_year+1900 );
		string	strYear(buf);
		if( tLen > 4 )tLen = 4;
		m_strDest += strYear.substr( 4-tLen, tLen );
	}
	/**
		@brief		���̏o��
		@param		tLen		�w�茅��
		@note		�����͒l�ɂ��ȉ��̏o�͌��ʂƂȂ�B
		@note		- 2�܂ŁFm�̐������̂܂܌��̌����ƂȂ�(1�̏ꍇ��10���ȍ~��2��)
		@note		- 3�F3�����̌������ƂȂ�
		@note		- 4�F���S����
		@note		- 5�ȏ�F1�����̌������ƂȂ�
	 */
	void	parseMonth( size_t tLen )
	{
		switch( tLen )
		{
		case 1:
		case 2:
			{
				// �����o��
				char	buf[512];
				_snprintf( buf, sizeof(buf), "%0*d", tLen, m_pTime->tm_mon+1 );
				m_strDest += buf;
			}
			break;
		case 3:
			{
				string	strMonth = _MONTH[m_pTime->tm_mon];
				m_strDest += strMonth.substr(0,3);
			}
			break;
		case 4:
			m_strDest += _MONTH[m_pTime->tm_mon];
			break;
		default:
			m_strDest += _MONTH[m_pTime->tm_mon][0];
			break;
		}
	}
	/**
		@brief		��/�j���̏o��
		@param		tLen		�w�茅��
		@note		�����͒l�ɂ��ȉ��̏o�͌��ʂƂȂ�B
		@note		- 2�܂ŁFd�̐������̂܂܌��̌����ƂȂ�(1�̏ꍇ��10���ȍ~��2��)
		@note		- 3�F�j��3����(Sun/Mon/Tue/Wed/Thu/Fri/Sat)
		@note		- 4�ȏ�F�j���S����(Sunday/Monday...)
	 */
	void	parseDay( size_t tLen )
	{
		switch( tLen )
		{
		case 1:
		case 2:
			{
				// ���t���o��
				char	buf[512];
				_snprintf( buf, sizeof(buf), "%0*d", tLen, m_pTime->tm_mday );
				m_strDest += buf;
			}
			break;
		case 3:
			{
				string	strWeek = _WEEK[m_pTime->tm_wday];
				m_strDest += strWeek.substr(0,3);
			}
			break;
		case 4:
			m_strDest += _WEEK[m_pTime->tm_wday];
			break;
		default:
			m_strDest += _WEEK[m_pTime->tm_wday][0];
			break;
		}
	}

	void	parseDayJa( size_t tLen )
	{
		switch( tLen )
		{
		case 3:
			{
				string	strWeek = _WEEK_JA[m_pTime->tm_wday];
				m_strDest += strWeek.substr(0,2);
			}
			break;
		default:
			m_strDest += _WEEK_JA[m_pTime->tm_wday];
			break;
		}
	}

	void	parseHour( size_t tLen )
	{
		char	buf[512];
		_snprintf( buf, sizeof(buf), "%0*d", tLen, m_pTime->tm_hour );
		m_strDest += buf;

		m_bMinute = true;
	}
	void	parseMinute( size_t tLen )
	{
		char	buf[512];
		_snprintf( buf, sizeof(buf), "%0*d", tLen, m_pTime->tm_min );
		m_strDest += buf;

		m_bMinute = false;
	}
	void	parseSecond( size_t tLen )
	{
		char	buf[512];
		_snprintf( buf, sizeof(buf), "%0*d", tLen, m_pTime->tm_sec );
		m_strDest += buf;
	}


	const char*	parseDateTime( const char* szFormat )
	{
		if( *szFormat < 0 )
		{
			// 2�o�C�g�����͂��̂܂܃X���[
			char kanji[3] = {szFormat[0], szFormat[1] };
			m_strDest += kanji;
			szFormat += (szFormat[1] != '\0') ? 1 : 0;
		} else
		{
			// 1�o�C�g�����̉��
			size_t	tLen = countChar( szFormat );
			char	c = _MYSTRLWR(*szFormat);

			switch(c)
			{
			case	'y':		// year
				parseYear( tLen );
				break;
			case	'm':		// month/minute
				if( tLen <= 2 && m_bMinute )
				{
					parseMinute( tLen );
				} else
				{
					parseMonth( tLen );
				}
				break;
			case	'd':		// day
				parseDay( tLen );
				break;
			case	'a':		// �j��
				if( tLen >= 3 )
				{
					parseDayJa( tLen );
				} else
				{
					string	str(tLen,*szFormat);
					m_strDest += str;
				}
				break;
			case	'h':		// hour
				parseHour( tLen );
				break;
			case	's':		// second
				parseSecond( tLen );
				break;
			default:
				{
					string	str(tLen,*szFormat);
					m_strDest += str;
				}
			}
			szFormat += tLen;
		}

		return	szFormat;
	}



public:
	CFormatDateTime( void )
		: m_bMinute(false)
		, m_pTime( NULL )
	{}

	const char*	ParseDate( const char* szFormat, tm* pTime )
	{
		// NULL�`�F�b�N
		if( pTime == NULL )
		{
			return NULL;
		}
		// �o�͐�̏�����
		m_strDest.clear();

		// �����̐ݒ�
		m_pTime = pTime;

		// �t�H�[�}�b�g�̉�́����Ԃ̖��ߍ���
		while( *szFormat )
		{
			szFormat = parseDateTime( szFormat );
		}

		return	m_strDest.c_str();
	}

	const char* GetDateTime( void )
	{
		return	m_strDest.empty() ? NULL : m_strDest.c_str();
	}

};
#endif	//	INCLUDE_FORMATDATETIME_HPP