// -- x65 cpu emulator -- //

namespace x65 {
	// define custom types
	typedef unsigned short wt;
	typedef unsigned char bt;
	typedef unsigned int dt;
	typedef void (*outf)(wt, bt);
	typedef bt (*inpf)(wt);

	// flag ids
	const bt BITC = 0;
	const bt BITZ = 1;
	const bt BITI = 2;
	const bt BITF = 3;
	const bt BITV = 6;
	const bt BITN = 7;

	// addressing modes
	enum Mode {
		NOT = 0,
		IMP, ACC, BUF, IMM, DIM, REL,
		DIR, DRX, DRY, ZPG, ZPX, ZPY,
		IND
	};

	// opcode data
	Mode opcodeMode[256] {
		IMP, DIM, DIM, NOT, DIR, DIR, DIR, IMP, IMP, DRX, DRX, NOT, DRX, DRY, DRY, IMP,
		REL, IMM, IMM, NOT, ACC, ZPG, ZPG, IMP, IMP, ZPX, ZPX, NOT, DRY, ZPY, ZPY, IMP,
		DIR, DIM, DIM, NOT, DIR, DIR, DIR, IMP, IMP, DRX, DRX, NOT, DRX, DRY, DRY, IMP,
		REL, IMM, IMM, NOT, ACC, ZPG, ZPG, IMP, IMP, ZPX, ZPX, NOT, DRY, ZPY, ZPY, IMP,
		IMP, DIM, IMP, NOT, DIR, DIR, DIR, IMP, IMP, DRX, DRX, NOT, DRX, DRY, DRY, IMP,
		REL, IMM, IMP, NOT, ACC, ZPG, ZPG, IMP, IMP, ZPX, ZPX, NOT, DRY, ZPY, ZPY, IMP,
		IMP, DIM, IMP, NOT, DIR, DIR, DIR, ACC, IMP, DRX, DRX, NOT, DRX, DRY, DRY, IMP,
		REL, IMM, IMP, NOT, ACC, ZPG, ZPG, ACC, IMP, ZPX, ZPX, NOT, DRY, ZPY, ZPY, IMP,
		IMP, DIM, BUF, NOT, DIR, DIR, DIR, NOT, IMP, DRX, DRX, NOT, DRX, DRY, DRY, IMP,
		REL, IMM, BUF, NOT, DIM, ZPG, ZPG, NOT, IMP, ZPX, ZPX, NOT, DRY, ZPY, ZPY, IMP,
		BUF, DIM, BUF, NOT, DIR, DIR, DIR, NOT, IMP, DRX, DIR, NOT, ACC, DRY, DRY, IMP,
		REL, IMM, BUF, NOT, DIM, ZPG, ZPG, NOT, IMP, ZPX, ZPG, NOT, DRY, ZPY, ZPY, IMP,
		BUF, DIM, BUF, NOT, DIR, DIR, DIR, NOT, IMP, DRX, DRX, NOT, DRX, DRY, DIR, REL,
		REL, IMM, BUF, NOT, DIM, ZPG, ZPG, NOT, IMP, ZPX, ZPX, NOT, BUF, ZPY, ZPG, DIR,
		BUF, IND, BUF, NOT, DIR, DIR, DIR, NOT, IMP, DRX, DRX, NOT, DRX, DRY, DRY, IND,
		REL, IMM, BUF, NOT, IMM, ZPG, ZPG, NOT, IMP, ZPX, ZPX, NOT, DRY, ZPY, ZPY, IND
	};

	// cpu object
	struct CPU {
		bool halt;
		bool wait;

		outf set;
		inpf get;
		wt a, b;
		wt x, y;
		wt s, i;
		wt l;
		bt p;
	};

	// misc operations
	inline neg(bt data) {
		return data & 0x80;
	};
	inline neg(wt data) {
		return data & 0x8000;
	};
	inline neg(int data) {
		return (wt)data & 0x8000;
	};

	// status operations
	inline void setBit(CPU& cpu, bt id, bool value) {
		cpu.p &= (1 << id) ^ 0xFF;
		cpu.p |= value << id;
	};
	inline bool getBit(CPU& cpu, bt id) {
		return cpu.p & (1 << id);
	};
	inline void update(CPU& cpu, bt data) {
		setBit(cpu, BITN, neg(data));
		setBit(cpu, BITZ, data == 0);
	};
	inline void update(CPU& cpu, wt data) {
		setBit(cpu, BITN, neg(data));
		setBit(cpu, BITZ, data == 0);
	};

	// write bytes
	inline void writeByte(CPU& cpu, wt addr, bt data) {
		cpu.set(addr, data);
	};
	inline void writeWord(CPU& cpu, wt addr, wt data) {
		cpu.set(addr + 0, data & 0xFF);
		cpu.set(addr + 1, data >> 0x8);
	};

	// fetch bytes
	inline bt nextByte(CPU& cpu) {
		return cpu.get(cpu.i++);
	};
	inline wt nextWord(CPU& cpu) {
	    bt d = cpu.get(cpu.i++);
		return d | cpu.get(cpu.i++) << 8;
	};
	inline wt readByte(CPU& cpu, wt addr) {
		return cpu.get(addr);
	};
	inline wt readWord(CPU& cpu, wt addr) {
		return cpu.get(addr) | cpu.get(addr + 1) << 8;
	};
	wt readDataW(CPU& cpu, Mode mode) {
		switch (mode) {
			case IMP: return 0x00;
			case ACC: return cpu.a;
			case BUF: return cpu.b;
			case IMM: return nextByte(cpu);
			case DIM: return nextWord(cpu);
			case REL: return cpu.i + (char)nextByte(cpu);
			case DIR: return readWord(cpu, nextWord(cpu));
			case DRX: return readWord(cpu, nextWord(cpu) + cpu.x);
			case DRY: return readWord(cpu, nextWord(cpu) + cpu.y);
			case ZPG: return readWord(cpu, nextByte(cpu));
			case ZPX: return readWord(cpu, nextByte(cpu) + cpu.x);
			case ZPY: return readWord(cpu, nextByte(cpu) + cpu.y);
			case IND: return cpu.x;
			default: return 0x00;
		};
	};
	bt readDataB(CPU& cpu, Mode mode) {
		switch (mode) {
			case IMP: return 0x00;
			case ACC: return (bt)cpu.a;
			case BUF: return (bt)cpu.b;
			case IMM: return nextByte(cpu);
			case DIR: return readByte(cpu, nextWord(cpu));
			case DRX: return readByte(cpu, nextWord(cpu) + cpu.x);
			case DRY: return readByte(cpu, nextWord(cpu) + cpu.y);
			case ZPG: return readByte(cpu, nextByte(cpu));
			case ZPX: return readByte(cpu, nextByte(cpu) + cpu.x);
			case ZPY: return readByte(cpu, nextByte(cpu) + cpu.y);
			default: return 0x00;
		};
	};
	wt readAddr(CPU& cpu, Mode mode) {
		switch (mode) {
			case REL: return cpu.i + (char)nextByte(cpu);
			case DIR: return nextWord(cpu);
			case DRX: return nextWord(cpu) + cpu.x;
			case DRY: return nextWord(cpu) + cpu.y;
			case ZPG: return nextByte(cpu);
			case ZPX: return nextByte(cpu) + cpu.x;
			case ZPY: return nextByte(cpu) + cpu.y;
			case IND: return cpu.x;
			default: return 0x00;
		};
	};

	// stack operations
	inline void stackInc(CPU& cpu) {
		cpu.s--;
		if (cpu.s < cpu.l)
            cpu.s = 0x1FFF;
	};
	inline void stackDec(CPU& cpu) {
		cpu.s++;
		if (cpu.s >= 0x2000)
            cpu.s = cpu.l;
	};

	// stack pushes
	void pushByte(CPU& cpu, bt data) {
		cpu.set(cpu.s, data);
		stackInc(cpu);
	};
	void pushWord(CPU& cpu, wt data) {
		cpu.set(cpu.s, data & 0xFF);
		stackInc(cpu);
		cpu.set(cpu.s, data >> 8);
		stackInc(cpu);
	};

	// stack pulls
	bt pullByte(CPU& cpu) {
		stackDec(cpu);
		return cpu.get(cpu.s);
	};
	wt pullWord(CPU& cpu) {
		stackDec(cpu);
		bt part = cpu.get(cpu.s);
		stackDec(cpu);
		return cpu.get(cpu.s) | part << 8;
	};

	// vector operations
	void vectorRST(CPU& cpu) {
		cpu.i = readWord(cpu, 0xFFFC);
		cpu.halt = false;
	};
	void vectorNMI(CPU& cpu) {
		pushWord(cpu, cpu.i);
		pushByte(cpu, cpu.p);
		cpu.i = readWord(cpu, 0xFFFE);
		cpu.wait = false;
	};
	void vectorIRQ(CPU& cpu) {
		if (getBit(cpu, BITI))
			return;

		pushWord(cpu, cpu.i);
		pushByte(cpu, cpu.p);
		cpu.i = readWord(cpu, 0xFFFA);
		cpu.wait = false;
	};

	// opcodes
	void NOP(CPU& cpu, Mode mode) {
		// no actions
	};
	void ERR(CPU& cpu, Mode mode) {
		// no actions
	};
	void WAI(CPU& cpu, Mode mode) {
		cpu.wait = true;
	};
	void JAM(CPU& cpu, Mode mode) {
		cpu.halt = true;
	};
	void PHP(CPU& cpu, Mode mode) {
		pushByte(cpu, cpu.p);
	};
	void PLP(CPU& cpu, Mode mode) {
		cpu.p = pullByte(cpu);
	};
	void PHA(CPU& cpu, Mode mode) {
		pushWord(cpu, cpu.a);
	};
	void PLA(CPU& cpu, Mode mode) {
		cpu.a = pullWord(cpu);
		update(cpu, cpu.a);
	};
	void PHB(CPU& cpu, Mode mode) {
		pushWord(cpu, cpu.b);
	};
	void PLB(CPU& cpu, Mode mode) {
		cpu.a = pullWord(cpu);
		update(cpu, cpu.b);
	};
	void PHX(CPU& cpu, Mode mode) {
		pushWord(cpu, cpu.x);
	};
	void PLX(CPU& cpu, Mode mode) {
		cpu.x = pullWord(cpu);
		update(cpu, cpu.x);
	};
	void PHY(CPU& cpu, Mode mode) {
		pushWord(cpu, cpu.y);
	};
	void PLY(CPU& cpu, Mode mode) {
		cpu.y = pullWord(cpu);
		update(cpu, cpu.y);
	};
	void PHD(CPU& cpu, Mode mode) {
		pushByte(cpu, cpu.a & 0xFF);
	};
	void PLD(CPU& cpu, Mode mode) {
		cpu.a |= pullByte(cpu);
		update(cpu, (bt)cpu.a);
	};
	void TAB(CPU& cpu, Mode mode) {
		cpu.b = cpu.a;
		update(cpu, cpu.b);
	};
	void TAX(CPU& cpu, Mode mode) {
		cpu.x = cpu.a;
		update(cpu, cpu.x);
	};
	void TAY(CPU& cpu, Mode mode) {
		cpu.y = cpu.a;
		update(cpu, cpu.y);
	};
	void TBA(CPU& cpu, Mode mode) {
		cpu.a = cpu.b;
		update(cpu, cpu.a);
	};
	void TXA(CPU& cpu, Mode mode) {
		cpu.a = cpu.x;
		update(cpu, cpu.a);
	};
	void TXY(CPU& cpu, Mode mode) {
		cpu.y = cpu.x;
		update(cpu, cpu.y);
	};
	void TYA(CPU& cpu, Mode mode) {
		cpu.a = cpu.y;
		update(cpu, cpu.a);
	};
	void TYX(CPU& cpu, Mode mode) {
		cpu.x = cpu.y;
		update(cpu, cpu.x);
	};
	void TXS(CPU& cpu, Mode mode) {
		cpu.s = cpu.x & 0x1FFF;
		if (cpu.s < cpu.l)
            cpu.s = 0x1FFF;
		update(cpu, cpu.s);
	};
	void TSX(CPU& cpu, Mode mode) {
		cpu.x = cpu.s;
		update(cpu, cpu.x);
	};
	void THD(CPU& cpu, Mode mode) {
		cpu.a &= 0xFF00;
		cpu.a |= cpu.a >> 8;
		update(cpu, (bt)cpu.a);
	};
	void TDH(CPU& cpu, Mode mode) {
		cpu.a &= 0x00FF;
		cpu.a |= (cpu.a & 0xFF) << 8;
		update(cpu, (bt)(cpu.a >> 8));
	};
	void CLC(CPU& cpu, Mode mode) {
		setBit(cpu, BITC, 0);
	};
	void SEC(CPU& cpu, Mode mode) {
		setBit(cpu, BITC, 1);
	};
	void CLI(CPU& cpu, Mode mode) {
		setBit(cpu, BITI, 0);
	};
	void SEI(CPU& cpu, Mode mode) {
		setBit(cpu, BITI, 1);
	};
	void CLF(CPU& cpu, Mode mode) {
		setBit(cpu, BITF, 0);
	};
	void SEF(CPU& cpu, Mode mode) {
		setBit(cpu, BITF, 1);
	};
	void CLV(CPU& cpu, Mode mode) {
		setBit(cpu, BITV, 0);
	};
	void INX(CPU& cpu, Mode mode) {
		cpu.x++;
		update(cpu, cpu.x);
	};
	void DEX(CPU& cpu, Mode mode) {
		cpu.x--;
		update(cpu, cpu.x);
	};
	void INY(CPU& cpu, Mode mode) {
		cpu.y++;
		update(cpu, cpu.y);
	};
	void DEY(CPU& cpu, Mode mode) {
		cpu.y--;
		update(cpu, cpu.y);
	};
	void INS(CPU& cpu, Mode mode) {
		stackDec(cpu);
	};
	void DES(CPU& cpu, Mode mode) {
		stackInc(cpu);
	};
	void ASL(CPU& cpu, Mode mode) {
		if (mode == ACC) {
			setBit(cpu, BITC, cpu.a & 0x8000);
			cpu.a <<= 1;
			update(cpu, cpu.a);
		} else {
			wt addr = readAddr(cpu, mode);
			bt v = cpu.get(addr);
			setBit(cpu, BITC, v & 0x8000);
			v <<= 1;
			cpu.set(addr, v);
			update(cpu, v);
		};
	};
	void LSR(CPU& cpu, Mode mode) {
		if (mode == ACC) {
			setBit(cpu, BITC, cpu.a & 1);
			cpu.a >>= 1;
			update(cpu, cpu.a);
		} else {
			wt addr = readAddr(cpu, mode);
			bt v = cpu.get(addr);
			setBit(cpu, BITC, v & 1);
			v >>= 1;
			cpu.set(addr, v);
			update(cpu, v);
		};
	};
	void ROL(CPU& cpu, Mode mode) {
		if (mode == ACC) {
			bool buffer = getBit(cpu, BITC);
			setBit(cpu, BITC, cpu.a & 0x8000);
			cpu.a <<= 1;
			cpu.a |= buffer;
			update(cpu, cpu.a);
		} else {
			wt addr = readAddr(cpu, mode);
			bool buffer = getBit(cpu, BITC);
			bt v = cpu.get(addr);
			setBit(cpu, BITC, v & 0x80);
			v <<= 1;
			v |= buffer;
			cpu.set(addr, v);
			update(cpu, v);
		};
	};
	void ROR(CPU& cpu, Mode mode) {
		if (mode == ACC) {
			bool buffer = getBit(cpu, BITC);
			setBit(cpu, BITC, cpu.a & 1);
			cpu.a >>= 1;
			cpu.a |= buffer << 15;
			update(cpu, cpu.a);
		} else {
			wt addr = readAddr(cpu, mode);
			bool buffer = getBit(cpu, BITC);
			bt v = cpu.get(addr);
			setBit(cpu, BITC, v & 1);
			v >>= 1;
			v |= buffer << 7;
			cpu.set(addr, v);
			update(cpu, v);
		};
	};
	void CMP(CPU& cpu, Mode mode) {
		wt f = cpu.a;
		wt s = readDataW(cpu, mode);

		setBit(cpu, BITN, neg(f - s));
		setBit(cpu, BITC, f >= s);
		setBit(cpu, BITZ, f == s);
	};
	void CPX(CPU& cpu, Mode mode) {
		wt f = cpu.x;
		wt s = readDataW(cpu, mode);

		setBit(cpu, BITN, neg(f - s));
		setBit(cpu, BITC, f >= s);
		setBit(cpu, BITZ, f == s);
	};
	void CPY(CPU& cpu, Mode mode) {
		wt f = cpu.y;
		wt s = readDataW(cpu, mode);

		setBit(cpu, BITN, neg(f - s));
		setBit(cpu, BITC, f >= s);
		setBit(cpu, BITZ, f == s);
	};
	void CMD(CPU& cpu, Mode mode) {
		bt f = cpu.a & 0xFF;
		bt s = readDataB(cpu, mode);

		setBit(cpu, BITN, bt(f - s) & 0x80);
		setBit(cpu, BITC, f >= s);
		setBit(cpu, BITZ, f == s);
	};
	void AND(CPU& cpu, Mode mode) {
		cpu.a &= readDataW(cpu, mode);
		update(cpu, cpu.a);
	};
	void ORA(CPU& cpu, Mode mode) {
		cpu.a |= readDataW(cpu, mode);
		update(cpu, cpu.a);
	};
	void XOR(CPU& cpu, Mode mode) {
		cpu.a ^= readDataW(cpu, mode);
		update(cpu, cpu.a);
	};
	void LTA(CPU& cpu, Mode mode) {
		cpu.a = readDataW(cpu, mode);
		update(cpu, cpu.a);
	};
	void LTB(CPU& cpu, Mode mode) {
		cpu.b = readDataW(cpu, mode);
		update(cpu, cpu.b);
	};
	void LTX(CPU& cpu, Mode mode) {
		cpu.x = readDataW(cpu, mode);
		update(cpu, cpu.x);
	};
	void LTY(CPU& cpu, Mode mode) {
		cpu.y = readDataW(cpu, mode);
		update(cpu, cpu.y);
	};
	void LTD(CPU& cpu, Mode mode) {
		cpu.a = (cpu.a & 0xFF00) | readDataB(cpu, mode);
		update(cpu, (bt)cpu.a);
	};
	void ADC(CPU& cpu, Mode mode) {
		dt res = cpu.a + readDataW(cpu, mode) + getBit(cpu, BITC);
		cpu.a = res & 0xFFFF;
		setBit(cpu, BITC, res >> 16);
		update(cpu, cpu.a);
	};
	void SBC(CPU& cpu, Mode mode) {
		dt res = cpu.a - readDataW(cpu, mode) + getBit(cpu, BITC) - 1;
		cpu.a = res & 0xFFFF;
		setBit(cpu, BITC, !(res >> 16));
		update(cpu, cpu.a);
	};
	void STZ(CPU& cpu, Mode mode) {
		writeByte(cpu, readAddr(cpu, mode), 0x00);
	};
	void STA(CPU& cpu, Mode mode) {
		writeWord(cpu, readAddr(cpu, mode), cpu.a);
	};
	void STB(CPU& cpu, Mode mode) {
		writeWord(cpu, readAddr(cpu, mode), cpu.b);
	};
	void STX(CPU& cpu, Mode mode) {
		writeWord(cpu, readAddr(cpu, mode), cpu.x);
	};
	void STY(CPU& cpu, Mode mode) {
		writeWord(cpu, readAddr(cpu, mode), cpu.y);
	};
	void STD(CPU& cpu, Mode mode) {
		writeByte(cpu, readAddr(cpu, mode), (bt)cpu.a);
	};
	void BPL(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITN) == 0)
			cpu.i = addr + 1;
	};
	void BMI(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITN) == 1)
			cpu.i = addr + 1;
	};
	void BVC(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITV) == 0)
			cpu.i = addr + 1;
	};
	void BVS(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITV) == 1)
			cpu.i = addr + 1;
	};
	void BCC(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITC) == 0)
			cpu.i = addr + 1;
	};
	void BCS(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITC) == 1)
			cpu.i = addr + 1;
	};
	void BNE(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITZ) == 0)
			cpu.i = addr + 1;
	};
	void BEQ(CPU& cpu, Mode mode) {
		wt addr = readAddr(cpu, mode);
		if (getBit(cpu, BITZ) == 1)
			cpu.i = addr + 1;
	};
	void BRA(CPU& cpu, Mode mode) {
		cpu.i = readAddr(cpu, mode) + 1;
	};
	void INC(CPU& cpu, Mode mode) {
		if (mode == ACC) {
			update(cpu, ++cpu.a);
		} else {
			wt addr = readAddr(cpu, mode);
			bt v = readByte(cpu, addr);
			update(cpu, ++v);
			writeByte(cpu, addr, v);
		};
	};
	void DEC(CPU& cpu, Mode mode) {
		if (mode == ACC) {
			update(cpu, --cpu.a);
		} else {
			wt addr = readAddr(cpu, mode);
			bt v = readByte(cpu, addr);
			update(cpu, --v);
			writeByte(cpu, addr, v);
		};
	};
	void BIT(CPU& cpu, Mode mode) {
		wt data = readDataW(cpu, mode);

		setBit(cpu, BITZ, (cpu.a & data) == 0);
		setBit(cpu, BITN, data & 0x8000);
		setBit(cpu, BITV, data & 0x4000);
	};
	void MUL(CPU& cpu, Mode mode) {
		if (getBit(cpu, BITF)) {
			cpu.a = (cpu.a * cpu.b) >> 8;
		} else {
			cpu.a = cpu.a * cpu.b;
		};
		update(cpu, cpu.a);
	};
	void DIV(CPU& cpu, Mode mode) {
		if (cpu.b == 0)
			return;
		if (getBit(cpu, BITF)) {
			cpu.a = (cpu.a << 8) / cpu.b;
		} else {
			cpu.a = cpu.a / cpu.b;
		};
		update(cpu, cpu.a);
	};
	void MOD(CPU& cpu, Mode mode) {
		if (cpu.b == 0)
			return;
		cpu.a = cpu.a % cpu.b;
		update(cpu, cpu.a);
	};
	void LTV(CPU& cpu, Mode mode) {
		cpu.l = cpu.x & 0x1FFF;
	};
	void JMP(CPU& cpu, Mode mode) {
		cpu.i = readAddr(cpu, mode);
	};
	void JSR(CPU& cpu, Mode mode) {
		pushWord(cpu, cpu.i + 2);
		cpu.i = readAddr(cpu, mode);
	};
	void RTS(CPU& cpu, Mode mode) {
		cpu.i = pullWord(cpu);
	};
	void RTI(CPU& cpu, Mode mode) {
		cpu.p = pullByte(cpu);
		cpu.i = pullWord(cpu);
	};

	// opcode jump table
	typedef void (*opcf)(CPU&, Mode);
	opcf opcodeTable[256] {
		NOP, AND, ADC, ERR, ASL, AND, ADC, CLC, JAM, AND, ADC, ERR, ASL, AND, ADC, TAB,
		BPL, AND, ADC, ERR, ASL, AND, ADC, SEC, WAI, AND, ADC, ERR, ASL, AND, ADC, TAX,
		JSR, ORA, SBC, ERR, LSR, ORA, SBC, CLI, INS, ORA, SBC, ERR, LSR, ORA, SBC, TAY,
		BMI, ORA, SBC, ERR, LSR, ORA, SBC, SEI, DES, ORA, SBC, ERR, LSR, ORA, SBC, TBA,
		RTI, XOR, INX, ERR, ROL, XOR, STZ, CLF, PHP, XOR, STZ, ERR, ROL, XOR, STZ, TXA,
		BVC, XOR, INY, ERR, ROL, XOR, STZ, SEF, PLP, XOR, STZ, ERR, ROL, XOR, STZ, TXY,
		RTS, LTA, DEX, ERR, ROR, LTA, STA, INC, PHA, LTA, STA, ERR, ROR, LTA, STA, TYA,
		BVS, LTA, DEY, ERR, ROR, LTA, STA, DEC, PLA, LTA, STA, ERR, ROR, LTA, STA, TYX,
		CLV, LTB, ADC, ERR, CMP, LTB, STB, ERR, PHB, LTB, STB, ERR, CMP, LTB, STB, TXS,
		BCC, LTB, SBC, ERR, CMP, LTB, STB, ERR, PLB, LTB, STB, ERR, CMP, LTB, STB, TSX,
		MUL, LTX, AND, ERR, CPX, LTX, STX, ERR, PHX, LTX, INC, ERR, BIT, LTX, STX, THD,
		BCS, LTX, ORA, ERR, CPX, LTX, STX, ERR, PLX, LTX, INC, ERR, CPX, LTX, STX, TDH,
		DIV, LTY, XOR, ERR, CPY, LTY, STY, ERR, PHY, LTY, STY, ERR, CPY, LTY, DEC, BRA,
		BNE, LTY, CMP, ERR, CPY, LTY, STY, ERR, PLY, LTY, STY, ERR, BIT, LTY, DEC, JMP,
		MOD, LTV, CPX, ERR, CMD, LTD, STD, ERR, PHD, LTD, STD, ERR, CMP, LTD, STD, JMP,
		BEQ, LTD, CPY, ERR, CMD, LTD, STD, ERR, PLD, LTD, STD, ERR, CMD, LTD, STD, JSR
	};

	// tick function
	void tick(CPU& cpu) {
		if (cpu.halt || cpu.wait)
			return;

		bt opcode = nextByte(cpu);
		opcodeTable[opcode](cpu, opcodeMode[opcode]);
	};
};
