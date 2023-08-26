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
        FILE *fp = fopen(filename, "r");
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
        SP = 0x01FF;
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
    void SetGenericStatus( Byte Value ) {
        Z = (Value == 0); 
        N = (Value & 0b10000000) > 0; 
    }

    void LSRSetStatus( Byte BitZero, Byte result ) {
        C = BitZero;
        Z = (result == 0);
        N = (result & 0b10000000) > 0;
    }

    void ROLSetStatus( Byte EffectedByte, Byte OldByte ) {
        SetGenericStatus( EffectedByte );
        C = (OldByte & 0b00000001) > 0;
    }

    void RORSetStatus( Byte EffectedByte, Byte OldByte ) {
        SetGenericStatus( EffectedByte );
        C = (OldByte & 0b10000000) > 0;
    }

    void LSR( Byte& Value, u32& Cycles ) { // Handles SetStatus and takes 1 cycle
        Byte BitZero = (0b00000001 & Value);
        Value = (Value >> 1);
        Cycles--;
        LSRSetStatus( BitZero, Value );
    }

    void ROL( Byte& Value, u32& Cycles ) { // Handles SetStatus and takes 1 cycle
        Byte OldByte = Value;
        Value = ( Value << 1 );
        Value |= C; // Could also be +=
        ROLSetStatus( Value, OldByte );
        Cycles--;
    }

    void ROR( Byte& Value, u32& Cycles ) { // Handles SetStatus and takes 1 cycle
        Byte OldByte = Value;
        Value = ( Value << 1 );
        Value |= C; // Could also be +=
        RORSetStatus( Value, OldByte );
        Cycles--;
    }

    void CheckPageOverflow( Word Value, Byte Adder, u32& Cycles ) {
        Value &= 0x00FF;
        Value += Adder;
        if (Value > 0xFF) { Cycles--; }
    }

    Byte CombineFlags() {
        Byte flags[] = {C, Z, I, D, B, V, N};
        Byte status = 0;
        for (Byte f : flags) {
            status = (status << 1) | f;
        }
        return status;
    }

    void SeparateFlags( Byte status ) {
        C = (bool)( status & 0b01000000 );
        Z = (bool)( status & 0b00100000 );
        I = (bool)( status & 0b00010000 );
        D = (bool)( status & 0b00001000 );
        B = (bool)( status & 0b00000100 );
        V = (bool)( status & 0b00000010 );
        N = (bool)( status & 0b00000001 );
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

        // LSR
        INS_LSR_ACC = 0x4A,
        INS_LSR_ZP = 0x46,
        INS_LSR_ZPX = 0x56,
        INS_LSR_ABS = 0x4E,
        INS_LSR_ABX = 0x5E,

        // NOP
        INS_NOP = 0xEA,

        // ORA
        INS_ORA_IM = 0x09,
        INS_ORA_ZP = 0x05,
        INS_ORA_ZPX = 0x15,
        INS_ORA_ABS = 0x0D,
        INS_ORA_ABX = 0x1D,
        INS_ORA_ABY = 0x19,
        INS_ORA_IX = 0x01,
        INS_ORA_IY = 0x11,

        // ROL
        INS_ROL_ACC = 0x2A,
        INS_ROL_ZP = 0x26,
        INS_ROL_ZPX = 0x36,
        INS_ROL_ABS = 0x2E,
        INS_ROL_ABX = 0x3E,

        // ROR
        INS_ROR_ACC = 0x6A,
        INS_ROR_ZP = 0x66,
        INS_ROR_ZPX = 0x76,
        INS_ROR_ABS = 0x6E,
        INS_ROR_ABX = 0x7E,

        INS_PHA = 0x48,
        INS_PHP = 0x08,
        INS_PLA = 0x68,
        INS_PLP = 0x28,
        INS_JSR = 0x20;

    void Execute( u32 Cycles, Mem& memory ) {
        while (Cycles > 0) {
            Byte Ins = FetchByte( Cycles, memory );
            printf("Instruction: 0x%x\n", Ins);

            switch( Ins ) {
                // LDA
                case INS_LDA_IM: {
                    A = FetchByte( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_ZP: {
                    A = LoadZeroPage( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_ZPX: {
                    A = LoadZeroPageX( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_ABS: {
                    A = LoadAbsolute( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_ABX: {
                    A = LoadAbsoluteX( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_ABY: {
                    A = LoadAbsoluteY( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_IX: {
                    A = LoadIndirectX( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_LDA_IY: {
                    A = LoadIndirectY( Cycles, memory );
                    SetGenericStatus( A );
                } break;

                // LDX
                case INS_LDX_IM: {
                    X = FetchByte( Cycles, memory );
                    SetGenericStatus( X );
                } break;
                case INS_LDX_ZP: {
                    X = LoadZeroPage( Cycles, memory );
                    SetGenericStatus( X );
                } break;
                case INS_LDX_ZPY: {
                    X = LoadZeroPageY( Cycles, memory );
                    SetGenericStatus( X );
                } break;
                case INS_LDX_ABS: {
                    X = LoadAbsolute( Cycles, memory );
                    SetGenericStatus( X );
                } break;
                case INS_LDX_ABY: {
                    X = LoadAbsoluteY( Cycles, memory, false );
                    SetGenericStatus( X );
                } break;

                // LDY
                case INS_LDY_IM: {
                    Y = FetchByte( Cycles, memory );
                    SetGenericStatus( Y );
                } break;
                case INS_LDY_ZP: {
                    Y = LoadZeroPage( Cycles, memory );
                    SetGenericStatus( Y );
                } break;
                case INS_LDY_ZPX: {
                    Y = LoadZeroPageX( Cycles, memory );
                    SetGenericStatus( Y );
                } break;
                case INS_LDY_ABS: {
                    Y = LoadAbsolute( Cycles, memory );
                    SetGenericStatus( Y );
                } break;
                case INS_LDY_ABX: {
                    Y = LoadAbsoluteX( Cycles, memory );
                    SetGenericStatus( Y );
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

                // NOP
                case INS_NOP: {
                    Cycles--;
                } break;

                // ORA
                case INS_ORA_IM: {
                    A |= FetchByte( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_ZP: {
                    A |= LoadZeroPage( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_ZPX: {
                    A |= LoadZeroPageX( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_ABS: {
                    A |= LoadAbsolute( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_ABX: {
                    A |= LoadAbsoluteX( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_ABY: {
                    A |= LoadAbsoluteY( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_IX: {
                    A |= LoadIndirectX( Cycles, memory );
                    SetGenericStatus( A );
                } break;
                case INS_ORA_IY: {
                    A |= LoadIndirectY( Cycles, memory );
                    SetGenericStatus( A );
                } break;

                // ROL
                case INS_ROL_ACC: {
                    ROL( A, Cycles );
                } break;
                case INS_ROL_ZP: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPage( Cycles, memory, ZeroPageAddr );
                    ROL( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_ROL_ZPX: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPageX( Cycles, memory, ZeroPageAddr );
                    ROL( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_ROL_ABS: {
                    Word AbsAddr;
                    Byte Value = LoadAbsolute( Cycles, memory, AbsAddr );
                    ROL( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles--;
                } break;
                case INS_ROL_ABX: {
                    Word AbsAddr;
                    Byte Value = LoadAbsoluteX( Cycles, memory, AbsAddr, false );
                    ROL( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles -= 2;
                } break;

                // ROR - untested but very similar to ROL
                case INS_ROR_ACC: {
                    ROR( A, Cycles );
                } break;
                case INS_ROR_ZP: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPage( Cycles, memory, ZeroPageAddr );
                    ROR( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_ROR_ZPX: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPageX( Cycles, memory, ZeroPageAddr );
                    ROR( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_ROR_ABS: {
                    Word AbsAddr;
                    Byte Value = LoadAbsolute( Cycles, memory, AbsAddr );
                    ROR( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles--;
                } break;
                case INS_ROR_ABX: {
                    Word AbsAddr;
                    Byte Value = LoadAbsoluteX( Cycles, memory, AbsAddr, false );
                    ROR( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles -= 2;
                } break;
                

                // Implied instructions
                case INS_PHA: {
                    memory[SP] = A;
                    SP--;
                    Cycles -= 2;
                } break;
                case INS_PHP: {
                    Byte status = CombineFlags();
                    memory[SP] = status;
                    SP--;
                    Cycles -= 2;
                } break;
                case INS_PLA: {
                    SP++;
                    A = memory[SP];    
                    Cycles -= 3;
                    SetGenericStatus( A );
                } break;
                case INS_PLP: {
                    SP++;
                    SeparateFlags(memory[SP]);   
                    Cycles -= 3;
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

    // mem[0x0200] = CPU::INS_LDA_IM;
    // mem[0x0201] = 0b00000001;
    // mem[0x0202] = CPU::INS_ROL_ACC;

    mem[0x6502] = 0x65;

    // end - inline cheat code
    mem.LoadFile("test.bin");
    cpu.Execute( 9, mem );

    printf("A = %x\n", cpu.A);
    printf("0x%x\n", mem[0x01FF]);


    return 0;
}
