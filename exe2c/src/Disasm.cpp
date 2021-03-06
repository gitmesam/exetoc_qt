
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include <cstdio>
#include <QString>
#include <QStringList>
#include "CISC.h"

#include "dasm.h"

//**************************************

typedef union MODRM
{
    uint8_t	v;
    struct
    {
        uint8_t	rm:3;
        uint8_t	reg:3;
        uint8_t	mod:2;
    };
} *PMODRM;

typedef union SIB
{
    uint8_t	v;
    struct
    {
        uint8_t	base:3;
        uint8_t	index:3;
        uint8_t	ss:2;
    };
} *PSIB;

static const char * SegReg[8]={
    "ES","CS","SS","DS","FS","GS","??","??"
};

static const char * RegByte[8]={
    "AL","CL","DL","BL","AH","CH","DH","BH"
};

static const char * RegWord[8]={
    "AX","CX","DX","BX","SP","BP","SI","DI"
};

static const char * RegDWord[8]={
    "EAX","ECX","EDX","EBX","ESP","EBP","ESI","EDI"
};

//******************************************************

static uint32_t	U_Size	= BIT32;
//	So default disassembler is bit16
//****************
uint32_t	SegPrefix;
//index of segment register (prefix)
//default is !!!_NOSEG_!!!
uint32_t	LockPrefix;
const char *	LockName;
uint32_t	RepPrefix;
const char *	RepName;

//****************
//**************************************

static XCPUCODE	xcpu;

XCPUCODE* Disasm::get_xcpu()
{
    return &xcpu;
}


//**************************************
struct INSTRUCTION
{

    OPCODETYPE	Opcode;
    const char *	InstName;
    OPDATATYPE	Opdata1;
    OPDATATYPE	Opdata2;
    OPDATATYPE	Opdata3;
};

static const INSTRUCTION grouptable1[8] =
{
    { C_ADD,		"ADD",		D_NONE,	D_NONE,	D_NONE },
    { C_OR,			"OR",		D_NONE,	D_NONE,	D_NONE },
    { C_ADC,		"ADC",		D_NONE,	D_NONE,	D_NONE },
    { C_SBB,		"SBB",		D_NONE,	D_NONE,	D_NONE },
    { C_AND,		"AND",		D_NONE,	D_NONE,	D_NONE },
    { C_SUB,		"SUB",		D_NONE,	D_NONE,	D_NONE },
    { C_XOR,		"CMP",		D_NONE,	D_NONE,	D_NONE },
    { C_CMP,		"CMP",		D_NONE,	D_NONE,	D_NONE }
};

static const INSTRUCTION grouptable2[8] =
{
    { C_ROL,		"ROL",		D_NONE,	D_NONE,	D_NONE },
    { C_ROR,		"ROR",		D_NONE,	D_NONE,	D_NONE },
    { C_RCL,		"RCL",		D_NONE,	D_NONE,	D_NONE },
    { C_RCR,		"RCR",		D_NONE,	D_NONE,	D_NONE },
    { C_SHL,		"SHL",		D_NONE,	D_NONE,	D_NONE },
    { C_SHR,		"SHR",		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_SAR,		"SAR",		D_NONE,	D_NONE,	D_NONE }
};

static const INSTRUCTION grouptable3[8] =
{
    { C_MOV,		"MOV",		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE }
};

static const INSTRUCTION grouptable4[8] =
{
    { C_TEST,		"TEST",		D_NONE,	D_IB,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_NOT,		"NOT",		D_NONE,	D_NONE,	D_NONE },
    { C_NEG,		"NEG",		D_NONE,	D_NONE,	D_NONE },
    { C_MUL,		"MUL",		D_NONE,	D_NONE,	D_NONE },
    { C_IMUL,		"IMUL",		D_NONE,	D_NONE,	D_NONE },
    { C_DIV,		"DIV",		D_NONE,	D_NONE,	D_NONE },
    { C_IDIV,		"IDIV",		D_NONE,	D_NONE,	D_NONE }
};

static const INSTRUCTION grouptable5[8] =
{
    { C_TEST,		"TEST",		D_NONE,	D_IV,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_NOT,		"NOT",		D_NONE,	D_NONE,	D_NONE },
    { C_NEG,		"NEG",		D_NONE,	D_NONE,	D_NONE },
    { C_MUL,		"MUL",		D_NONE,	D_NONE,	D_NONE },
    { C_IMUL,		"IMUL",		D_NONE,	D_NONE,	D_NONE },
    { C_DIV,		"DIV",		D_NONE,	D_NONE,	D_NONE },
    { C_IDIV,		"IDIV",		D_NONE,	D_NONE,	D_NONE }
};

static const INSTRUCTION grouptable6[8] =
{
    { C_INC,		"INC",		D_NONE,	D_NONE,	D_NONE },
    { C_DEC,		"DEC",		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE }
};

static const INSTRUCTION grouptable7[8] =
{
    { C_INC,		"INC",		D_NONE,	D_NONE,	D_NONE },
    { C_DEC,		"DEC",		D_NONE,	D_NONE,	D_NONE },
    { C_CALL,		"CALL",		D_NONE,	D_NONE,	D_NONE },
    { C_CALLFAR,	"CALL FAR",	D_MP,	D_NONE,	D_NONE },
    { C_JMP,		"JMP",		D_NONE,	D_NONE,	D_NONE },
    { C_JMPFAR,		"JMP  FAR",	D_MP,	D_NONE,	D_NONE },
    { C_PUSH,		"PUSH",		D_NONE,	D_NONE,	D_NONE },
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE }
};

//**************************************

static const INSTRUCTION	instruction0FH[] =
{
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x00,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x01,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x02,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x03,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x04,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x05,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x06,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x07,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x08,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x09,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x0a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x0b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x0c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x0d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x0e,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x0f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x10,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x11,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x12,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x13,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x14,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x15,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x16,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x17,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x18,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x19,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x1a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x1b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x1c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x1d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x1e,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x1f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x20,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x21,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x22,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x23,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x24,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x25,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x26
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x27,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x28,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x29,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x2a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x2b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x2c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x2d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x2e
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x2f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x30,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x31,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x32,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x33,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x34,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x35,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x36
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x37,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x38,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x39,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x3a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x3b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x3c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x3d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x3e
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x3f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x40,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x41,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x42,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x43,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x44,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x45,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x46,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x47,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x48,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x49,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x4a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x4b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x4c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x4d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x4e,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x4f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x50,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x51,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x52,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x53,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x54,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x55,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x56,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x57,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x58,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x59,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x5a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x5b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x5c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x5d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x5e,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x5f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x60,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x61,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x62,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x63,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x64
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x65
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x66
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x67
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x68,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x69,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x6a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x6b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x6c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x6d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x6e,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x6f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x70,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x71,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x72,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x73,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x74,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x75,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x76,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x77,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x78,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x79,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x7a,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x7b,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x7c,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x7d,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x7e,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0x7f,
    { C_JO,			"JO",		D_JV,	D_NONE,	D_NONE },	//0x80,
    { C_JNO,		"JNO",		D_JV,	D_NONE,	D_NONE },	//0x81,
    { C_JB,			"JB",		D_JV,	D_NONE,	D_NONE },	//0x82,
    { C_JNB,		"JNB",		D_JV,	D_NONE,	D_NONE },	//0x83,
    { C_JZ,			"JZ",		D_JV,	D_NONE,	D_NONE },	//0x84,
    { C_JNZ,		"JNZ",		D_JV,	D_NONE,	D_NONE },	//0x85,
    { C_JNA,		"JNA",		D_JV,	D_NONE,	D_NONE },	//0x86,
    { C_JA,			"JA",		D_JV,	D_NONE,	D_NONE },	//0x87,
    { C_JS,			"JS",		D_JV,	D_NONE,	D_NONE },	//0x88,
    { C_JNS,		"JNS",		D_JV,	D_NONE,	D_NONE },	//0x89,
    { C_JP,			"JP",		D_JV,	D_NONE,	D_NONE },	//0x8a,
    { C_JNP,		"JNP",		D_JV,	D_NONE,	D_NONE },	//0x8b,
    { C_JL,			"JL",		D_JV,	D_NONE,	D_NONE },	//0x8c,
    { C_JNL,		"JNL",		D_JV,	D_NONE,	D_NONE },	//0x8d,
    { C_JLE,		"JLE",		D_JV,	D_NONE,	D_NONE },	//0x8e,
    { C_JNLE,		"JNLE",		D_JV,	D_NONE,	D_NONE },	//0x8f,
    { C_SETO,		"SETO",		D_EB,	D_NONE,	D_NONE },	//0x90,
    { C_SETNO,		"SETNO",	D_EB,	D_NONE,	D_NONE },	//0x91,
    { C_SETB,		"SETB",		D_EB,	D_NONE,	D_NONE },	//0x92,
    { C_SETNB,		"SETNB",	D_EB,	D_NONE,	D_NONE },	//0x93,
    { C_SETZ,		"SETZ",		D_EB,	D_NONE,	D_NONE },	//0x94,
    { C_SETNZ,		"SETNZ",	D_EB,	D_NONE,	D_NONE },	//0x95,
    { C_SETNA,		"SETNA",	D_EB,	D_NONE,	D_NONE },	//0x96,
    { C_SETA,		"SETA",		D_EB,	D_NONE,	D_NONE },	//0x97,
    { C_SETS,		"SETS",		D_EB,	D_NONE,	D_NONE },	//0x98,
    { C_SETNS,		"SETNS",	D_EB,	D_NONE,	D_NONE },	//0x99,
    { C_SETP,		"SETP",		D_EB,	D_NONE,	D_NONE },	//0x9a,
    { C_SETNP,		"SETNP",	D_EB,	D_NONE,	D_NONE },	//0x9b,
    { C_SETL,		"SETL",		D_EB,	D_NONE,	D_NONE },	//0x9c,
    { C_SETNL,		"SETNL",	D_EB,	D_NONE,	D_NONE },	//0x9d,
    { C_SETLE,		"SETLE",	D_EB,	D_NONE,	D_NONE },	//0x9e,
    { C_SETNLE,		"SETNLE",	D_EB,	D_NONE,	D_NONE },	//0x9f,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa0,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa1,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa2,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa3,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa4,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa5,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa6,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa7,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa8,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xa9,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xaa,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xab,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xac,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xad,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xae,
    //	{ C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xaf,		//----
    { C_IMUL,		"IMUL",		D_GV,	D_EV,	D_NONE },	//0xaf,		//++++
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xb0,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xb1,
    { C_LSS,		"LSS",		D_GV,	D_MP,	D_NONE },	//0xb2,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xb3,
    { C_LFS,		"LFS",		D_GV,	D_MP,	D_NONE },	//0xb4,
    { C_LGS,		"LGS",		D_GV,	D_MP,	D_NONE },	//0xb5,
    { C_MOVZX,		"MOVZX",	D_GV,	D_EB,	D_NONE },	//0xb6,
    { C_MOVZX,		"MOVZX",	D_GV,	D_EW,	D_NONE },	//0xb7,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xb8,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xb9,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xba,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xbb,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xbc,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xbd,
    { C_MOVSX,		"MOVSX",	D_GV,	D_EB,	D_NONE },	//0xbe,
    { C_MOVSX,		"MOVSX",	D_GV,	D_EW,	D_NONE },	//0xbf,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc0,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc1,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc2,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc3,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc4,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc5,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc6,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc7,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc8,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xc9,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xca,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xcb,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xcc,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xcd,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xce,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xcf,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd0,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd1,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd2,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd3,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd4,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd5,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd6,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd7,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd8,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd9,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xda,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdb,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdc,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdd,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xde,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdf,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe0
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe1
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe2
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe3
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe4
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe5
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe6
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe7
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe8
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xe9
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xea
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xeb
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xec
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xed
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xee
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xef
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf0
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf1,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf2
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf3
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf4
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf5
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd6,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd7,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf8
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf9
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xfa
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xfb
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xfc
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xfd
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xfe,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE }	//0xff,
};


static const INSTRUCTION	instruction[] =
{
    { C_ADD,		"ADD",		D_EB,	D_GB,	D_NONE },	//0x00,
    { C_ADD,		"ADD",		D_EV,	D_GV,	D_NONE },	//0x01,
    { C_ADD,		"ADD",		D_GB,	D_EB,	D_NONE },	//0x02,
    { C_ADD,		"ADD",		D_GV,	D_EV,	D_NONE },	//0x03,
    { C_ADD,		"ADD",		D_AL,	D_IB,	D_NONE },	//0x04,
    { C_ADD,		"ADD",		D_AXV,	D_IV,	D_NONE },	//0x05,
    { C_PUSH,		"PUSH",		D_ES,	D_NONE,	D_NONE },	//0x06,
    { C_POP,		"POP",		D_ES,	D_NONE,	D_NONE },	//0x07,
    { C_OR,			"OR",		D_EB,	D_GB,	D_NONE },	//0x08,
    { C_OR,			"OR",		D_EV,	D_GV,	D_NONE },	//0x09,
    { C_OR,			"OR",		D_GB,	D_EB,	D_NONE },	//0x0a,
    { C_OR,			"OR",		D_GV,	D_EV,	D_NONE },	//0x0b,
    { C_OR,			"OR",		D_AL,	D_IB,	D_NONE },	//0x0c,
    { C_OR,			"OR",		D_AXV,	D_IV,	D_NONE },	//0x0d,
    { C_PUSH,		"PUSH",		D_CS,	D_NONE,	D_NONE },	//0x0e,
    { C_0FH,(const char *)instruction0FH,D_NONE,D_NONE,	D_NONE },	//0x0f,
    { C_ADC,		"ADC",		D_EB,	D_GB,	D_NONE },	//0x10,
    { C_ADC,		"ADC",		D_EV,	D_GV,	D_NONE },	//0x11,
    { C_ADC,		"ADC",		D_GB,	D_EB,	D_NONE },	//0x12,
    { C_ADC,		"ADC",		D_GV,	D_EV,	D_NONE },	//0x13,
    { C_ADC,		"ADC",		D_AL,	D_IB,	D_NONE },	//0x14,
    { C_ADC,		"ADC",		D_AXV,	D_IV,	D_NONE },	//0x15,
    { C_PUSH,		"PUSH",		D_SS,	D_NONE,	D_NONE },	//0x16,
    { C_POP,		"POP",		D_SS,	D_NONE,	D_NONE },	//0x17,
    { C_SBB,		"SBB",		D_EB,	D_GB,	D_NONE },	//0x18,
    { C_SBB,		"SBB",		D_EV,	D_GV,	D_NONE },	//0x19,
    { C_SBB,		"SBB",		D_GB,	D_EB,	D_NONE },	//0x1a,
    { C_SBB,		"SBB",		D_GV,	D_EV,	D_NONE },	//0x1b,
    { C_SBB,		"SBB",		D_AL,	D_IB,	D_NONE },	//0x1c,
    { C_SBB,		"SBB",		D_AXV,	D_IV,	D_NONE },	//0x1d,
    { C_PUSH,		"PUSH",		D_DS,	D_NONE,	D_NONE },	//0x1e,
    { C_POP,		"POP",		D_DS,	D_NONE,	D_NONE },	//0x1f,
    { C_AND,		"AND",		D_EB,	D_GB,	D_NONE },	//0x20,
    { C_AND,		"AND",		D_EV,	D_GV,	D_NONE },	//0x21,
    { C_AND,		"AND",		D_GB,	D_EB,	D_NONE },	//0x22,
    { C_AND,		"AND",		D_GV,	D_EV,	D_NONE },	//0x23,
    { C_AND,		"AND",		D_AL,	D_IB,	D_NONE },	//0x24,
    { C_AND,		"AND",		D_AXV,	D_IV,	D_NONE },	//0x25,
    { C_SEGPREFIX,	"ES:",		OPDATATYPE(_ES_),	D_NONE,	D_NONE },	//0x26
    { C_DAA,		"DAA",		D_NONE,	D_NONE,	D_NONE },	//0x27,
    { C_SUB,		"SUB",		D_EB,	D_GB,	D_NONE },	//0x28,
    { C_SUB,		"SUB",		D_EV,	D_GV,	D_NONE },	//0x29,
    { C_SUB,		"SUB",		D_GB,	D_EB,	D_NONE },	//0x2a,
    { C_SUB,		"SUB",		D_GV,	D_EV,	D_NONE },	//0x2b,
    { C_SUB,		"SUB",		D_AL,	D_IB,	D_NONE },	//0x2c,
    { C_SUB,		"SUB",		D_AXV,	D_IV,	D_NONE },	//0x2d,
    { C_SEGPREFIX,	"CS:",		OPDATATYPE(_CS_),	D_NONE,	D_NONE },	//0x2e
    { C_DAS,		"DAS",		D_NONE,	D_NONE,	D_NONE },	//0x2f,
    { C_XOR,		"XOR",		D_EB,	D_GB,	D_NONE },	//0x30,
    { C_XOR,		"XOR",		D_EV,	D_GV,	D_NONE },	//0x31,
    { C_XOR,		"XOR",		D_GB,	D_EB,	D_NONE },	//0x32,
    { C_XOR,		"XOR",		D_GV,	D_EV,	D_NONE },	//0x33,
    { C_XOR,		"XOR",		D_AL,	D_IB,	D_NONE },	//0x34,
    { C_XOR,		"XOR",		D_AXV,	D_IV,	D_NONE },	//0x35,
    { C_SEGPREFIX,	"SS:",		OPDATATYPE(_SS_),	D_NONE,	D_NONE },	//0x36
    { C_AAA,		"AAA",		D_NONE,	D_NONE,	D_NONE },	//0x37,
    { C_CMP,		"CMP",		D_EB,	D_GB,	D_NONE },	//0x38,
    { C_CMP,		"CMP",		D_EV,	D_GV,	D_NONE },	//0x39,
    { C_CMP,		"CMP",		D_GB,	D_EB,	D_NONE },	//0x3a,
    { C_CMP,		"CMP",		D_GV,	D_EV,	D_NONE },	//0x3b,
    { C_CMP,		"CMP",		D_AL,	D_IB,	D_NONE },	//0x3c,
    { C_CMP,		"CMP",		D_AXV,	D_IV,	D_NONE },	//0x3d,
    { C_SEGPREFIX,	"DS:",		OPDATATYPE(_DS_),	D_NONE,	D_NONE },	//0x3e
    { C_AAS,		"AAS",		D_NONE,	D_NONE,	D_NONE },	//0x3f,
    { C_INC,		"INC",		D_AXV,	D_NONE,	D_NONE },	//0x40,
    { C_INC,		"INC",		D_CXV,	D_NONE,	D_NONE },	//0x41,
    { C_INC,		"INC",		D_DXV,	D_NONE,	D_NONE },	//0x42,
    { C_INC,		"INC",		D_BXV,	D_NONE,	D_NONE },	//0x43,
    { C_INC,		"INC",		D_SPV,	D_NONE,	D_NONE },	//0x44,
    { C_INC,		"INC",		D_BPV,	D_NONE,	D_NONE },	//0x45,
    { C_INC,		"INC",		D_SIV,	D_NONE,	D_NONE },	//0x46,
    { C_INC,		"INC",		D_DIV,	D_NONE,	D_NONE },	//0x47,
    { C_DEC,		"DEC",		D_AXV,	D_NONE,	D_NONE },	//0x48,
    { C_DEC,		"DEC",		D_CXV,	D_NONE,	D_NONE },	//0x49,
    { C_DEC,		"DEC",		D_DXV,	D_NONE,	D_NONE },	//0x4a,
    { C_DEC,		"DEC",		D_BXV,	D_NONE,	D_NONE },	//0x4b,
    { C_DEC,		"DEC",		D_SPV,	D_NONE,	D_NONE },	//0x4c,
    { C_DEC,		"DEC",		D_BPV,	D_NONE,	D_NONE },	//0x4d,
    { C_DEC,		"DEC",		D_SIV,	D_NONE,	D_NONE },	//0x4e,
    { C_DEC,		"DEC",		D_DIV,	D_NONE,	D_NONE },	//0x4f,
    { C_PUSH,		"PUSH",		D_AXV,	D_NONE,	D_NONE },	//0x50,
    { C_PUSH,		"PUSH",		D_CXV,	D_NONE,	D_NONE },	//0x51,
    { C_PUSH,		"PUSH",		D_DXV,	D_NONE,	D_NONE },	//0x52,
    { C_PUSH,		"PUSH",		D_BXV,	D_NONE,	D_NONE },	//0x53,
    { C_PUSH,		"PUSH",		D_SPV,	D_NONE,	D_NONE },	//0x54,
    { C_PUSH,		"PUSH",		D_BPV,	D_NONE,	D_NONE },	//0x55,
    { C_PUSH,		"PUSH",		D_SIV,	D_NONE,	D_NONE },	//0x56,
    { C_PUSH,		"PUSH",		D_DIV,	D_NONE,	D_NONE },	//0x57,
    { C_POP,		"POP",		D_AXV,	D_NONE,	D_NONE },	//0x58,
    { C_POP,		"POP",		D_CXV,	D_NONE,	D_NONE },	//0x59,
    { C_POP,		"POP",		D_DXV,	D_NONE,	D_NONE },	//0x5a,
    { C_POP,		"POP",		D_BXV,	D_NONE,	D_NONE },	//0x5b,
    { C_POP,		"POP",		D_SPV,	D_NONE,	D_NONE },	//0x5c,
    { C_POP,		"POP",		D_BPV,	D_NONE,	D_NONE },	//0x5d,
    { C_POP,		"POP",		D_SIV,	D_NONE,	D_NONE },	//0x5e,
    { C_POP,		"POP",		D_DIV,	D_NONE,	D_NONE },	//0x5f,
    { C_PUSHA,		"PUSHA",	D_V,	D_NONE,	D_NONE },	//0x60,
    { C_POPA,		"POPA",		D_V,	D_NONE,	D_NONE },	//0x61,
    { C_BOUND,		"BOUND",	D_GV,	D_MA,	D_NONE },	//0x62,
    { C_ARPL,		"ARPL",		D_EW,	D_GW,	D_NONE },	//0x63,
    { C_SEGPREFIX,	"FS:",		OPDATATYPE(_FS_),	D_NONE,	D_NONE },	//0x64
    { C_SEGPREFIX,	"GS:",		OPDATATYPE(_GS_),	D_NONE,	D_NONE },	//0x65
    { C_OPRSIZE,	NULL,		D_NONE,	D_NONE,	D_NONE },	//0x66
    { C_ADRSIZE,	NULL,		D_NONE,	D_NONE,	D_NONE },	//0x67
    { C_PUSH,		"PUSH",		D_IV,	D_NONE,	D_NONE },	//0x68,
    { C_IMUL,		"IMUL",		D_GV,	D_EV,	D_IV   },	//0x69,
    { C_PUSH,		"PUSH",		D_SB,	D_NONE,	D_NONE },	//0x6a,
    { C_IMUL,		"IMUL",		D_GV,	D_EV,	D_IB   },	//0x6b,
    { C_INS,		"INS",		D_XB,	D_DX,	D_NONE },	//0x6c,
    { C_INS,		"INS",		D_XV,	D_DX,	D_NONE },	//0x6d,
    { C_OUTS,		"OUTS",		D_DX,	D_XB,	D_NONE },	//0x6e,
    { C_OUTS,		"OUTS",		D_DX,	D_XV,	D_NONE },	//0x6f,
    { C_JO,			"JO",		D_JB,	D_NONE,	D_NONE },	//0x70,
    { C_JNO,		"JNO",		D_JB,	D_NONE,	D_NONE },	//0x71,
    { C_JB,			"JB",		D_JB,	D_NONE,	D_NONE },	//0x72,
    { C_JNB,		"JNB",		D_JB,	D_NONE,	D_NONE },	//0x73,
    { C_JZ,			"JZ",		D_JB,	D_NONE,	D_NONE },	//0x74,
    { C_JNZ,		"JNZ",		D_JB,	D_NONE,	D_NONE },	//0x75,
    { C_JNA,		"JNA",		D_JB,	D_NONE,	D_NONE },	//0x76,
    { C_JA,			"JA",		D_JB,	D_NONE,	D_NONE },	//0x77,
    { C_JS,			"JS",		D_JB,	D_NONE,	D_NONE },	//0x78,
    { C_JNS,		"JNS",		D_JB,	D_NONE,	D_NONE },	//0x79,
    { C_JP,			"JP",		D_JB,	D_NONE,	D_NONE },	//0x7a,
    { C_JNP,		"JNP",		D_JB,	D_NONE,	D_NONE },	//0x7b,
    { C_JL,			"JL",		D_JB,	D_NONE,	D_NONE },	//0x7c,
    { C_JNL,		"JNL",		D_JB,	D_NONE,	D_NONE },	//0x7d,
    { C_JLE,		"JLE",		D_JB,	D_NONE,	D_NONE },	//0x7e,
    { C_JNLE,		"JNLE",		D_JB,	D_NONE,	D_NONE },	//0x7f,
    { C_GROUP,(const char *)grouptable1,D_EB,	D_IB,	D_NONE },	//0x80,
    { C_GROUP,(const char *)grouptable1,D_EV,	D_IV,	D_NONE },	//0x81,
    { C_GROUP,(const char *)grouptable1,D_EB,	D_IB,	D_NONE },	//0x82,
    { C_GROUP,(const char *)grouptable1,D_EV,	D_SB,	D_NONE },	//0x83,
    { C_TEST,		"TEST",		D_EB,	D_GB,	D_NONE },	//0x84,
    { C_TEST,		"TEST",		D_EV,	D_GV,	D_NONE },	//0x85,
    { C_XCHG,		"XCHG",		D_EB,	D_GB,	D_NONE },	//0x86,
    { C_XCHG,		"XCHG",		D_EV,	D_GV,	D_NONE },	//0x87,
    { C_MOV,		"MOV",		D_EB,	D_GB,	D_NONE },	//0x88,
    { C_MOV,		"MOV",		D_EV,	D_GV,	D_NONE },	//0x89,
    { C_MOV,		"MOV",		D_GB,	D_EB,	D_NONE },	//0x8a,
    { C_MOV,		"MOV",		D_GV,	D_EV,	D_NONE },	//0x8b,
    { C_MOV,		"MOV",		D_EW,	D_SW,	D_NONE },	//0x8c,
    { C_LEA,		"LEA",		D_GV,	D_MV,	D_NONE },	//0x8d,
    { C_MOV,		"MOV",		D_SW,	D_EW,	D_NONE },	//0x8e,
    { C_POP,		"POP",		D_EV,	D_NONE,	D_NONE },	//0x8f,
    { C_NOP,		"NOP",		D_NONE,	D_NONE,	D_NONE },	//0x90,
    { C_XCHG,		"XCHG",		D_CXV,	D_AXV,	D_NONE },	//0x91,
    { C_XCHG,		"XCHG",		D_DXV,	D_AXV,	D_NONE },	//0x92,
    { C_XCHG,		"XCHG",		D_BXV,	D_AXV,	D_NONE },	//0x93,
    { C_XCHG,		"XCHG",		D_SPV,	D_AXV,	D_NONE },	//0x94,
    { C_XCHG,		"XCHG",		D_BPV,	D_AXV,	D_NONE },	//0x95,
    { C_XCHG,		"XCHG",		D_SIV,	D_AXV,	D_NONE },	//0x96,
    { C_XCHG,		"XCHG",		D_DIV,	D_AXV,	D_NONE },	//0x97,
    { C_CBW,		"CBW",		D_NONE,	D_NONE,	D_NONE },	//0x98,
    { C_CWD,		"CWD",		D_NONE,	D_NONE,	D_NONE },	//0x99,
    { C_CALLFAR,	"CALL",		D_JP,	D_NONE,	D_NONE },	//0x9a,
    { C_WAIT,		"WAIT",		D_NONE,	D_NONE,	D_NONE },	//0x9b,
    { C_PUSHF,		"PUSHF",	D_V,	D_NONE,	D_NONE },	//0x9c,
    { C_POPF,		"POPF",		D_V,	D_NONE,	D_NONE },	//0x9d,
    { C_SAHF,		"SAHF",		D_V,	D_NONE,	D_NONE },	//0x9e,
    { C_LAHF,		"LAHF",		D_V,	D_NONE,	D_NONE },	//0x9f,
    { C_MOV,		"MOV",		D_AL,	D_OB,	D_NONE },	//0xa0,
    { C_MOV,		"MOV",		D_AXV,	D_OV,	D_NONE },	//0xa1,
    { C_MOV,		"MOV",		D_OB,	D_AL,	D_NONE },	//0xa2,
    { C_MOV,		"MOV",		D_OV,	D_AXV,	D_NONE },	//0xa3,
    { C_MOVS,		"MOVS",		D_XB,	D_NONE,	D_NONE },	//0xa4,
    { C_MOVS,		"MOVS",		D_XV,	D_NONE,	D_NONE },	//0xa5,
    { C_CMPS,		"CMPS",		D_XB,	D_NONE,	D_NONE },	//0xa6,
    { C_CMPS,		"CMPS",		D_XV,	D_NONE,	D_NONE },	//0xa7,
    { C_TEST,		"TEST",		D_AL,	D_IB,	D_NONE },	//0xa8,
    { C_TEST,		"TEST",		D_AXV,	D_IV,	D_NONE },	//0xa9,
    { C_STOS,		"STOS",		D_XB,	D_NONE,	D_NONE },	//0xaa,
    { C_STOS,		"STOS",		D_XV,	D_NONE,	D_NONE },	//0xab,
    { C_LODS,		"LODS",		D_XB,	D_NONE,	D_NONE },	//0xac,
    { C_LODS,		"LODS",		D_XV,	D_NONE,	D_NONE },	//0xad,
    { C_SCAS,		"SCAS",		D_XB,	D_NONE,	D_NONE },	//0xae,
    { C_SCAS,		"SCAS",		D_XV,	D_NONE,	D_NONE },	//0xaf,
    { C_MOV,		"MOV",		D_AL,	D_IB,	D_NONE },	//0xb0,
    { C_MOV,		"MOV",		D_CL,	D_IB,	D_NONE },	//0xb1,
    { C_MOV,		"MOV",		D_DL,	D_IB,	D_NONE },	//0xb2,
    { C_MOV,		"MOV",		D_BL,	D_IB,	D_NONE },	//0xb3,
    { C_MOV,		"MOV",		D_AH,	D_IB,	D_NONE },	//0xb4,
    { C_MOV,		"MOV",		D_CH,	D_IB,	D_NONE },	//0xb5,
    { C_MOV,		"MOV",		D_DH,	D_IB,	D_NONE },	//0xb6,
    { C_MOV,		"MOV",		D_BH,	D_IB,	D_NONE },	//0xb7,
    { C_MOV,		"MOV",		D_AXV,	D_IV,	D_NONE },	//0xb8,
    { C_MOV,		"MOV",		D_CXV,	D_IV,	D_NONE },	//0xb9,
    { C_MOV,		"MOV",		D_DXV,	D_IV,	D_NONE },	//0xba,
    { C_MOV,		"MOV",		D_BXV,	D_IV,	D_NONE },	//0xbb,
    { C_MOV,		"MOV",		D_SPV,	D_IV,	D_NONE },	//0xbc,
    { C_MOV,		"MOV",		D_BPV,	D_IV,	D_NONE },	//0xbd,
    { C_MOV,		"MOV",		D_SIV,	D_IV,	D_NONE },	//0xbe,
    { C_MOV,		"MOV",		D_DIV,	D_IV,	D_NONE },	//0xbf,
    { C_GROUP,(const char *)grouptable2,D_EB,	D_IB,	D_NONE },	//0xc0,
    { C_GROUP,(const char *)grouptable2,D_EV,	D_IB,	D_NONE },	//0xc1,
    { C_RET,		"RET",		D_IW,	D_NONE,	D_NONE },	//0xc2,
    { C_RET,		"RET",		D_NONE,	D_NONE,	D_NONE },	//0xc3,
    { C_LES,		"LES",		D_GV,	D_MP,	D_NONE },	//0xc4,
    { C_LDS,		"LDS",		D_GV,	D_MP,	D_NONE },	//0xc5,
    { C_GROUP,(const char *)grouptable3,D_EB,	D_IB,	D_NONE },	//0xc6,
    { C_GROUP,(const char *)grouptable3,D_EV,	D_IV,	D_NONE },	//0xc7,
    { C_ENTER,		"ENTER",	D_IW,	D_IB,	D_NONE },	//0xc8,
    { C_LEAVE,		"LEAVE",	D_NONE,	D_NONE,	D_NONE },	//0xc9,
    { C_RETF,		"RETF",		D_IW,	D_NONE,	D_NONE },	//0xca,
    { C_RETF,		"RETF",		D_NONE,	D_NONE,	D_NONE },	//0xcb,
    { C_INT3,		"INT3",		D_NONE,	D_NONE,	D_NONE },	//0xcc,
    { C_INT,		"INT",		D_IB,	D_NONE,	D_NONE },	//0xcd,
    { C_INTO,		"INTO",		D_NONE,	D_NONE,	D_NONE },	//0xce,
    { C_IRET,		"IRET",		D_NONE,	D_NONE,	D_NONE },	//0xcf,
    { C_GROUP,(const char *)grouptable2,D_EB,	D_1,	D_NONE },	//0xd0,
    { C_GROUP,(const char *)grouptable2,D_EV,	D_1,	D_NONE },	//0xd1,
    { C_GROUP,(const char *)grouptable2,D_EB,	D_CL,	D_NONE },	//0xd2,
    { C_GROUP,(const char *)grouptable2,D_EV,	D_CL,	D_NONE },	//0xd3,
    { C_AAM,		"AAM",		D_IB,	D_NONE,	D_NONE },	//0xd4,
    { C_AAD,		"AAD",		D_IB,	D_NONE,	D_NONE },	//0xd5,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd6,
    { C_XLAT,		"XLAT",		D_NONE,	D_NONE,	D_NONE },	//0xd7,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd8,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xd9,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xda,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdb,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdc,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdd,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xde,
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xdf,
    { C_LOOPNZ,		"LOOPNZ",	D_JB,	D_NONE,	D_NONE },	//0xe0
    { C_LOOPZ,		"LOOPZ",	D_JB,	D_NONE,	D_NONE },	//0xe1
    { C_LOOP,		"LOOP",		D_JB,	D_NONE,	D_NONE },	//0xe2
    { C_JCXZ,		"JCXZ",		D_JB,	D_NONE,	D_NONE },	//0xe3
    { C_IN,			"IN",		D_AL,	D_IB,	D_NONE },	//0xe4
    { C_IN,			"IN",		D_AXV,	D_IB,	D_NONE },	//0xe5
    { C_OUT,		"OUT",		D_IB,	D_AL,	D_NONE },	//0xe6
    { C_OUT,		"OUT",		D_IB,	D_AXV,	D_NONE },	//0xe7
    { C_CALL,		"CALL",		D_JV,	D_NONE,	D_NONE },	//0xe8
    { C_JMP,		"JMP",		D_JV,	D_NONE,	D_NONE },	//0xe9
    { C_JMPFAR,		"JMP",		D_JP,	D_NONE,	D_NONE },	//0xea
    { C_JMP,		"JMP",		D_JB,	D_NONE,	D_NONE },	//0xeb
    { C_IN,			"IN",		D_AL,	D_DX,	D_NONE },	//0xec
    { C_IN,			"IN",		D_AXV,	D_DX,	D_NONE },	//0xed
    { C_OUT,		"OUT",		D_DX,	D_AL,	D_NONE },	//0xee
    { C_OUT,		"OUT",		D_DX,	D_AXV,	D_NONE },	//0xef
    { C_LOCK,		"LOCK",		D_NONE,	D_NONE,	D_NONE },	//0xf0
    { C_ERROR,		NULL,		D_NONE,	D_NONE,	D_NONE },	//0xf1,
    { C_REPNZ,		"REPNZ",	D_NONE,	D_NONE,	D_NONE },	//0xf2
    { C_REPZ,		"REPZ",		D_NONE,	D_NONE,	D_NONE },	//0xf3
    { C_HLT,		"HLT",		D_NONE,	D_NONE,	D_NONE },	//0xf4
    { C_CMC,		"CMC",		D_NONE,	D_NONE,	D_NONE },	//0xf5
    { C_GROUP,(const char *)grouptable4,D_EB,	D_NONE,	D_NONE },	//0xf6,
    { C_GROUP,(const char *)grouptable5,D_EV,	D_NONE,	D_NONE },	//0xf7,
    { C_CLC,		"CLC",		D_NONE,	D_NONE,	D_NONE },	//0xf8
    { C_STC,		"STC",		D_NONE,	D_NONE,	D_NONE },	//0xf9
    { C_CLI,		"CLI",		D_NONE,	D_NONE,	D_NONE },	//0xfa
    { C_STI,		"STI",		D_NONE,	D_NONE,	D_NONE },	//0xfb
    { C_CLD,		"CLD",		D_NONE,	D_NONE,	D_NONE },	//0xfc
    { C_STD,		"STD",		D_NONE,	D_NONE,	D_NONE },	//0xfd
    { C_GROUP,(const char *)grouptable6,D_EB,	D_NONE,	D_NONE },	//0xfe,
    { C_GROUP,(const char *)grouptable7,D_EV,	D_NONE,	D_NONE }	//0xff,
};

//******************************************************


void st_IDA_OUT::output(QString & buf)
{
    QStringList bld;
    if (!this->LockName.empty())
        buf += QString("%1 ").arg(this->LockName.c_str());
    if (!this->RepName.empty())
        buf += QString("%1 ").arg(this->RepName.c_str());
    if (!this->CmdStr.empty())
    {
        buf += QString("%1 ").arg(this->CmdStr.c_str());;
        buf=buf.leftJustified(8,' ');
    }
    //par1
    buf += (this->Par1Ptr + this->Par1SegPrefix + this->Par1Str).c_str();
    if(has_param2())
        bld.append(QString(',') + (this->Par2Ptr + this->Par2SegPrefix + this->Par2Str).c_str());
    if(has_param3())
        bld.append(QString(',') + (this->Par3Str.c_str()));
    buf += bld.join(",");
}
uint32_t   Disasm::Disassembler_X(uint8_t * codebuf, uint32_t eip, st_IDA_OUT* idaout)
{
    //    if (memcmp(codebuf, "\x0F\xAF\xC0", 3) == 0)
    //    {
    //       memcmp(codebuf, "\x0F\xAF\xC0", 3);	//imul eax, eax
    //   }
    m_idaout = idaout;

    UasmCode	= codebuf;
    BaseAddress	= eip;

    // Initialize data
    CodeCount	= 0;       /*  It's code's number.   */
    OpSize		= U_Size;
    AdrSize		= U_Size;
    SegPrefix	= _NOSEG_;
    LockPrefix	= NOPREFIX;
    RepPrefix	= NOPREFIX;
    U_ErrorCode	= ERR_NOERROR;

    DisassemblerOne();

    // Has any error?
    if (U_ErrorCode != ERR_NOERROR)
    {
        this->m_idaout->CmdStr = "INVALID_CODE";
        return 1;
    }

    return CodeCount;
}
/*
void proc1();
int main(int argc,char** argv)
{
    proc1();
    return 0;

    if(argc<2)
    {
        dbgout("Usage: disasm file.com\n");
        return 1;
    }

    char	buf[128];
        FILE *fp = fopen(argv[1],"rb");

        uint32_t	filelen	= filelength(fileno(fp));
        uint8_t *	codebuf = new uint8_t[filelen+1];

        fread(codebuf,filelen,1,fp);
        fclose(fp);

        Set_Disassembler_Mode(BIT32);

        for(uint32_t len=0;len < filelen;)
        {
        uint32_t pos = len+0x100;
                len += Disassembler(buf,codebuf+len,pos);
                dbgout("%08x %s\n",pos,buf);
        }

        delete codebuf;

        return 0;
}
*/

//******************************************************

void	Disasm::OpSizePrefix()
{
    if( OpSize==BIT16 )
        OpSize=BIT32;
    else
        OpSize=BIT16;
}

void	Disasm::AdrSizePrefix()
{
    if( AdrSize==BIT16 )
        AdrSize=BIT32;
    else
        AdrSize=BIT16;
}

/***********************************************************/

/*
void	SetName(char * name)
{
        Putchs(name);
        SetBufPtrTox2();
}*/

uint8_t	PeekB(const uint8_t * codebuf)
{
    return *codebuf;
}

uint16_t	PeekW(const uint8_t * codebuf)
{
    return *(uint16_t *)codebuf;
}

uint32_t	PeekD(const uint8_t * codebuf)
{
    return *(uint32_t *)codebuf;
}

uint8_t Disasm::GetByte()
{
    return *(UasmCode+CodeCount);
}

uint16_t Disasm::GetWord()
{
    return *(uint16_t*)(UasmCode+CodeCount);
}

uint32_t	Disasm::GetDWord()
{
    return *(uint32_t*)(UasmCode+CodeCount);
}

uint8_t Disasm::GetByteEx()
{
    uint8_t	v = *(UasmCode+CodeCount);
    CodeCount++;
    return v;
}

uint16_t Disasm::GetWordEx()
{
    uint16_t	v = *(uint16_t*)(UasmCode+CodeCount);
    CodeCount+=2;
    return v;
}

uint32_t	Disasm::GetDWordEx()
{
    uint32_t	v = *(uint32_t*)(UasmCode+CodeCount);
    CodeCount+=4;
    return v;
}

/***********************************************************/

void	Disasm::SetError(uint32_t errcode)
{
    U_ErrorCode = errcode;
}

/***********************************************************/
// Global Function
// Set all of the field(s) of OPERITEM except **RWFLAG** and **SEG_INDEX**

uint8_t Disasm::Global_GetSize(enum_SizeKind srcsize)
{
    switch(srcsize)
    {
        case SIZE_B:
            return 1;

        case SIZE_V:
            if (OpSize == BIT16)
                return 2;
            else
                return 4;

        case SIZE_W:
            return 2;

        case SIZE_D:
            return 4;

        case SIZE_A:
            return 8;

        case SIZE_P:
            if (U_Size == BIT16)
                return 4;
            else if (OpSize == BIT16)
                return 4;
            else
                return 6;
    }
    return 0;
}

uint32_t Global_SIB(char * outbuf,uint8_t * codebuf,OPERITEM *op,uint32_t mod)
{
    SIB sib;

    sib.v=PeekB(codebuf);

    char buf[128];

    op->addr.base_reg_index=sib.base;
    op->addr.off_reg_index=sib.index;
    if (sib.ss==0)
        op->addr.off_reg_scale=1;
    else if (sib.ss==1)
        op->addr.off_reg_scale=2;
    else if (sib.ss==2)
        op->addr.off_reg_scale=4;
    else
        op->addr.off_reg_scale=8;

    // I have some mistakes! The manual say if index equal 4 there are no any scale register,
    // but I understood that's invalid code at first.
    if (sib.index == 4)
    {
        op->addr.off_reg_index	= _NOREG_;
        op->addr.off_reg_scale	= 0;

        if (sib.base == 5 && mod == 0)
        {
            op->addr.base_reg_index	= _NOREG_;
            op->addr.off_value		= PeekD(codebuf + 1);
            sprintf(buf, "[%08lX]", op->addr.off_value);
            return 5;
        }

        sprintf(buf, "%s", RegDWord[sib.base]);
    }
    else
    {
        if (sib.base == 5 && mod == 0)
        {
            op->addr.base_reg_index	= _NOREG_;
            op->addr.off_value		= PeekD(codebuf + 1);
            sprintf(outbuf, "[%s*%d+%08lX]",
                    RegDWord[op->addr.off_reg_index],
                    op->addr.off_reg_scale,
                    op->addr.off_value
                    );
            return 5;
        }

        sprintf(buf,"%s+%s*%d",
                RegDWord[sib.base],
                RegDWord[sib.index],
                op->addr.off_reg_scale
                );
    }

    if (mod == 1)
    {
        op->addr.off_value=PeekB(codebuf+1);
        sprintf(outbuf,"[%s+%02lX]",buf,op->addr.off_value);
        return 2;
    }
    else if (mod == 2)
    {	//Now mod is 2
        op->addr.off_value=PeekD(codebuf+1);
        sprintf(outbuf,"[%s+%08lX]",buf,op->addr.off_value);
        return 5;
    }
    else
    {
        sprintf(outbuf,"[%s]",buf);
        return 1;
    }
}

uint32_t Disasm::Global_MEMORY(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    MODRM	modrm;
    uint32_t	len=1;

    //uint8_t	by;
    //uint16_t	wo;
    //uint32_t	dw;

    modrm.v	= PeekB(codebuf);

    if( AdrSize==BIT32 )
    {
        op->mode		= OP_Address;
        //op->addr.reg_size=sizeof(uint32_t);
        op->addr.base_reg_index=modrm.rm;
        op->addr.off_reg_index=_NOREG_;
        op->addr.off_reg_scale=1;
        op->addr.off_value=0;

        if (modrm.rm==4)
            return Global_SIB(outbuf,codebuf+1,op,modrm.mod)+len;

        if (modrm.rm==5&&modrm.mod==0)
        {
            op->addr.base_reg_index=_NOREG_;
            op->addr.off_value=PeekD(codebuf+1);
            sprintf(outbuf,"[%08lX]",op->addr.off_value);
            len+=4;
            return len;
        }

        if (modrm.mod==0)
        {
            sprintf(outbuf,"[%s]",RegDWord[modrm.rm]);
        }
        else if (modrm.mod==1)
        {
            signed char signoffset = (signed char)PeekB(codebuf+1);
            op->addr.off_value=(uint32_t)(signed int)signoffset;
            if (signoffset < 0)
                sprintf(outbuf,"[%s-%02X]",RegDWord[modrm.rm],-signoffset);
            else
                sprintf(outbuf,"[%s+%02X]",RegDWord[modrm.rm],signoffset);
            len+=1;
        }
        else
        {	//mod is 2
            op->addr.off_value=PeekD(codebuf+1);
            sprintf(outbuf,"[%s+%08lX]",RegDWord[modrm.rm],op->addr.off_value);
            len+=4;
        }
    }
    else
    {
        op->mode		= OP_Address;
        //op->addr.reg_size=sizeof(uint16_t);
        op->addr.base_reg_index=_NOREG_;
        op->addr.off_reg_index=_NOREG_;
        op->addr.off_reg_scale=1;
        op->addr.off_value=0;

        if (modrm.rm==6&&modrm.mod==0)
        {
            op->addr.off_value=PeekW(codebuf+1);
            sprintf(outbuf,"[%04lX]",op->addr.off_value);
            len+=2;
            return len;
        }

        const char *reg16off;
        switch(modrm.rm)
        {
            case 0:
                op->addr.base_reg_index=_BX_;
                op->addr.off_reg_index=_SI_;
                reg16off="BX+SI";
                break;
            case 1:
                op->addr.base_reg_index=_BX_;
                op->addr.off_reg_index=_DI_;
                reg16off="BX+DI";
                break;
            case 2:
                op->addr.base_reg_index=_BP_;
                op->addr.off_reg_index=_SI_;
                reg16off="BP+SI";
                break;
            case 3:
                op->addr.base_reg_index=_BP_;
                op->addr.off_reg_index=_DI_;
                reg16off="BP+DI";
                break;
            case 4:
                op->addr.base_reg_index=_SI_;
                reg16off="SI";
                break;
            case 5:
                op->addr.base_reg_index=_DI_;
                reg16off="DI";
                break;
            case 6:
                op->addr.base_reg_index=_BP_;
                reg16off="BP";
                break;
            case 7:
                op->addr.base_reg_index=_BX_;
                reg16off="BX";
        }

        if (modrm.mod==0)
        {
            sprintf(outbuf,"[%s]",reg16off);
        }
        else if (modrm.mod==1)
        {
            signed char signoffset = (signed char)PeekB(codebuf+1);
            op->addr.off_value=(uint32_t)(signed int)signoffset;
            if (signoffset < 0)
                sprintf(outbuf,"[%s-%02X]",reg16off,-signoffset);
            else
                sprintf(outbuf,"[%s+%02X]",reg16off,signoffset);
            len+=1;
        }
        else
        {	//mod is 2
            op->addr.off_value=PeekW(codebuf+1);
            sprintf(outbuf,"[%s+%04lX]",reg16off,op->addr.off_value);
            len+=2;
        }
    }
    return len;
}

uint32_t Disasm::Global_MODRM(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    MODRM	modrm;
    modrm.v	= PeekB(codebuf);

    if (modrm.mod==3)
    {
        op->mode = OP_Register;
        const char ** register_name_table;
        switch(op->opersize)
        {
            case 1: register_name_table=RegByte; break;
            case 2: register_name_table=RegWord; break;
            case 4:
            default:
                register_name_table=RegDWord; break;
        }
        strcpy(outbuf,register_name_table[modrm.rm]);
        *((llvm::MCOperand *)op) = llvm::MCOperand::CreateReg(modrm.rm);
        return 1;
    }
    return Global_MEMORY(outbuf,codebuf,op);
}

uint32_t Disasm::Global_OFFSET(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    op->mode=OP_Address;

    //op->addr.reg_size		= 0;
    op->addr.base_reg_index	= _NOREG_;
    op->addr.off_reg_index	= _NOREG_;
    op->addr.off_reg_scale	= 0;

    if (AdrSize == BIT16)
    {
        op->addr.off_value	= PeekW(codebuf);
        sprintf(outbuf,"[%04lX]",op->addr.off_value);
        return sizeof(uint16_t);
    }
    else
    {
        op->addr.off_value	= PeekW(codebuf);
        sprintf(outbuf,"[%08lX]",op->addr.off_value);
        return sizeof(uint32_t);
    }
}

uint32_t Global_REG(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    MODRM	modrm;

    modrm.v=PeekB(codebuf);

    op->mode = OP_Register;

    if (op->opersize == 1)
    {
        strcpy(outbuf,RegByte[modrm.reg]);
    }
    else if (op->opersize == 2)
    {
        strcpy(outbuf,RegWord[modrm.reg]);
    }
    else
    {
        strcpy(outbuf,RegDWord[modrm.reg]);
    }
    *((llvm::MCOperand *)op) = llvm::MCOperand::CreateReg(modrm.reg);

    return 0;
}

uint32_t Global_SREG(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    MODRM	modrm;
    modrm.v	= PeekB(codebuf);

    op->mode = OP_Segment;
    strcpy(outbuf,SegReg[modrm.reg]);
    op->sreg.sreg_index=modrm.reg;

    return 0;
}

uint32_t Global_IMMED(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    op->mode = OP_Immed;

    if (op->opersize == 1)
    {
        *((llvm::MCOperand *)op) = llvm::MCOperand::CreateImm((uint64_t)(int)(signed char)PeekB(codebuf));
        sprintf(outbuf,"%02X",(uint8_t)op->getImm());
    }
    else if (op->opersize == 2)
    {
        *((llvm::MCOperand *)op) = llvm::MCOperand::CreateImm(PeekW(codebuf));
        sprintf(outbuf,"%04lX",op->getImm());
    }
    else
    {
        *((llvm::MCOperand *)op) = llvm::MCOperand::CreateImm(PeekD(codebuf));
        sprintf(outbuf,"%08lX",op->getImm());
    }

    return op->opersize;
}

uint32_t Global_SIGNEDIMMED(char * outbuf,uint8_t * codebuf,OPERITEM &op)
{
    op.mode = OP_Immed;

    signed char by = (signed char)PeekB(codebuf);
    if (by < 0)
        sprintf(outbuf,"-%02X",-by);
    else
        sprintf(outbuf,"+%02X",by);
    ((llvm::MCOperand &)op) = llvm::MCOperand::CreateImm((uint64_t)(int)by);

    return 1;
}

uint32_t Disasm::Global_NEARPTR(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    op->mode = OP_Near;
    op->nearptr.offset = BaseAddress + CodeCount;
    if (op->opersize == 1)
        op->nearptr.offset += 1 + (int)(char)PeekB(codebuf);
    else if (op->opersize == 2)
        op->nearptr.offset += 2 + (int)(short)PeekW(codebuf);
    else
        op->nearptr.offset += 4 + (int)PeekD(codebuf);

    if (U_Size == BIT16)
        sprintf(outbuf,"%04lX",op->nearptr.offset);
    else
        sprintf(outbuf,"%08lX",op->nearptr.offset);

    return op->opersize;
}

uint32_t Disasm::Global_FARPTR(char * outbuf,uint8_t * codebuf,OPERITEM *op)
{
    op->mode = OP_Far;

    if (op->opersize == 4)
    {
        op->farptr.offset = PeekW(codebuf);
        op->farptr.segment= PeekW(codebuf+2);
    }
    else
    {
        op->farptr.offset = PeekD(codebuf);
        op->farptr.segment= PeekW(codebuf+4);
    }

    if (U_Size == BIT16)
        sprintf(outbuf,"%04lX:%04lX",op->farptr.segment,op->farptr.offset);
    else
        sprintf(outbuf,"%04lX:%08lX",op->farptr.segment,op->farptr.offset);

    return op->opersize;
}

/***********************************************************/

uint32_t	Disasm::ProcessOpdata(uint32_t opdata,OPERITEM *op,char * outbuf,uint32_t codepos)
{
    op->mode		= OP_Invalid;
    switch(opdata)
    {
        case D_NONE:
            break;

        case D_EB:
        case D_EW:
        case D_EV:

            if (opdata == D_EB)
                op->opersize = Global_GetSize(SIZE_B);
            else if (opdata == D_EW)
                op->opersize = Global_GetSize(SIZE_W);
            else
                op->opersize = Global_GetSize(SIZE_V);

            return Global_MODRM(outbuf,UasmCode+CodeCount,op);

        case D_GB:
        case D_GW:
        case D_GV:

            if (opdata == D_GB)
                op->opersize = Global_GetSize(SIZE_B);
            else if (opdata == D_GW)
                op->opersize = Global_GetSize(SIZE_W);
            else
                op->opersize = Global_GetSize(SIZE_V);

            return Global_REG(outbuf,UasmCode+CodeCount,op);

        case D_IB:
        case D_IW:
        case D_IV:

            if (opdata == D_IB)
                op->opersize = Global_GetSize(SIZE_B);
            else if (opdata == D_IW)
                op->opersize = Global_GetSize(SIZE_W);
            else
                op->opersize = Global_GetSize(SIZE_V);

            return Global_IMMED(outbuf,UasmCode+codepos,op);

        case D_SB:
            op->opersize = Global_GetSize(SIZE_B);
            return Global_SIGNEDIMMED(outbuf,UasmCode+codepos,*op);

        case D_SW:
            op->opersize = Global_GetSize(SIZE_W);
            return Global_SREG(outbuf,UasmCode+CodeCount,op);

        case D_MV:
        case D_MP:
        case D_MA:

            if (opdata == D_MV)
                op->opersize = Global_GetSize(SIZE_V);
            else if (opdata == D_MP)
                op->opersize = Global_GetSize(SIZE_P);
            else
                op->opersize = Global_GetSize(SIZE_A);

            return Global_MEMORY(outbuf,UasmCode+CodeCount,op);

        case D_OB:
        case D_OV:
            if (opdata == D_OB)
                op->opersize = Global_GetSize(SIZE_B);
            else
                op->opersize = Global_GetSize(SIZE_V);

            return Global_OFFSET(outbuf,UasmCode+codepos,op);

        case D_JB:
            op->opersize = 1;
            return Global_NEARPTR(outbuf,UasmCode+CodeCount,op);

        case D_JV:
            op->opersize = (U_Size == BIT32) ? 4 : 2;
            return Global_NEARPTR(outbuf,UasmCode+CodeCount,op);

        case D_JP:
            op->opersize = Global_GetSize(SIZE_P);
            return Global_FARPTR(outbuf,UasmCode+CodeCount,op);

        case D_RD:
        case D_CD:
        case D_DD:
            // Not support now
            SetError(ERR_INVALIDCODE);
            break;

        case D_1:
            *((llvm::MCOperand *)op) = llvm::MCOperand::CreateImm(1);
            op->mode			= OP_Immed;
            op->opersize		= 1;
            strcpy(outbuf,"1");
            break;

        case D_AL: case D_CL: case D_DL: case D_BL:
        case D_AH: case D_CH: case D_DH: case D_BH:
            op->mode			= OP_Register;
            op->opersize		= Global_GetSize(SIZE_B);
            *((llvm::MCOperand *)op) = llvm::MCOperand::CreateReg(opdata-D_AL);
            strcpy(outbuf,RegByte[opdata-D_AL]);
            break;

        case D_AX: case D_CX: case D_DX: case D_BX:
        case D_SP: case D_BP: case D_SI: case D_DI:
            op->mode			= OP_Register;
            op->opersize		= Global_GetSize(SIZE_W);
            *((llvm::MCOperand *)op) = llvm::MCOperand::CreateReg(opdata-D_AX);
            strcpy(outbuf,RegWord[opdata-D_AX]);
            break;

        case D_AXV: case D_CXV: case D_DXV: case D_BXV:
        case D_SPV: case D_BPV: case D_SIV: case D_DIV:
            op->mode			= OP_Register;
            op->opersize 		= Global_GetSize(SIZE_V);
            *((llvm::MCOperand *)op) = llvm::MCOperand::CreateReg(opdata-D_AXV);

            if (op->opersize == 2)
                strcpy(outbuf,RegWord[opdata-D_AXV]);
            else
                strcpy(outbuf,RegDWord[opdata-D_AXV]);

            break;

        case D_ES:
        case D_CS:
        case D_SS:
        case D_DS:
        case D_FS:
        case D_GS:
            op->mode			= OP_Segment;
            op->opersize 		= Global_GetSize(SIZE_V);
            op->sreg.sreg_index	= opdata-D_ES;
            strcpy(outbuf,SegReg[opdata-D_ES]);
            break;
    }
    return 0;
}

std::string	Disasm::ProcessSegPrefix(OPERITEM *op)
{
    std::string retn;
    if (op->mode != OP_Address)
        return retn;

    uint32_t	defseg = _DS_;
    if (op->addr.base_reg_index == _EBP_ || op->addr.base_reg_index == _ESP_)
        defseg = _SS_;

    if (SegPrefix != _NOSEG_ && SegPrefix != defseg)
    {
        retn = SegReg[SegPrefix];
        retn += ':';
        defseg = SegPrefix;
    }

    op->addr.seg_index = (uint8_t)defseg;
    SegPrefix = _NOSEG_;
    return retn;
}

void	Disasm::ProcessInstruction(	uint32_t	opcode,
                                    const char *	instname,
                                    uint32_t	opdata1,
                                    uint32_t	opdata2,
                                    uint32_t	opdata3)
{
    if (opcode == C_ERROR)
    {
        SetError(ERR_INVALIDCODE);
        return;
    }

    // Process LOCK and REPZ/REPNZ prefix
    if (LockPrefix != NOPREFIX)
    {
        this->m_idaout->LockName = LockName;
        //Putchs(LockName);
        //SetBufPtrTox2();
    }
    if (RepPrefix != NOPREFIX)
    {
        this->m_idaout->RepName = RepName;
        //Putchs(RepName);
        //SetBufPtrTox2();
    }

    xcpu.opcode		= (OPCODETYPE)opcode;
    xcpu.lockflag	= (uint8_t)LockPrefix;
    xcpu.repeatflag	= (uint8_t)RepPrefix;

    // Process DIFFERENTNAME opcode-property
    switch(opcode)
    {
        case C_CBW:
            this->m_idaout->CmdStr = (OpSize == BIT16) ? instname : "CWDE";
            break;

        case C_CWD:
            this->m_idaout->CmdStr = (OpSize == BIT16) ? instname : "CDQ";
            break;

        case C_JCXZ:
            this->m_idaout->CmdStr = (OpSize == BIT16) ? instname : "JECXZ";
            break;

        default:
            this->m_idaout->CmdStr = instname;
            switch(opdata1)
            {
                case D_V:
                    if (OpSize == BIT32)
                        this->m_idaout->CmdStr += 'D';
                    break;

                case D_XB:
                    this->m_idaout->CmdStr += 'B';
                    break;

                case D_XV:
                    if (OpSize == BIT32)
                        this->m_idaout->CmdStr += 'D';
                    else
                        this->m_idaout->CmdStr += 'W';
                    break;
            }
    }
    // Ok, now we have completed processing InstName

    //SetBufPtrTox2();

    // Now we begin to process opdata
    char	outbuf1[128];
    char	outbuf2[128];
    char	outbuf3[128];

    uint32_t	codelen = CodeCount;
    codelen += ProcessOpdata(opdata1,&xcpu.op[0],outbuf1,codelen);
    codelen += ProcessOpdata(opdata2,&xcpu.op[1],outbuf2,codelen);
    codelen += ProcessOpdata(opdata3,&xcpu.op[2],outbuf3,codelen);
    // Ok, we have output the result of disassemble based on OPERITEM.

    if (xcpu.op[0].mode != OP_Invalid)
    {
        // Process 1st Opdata WORD PTR prefix
        if (xcpu.op[0].mode == OP_Address &&
                xcpu.opcode != C_JMP &&
                xcpu.opcode != C_CALL &&
                xcpu.opcode != C_JMPFAR &&
                xcpu.opcode != C_CALLFAR &&
                (xcpu.op[1].mode == OP_Immed || xcpu.op[1].mode == OP_Invalid) )
        {
            switch(xcpu.op[0].opersize)
            {
                case 1:
                    this->m_idaout->Par1Ptr = "BYTE PTR ";
                    break;

                case 2:
                    this->m_idaout->Par1Ptr = "WORD PTR ";
                    break;

                case 4:
                    this->m_idaout->Par1Ptr = "DWORD PTR ";
                    break;
            }
        }
        this->m_idaout->Par1SegPrefix = ProcessSegPrefix(&xcpu.op[0]);
        this->m_idaout->Par1Str = outbuf1;
    }

    if (xcpu.op[1].mode != OP_Invalid)
    {
        // Process 2nd Opdata WORD PTR prefix
        if (xcpu.op[1].mode == OP_Address &&
                xcpu.op[1].opersize < xcpu.op[0].opersize )
            //(xcpu.opcode == C_MOVZX || xcpu.opcode == C_MOVSX))
        {
            if (xcpu.op[1].opersize == 1)
                this->m_idaout->Par2Ptr = "BYTE PTR ";
            else
                this->m_idaout->Par2Ptr = "WORD PTR ";
        }
        m_idaout->Par2SegPrefix = ProcessSegPrefix(&xcpu.op[1]);
        m_idaout->Par2Str = outbuf2;
    }

    if (xcpu.op[2].mode != OP_Invalid)
    {
        m_idaout->Par3Str = outbuf3;
    }

    CodeCount = codelen;
    // Complete it!

    // Write code here to simulate instruction.
    // VMSimulate(&xpcu);
}

void	Disasm::ProcessGroup(const INSTRUCTION * pG,const INSTRUCTION & inst)
{
    MODRM	modrm;
    modrm.v	= GetByte();

    uint32_t	opdata1 = inst.Opdata1;
    uint32_t	opdata2 = inst.Opdata2;
    uint32_t	opdata3 = inst.Opdata3;

    if (pG[modrm.reg].Opdata1 != D_NONE)
        opdata1 = pG[modrm.reg].Opdata1;

    if (pG[modrm.reg].Opdata2 != D_NONE)
        opdata2 = pG[modrm.reg].Opdata2;

    if (pG[modrm.reg].Opdata3 != D_NONE)
        opdata3 = pG[modrm.reg].Opdata3;

    ProcessInstruction(	pG[modrm.reg].Opcode,
            pG[modrm.reg].InstName,
            opdata1,
            opdata2,
            opdata3
            );
}

void	Disasm::DisassemblerOne()
{
    uint8_t by = GetByteEx();

    switch(instruction[by].Opcode)
    {
        case C_ERROR:
            SetError(ERR_INVALIDCODE);
            break;

        case C_GROUP:		// It'a group. If this, InstName specifies the group table.
            ProcessGroup((const INSTRUCTION *)(instruction[by].InstName),instruction[by]);
            break;

        case C_0FH:			// It's 0FH instruction. If this, InstName specifies the 0FH table.
            by = GetByteEx();
            switch(instruction0FH[by].Opcode)
            {
                case C_ERROR:
                    break;

                case C_GROUP:	// It'a group. If this, InstName specifies the group table.
                    ProcessGroup((const INSTRUCTION *)(instruction0FH[by].InstName),instruction0FH[by]);
                    break;

                default:
                    ProcessInstruction(	instruction0FH[by].Opcode,
                                        instruction0FH[by].InstName,
                                        instruction0FH[by].Opdata1,
                                        instruction0FH[by].Opdata2,
                                        instruction0FH[by].Opdata3
                                        );
                    break;
            }
            break;

        case C_SEGPREFIX:	// Segment Prefix
            SegPrefix		= instruction[by].Opdata1;
            DisassemblerOne();
            break;

        case C_OPRSIZE:		// Operand size change.
            OpSizePrefix();
            DisassemblerOne();
            break;

        case C_ADRSIZE:		// Address size change.
            AdrSizePrefix();
            DisassemblerOne();
            break;

        case C_LOCK:		// LOCK prefix
            LockPrefix	= 1;
            LockName	= instruction[by].InstName;
            DisassemblerOne();
            break;

        case C_REPZ:		// REPZ prefix
            RepPrefix	= REPZ_PREFIX;
            RepName		= instruction[by].InstName;
            DisassemblerOne();
            break;

        case C_REPNZ:		// REPNZ prefix
            RepPrefix	= REPNZ_PREFIX;
            RepName		= instruction[by].InstName;
            DisassemblerOne();
            break;

        default:
            ProcessInstruction(	instruction[by].Opcode,
                                instruction[by].InstName,
                                instruction[by].Opdata1,
                                instruction[by].Opdata2,
                                instruction[by].Opdata3
                                );
            break;
    }
}
