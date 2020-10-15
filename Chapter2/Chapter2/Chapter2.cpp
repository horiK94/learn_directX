#include <iostream>
#include <stdio.h>
//定数: コンパイル前に変換される
#define CHAR_NAME 50
#define HOGE

using namespace std;

//関数のプロトタイプ宣言をしないと使えない(上から実行するので)
int GetTwo();
void no_swap(int, int);
void swap(int*, int*);

struct Member
{
	int member1;
	int member2;
};

//構造体の場合、32bitより大きいなら参照渡しのほうが速度が早い
//参照渡ししたいけど、値は外部で書き換えてほしくない場合はconstをつける
int func(const Member* m)
{
	if (m->member1 < m->member2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int globalx;

int main()
{
	//cout << "Hello" << endl;

	//int a[5] = { 1, 10, 100, 1000, 10000 };

	//struct Person
	//{
	//	//メンバ変数
	//	char* name;
	//	int age;
	//} q;
	//Person p;
	//p.age = 15;

	////こちらでも定数を指定できる(型が指定できるのでこっちを使おう)
	//const int CHAR_MINIMUM = 10;

	//enum {
	//	NUM_A, NUM_B, NUM_C
	//};

	//enum eXYZ {
	//	NUM_X, NUM_Y, NUM_Z
	//};

	//eXYZ num = NUM_Y;
	//cout << num;

	//GetTwo();

	//intのメモリアドレス型
	//int* pi;
	//int val = 10;
	//pi = &val;
	//cout << pi << endl;		//アドレスが返される
	//cout << *pi << endl;	//アドレス内の値を返す

	//int a = 1;
	//int b = 2;
	//no_swap(a, b);
	//cout << a << ", " << b << endl;

	//swap(a, b);
	//cout << a << ", " << b << endl;

	//Member m;
	//m.member1 = 2;
	//m.member2 = 1;
	//cout << func(&m) << endl;

	int val = 10;
	int* valp = &val;
	//ポインタのポインタ(ダブルポインタ)
	//関数内で動的確保したメモリアドレスを渡すときに使う
	int** valpp = &valp;

	int autox;
	int* dynx = new int();

	cout << "グローバル変数: " << &globalx << endl;
	cout << "自動確保変数: " << &autox << endl;
	cout << "動的確保変数: " << &dynx << endl;

	//動的確保した変数の開
	delete dynx;

	int* valArray = new int[4];
	valArray[0] = 10;
	//配列の解放はdelete[]
	delete[] valArray;

#ifdef HOGE
	cout << "hogeがdefineされている" << endl;
#endif
#ifndef HOGE
	cout << "hogeがdefineされていない" << endl;
#endif // !HOGE
}

void no_swap(int a, int b)
{
	int tmp = a;
	a = b;
	b = tmp;
}

void swap(int* a, int* b)
{
	int* tmp = a;
	a = b;
	b = tmp;
}

int GetTwo()
{
	return 2;
}

