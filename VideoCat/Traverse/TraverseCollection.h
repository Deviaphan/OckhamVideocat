#pragma once
#include "../Entry.h"
#include <vector>

namespace Traverse
{
	enum Result
	{
		STOP, // совсем остановить обход
		CONTINUE, // продолжать обход
		GOBACK, // Не выполнять обход дочерних, вернуться к родителю

		Result_Force32 = 0x7fffffff
	};


	struct Base
	{
		virtual Result Run( Entry & entry ) = 0;
	};


	struct GetAll : public Base
	{
		std::vector<EntryHandle> allHandles;
		bool includeEpisodes = true;

		Result Run( Entry & entry ) override
		{
			if( !includeEpisodes )
			{
				// если не нужно проверять эпизоды, то не нужно проверять дочерние элементы папки с сериалом
				if( entry.IsTV() )
					return GOBACK;
			}

			allHandles.push_back( entry.thisEntry );
			return CONTINUE;
		}
	};

}
