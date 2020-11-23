#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>
#include <dinput.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

struct Model
{
	//���b�V��
	LPD3DXMESH pMesh;
	//�}�e���A���̔z��
	D3DMATERIAL9* pMaterial;
	//�e�N�X�`���̔z��
	LPDIRECT3DTEXTURE9* pTexture;
	//�}�e���A���̐�
	DWORD numMaterials;

	//�f�[�^�������Ă��邩�̃t���O
	bool used;
};

//�O���[�o���ϐ�
//���̃\�[�X�R�[�h�ł��g�p�ł���悤��extern������
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3DDeivece;

extern float g_aspect;

extern Model g_models[];

//�֐��v���g�^�C�v�錾
HRESULT InitD3DWindow(LPCTSTR winTitle, int w, int h);
int LoadModel(LPCTSTR fileName);
void RenderModel(int idx);
const char* GetKeyState();
void setTimer(int idx, DWORD time);
BOOL isTimerGoal(int idx);
//�o�ߎ��Ԃ��擾����
DWORD getPassedTime(int idx);