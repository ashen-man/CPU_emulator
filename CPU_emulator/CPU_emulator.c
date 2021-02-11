//『動かしてわかるCPUの作り方10講』（技術評論社）の第3講より
// CPU_emulator.c
#include <stdio.h>

#define MOV		 0 // 命令コードのインデックス
#define ADD		 1
#define SUB	     2
#define AND	     3
#define OR	     4
#define SL	     5
#define SR	     6
#define SRA	     7
#define LDL		 8
#define LDH		 9
#define CMP		10
#define JE		11
#define JMP		12
#define LD		13
#define ST		14
#define HLT		15

#define REG0	0 // 汎用レジスタのインデックス
#define REG1	1
#define REG2	2
#define REG3	3
#define REG4	4
#define REG5	5
#define REG6	6
#define REG7	7

short reg[8];            // 汎用レジスタ
short rom[256];          // メインメモリのプログラム領域
short ram[256];          // メインメモリのデータ領域

void assembler(void);    // 簡易アセンブラ関数のプロトタイプ宣言

short mov(short, short); // 簡易アセンブラで使用する関数のプロトタイプ宣言
short add(short, short);
short sub(short, short);
short and(short, short);
short or (short, short);
short sl(short);
short sr(short);
short sra(short);
short ldl(short, short);
short ldh(short, short);
short cmp(short, short);
short je(short);
short jmp(short);
short ld(short, short);
short st(short, short);
short hlt(void);

short op_code(short); // インストラクションレジスタから
short op_regA(short); // 命令コードやオペランドを抽出する関数のプロトタイプ宣言
short op_regB(short);
short op_data(short);
short op_addr(short);

void main(void) {
	short	pc;	      // プログラムカウンタ 
	short	ir;		  // インストラクションレジスタ
	short	flag_eq;  // 比較用フラグ

	assembler();

	pc = 0;      // プログラムカウンタの初期化
	flag_eq = 0; // 比較用フラグの初期化

	do {
		// メインメモリのプログラム領域の機械語を順番に読み出し、
		// インストラクションレジスタに保存
		ir = rom[pc];

		printf(" %5d  %5x  %5d  %5d  %5d  %5d\n",
			pc, ir, reg[0], reg[1], reg[2], reg[3]);

		pc = pc + 1;

		// インストラクションレジスタの15~12bitの命令コードを取り出す
		// 命令コードを判別し、命令ごとの処理を実行する
		switch (op_code(ir)) {
			case MOV:	reg[op_regA(ir)] = reg[op_regB(ir)];
						break;
			case ADD:	reg[op_regA(ir)] = reg[op_regA(ir)] + reg[op_regB(ir)];
						break;
			case SUB:	reg[op_regA(ir)] = reg[op_regA(ir)] - reg[op_regB(ir)];
						break;
			case AND:	reg[op_regA(ir)] = reg[op_regA(ir)] & reg[op_regB(ir)];
						break;
			case OR:	reg[op_regA(ir)] = reg[op_regA(ir)] | reg[op_regB(ir)];
						break;
			case SL:	reg[op_regA(ir)] = reg[op_regA(ir)] << 1;
						break;
			case SR:	reg[op_regA(ir)] = reg[op_regA(ir)] >> 1;
						break;
			case SRA:	reg[op_regA(ir)] = (reg[op_regA(ir)] & 0x8000) | (reg[op_regA(ir)] >> 1);
						break;
			case LDL:	reg[op_regA(ir)] = (reg[op_regA(ir)] & 0xff00) | (op_data(ir) & 0x00ff);
						break;
			case LDH:	reg[op_regA(ir)] = (op_data(ir) << 8) | (reg[op_regA(ir)] & 0x00ff);
						break;
			case CMP:	if (reg[op_regA(ir)] == reg[op_regB(ir)]) {
							flag_eq = 1;
						}else {
							flag_eq = 0;
						}
						break;
			case JE:	if (flag_eq == 1) pc = op_addr(ir);
						break;
			case JMP:	pc = op_addr(ir);
						break;
			case LD:	reg[op_regA(ir)] = ram[op_addr(ir)];
						break;
			case ST:	ram[op_addr(ir)] = reg[op_regA(ir)];
						break;
			default:
						break;
		}
	} while (op_code(ir) != HLT); // 停止命令

	printf("ram[64] = %d \n", ram[64]);
}

void assembler(void) {
	// アセンブラの処理をメインメモリのプログラム領域に配置
	rom[0] = ldh(REG0, 0);			// REG0(H) ←  0
	rom[1] = ldl(REG0, 0);			// REG0(L) ←  0
	rom[2] = ldh(REG1, 0);			// REG1(H) ←  0
	rom[3] = ldl(REG1, 1);			// REG1(L) ←  1
	rom[4] = ldh(REG2, 0);			// REG2(H) ←  0
	rom[5] = ldl(REG2, 0);			// REG2(L) ←  0
	rom[6] = ldh(REG3, 0);			// REG3(H) ←  0
	rom[7] = ldl(REG3, 10);			// REG3(L) ←  10
	rom[8] = add(REG2, REG1);		// REG2 ← REG2 + REG1
	rom[9] = add(REG0, REG2);		// REG0 ← REG0 + REG2
	rom[10] = st(REG0, 64);			// REG0をメモリ(I/O)の64番地に保存
	rom[11] = cmp(REG2, REG3);		// REG2とREG3を比較
	rom[12] = je(14);				// 一致したら14番地にジャンプ
	rom[13] = jmp(8);				// 無条件に8番地にジャンプ
	rom[14] = hlt();				// CPUの停止

}

// 関数mov(move)本体
// 命令コード、オペランドからなる機械語を返す
short mov(short ra, short rb) {
	return ((MOV << 11) | (ra << 8) | (rb << 5));
}
// 関数add(addition)本体
short add(short ra, short rb) {
	return ((ADD << 11) | (ra << 8) | (rb << 5));
}
// 関数sub(subtraction)本体
short sub(short ra, short rb) {
	return ((SUB << 11) | (ra << 8) | (rb << 5));
}
// 関数and(logical and)本体
short and(short ra, short rb) {
	return ((AND << 11) | (ra << 8) | (rb << 5));
}
// 関数or(logical or)本体
short or (short ra, short rb) {
	return ((OR << 11) | (ra << 8) | (rb << 5));
}
// 関数sl(shift left)本体
short sl(short ra) {
	return ((SL << 11) | (ra << 8));
}
// 関数sr(shift right)本体
short sr(short ra) {
	return ((SR << 11) | (ra << 8));
}
// 関数sra(shift right arithmetic)本体
short sra(short ra) {
	return ((SRA << 11) | (ra << 8));
}
// 関数ldl(load immediate value low)本体
short ldl(short ra, short ival) {
	return ((LDL << 11) | (ra << 8) | (ival & 0x00ff));
}

// 関数ldh(load immediate value high)本体
short ldh(short ra, short ival) {
	return ((LDH << 11) | (ra << 8) | (ival & 0x00ff));
}

// 関数cmp(compare)本体
short cmp(short ra, short rb) {
	return ((CMP << 11) | (ra << 8) | (rb << 5));
}

// 関数je(jump equal)本体
short je(short addr) {
	return ((JE << 11) | (addr & 0x00ff));
}

// 関数jmp(jump)本体
short jmp(short addr) {
	return ((JMP << 11) | (addr & 0x00ff));
}

// 関数ld(load memory)本体
short ld(short ra, short addr) {
	return ((LD << 11) | (ra << 8) | (addr & 0x00ff));
}

// 関数st(store memory)本体
short st(short ra, short addr) {
	return ((ST << 11) | (ra << 8) | (addr & 0x00ff));
}

// 関数hlt(halt)本体
short hlt(void) {
	return (HLT << 11);
}

// 関数op_code本体
short op_code(short ir) {
	return  (ir >> 11);
}

// 関数op_regA本体
// 10~8bitに保存されているレジスタ番号を返す
short op_regA(short ir) {
	return ((ir >> 8) & 0x0007);
}

// 関数op_regB本体
short op_regB(short ir) {
// 7~5bitに保存されているレジスタ番号を返す
	return ((ir >> 5) & 0x0007);
}

// 関数op_data本体
short op_data(short ir) {
	return (ir & 0x00ff);
}

// 関数op_addr本体
short op_addr(short ir) {
	return (ir & 0x00ff);
}
