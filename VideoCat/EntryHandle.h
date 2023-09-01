#pragma once

#include "FileStore/access.h"
#include "FileStore/types.h"

using PathHashType = unsigned long long;
using EntityType = unsigned int;

// Дескриптор объекта
class EntryHandle
{
public:
	enum
	{
		IS_NONE = 0,
		IS_ROOT = 1, // Корневая директория
		IS_FILE = 1 << 1, // Папка/файл на внутреннем, внешнем или сетевом диске
		IS_LINK = 1 << 2, // объект-гиперссылка на страницу с онлайн фильмом/сериалов
		IS_VIRTUAL = 1 << 3, // Виртуальная папка/файл, для добавления физически отсутствующих папок и файлов (dvd болванок или просто отсутствующих фильмов)
		IS_PERSON = 1 << 4, // объект-персона (актёр, режиссёр, сценарист). Добавляется внутрь виртуальной папки
	};

public:
	PathHashType hash; // хэш пути к файлу. Для физически отсутствующих просто сгенерированный идентификатор.
	EntityType type;

	static const EntryHandle EMPTY;

public:
	explicit EntryHandle( PathHashType h = PathHashType(), EntityType t = IS_NONE )
		: hash( h )
		, type( t )
	{}

	bool operator == ( const EntryHandle & rhs ) const
	{
		if( type != rhs.type )
			return false;
		if( hash != rhs.hash )
			return false;
		return true;
	}

	bool operator != ( const EntryHandle & rhs ) const
	{
		return !(*this == rhs);
	}

	bool operator < ( const EntryHandle & rhs ) const
	{
		if( type < rhs.type )
			return true;
		if( type > rhs.type )
			return false;
		return hash < rhs.hash;
	}

public:
	static CString ToString( PathHashType h, EntityType type );
	static CString ToBase64( PathHashType h, EntityType type );

	CString ToString() const
	{
		return ToString( hash, type );
	}

	CString ToBase64() const
	{
		return ToBase64( hash, type );
	}

public:

	bool IsRoot() const
	{
		return (type & IS_ROOT) != 0;
	}

	bool IsReal() const
	{
		return (type & IS_FILE) != 0;
	}

	bool IsVirtual() const
	{
		return (type & IS_VIRTUAL) != 0;
	}

	bool IsLink() const
	{
		return (type & IS_LINK) != 0;
	}

	bool IsPerson() const
	{
		return (type & IS_PERSON) != 0;
	}

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		// Версионность объекта не поддерживается! POD для быстрой сериализации
		ar & hash;
		ar & type;
	}
};

FEMSTORE_SET_POD_TYPE( EntryHandle );
