#pragma once
#include "Chapter7.h"

struct Building
{
	LPCTSTR xname;
	D3DXVECTOR3 pos;
	int hmodel;
};

const int MAX_BUILDING = 4;

Building buildings[MAX_BUILDING] = {
	{_T("floor1.x"), D3DXVECTOR3(-6, 0, 6), -1},
	{_T("floor2.x"), D3DXVECTOR3(6, 0, -6), -1},
	{_T("floor3.x"), D3DXVECTOR3(-6, 0, -6), -1},
	{_T("floor4.x"), D3DXVECTOR3(6, 0, 6), -1},
};
