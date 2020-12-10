#pragma once
#include "Chapter7.h"

struct Building
{
	LPCTSTR xname;
	D3DXVECTOR3 pos;
	int hmodel;
};

struct BoundingBox
{
	D3DXVECTOR3 minVec;
	D3DXVECTOR3 maxVec;
};

const int MAX_BUILDING = 20;

BoundingBox boundingBoxes[MAX_BUILDING];
Building buildings[MAX_BUILDING] =
{
	{ _T("floor1.x"), D3DXVECTOR3(-6, 0,  6), -1},
	{ _T("floor2.x"), D3DXVECTOR3(6, 0,  6), -1},
	{ _T("floor3.x"), D3DXVECTOR3(-6, 0, -6), -1},
	{ _T("floor4.x"), D3DXVECTOR3(6, 0, -6), -1},
	{ _T("roof1.x"),  D3DXVECTOR3(-6,12, -6), -1},
	{ _T("roof2.x"),  D3DXVECTOR3(6,12, -6), -1},
	{ _T("roof3.x"),  D3DXVECTOR3(-6,12,  6), -1},
	{ _T("roof4.x"),  D3DXVECTOR3(6,12,  6), -1},
	{ _T("paneln1.x"),D3DXVECTOR3(6, 6, 12), -1},
	{ _T("paneln2.x"),D3DXVECTOR3(-6, 6, 12), -1},
	{ _T("panels1.x"),D3DXVECTOR3(-6, 6,-12), -1},
	{ _T("panels2.x"),D3DXVECTOR3(6, 6,-12), -1},
	{ _T("panelw1.x"),D3DXVECTOR3(-12, 6,  6), -1},
	{ _T("panelw2.x"),D3DXVECTOR3(-12, 6, -6), -1},
	{ _T("panele1.x"),D3DXVECTOR3(12, 6,  6), -1},
	{ _T("panele2.x"),D3DXVECTOR3(12, 6, -6), -1},
	{ _T("house.x"),  D3DXVECTOR3(4, 2,  4), -1},
	{ _T("ball.x"),   D3DXVECTOR3(4, 0.8f, -1), -1},
	{ _T("tree.x"),   D3DXVECTOR3(-4, 3, -4), -1},
	{ _T("leaf.x"),   D3DXVECTOR3(-4, 8, -4), -1}
};