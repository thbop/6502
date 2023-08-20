/*
    All of this code was typed by Thbop
    References:
        1. The initial inspiration and reference for this project was provided by Dave Poo's video:
        https://youtu.be/qJgsuQoy9bc
        2. https://web.archive.org/web/20200610195848/http://www.obelisk.me.uk/6502/
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
        PC = 0xFFFC;
        SP = 0x0100;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;

        memory.Initialize();
    }

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

    // opcodes
    static constexpr Byte
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_LDA_ABS = 0xAD,
        INS_LDA_ABX = 0xBD,
        INS_LDA_ABY = 0xB9,
        INS_LDA_IX = 0xA1,
        INS_LDA_IY = 0xB1,
        INS_JSR = 0x20;

    void LDASetStatus() {
        // Set means is value of 1 or True
        Z = (A == 0); // Set if A = 0
        N = (A & 0b10000000) > 0; // Set if bit 7 of A is set
    }

    void CheckPageOverflow( Word Value, Byte Adder, u32& Cycles ) {
        Value &= 0x00FF;
        Value += Adder;
        if (Value > 0xFF) { Cycles--; }
    }

    void Execute( u32 Cycles, Mem& memory ) {
        while (Cycles > 0) {
            Byte Ins = FetchByte( Cycles, memory );

            switch( Ins ) {
                case INS_LDA_IM: {
                    Byte Value = FetchByte( Cycles, memory );
                    A = Value;
                    LDASetStatus();
                } break;
                case INS_LDA_ZP: {
                    Byte ZeroPageAddr = FetchByte( Cycles, memory );
                    A = ReadByte( ZeroPageAddr, Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ZPX: {
                    Byte ZeroPageAddr = FetchByte( Cycles, memory );
                    ZeroPageAddr += X; // No overflow handling...
                    Cycles--;
                    A = ReadByte( ZeroPageAddr, Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ABS: {
                    Word AbsAddr = FetchWord( Cycles, memory );
                    A = ReadByte( AbsAddr, Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ABX: {
                    Word AbsAddr = FetchWord( Cycles, memory );
                    AbsAddr += X;
                    CheckPageOverflow( AbsAddr, X, Cycles ); // Checks if the LSB crossed the page

                    A = ReadByte( AbsAddr, Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_ABY: {
                    Word AbsAddr = FetchWord( Cycles, memory );
                    AbsAddr += Y;
                    CheckPageOverflow( AbsAddr, Y, Cycles );

                    A = ReadByte( AbsAddr, Cycles, memory );
                    LDASetStatus();
                } break;
                case INS_LDA_IX: {
                    Byte ZeroPageAddr = FetchByte( Cycles, memory );
                    ZeroPageAddr += X;
                    Cycles--;
                    
                    Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
                    A = ReadByte( TargetAddr, Cycles, memory );

                    LDASetStatus();
                } break;
                case INS_LDA_IY: {
                    Byte ZeroPageAddr = FetchByte( Cycles, memory );

                    Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
                    CheckPageOverflow( TargetAddr, Y, Cycles );
                    TargetAddr += Y; // No overflow handling
                    
                    
                    A = ReadByte( TargetAddr, Cycles, memory );

                    LDASetStatus();
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
    cpu.Y = 0x10;
    mem[0x86] = 0xFF;
    mem[0x87] = 0x40;
    mem[0x4100] = 0x64;

    mem[0xFFFC] = CPU::INS_LDA_IY;
    mem[0xFFFD] = 0x86;


    // end - inline cheat code

    cpu.Execute( 6, mem );

    printf("A = $%x\n", cpu.A);


    return 0;
}