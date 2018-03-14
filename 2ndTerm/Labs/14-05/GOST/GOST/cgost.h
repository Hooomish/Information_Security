#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#ifndef _RWSTD_NO_NAMESPACE
using namespace std;
#endif


#pragma once
/*=========================================================*/
/*                       ����, ������������ � ���������   */
/* ��������� ����, ������������ ����������� Windows, ��� ���������������
������� � ���, ���, ����� ������ ���� ����������� �/� �����������
� UNIX/Linux ������� ��� ��� ����������, �� ��� ������ ��������� �
����������, ����� ���������� ����� � UNIX, ��� ���� ������ � ����������� ���,
����, ��� �������� ��������� ��� Win32. ������ �������, �������� ������������
������ r��������� �� Borland. �, �������������� "������" ����������� C/C++,
�� �������� � ������� ���������� ���������, ���, ����� ����������.
� ������ ������, ����� - C++Builder 6 : ������ ������...
�������� �� ������� ������������� ������ ��������������� ���������,
�������� �������� ������� ������������, �, ����� �������� ��� � ��������� ��������������
������. �������� ������������� ���������, ������ ������� �� ����������:
1. ����������� ��������: http://www.rsdn.ru
����� ���������, ������������ ��������: ������ ��������� mailto:avin@chat.ru
��������: http://www.enlight.ru/crypto
2. ������� �������� internet
�������� ����� �������������� ��������, � ����� ����������, � �� �������� �����������
� ����� ������ ������������ ����������. ��, ����� �����������, ��������� �� ������
������ �����, �� � ����� ������������, ��� �� ��������� ��� �����������.
��������� ���������� ���������� ����� _rand32(),
�������� �� ������������� D. Knuth � H.W. Lewis.
�������� ��������� ����������������. � ��������� �������, ���� ��� ������������,
������ ��������, �.�. ����� ����� �� ��������,
:"���� ��� ���-�� ������ � ���������� ������ - ������, ��� ��������� �������".
�������� �� ������ ������������ "����������", ��, ������ ������. � ��������,
������������ ���������� �������,�� ������ ������, ��� ������������ � �������.
��� ����������, ���� ��������������, ��� �� �����������, �.�. ��� �������������� � ������
���������� ��� ������������ / ����������� ������ � ������� �������� � ������ � ������� ��������.
��� ������� �������� ����������� �����, ��, � ������ ������, ��� ���������� �� CPU,
������ � �������� ��� � ������ (�������� �����),� ��������� �������� ������ ����������� ����.
----------------------------------------------------------------------------------------------
����� ������ ����������: ������ ������  vic-ivdel@mail.ru
����� 2005 �.
----------------------------------------------------------------------------------------------
������� ��� ������������ ������.
����-���   ���   ������� ���
�������, ����� �������.
----------------------------------------------------------------------------------------------
���� �� ������ �������� � ���� ������� /������ - ����������, ���������� � ���� �����������,
��� ������ �� ��������.                                                                      */
typedef unsigned __int64 UINT_64;
typedef unsigned long   UINT_32;
typedef unsigned long   Key32[8];
typedef unsigned char   BYTE;
typedef unsigned char*  LPBYTE;
typedef const unsigned char*  LPCBYTE;
typedef const char*     LPCSTR;
typedef char*           LPSTR;
typedef unsigned char   BLOCK[16][8];
typedef enum TYPEKOD { fbase, fgame, fgame_os }TTYPEKOD;
typedef enum MODEKOD { b64, b256 }TMODEKOD;
/*=========================================================*/
/*        ��������������� �������                          */
extern "C" {
	LPSTR Int32ToBin(UINT_32 X);
	LPSTR Int64ToBin(UINT_64 X);
	LPSTR Int32ToHex(LPSTR s, UINT_32 Value);
	LPSTR Int64ToHex(LPSTR s, UINT_64 Value);
	LPCSTR Int32ToStr(LPSTR s, UINT_32 x);
	LPCSTR Int64ToStr(UINT_64 N);
	UINT_32 GetSizeFile(const char* fname);
	UINT_64 StrToInt64(LPCBYTE text);
	void Seed(UINT_32 dum);
	UINT_32 _rand32(void);
	bool SaveDataFile(LPCSTR fname, void *data, size_t size);
	bool LoadDataFile(LPCSTR fname, void *data, size_t size);
};
/*=========================================================*/
/*                 ��������� ���������� ����������         */
typedef struct {
	Key32   K;    // 256 ������ ���� (8 � 32  = 256  ��� = 32 �����)
	UINT_64 SP;   // �������������
	BLOCK   BS;   // ������� �����   (16 � 8  = 1024 ��� = 128 ����)
	TYPEKOD type; // ��� ����������� fbase = ������, fgame = ������������, fgame_os = �����. � ��
	MODEKOD mode; // ����� ����������� b64 = �����������, b256 = ������
	void SetTable(BLOCK bl)
	{
		int i, j;
		for (i = 0; i<16; i++)
			for (j = 0; j<8; j++)
				BS[i][j] = bl[i][j];
	};
	void GetTable(BLOCK &bl)
	{
		int i, j;
		for (i = 0; i<16; i++)
			for (j = 0; j<8; j++)
				bl[i][j] = BS[i][j];
	};
	void SetKey(Key32 key)
	{
		int i;
		for (i = 0; i<8; i++)
			K[i] = key[i];
	};
	void GetKey(Key32 &key)
	{
		int i;
		for (i = 0; i<8; i++)
			key[i] = K[i];
	};
}GOSTPAR, *LPGOSTPAR;
/*=========================================================*/
/*                 ����� ��������� ���� 28147-89           */
class TGost
{
public:
	TGost(void);
	~TGost(void);
private:
	int icount_iter;
	UINT_64 Kode(UINT_64 N, UINT_64 &X, bool kript = true);
	UINT_64 SK;
	Key32 K;
	UINT_32 N1;
	UINT_32 N2;
	BLOCK   BS;
	TYPEKOD type;
	MODEKOD mode;
	GOSTPAR param;
	struct {
		unsigned ui4_0 : 4;
	};
	/*-----------------------------------------------*/
	UINT_32 ROL(UINT_32 X, BYTE n);
	UINT_64 SWAP64(UINT_64 X);
	UINT_64 SWAP32(UINT_32 N1, UINT_32 N2);
	UINT_64 RPGCH(UINT_64 N);
	UINT_32 ReplaceBlock(UINT_32 x);
	UINT_64 BaseKod(UINT_64 N, UINT_32 X);
	UINT_64 CodeGost(UINT_64 N);
	UINT_64 DeCodeGost(UINT_64 N);
	bool IsExistsBlock(BYTE* b, BYTE ch);
public:
	void InitGost(UINT_64 S, UINT_32 k32[], BLOCK tbl, TYPEKOD tk = fgame_os, MODEKOD mk = b256);
	void InitGost(GOSTPAR gp);
	void InitGost();
	UINT_32 KodeFile(const char *fname_in, const char* fname_out, bool kript = true);

	bool SaveParametr(LPCSTR fname, void *data = NULL);
	bool LoadParametr(LPCSTR fname, void *data = NULL);

	LPGOSTPAR GetParamter();
	void    SetParamter(LPGOSTPAR par);

	void InitTable(BLOCK & bl);
	void InitKey(Key32 & k);
	bool SetKey(char* KeyCh);
	void SetKey(const UINT_32 *a);
	UINT_32* GetKey(Key32 &key);
	UINT_64 GetSinhro();
	UINT_64 SetSinhro(UINT_64 X);

	TYPEKOD GetType();
	MODEKOD GetMode();
	void SetType(TYPEKOD tk);
	void SetMode(MODEKOD mk);
	void SetBlock(const BLOCK b);

	UINT_64 GostGUM(UINT_64 N, UINT_64 &X);
	UINT_64 GostGamOS(UINT_64 N, UINT_64 &X, bool kode = true);
	UINT_64 GetImito(UINT_64 X);

	UINT_64 KriptData(UINT_64 N, bool kode = true);

	void Clear(void);

	UINT_32 GetRand32();
	UINT_64 GetRand64();
	UINT_32 Rand32Char();
	BLOCK* GetTable(BLOCK & bl);
};






