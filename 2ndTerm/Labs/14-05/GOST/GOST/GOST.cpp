#include "stdafx.h"
#include "cgost.h"
/*��� ����������� ��� ��������� ��������� � ���������� ���������*/
#define C1   0x1010101
#define C2   0x1010104
#define m32  0x4000000000000
#define m321 0xFFFFFFFF
/* ���������� ���������� ��*/
static UINT_32 iran;
/**/
const BLOCK Sb =   //������� ����� - ������ ����������� ��������
{
	{ 4, 14,  5,  7,  6,  4, 13,  1 },
	{ 10, 11,  8, 13, 12, 11, 11, 15 },
	{ 9,  4,  1, 10,  7, 10,  4, 13 },
	{ 2, 12, 13,  1,  1,  0,  1,  0 },
	{ 13,  6, 10,  0,  5,  7,  3,  5 },
	{ 8, 13,  3,  8, 15,  2, 15,  7 },
	{ 0, 15,  4,  9, 13,  1,  5, 10 },
	{ 14, 10,  2, 15,  8, 13,  9,  4 },
	{ 6,  2, 14, 14,  4,  3,  0,  9 },
	{ 11,  3, 15,  4, 10,  6, 10,  2 },
	{ 1,  8, 12,  6,  9,  8, 14,  3 },
	{ 12,  1,  7, 12, 14,  5,  7, 14 },
	{ 7,  0,  6, 11,  0,  9,  6,  6 },
	{ 15,  7,  0,  2,  3, 12,  8, 11 },
	{ 5,  5,  9,  5, 11, 15,  2,  8 },
	{ 3,  9, 11,  3,  2, 14, 12, 12 }
};
const Key32 K32 = //���� - 256 ��� - ������ ����������� ��������
{
	0x10CBC8CD,
	0x2FC0CFC5,
	0x34C0CCC1,
	0x92CEC2C8,
	0x81CCD8C3,
	0x705DCDC8,
	0x64C8D8C2,
	0x5ACECBDF

};

/*=============== ������ ��������� ����������� ================================*/
/*============================================================================*/
/*************** ����������� �� ��������� ********************************/
TGost::TGost(void)
{
	type = fgame_os; // �� ���������, �������� ������������� � �������� ������
	mode = b256;     // �� ���������, �������� ������ ������ - 256 ���
	SetBlock(Sb);    // �������������� ������� �����
	icount_iter = 7; // ���������� �������� ����������� ����� = 7
	SetKey(K32);     // ������������� �����
	time_t t = 0;
	time(&t);
	Seed(t);  // ������������� ���������� �� (������ ���� �����)
}
/*************** ���������� ***********************************************/
TGost::~TGost(void)
{
}
/*************** ����� ����������� ����� � �� n ����� *********************/
UINT_32 TGost::ROL(UINT_32 X, BYTE n)
{
	_asm {
		mov  eax, X
		mov  cl, n
		rol  eax, cl
		mov  X, eax
	}

	return UINT_32(X);
}
/********************* ������� ����� 2 � 32 = 64 ���� **************************/
UINT_64 TGost::SWAP32(UINT_32 N1, UINT_32 N2)
{
	UINT_64 N;
	N = N1;
	N = (N << 32) | N2;
	return UINT_64(N);
}
/********************* ������� ����� 64 ����*************************************/
UINT_64 TGost::SWAP64(UINT_64 X)
{
	UINT_64 N;
	UINT_32 N1, N2;
	N1 = UINT_32(X);     // �������
	N2 = X >> 32;         // �������
	N = N1;
	N = (N << 32) | N2;
	return N;
}
/*********************** ��������� �� ******************************************/
UINT_64 TGost::RPGCH(UINT_64 N)
{
	UINT_32 N1;
	UINT_32 N2;
	N1 = UINT_32(N);//�������
	N2 = N >> 32;  //�������
	N1 = (N1 + C1) % 0x100000000; //(x1 + c1)mod 2^32     { c1 = 1010101}
	N2 = (N2 + C2) % 0xFFFFFFFF;  //(x2 + c2)mod (2^32-1) { c2 = 1010104}
	N = N2;
	N = (N << 32) | N1;
	return N;
}

/*****************  ����������� ������� ������ - ������� ������ ****************/
UINT_32 TGost::ReplaceBlock(UINT_32 x)
{
	int i;
	UINT_32 res = 0UL;
	for (i = 7; i >= 0; i--)
	{
		ui4_0 = x >> (i * 4);
		ui4_0 = BS[ui4_0][i];
		res = (res << 4) | ui4_0;
	}
	return res;
}
/* �������� �������� ����������� ���� 28147-89 ********************************/
UINT_64 TGost::BaseKod(UINT_64 N, UINT_32 X)
{
	UINT_32 N1, N2, S = 0UL;    //��������� �������� � ������� ������
	N1 = UINT_32(N);     // �������
	N2 = N >> 32;         // �������
	S = N1 + X % 0x4000000000000;//�������� �� ������ 2^32
	S = ReplaceBlock(S);//������� ������ - �� 4 ����
	S = ROL(S, 11);    //����������� ����� �� 11 ��� � ����
	_asm {             // ����������� ��� ( S = S xor N2)
		mov eax, N2
		xor S, eax
	}
	// ������ 32-� ����� �������
	N2 = N1;
	N1 = S; // ��������� �����������
	return SWAP32(N2, N1);// ������ ������� ������� �� ������� �����  -> N_64
}
/*---   ���������� �� ���� 28147-89 -----------------------------------*/
UINT_64 TGost::CodeGost(UINT_64 N)
{
	int i, j;
	// ������ ���� - 24 �������� (3 � icount_iter)
	for (i = 0; i<3; i++)
	{
		for (j = 0; j <= icount_iter; j++)
		{
			N = BaseKod(N, K[j]);
		}
	}
	// ������ ���� - icount_iter ��������
	for (i = icount_iter; i >= 0; i--)
	{
		N = BaseKod(N, K[i]);
	}
	// ����� ���� 32 �������� - 32/8 = 4 - ���� ��������� ����� ����.
	return SWAP64(N);// �������� ������� 32-� ��������� �����, ������� 64 ����.
}
/*-- ������������� �� ���� 28147-89 -------------------------------------*/
UINT_64 TGost::DeCodeGost(UINT_64 N)
{
	int i, j;
	for (i = 0; i <= icount_iter; i++)
	{
		N = BaseKod(N, K[i]);
	}

	for (i = 0; i<3; i++)
	{
		for (int j = icount_iter; j >= 0; j--)
		{
			N = BaseKod(N, K[j]);
		}
	}

	return SWAP64(N);
}
/*-----����������� � ������������ �� ���� - �������� � 64-� �������. ������� -------------*/
UINT_64 TGost::GostGUM(UINT_64 N, UINT_64 &X)
{
	X = RPGCH(X);
	X = CodeGost(X);
	N = N^X;
	return N;
}
/*---------------- ����������� �� ���� � ������������� � �������� ������ ------------*/
UINT_64 TGost::GostGamOS(UINT_64 N, UINT_64&X, bool kode)
{
	UINT_64 T;
	T = N;
	N = N^CodeGost(X);
	X = (kode) ? N : T;
	return N;
}
/*-------------------- ���������� ������������ -----------------------------------*/
UINT_64 TGost::GetImito(UINT_64 X)
{
	// ��������� ������������.�� �� ����� ��� � ��� �������.
	BYTE j;
	UINT_32 A;
	N1 = UINT_32(X);
	N2 = X >> 32;
	for (j = 1; j <= 12; j++)
	{
		A = N2^ReplaceBlock(N1) % 0x1000000000;
		N2 = N1;
		N1 = A;
	}
	return (N2 << 32) | N1;
}

/*���������� �� ��������� ���������*/
UINT_64 TGost::KriptData(UINT_64 N, bool kode)
{
	if (kode)
		N = CodeGost(N);
	else
		N = DeCodeGost(N);
	return UINT_64(N);
}
/*---------�������� �������� ������������- ����������� �����
fname_in � fname_out, ����� �������� � ��������� �����
kript - ����, ����������� �� ����������(true) ��� ������������� (false)---------*/
UINT_32 TGost::KodeFile(const char *fname_in, const char* fname_out, bool kript)
{
	FILE *FileIn, *FileOut;
	UINT_32 nitem = 0;
	long count, fsize, readf;
	UINT_64 X = 0UL, *N;// = 0UL;
	X = SK; // ������� �������������
	if ((FileIn = fopen(fname_in, "rb")) == NULL)
	{
		return 0;
	}

	if ((FileOut = fopen(fname_out, "wb+")) == NULL)
	{
		fclose(FileIn);
		return 0;
	}
	if ((fsize = GetSizeFile(fname_in)) == 0)//�������� ������ �����
	{
		fclose(FileIn);
		fclose(FileOut);
		return 0;
	}
	nitem = fsize / 8 + (fsize % 8 >0); // ������������� ����� 64 ��� ���������
	N = (UINT_64*)calloc(nitem, sizeof(UINT_64)); // ��������� ��� ��� ������
	readf = fread(N, sizeof(UINT_64), nitem, FileIn); // ������ ���� ����, ���������� - ���� � 64 ��� ����.
	if (readf>0)
	{
		for (UINT_32 i = 0; i < nitem; i++)
		{
			N[i] = Kode(N[i], X, kript);// ��������
			count += fwrite(&N[i], sizeof(UINT_64), 1, FileOut);//����� �� 64 ���� � ����
		}
	}
L:  free(N);
	fclose(FileIn);
	fclose(FileOut);
	return count * 8;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*-----  ���������� �������, ���������� �� KodeFile------------------------------*/
UINT_64 TGost::Kode(UINT_64 N, UINT_64 &X, bool kript)
{
	UINT_64 RES = 0UL;
	switch (type)// ������������ �� ������ ����������
	{
	case fbase:RES = (kript) ? CodeGost(N) : DeCodeGost(N); break;
	case fgame:RES = GostGUM(N, X); break;
	case fgame_os:RES = GostGamOS(N, X, kript); break;
	}

	return RES; //�������������� ����

}
/*=============== ����� ��������� ����������� ================================*/
/*============ ������ ��������������� ������� ������ =========================*/
/* ������������� ������������� ***********************************************/
/******************** ���������� ����� - ������������� *************************/
UINT_64 TGost::GetSinhro()
{
	UINT_64 r = _rand32();
	r = (r << 32) | _rand32();
	return r;
}
// ������������� ������������� -----------------------------------------------
UINT_64 TGost::SetSinhro(UINT_64 X)
{
	SK = X;
	return UINT_64(SK);
}

/*************** ��������� ����� 8 � 8 = 64 = 2 � 32 ���� ****************/
bool TGost::SetKey(char* KeyCh)
{
	int i = 1;
	if (strlen(KeyCh)<8)
		return false;
	K[0] = 0UL; K[1] = 0UL;
	K[0] = (BYTE)KeyCh[0];
	for (; i < 4; i++)
		K[0] = (K[0] << 8) | (BYTE)KeyCh[i];
	K[1] = (BYTE)KeyCh[i++];
	for (; i < 8; i++)
		K[1] = (K[1] << 8) | (BYTE)KeyCh[i];
	return true;
}
/*   ��������� ����� **********************************************************/
UINT_32* TGost::GetKey(Key32 &key)
{
	int i;
	for (i = 0; i<8; i++)
		key[i] = K[i];
	return K;
}
/*--------������� ������ ��������� ---------------------------*/
void TGost::Clear(void)
{
	int i, j;
	SK = 0;
	for (i = 0; i<8; i++)
		K[i] = 0;

	for (i = 0; i<16; i++)
	{
		for (j = 0; j<8; j++)
			BS[i][j] = 0;
	}

}

/*--------- ������������� ��������� --------------------------*/
void TGost::InitGost(UINT_64 S, UINT_32 k32[], BLOCK tbl, TYPEKOD tk, MODEKOD mk)
{
	SetKey(k32);  // ��������� �����
	SetBlock(tbl);// ��������� �������
	SetType(tk);  // ��� ���������
	SetMode(mk);    // ����� ����������
	SK = S;     // �������������
}
/*--------- ������������� ��������� --------------------------*/
void TGost::InitGost(GOSTPAR gp)
{
	SetKey(gp.K);      // ��������� �����
	SetBlock(gp.BS);   // ��������� �������
	SetType(gp.type);  // ��� ���������
	SetMode(gp.mode);  // ����� ����������
	SK = gp.SP;     // �������������
}
/*--------- ������������� ��������� --------------------------*/
void TGost::InitGost()
{
	param.GetKey(K);   // ��������� �����
	param.GetTable(BS);// ��������� �������
	SetType(param.type);  // ��� ���������
	SetMode(param.mode);  // ����� ����������
	SK = param.SP;     // �������������
}
/* ��������� ����� - ������� ���� -> ���� �����*/
void TGost::SetKey(const UINT_32 a[])
{
	for (int i = 0; i<8; i++)
		K[i] = a[i];
}

/*------ ��������� ������� ����� -------------------------------*/
void TGost::SetBlock(const BLOCK b)
{
	int i, j;
	for (i = 0; i<16; i++)
	{
		for (j = 0; j<8; j++)
			BS[i][j] = b[i][j];
	}
}
/*------ ��������� ������----------------------------*/

void TGost::SetMode(MODEKOD mk)
{
	mode = mk;
	icount_iter = (mode == b256) ? 7 : 1;
}
/*----- ��������� ���� ����������-------------------------------*/
void TGost::SetType(TYPEKOD tk)
{
	type = tk;
}
/*------�������� ����� ---------------------------------*/
MODEKOD TGost::GetMode()
{
	return mode;
}
/*---- �������� ��� ���������� -----------------------------------*/
TYPEKOD TGost::GetType()
{
	return type;
}

/*-------------------------------------------------------------*/
UINT_32 TGost::GetRand32()
{
	return _rand32();
}
/*-------------------------------------------------------------*/
UINT_64 TGost::GetRand64()
{
	UINT_64 x = 0UL;
	x = _rand32();
	x = (x << 32) | _rand32();
	return x;
}

/*-------------------------------------------------------------*/
UINT_32 TGost::Rand32Char()
{
	int i = 1;
	char ch = 0;
	UINT_32 x = 0Ul;
	do {
		ch = _rand32() >> 24;
		if (isalnum(ch))
			x = ch;
	} while (x == 0);
	while (i<4)
	{
		ch = _rand32() >> 24;
		if (isalnum(ch))
		{
			x = (x << 8) | ch;
			i++;
		}
	}
	return x;
}

/*-------------------------------------------------------------*/
void TGost::InitTable(BLOCK & bl)
{
	int i, j = 0;
	BYTE b[16];

	for (i = 0; i<8; i++)
	{
		memset(b, 100, 16);
		do
		{
			ui4_0 = _rand32() >> 16;
			if (IsExistsBlock(b, ui4_0))continue;
			b[j] = ui4_0;
			bl[j++][i] = ui4_0;
		} while (j<16);
		j = 0;
	}
}

// ���������� ������� ����
BLOCK* TGost::GetTable(BLOCK & bl)
{
	int i, j;
	for (i = 0; i<16; i++)
		for (j = 0; j<8; j++)
			bl[i][j] = BS[i][j];
	return &BS;
}
/*-------------------------------------------------------------*/
void TGost::InitKey(Key32 & k)
{
	for (int i = 0; i<8; i++)
		k[i] = _rand32();
}
/*-------------------------------------------------------------*/
bool TGost::IsExistsBlock(BYTE* b, BYTE ch)
{
	int i;
	for (i = 0; i<16; i++)
		if (b[i] == ch) return true;
	return false;
}

/*        ���������� ���������� ����������      ---------------------------------------*/
bool TGost::SaveParametr(LPCSTR fname, void *data)
{
	FILE * f;
	bool ret = false;
	LPGOSTPAR p = (LPGOSTPAR)data;
	if (p == NULL)
		p = GetParamter();
	if ((f = fopen(fname, "wb+")) == NULL)
	{
		return ret;
	}

	ret = fwrite(p, sizeof(GOSTPAR), 1, f);
	fclose(f);
	return ret;
}

/*        �������� ���������� �� �����          ---------------------------------------*/
bool TGost::LoadParametr(LPCSTR fname, void *data)
{
	FILE * f;
	bool ret = false;
	LPGOSTPAR p = (LPGOSTPAR)data;
	if (p == NULL)
		p = &param;
	if ((f = fopen(fname, "rb")) == NULL)
	{
		return ret;
	}

	ret = fread(p, sizeof(GOSTPAR), 1, f);
	fclose(f);
	return ret;
}
/* ���������� ��������� */
LPGOSTPAR TGost::GetParamter()
{
	int i, j;
	param.SetKey(K);
	param.SetTable(BS);
	param.SP = SK;
	param.type = type;
	param.mode = mode;
	return &param;
}

/* ������������� ��������� */
void TGost::SetParamter(LPGOSTPAR par)
{
	param = *par;
	InitGost(param);
}
//************ ������� ************************************************
/*------------�������������� ����� (32 ����) � �������� ������ --------------------------*/
LPSTR Int32ToBin(UINT_32 X)
{
	int  i;
	char s[64];

	for (i = 0; i<32; i++)
	{
		if (((X << (i - 1)) >> 31) == 0)
			s[i] = '0';
		else
			s[i] = '1';
	}

	return (LPSTR)s;
}
/*------------�������������� ����� (64 ����) � �������� ������ --------------------------*/
LPSTR Int64ToBin(UINT_64 X)
{
	int  i;
	char s[64];

	for (i = 0; i<64; i++)
	{
		if (((X << (i - 1)) >> 63) == 0)
			s[i] = '0';
		else
			s[i] = '1';
	}
	return s;

}

/*-------------------------------------------------------------*/
LPSTR Int32ToHex(LPSTR s, UINT_32 Value)
{
	memset(s, 0, 10);
	sprintf(s, "%LX", Value);
	return (LPSTR)s;
}

/*-------------------------------------------------------------*/
LPSTR Int64ToHex(LPSTR s, UINT_64 Value)
{
	sprintf(s, "%LX", Value);
	return (LPSTR)s;
}
/*-------- �������� ������ ����� � ������ (�� ���������� windows.h)----------------------------*/
UINT_32 GetSizeFile(const char* fname)
{
	FILE* f;
	UINT_32 fsize = 0UL;
	if ((f = fopen(fname, "rb")) == NULL)
		return 0;
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	fclose(f);
	return fsize;
}
/*-------------------- ���������� ������ � UINT64 �������� - 8 ��� ----------------*/
UINT_64 StrToInt64(LPCBYTE text)
{
	UINT_64 N = 0UL;
	BYTE ch;
	for (int i = 0; i<7; i++)
	{
		if (text[i] == 0)
		{
			N = N << (i * 8);
			return N;
		}
		ch = text[i];
		N = (N | ch);
		N = N << 8;
	}
	ch = text[7];
	N = (N | ch);
	return N;
}
/*---------64 ��� ����� � ������ - �� ������ -------------------------*/
LPCSTR Int64ToStr(UINT_64 N)
{
	int i = 7;
	char buf[9] = { 0 };
	for (; i >= 0; i--)
		buf[i] = (N >> (8 * i));

	return (LPCSTR)buf;
}
/*-------------------------------------------------------------*/
LPCSTR Int32ToStr(LPSTR s, UINT_32 x)
{
	memset(s, 0, 5);
	for (int i = 3; i >= 0; i--)
		s[i] = ((x << (8 * i)) >> 24);
	return (LPCSTR)s;
}
/* ������������� ��������� ��*/
void Seed(UINT_32 dum)
{
	iran = dum;
}
/* ��������� ��  - 32 ���� ��� ����� --------------*/
UINT_32 _rand32(void)
{
	iran = 1664525L * iran + 1013904223L;
	return(iran);
}
/*  ���������� ������ � ����� - �������� �����*/
bool SaveDataFile(LPCSTR fname, void *data, size_t size)
{
	FILE * f;
	bool ret = false;
	if ((f = fopen(fname, "wb+")) == NULL)
	{
		return ret;
	}

	ret = fwrite(data, size, 1, f);
	return ret;
}
/*  ������ ������ �� ����� - �������� �����*/
bool LoadDataFile(LPCSTR fname, void *data, size_t size)
{
	FILE * f;
	bool ret = false;
	if ((f = fopen(fname, "rb")) == NULL)
	{
		return ret;
	}

	ret = fread(data, size, 1, f);
	fclose(f);
	return ret;
}