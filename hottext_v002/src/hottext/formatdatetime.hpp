#ifndef	INCLUDE_FORMATDATETIME_HPP
#define	INCLUDE_FORMATDATETIME_HPP

#include <ctime>

/// 一文字のみ小文字変換
#define	_MYSTRLWR(x)	(( (x) >= 'A' && (x)<='Z') ? (x)-'A'+'a' : (x) )

/**
	@brief		日時を指定した文字列の出力形式に置換
	@note		基本的にExcel準拠。ただし、完全互換ではないので注意。
 */
class	CFormatDateTime 
{
private:
	tm*		m_pTime;		/// ローカル時間(変換後)
	bool	m_bMinute;		/// 'm'を分と判断する？
	string	m_strDest;		/// 出力先

	static const char*	_MONTH[];		/// 月
	static const char*	_WEEK[];		/// 曜日
	static const char*	_WEEK_JA[];		/// 曜日(日本語)


	/**
		@brief		同じ文字が続く個数の取得
		@return		先頭と同じ文字が続く個数
		@param		pStr		対象文字列
	 */
	size_t		countChar( const char* pStr )
	{
		const char*	pStart = pStr;
		for( ; *pStr == *pStart ; ++pStr )
			;
		return	static_cast<size_t>( pStr - pStart );
	}

	/**
		@brief		年(西暦)の出力
		@param		tLen		指定桁数
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
		@brief		月の出力
		@param		tLen		指定桁数
		@note		桁数は値により以下の出力結果となる。
		@note		- 2個まで：mの数がそのまま月の桁数となる(1個の場合は10月以降は2桁)
		@note		- 3：3文字の月文字となる
		@note		- 4：月全文字
		@note		- 5以上：1文字の月文字となる
	 */
	void	parseMonth( size_t tLen )
	{
		switch( tLen )
		{
		case 1:
		case 2:
			{
				// 月を出力
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
		@brief		日/曜日の出力
		@param		tLen		指定桁数
		@note		桁数は値により以下の出力結果となる。
		@note		- 2個まで：dの数がそのまま月の桁数となる(1個の場合は10日以降は2桁)
		@note		- 3個：曜日3文字(Sun/Mon/Tue/Wed/Thu/Fri/Sat)
		@note		- 4以上：曜日全文字(Sunday/Monday...)
	 */
	void	parseDay( size_t tLen )
	{
		switch( tLen )
		{
		case 1:
		case 2:
			{
				// 日付を出力
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
			// 2バイト文字はそのままスルー
			char kanji[3] = {szFormat[0], szFormat[1] };
			m_strDest += kanji;
			szFormat += (szFormat[1] != '\0') ? 1 : 0;
		} else
		{
			// 1バイト文字の解析
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
			case	'a':		// 曜日
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
		// NULLチェック
		if( pTime == NULL )
		{
			return NULL;
		}
		// 出力先の初期化
		m_strDest.clear();

		// 時刻の設定
		m_pTime = pTime;

		// フォーマットの解析＆時間の埋め込み
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