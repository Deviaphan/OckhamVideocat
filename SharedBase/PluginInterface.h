#pragma once
#include <string>
#include "EntryBase.h"

// Плагин должен реализовывать все или часть этих функций, в зависимости от того, что он должен делать
// Имена и прототипы фукнций должны совпадать с using, т.к. функции ищутся в dll по имени.
// Все функции экспортируются через С интерфейс и stdcall

// флаги операций, поддерживаемых плагином
enum PluginOperations: unsigned int
{
	Operation_FillFilmContent = 1, // есть функционал заполнения карточки фильма по не точному названию (по имени файла)
	Operation_AutofillFilmContent = 2, // есть функционал заполнения карточки (карточек) без вмешательства пользователя. Можно заполнять сразу для кучи фильмов.
};


// Флаг того, что это совместимый плагин. Функция должна всегда возвращать true.
using IsVideoCatPlugin = bool ( __stdcall * )();

// Возвращает уникальный идентификатор плагина. Это можнт быть GUID или любая уникальная строка.
using GetPluginID = std::string ( __stdcall * )();

// Получить название плагина
using GetPluginName = std::wstring ( __stdcall * )();

// Получить описание плагина
using GetPluginDescription = std::wstring ( __stdcall * )();

// Получить флаги операций, поддерживаемых плагином
using GetSuportedOperations = int ( __stdcall * )();

// Выполнить заполнение карточки
using LoadFilmInformation = bool ( __stdcall * )(EntryBase * entry);



extern "C"
{

	//__declspec(dllexport) wchar_t * __stdcall SIR_GetName( void );
}
