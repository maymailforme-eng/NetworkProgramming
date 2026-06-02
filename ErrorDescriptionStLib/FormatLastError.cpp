#include "pch.h"                                    //для компиляции - файл предкомпилированных заголовков.

#include "FormatLastError.h"

#include <Windows.h>

const char* FormatLastError(int error)
{

    static char buffer[512];                        //статический что бы не затереть массив при выходе.

    FormatMessageA(
                                                    //Различные флаги модификаторы
        //FORMAT_MESSAGE_ALLOCATE_BUFFER |          // Windows сама выделит память под строку ошибки - нужно потом очищать 
        FORMAT_MESSAGE_FROM_SYSTEM |                // искать текст ошибки в системных таблицах Windows
        FORMAT_MESSAGE_IGNORE_INSERTS,              // игнорировать параметры вида %1 %2 внутри сообщения

        NULL,                                       // источник сообщений.
                                                    // NULL = использовать системные таблицы Windows

        error,                                      // код ошибки, для которого нужно получить текст
                                                    // например WSAGetLastError()

        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // язык сообщения.
                                                    // использовать язык системы по умолчанию

        buffer,                                     // адрес указателя на буфер.
                                                    // Windows запишет сюда адрес выделенной строки

        sizeof(buffer),                             // размер буфера.
                                                    // 0 потому что память выделяет сама Windows

        NULL                                        // дополнительные параметры для подстановок (%1 %2)
                                                    // не используются
    );



    return buffer;
}