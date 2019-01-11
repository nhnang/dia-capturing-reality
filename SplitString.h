#pragma once
#include "stdafx.h"
#include <vector>

namespace V4 {

class SplitString
{
	public :
		SplitString(){}

		~SplitString(){ clear();  }

		USHORT Split( LPCWSTR str, WCHAR ch )
		{
			USHORT s = 0, e=0;
			std::vector<std::pair<USHORT, USHORT>> ar;

			int total_len = 0;

			for( int i =0; i < lstrlen(str); i++ )
			{
				if ( str[i] == ch )
				{
					std::pair<USHORT, USHORT> s1( s,e );
					ar.push_back(s1);

					total_len += (e-s+1);
					s = e + 1;
				}		
				e++;
			}

			if ( s!=e )
			{
				std::pair<USHORT, USHORT> s1( s,e );
				ar.push_back(s1);
				total_len += (e-s+1);
			}

			WCHAR* wx = new WCHAR[ total_len ];
			LPWSTR sb = wx;

			for(auto& it : ar )
			{
				int len = it.second-it.first;
						
				memcpy(sb, str+it.first, sizeof(WCHAR)*len);

				sb[len] = 0;
				sb += len+1;
			}

			
			cxt_.buf = (LPWSTR*)wx;
			cxt_.total_len = total_len;
			cxt_.cnt = (USHORT)ar.size();
			cxt_.idx = new USHORT[cxt_.cnt];
			int j = 0;
			for(auto& it : ar )
				cxt_.idx[j++] = it.first;

			return cxt_.cnt;
	
		}

		LPCWSTR operator[] (USHORT idx )
		{	
			if ( idx >= cxt_.cnt )
				return NULL;

			LPWSTR p = (LPWSTR) cxt_.buf;
			return (LPCWSTR)&p[cxt_.idx[idx]];
		}
		
		USHORT size() const {cxt_.cnt; }

		void clear(){ cxt_.Clear(); }

	private :
		struct Context
		{
			Context()
			{
				idx = nullptr;
				buf = nullptr;
				total_len = 0;
				cnt = 0;
			}

			void Clear()
			{
				delete [](WCHAR*)buf;
				delete [] idx;
				buf = nullptr;
				idx = nullptr;
				total_len = 0;
				cnt = 0;
			}

			LPWSTR* buf;
			USHORT* idx;
			USHORT total_len;
			USHORT cnt;
		};

		Context cxt_;
 };

};