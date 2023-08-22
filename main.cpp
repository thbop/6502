/*
    All of this code was typed by Thbop
    References:
        1. The initial inspiration and reference for this project was provided by Dave Poo's video:
        https://youtu.be/qJgsuQoy9bc
        2. https://web.archive.org/web/20200610195848/http://www.obelisk.me.uk/6502/
        3. https://6502.co.uk/
        4. http://www.emulator101.com/6502-addressing-modes.html
*/

#include <stdio.h>
#include <stdlib.h>


using Byte = unsigned char; // 8 bit, FF
using Word = unsigned short; // 16 bit, FFFF

using u32 = unsigned int;

struct Mem {
    static constexpr u32 MAX_MEM = 1024 * 64;
    /*
        $0000 - $00FF (256): ZERO PAGE - Fast memory like registers
        $0100 - $01FF (256): System stack
        $0200 - $FFF9 (65018): Regular RAM
        $FFFA - $FFFF (6): Other stuff. *2 /architecture.html
    */
    Byte Data[MAX_MEM];

    void Initialize() {
        for ( u32 i = 0; i < MAX_MEM; i++ ) {
            Data[i] = 0;
        }
    }

    // Read 1 byte
    Byte operator[]( u32 Address ) const {
        if (Address > 0xFFFF) {
            printf("Memory operation reading address \"%x\" exceeds memory limit of $FFFF\n", Address);
        }
        return Data[Address];
    }
    // write 1 byte
    Byte& operator[]( u32 Address ) {
        if (Address > 0xFFFF) {
            printf("Memory operation writing address \"%x\" exceeds memory limit of $FFFF\n", Address);
        }
        return Data[Address];
    }

    // Writes two bytes
    void WriteWord( Word Value, u32 Address, u32& Cycles ) {
        Data[Address] = Value & 0xFF;
        Data[Address + 1] = (Value >> 8);
        Cycles -= 2;
    }

    void LoadFile( const char *filename, Word startAddress=0x0200 ) {
        // Code adapted from that written by Bard (https://bard.google.com/)
        FILE *fp = fopen("test.bin", "r");
        if (fp == NULL) {
            printf("Could not open file.\n");
        }
        char line[MAX_MEM - 0x206]; // Max memory - 256 * 2 + 6
        for (int i = 0; fgets(line, sizeof(line), fp); i++) {
            int value = strtol(line, NULL, 16);
            Data[i + startAddress] = (Byte)value;
        }
        fclose(fp);
    }
};


struct CPU {
    Word PC; // Program Counter
    Word SP; // Stack Pointer

    Byte A, X, Y; // Registers

    Byte C : 1; // Carry
    Byte Z : 1; // Zero
    Byte I : 1; // Interrupt Disable
    Byte D : 1; // Decimal Mode
    Byte B : 1; // Break Command
    Byte V : 1; // Overflow
    Byte N : 1; // Negative


    void Reset( Mem& memory ) {
        PC = 0x0200; // Natively 0xFFFC
        SP = 0x0100;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;

        memory.Initialize();
    }

    // Fetch, Read, Write
    Byte FetchByte( u32& Cycles, Mem& memory ) {
        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;
    }

    Word FetchWord( u32& Cycles, Mem& memory ) {
        Word Data = memory[PC];
        PC++;

        Data |= (memory[PC] << 8 ); // Combines two Bytes into a Word
        PC++;
        Cycles -= 2;

        return Data;
    }

    Byte ReadByte( Byte Address, u32& Cycles, Mem& memory ) {
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }
    Byte ReadByte( Word Address, u32& Cycles, Mem& memory ) {
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    Word ReadWord( Byte Address, u32& Cycles, Mem& memory ) {
        Word Data = memory[Address];
        Data |= (memory[Address + 1] << 8 ); // Combines two Bytes into a Word
        Cycles -= 2;

        return Data;
    }
    Word ReadWord( Word Address, u32& Cycles, Mem& memory ) {
        Word Data = memory[Address];
        Data |= (memory[Address + 1] << 8 ); // Combines two Bytes into a Word
        Cycles -= 2;

        return Data;
    }

    // Helper functions
    void LDASetStatus() {
        // Set means is value of 1 or True
        Z = (A == 0); // Set if A = 0
        N = (A & 0b10000000) > 0; // Set if bit 7 of A is set
    }

    void LDXSetStatus() {
        Z = (X == 0);
        N = (X & 0b10000000) > 0;
    }

    void LDYSetStatus() {
        Z = (Y == 0);
        N = (X & 0b10000000) > 0;
    }

    void LSRSetStatus( Byte BitZero, Byte result ) {
        C = BitZero;
        Z = (result == 0);
        N = (result & 0b10000000) > 0;
    }

    void LSR( Byte& Value, u32& Cycles ) { // Handles SetStatus and takes 1 cycle
        Byte BitZero = (0b00000001 & Value);
        Value = (Value >> 1);
        Cycles--;
        LSRSetStatus( BitZero, Value );
    }

    void CheckPageOverflow( Word Value, Byte Adder, u32& Cycles ) {
        Value &= 0x00FF;
        Value += Adder;
        if (Value > 0xFF) { Cycles--; }
    }

    // Addressing modes
    Byte LoadZeroPage( u32& Cycles, Mem& memory ) { // 2 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        return ReadByte( ZeroPageAddr, Cycles, memory );
    }
    Byte LoadZeroPage( u32& Cycles, Mem& memory, Byte& ZeroPageAddr ) { // 2 cycles
        ZeroPageAddr = FetchByte( Cycles, memory );
        return ReadByte( ZeroPageAddr, Cycles, memory );
    }
    
    Byte LoadZeroPageX( u32& Cycles, Mem& memory ) { // 3 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        ZeroPageAddr += X; // Wraps around the Zero Page
        Cycles--;

        return ReadByte( ZeroPageAddr, Cycles, memory );
    }
    Byte LoadZeroPageX( u32& Cycles, Mem& memory, Byte& ZeroPageAddr ) { // 3 cycles
        ZeroPageAddr = FetchByte( Cycles, memory );
        ZeroPageAddr += X; // Wraps around the Zero Page
        Cycles--;

        return ReadByte( ZeroPageAddr, Cycles, memory );
    }

    Byte LoadZeroPageY( u32& Cycles, Mem& memory ) { // 3 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        ZeroPageAddr += Y; // Wraps around the Zero Page
        Cycles--;

        return ReadByte( ZeroPageAddr, Cycles, memory );
    }

    Byte LoadAbsolute( u32& Cycles, Mem& memory ) { // 3 cycles
        Word AbsAddr = FetchWord( Cycles, memory );
        return ReadByte( AbsAddr, Cycles, memory );
    }
    Byte LoadAbsolute( u32& Cycles, Mem& memory, Word& AbsAddr ) { // 3 cycles
        AbsAddr = FetchWord( Cycles, memory );
        return ReadByte( AbsAddr, Cycles, memory );
    }

    Byte LoadAbsoluteX( u32& Cycles, Mem& memory, bool PageCrossable=true ) { // 3-4 cycles
        Word AbsAddr = FetchWord( Cycles, memory );
        AbsAddr += X;
        if (PageCrossable) { CheckPageOverflow( AbsAddr, X, Cycles ); } // Checks if the LSB crossed the page
        return ReadByte( AbsAddr, Cycles, memory );
    }
    Byte LoadAbsoluteX( u32& Cycles, Mem& memory, Word& AbsAddr, bool PageCrossable=true ) { // 3-4 cycles
        AbsAddr = FetchWord( Cycles, memory );
        AbsAddr += X;
        if (PageCrossable) { CheckPageOverflow( AbsAddr, X, Cycles ); } // Checks if the LSB crossed the page
        return ReadByte( AbsAddr, Cycles, memory );
    }

    Byte LoadAbsoluteY( u32& Cycles, Mem& memory, bool PageCrossable=true ) { // 3-4 cycles
        Word AbsAddr = FetchWord( Cycles, memory );
        AbsAddr += Y;
        if (PageCrossable) { CheckPageOverflow( AbsAddr, Y, Cycles ); }
        return ReadByte( AbsAddr, Cycles, memory );
    }

    Byte LoadIndirectX( u32& Cycles, Mem& memory ) { // 5 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        ZeroPageAddr += X; // Wrap around Zero Page
        Cycles--;

        Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
        return ReadByte( TargetAddr, Cycles, memory );
    }

    Byte LoadIndirectY( u32& Cycles, Mem& memory ) { // 4-5 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );

        Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
        CheckPageOverflow( TargetAddr, Y, Cycles );
        TargetAddr += Y;
                    
        return ReadByte( TargetAddr, Cycles, memory );
    }

    // opcodes
    static constexpr Byte
        // LDA
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_LDA_ABS = 0xAD,
        INS_LDA_ABX = 0xBD,
        INS_LDA_ABY = 0xB9,
        INS_LDA_IX = 0xA1,
        INS_LDA_IY = 0xB1,

        // LDX
        INS_LDX_IM = 0xA2,
        INS_LDX_ZP = 0xA6,
        INS_LDX_ZPY = 0xB6,
        INS_LDX_ABS = 0xAE,
        INS_LDX_ABY = 0xBE,

        // LDY
        INS_LDY_IM = 0xA0,
        INS_LDY_ZP = 0xA4,
        INS_LDY_ZPX = 0xB4,
        INS_LDY_ABS = 0xAC,
        INS_LDY_ABX = 0xBC,

        INS_LSR_ACC = 0x4A,
        INS_LSR_ZP = 0x46,
        INS_LSR_ZPX = 0x56,
        INS_LSR_ABS = 0x4E,
        INS_LSR_ABX = 0x5E,

        INS_JSR = 0x20;

    void Execute( u32 Cycles, Mem& memory ) {
        while (Cycles > 0) {
            Byte Ins = FetchByte( Cycles, memory );

            switch( Ins ) {
                // LDA
                case INS_LDA_IM: {
                    A = FetchByte( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ZP: {
                    A = LoadZeroPage( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ZPX: {
                    A = LoadZeroPageX( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ABS: {
                    A = LoadAbsolute( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ABX: {
                    A = LoadAbsoluteX( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ABY: {
                    A = LoadAbsoluteY( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_IX: {
                    A = LoadIndirectX( Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_IY: {
                    A = LoadIndirectY( Cycles, memory );
                    LDASetStatus();
                } break;

                // LDX
                case INS_LDX_IM: {
                    X = FetchByte( Cycles, memory );
                    LDXSetStatus();
                } break;
                case INS_LDX_ZP: {
                    X = LoadZeroPage( Cycles, memory );
                    LDXSetStatus();
                } break;
                case INS_LDX_ZPY: {
                    X = LoadZeroPageY( Cycles, memory );
                    LDXSetStatus();
                } break;
                case INS_LDX_ABS: {
                    X = LoadAbsolute( Cycles, memory );
                    LDXSetStatus();
                } break;
                case INS_LDX_ABY: {
                    X = LoadAbsoluteY( Cycles, memory, false );
                    LDXSetStatus();
                } break;

                // LDY
                case INS_LDY_IM: {
                    Y = FetchByte( Cycles, memory );
                    LDYSetStatus();
                } break;
                case INS_LDY_ZP: {
                    Y = LoadZeroPage( Cycles, memory );
                    LDYSetStatus();
                } break;
                case INS_LDY_ZPX: {
                    Y = LoadZeroPageX( Cycles, memory );
                    LDYSetStatus();
                } break;
                case INS_LDY_ABS: {
                    Y = LoadAbsolute( Cycles, memory );
                    LDYSetStatus();
                } break;
                case INS_LDY_ABX: {
                    Y = LoadAbsoluteX( Cycles, memory );
                    LDYSetStatus();
                } break;

                // LSR
                case INS_LSR_ACC: {
                    LSR( A, Cycles ); // LSR handles SetStatus
                } break;
                case INS_LSR_ZP: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPage( Cycles, memory, ZeroPageAddr );
                    LSR( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_LSR_ZPX: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPageX( Cycles, memory, ZeroPageAddr );
                    LSR( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_LSR_ABS: {
                    Word AbsAddr;
                    Byte Value = LoadAbsolute( Cycles, memory, AbsAddr );
                    LSR( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles--;
                } break;
                case INS_LSR_ABX: {
                    Word AbsAddr;
                    Byte Value = LoadAbsoluteX( Cycles, memory, AbsAddr, false );
                    LSR( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles -= 2;
                } break;

                case INS_JSR: {
                    Word SubAddr = FetchWord( Cycles, memory );
                    memory.WriteWord( PC - 1, SP, Cycles );
                    // Increment the Stack Pointer
                    Cycles--;
                    PC = SubAddr;
                    Cycles--;
                } break;

                default: {
                    printf("Instruction not handled $%x\n", Ins);
                } break;
            }
        }
    }
};


int main() {
    Mem mem;


    CPU cpu;
    cpu.Reset( mem );

    // start - inline cheat code
    mem[0x4576] = 0x12;

    mem[0x0200] = CPU::INS_LDX_IM;
    mem[0x0201] = 0x12;
    mem[0x0202] = CPU::INS_LSR_ABX;
    mem[0x0203] = 0x64;
    mem[0x0204] = 0x45;


    // end - inline cheat code
    // mem.LoadFile("test.s");
    cpu.Execute( 9, mem );

    // printf("A = %x\n", cpu.A);
    printf("%x\n", mem[0x4576]);



    return 0;
}
