// bst_speak.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "iostream.h"
#include "bst.h"

int main(int argc, char* argv[])
{
	HWND hWnd = CreateWindowA("STATIC","dummy",WS_VISIBLE,0,0,100,100,NULL,NULL,NULL,NULL);
	int instance = 0;
	char test[] = "This is some example test }";

	HANDLE ldll = LoadLibrary("bst.dll");

	int stat = bstCreate(instance);

	cout << instance;
	cout << stat;

	return 0;
}

