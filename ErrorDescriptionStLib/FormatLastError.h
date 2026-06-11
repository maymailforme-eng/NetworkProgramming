//#pragma once
//
//
//const char* FormatLastError(int error);


#pragma once

#include<Windows.h>
//#include<stdio.h>	//C
#include<cstdio>	//C++

CHAR* FormatLastError(DWORD dwError, CHAR szError[]);