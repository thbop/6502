#include <stdio.h>
#include <stdlib.h>


using Byte = unsigned char; // 8 bit, FF
using Word = unsigned short; // 16 bit, FFFF

using u32 = unsigned int;

struct Mem {
    static constexpr u32 MAX_MEM = 1024 * 64;
    /*
        $0000 - $00FF - ZERO PAGE: Fast memory like registers
        
    */
    Byte Data[MAX_MEM];

    void Initialize() {
        for ( u32 i = 0; i < MAX_MEM; i++ ) {
            Data[i] = 0;
        }
    }

    // Read 1 byte
    Byte operator[]( u32 Address ) const {
        // assert here Address is < MAX_MEM
        return Data[Address];
    }
    // write 1 byte
    Byte& operator[]( u32 Address ) {
        // assert here Address is < MAX_MEM
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

        Data |= (memory[PC] << 8 );
        PC++;
        Cycles -= 2;

        return Data;
    }

    Byte ReadByte( Byte Address, u32& Cycles, Mem& memory ) {
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    // opcodes
    static constexpr Byte
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_JSR = 0x20;

    void LDASetStatus() {
        // Set means is value of 1 or True
        Z = (A == 0); // Set if A = 0
        N = (A & 0b10000000) > 0; // Set if bit 7 of A is set
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
                
                case INS_JSR: {
                    Word SubAddr = FetchWord( Cycles, memory );
                    memory.WriteWord( PC - 1, SP, Cycles );
                    // Increment the Stack Pointer
                    Cycles--;
                    PC = SubAddr;
                    Cycles--;
                } break;

                default: {
                    printf("Instruction not handled %d", Ins);
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
    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x84;
    // end - inline cheat code

    cpu.Execute( 3, mem );


    return 0;
}