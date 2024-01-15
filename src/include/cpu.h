/*
    References:
        1. The initial inspiration and reference for this project was provided by Dave Poo's video:
        https://youtu.be/qJgsuQoy9bc
        2. https://web.archive.org/web/20200610195848/http://www.obelisk.me.uk/6502/
        3. https://6502.co.uk/
        4. http://www.emulator101.com/6502-addressing-modes.html
*/


#ifndef CPU_H
#define CPU_H



using SByte = char; // Signed Byte
using Byte = unsigned char; // 8 bit, FF
using Word = unsigned short; // 16 bit, FFFF

using u32 = unsigned int;


// struct Printer {
//     // A digital printer that outputs a specific memory address to a file.
//     FILE *fp = fopen("printer.txt", "a");

//     void Print( Byte Value ) {
//         fprintf(fp, "%d\n", Value);
//     }

//     void PowerOff() {
//         fclose(fp);
//     }
    
// };

struct Mem {
    static constexpr u32 MAX_MEM = 1024 * 64;
    /*
        $0000 - $00FF (256): ZERO PAGE - Fast memory like registers
        $0100 - $01FF (256): System stack
        $0200 - $FFF8 (65017): Regular RAM
        $FFF9            : Printer address
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

    void LoadFile( const char *filename, Word startAddress=0x8000 ) {
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
    void PrintRegion( Word start, Word end ) {
        for (Word i = start; i <= end; i++) {
            printf("%p: %p\n", i, Data[i]);
        }
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
        PC = 0x8000; // Natively 0xFFFC
        SP = 0x01FF;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;

        memory.Initialize();
    }

    // Fetch, Read, Write
    Byte FetchByte( u32& Cycles, Mem& memory, bool IncrementPC = true ) {
        Byte Data = memory[PC];
        if ( IncrementPC ) { PC++; }
        Cycles--;
        return Data;
    }
    SByte FetchSByte( u32& Cycles, Mem& memory, bool IncrementPC = true ) {
        return FetchByte( Cycles, memory, IncrementPC );
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

    void BITSsetStatus( Byte Value ) {
        Z = !(Value & A);
        V = ( 0b01000000 & Value ) > 0;
        N = ( 0b10000000 & Value ) > 0;
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

    void ADC( Byte Value ) { // Takes 0 cycles and sets status
        /*
            Code adapted from: https://github.com/davepoo/6502Emulator/blob/master/6502/6502Lib/src/private/m6502.cpp
            Negative numbers https://stackoverflow.com/questions/45397461/are-the-bytes-stored-in-6502-memory-signed-or-unsigned
                $80 | $FF = -128 | -1
                $00 | $7F =   0  | 127
        */
        const bool AreSignsSame = !( (A ^ Value) & 0b10000000 );

        Word Sum = A;
        Sum += Value + C;
        A = ( Sum & 0xFF );
                    
        // Flag set
        C = Sum > 0xFF;
        V = AreSignsSame && ( (A ^ Value) & 0b10000000 ); // I still need to work out how this line works
        SetGenericStatus( A );
    }

    void ASL( Byte& Value, u32& Cycles ) { // handles flags and takes 1 cycle
        C = ( Value & 0b10000000 ) > 0;
        Value = ( Value << 1 );
        Cycles--;
        SetGenericStatus( Value );
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

    void WriteZeroPage( Byte& From, u32& Cycles, Mem& memory ) { // 2 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        memory[ZeroPageAddr] = From;
        Cycles--;
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

    void WriteZeroPageX( Byte& From, u32& Cycles, Mem& memory ) { // 3 cycles - untested
        Byte ZeroPageAddr = FetchByte( Cycles, memory ) + X;
        memory[ZeroPageAddr] = From;
        Cycles -= 2;
    }

    Byte LoadZeroPageY( u32& Cycles, Mem& memory ) { // 3 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        ZeroPageAddr += Y; // Wraps around the Zero Page
        Cycles--;

        return ReadByte( ZeroPageAddr, Cycles, memory );
    }

    void WriteZeroPageY( Byte& From, u32& Cycles, Mem& memory ) { // 3 cycles - untested
        Byte ZeroPageAddr = FetchByte( Cycles, memory ) + Y;
        memory[ZeroPageAddr] = From;
        Cycles -= 2;
    }

    Byte LoadAbsolute( u32& Cycles, Mem& memory ) { // 3 cycles
        Word AbsAddr = FetchWord( Cycles, memory );
        return ReadByte( AbsAddr, Cycles, memory );
    }
    Byte LoadAbsolute( u32& Cycles, Mem& memory, Word& AbsAddr ) { // 3 cycles
        AbsAddr = FetchWord( Cycles, memory );
        return ReadByte( AbsAddr, Cycles, memory );
    }

    void WriteAbsolute( Byte& From, u32& Cycles, Mem& memory ) { // 3 cycles
        Word AbsAddr = FetchWord( Cycles, memory );
        memory[AbsAddr] = From;
        Cycles--;


        // Printer stuff; not very realistic but still funny
        // if (AbsAddr == 0xFFF9) {
        //     printer.Print(A);
        // }

        // Graphics monitoring
        // Bad code structure, maybe: https://stackoverflow.com/questions/9568150/what-is-a-c-delegate (Option 3)
        if ( memory[0xD013] ) {
            W_PushText( memory[0xD012] );
        }
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

    void WriteAbsoluteX( Byte& From, u32& Cycles, Mem& memory ) { // 4 cycles - untested
        Word AbsAddr = FetchWord( Cycles, memory ) + X;
        memory[AbsAddr] = From;
        Cycles -= 2;
    }

    Byte LoadAbsoluteY( u32& Cycles, Mem& memory, bool PageCrossable=true ) { // 3-4 cycles
        Word AbsAddr = FetchWord( Cycles, memory );
        AbsAddr += Y;
        if (PageCrossable) { CheckPageOverflow( AbsAddr, Y, Cycles ); }
        return ReadByte( AbsAddr, Cycles, memory );
    }

    void WriteAbsoluteY( Byte& From, u32& Cycles, Mem& memory ) { // 4 cycles - untested
        Word AbsAddr = FetchWord( Cycles, memory ) + Y;
        memory[AbsAddr] = From;
        Cycles -= 2;
    }

    Byte LoadIndirectX( u32& Cycles, Mem& memory ) { // 5 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );
        ZeroPageAddr += X; // Wrap around Zero Page
        Cycles--;

        Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
        return ReadByte( TargetAddr, Cycles, memory );
    }

    void WriteIndirectX( Byte& From, u32& Cycles, Mem& memory ) { // 5 cycles - untested
        Byte ZeroPageAddr = FetchWord( Cycles, memory ) + X;
        Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
        memory[TargetAddr] = From;

        Cycles--;
    }

    Byte LoadIndirectY( u32& Cycles, Mem& memory ) { // 4-5 cycles
        Byte ZeroPageAddr = FetchByte( Cycles, memory );

        Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory );
        CheckPageOverflow( TargetAddr, Y, Cycles );
        TargetAddr += Y;
                    
        return ReadByte( TargetAddr, Cycles, memory );
    }

    void WriteIndirectY( Byte& From, u32& Cycles, Mem& memory ) { // 5 cycles - untested
        Byte ZeroPageAddr = FetchWord( Cycles, memory );
        Word TargetAddr = ReadWord( ZeroPageAddr, Cycles, memory ) + Y;
        memory[TargetAddr] = From;

        Cycles--;
    }

    // Stack operations
    void Push( Byte Value, Mem& memory ) {
        memory[SP] = Value;
        SP--;
    }
    void Push( Word Value, u32& Cycles, Mem& memory ) {
        memory.WriteWord( Value, SP - 1, Cycles );
        SP -= 2;
    }
    Byte Pull( Mem& memory ) {
        SP++;
        Byte Value = memory[SP];
        return Value;
    }
    Word Pull( u32& Cycles, Mem& memory ) {
        SP++;
        Word Value = ReadWord( SP, Cycles, memory );
        SP++;
        return Value;
    }

    // opcodes
    static constexpr Byte
        // ADC
        INS_ADC_IM = 0x69,
        INS_ADC_ZP = 0x65,
        INS_ADC_ZPX = 0x75,
        INS_ADC_ABS = 0x6D,
        INS_ADC_ABX = 0x7D,
        INS_ADC_ABY = 0x79,
        INS_ADC_IX = 0x61,
        INS_ADC_IY = 0x71,

        // AND
        INS_AND_IM = 0x29,
        INS_AND_ZP = 0x25,
        INS_AND_ZPX = 0x35,
        INS_AND_ABS = 0x2D,
        INS_AND_ABX = 0x3D,
        INS_AND_ABY = 0x39,
        INS_AND_IX = 0x21,
        INS_AND_IY = 0x31,

        // Branch
        INS_BCC = 0x90,
        INS_BCS = 0xB0,
        INS_BEQ = 0xF0,
        INS_BMI = 0x30,
        INS_BNE = 0xD0,
        INS_BPL = 0x10,
        INS_BVC = 0x50,
        INS_BVS = 0x70,

        // ASL
        INS_ASL_ACC = 0x0A,
        INS_ASL_ZP = 0x06,
        INS_ASL_ZPX = 0x16,
        INS_ASL_ABS = 0x0E,
        INS_ASL_ABX = 0x1E,

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

        // STA
        INS_STA_ZP = 0x85,
        INS_STA_ZPX = 0x95,
        INS_STA_ABS = 0x8D,
        INS_STA_ABX = 0x9D,
        INS_STA_ABY = 0x99,
        INS_STA_IX = 0x81,
        INS_STA_IY = 0x91,

        // STX
        INS_STX_ZP = 0x86,
        INS_STX_ZPY = 0x96,
        INS_STX_ABS = 0x8E,

        // STY
        INS_STY_ZP = 0x84,
        INS_STY_ZPX = 0x94,
        INS_STY_ABS = 0x8C,

        // BIT
        INS_BIT_ZP = 0x24,
        INS_BIT_ABS = 0x2C,

        // Implied
        INS_PHA = 0x48,
        INS_PHP = 0x08,
        INS_PLA = 0x68,
        INS_PLP = 0x28,
        INS_RTS = 0x60,

        // Jumps
        INS_JMP_ABS = 0x4C,
        INS_JMP_IND = 0x6C,
        INS_JSR = 0x20;

    void Execute( u32 Cycles, Mem& memory, bool debug ) {
        // while ((int)Cycles > 0) {
            Byte Ins = FetchByte( Cycles, memory );

            // Debug
            if (debug) {
                printf("PC: %p | ", PC);
                printf("Instruction: 0x%x | ", Ins);
                printf("A: 0x%x | ", A);
                printf("Cycles: %d\n", Cycles);
            }
            

            switch( Ins ) {
                // ADC
                case INS_ADC_IM: {
                    Byte Value = FetchByte( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_ZP: {
                    Byte Value = LoadZeroPage( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_ZPX: {
                    Byte Value = LoadZeroPageX( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_ABS: {
                    Byte Value = LoadAbsolute( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_ABX: {
                    Byte Value = LoadAbsoluteX( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_ABY: {
                    Byte Value = LoadAbsoluteY( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_IX: {
                    Byte Value = LoadIndirectX( Cycles, memory );
                    ADC( Value );
                } break;
                case INS_ADC_IY: {
                    Byte Value = LoadIndirectY( Cycles, memory );
                    ADC( Value );
                } break;

                // AND
                case INS_AND_IM: {
                    Byte Value = FetchByte( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_ZP: {
                    Byte Value = LoadZeroPage( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_ZPX: {
                    Byte Value = LoadZeroPageX( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_ABS: {
                    Byte Value = LoadAbsolute( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_ABX: {
                    Byte Value = LoadAbsoluteX( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_ABY: {
                    Byte Value = LoadAbsoluteY( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_IX: {
                    Byte Value = LoadIndirectX( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;
                case INS_AND_IY: {
                    Byte Value = LoadIndirectY( Cycles, memory );
                    A &= Value;
                    SetGenericStatus( A );
                } break;

                // ASL
                case INS_ASL_ACC: {
                    ASL( A, Cycles );
                } break;
                case INS_ASL_ZP: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPage( Cycles, memory, ZeroPageAddr );
                    ASL( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_ASL_ZPX: {
                    Byte ZeroPageAddr;
                    Byte Value = LoadZeroPageX( Cycles, memory, ZeroPageAddr );
                    ASL( Value, Cycles );
                    memory[ZeroPageAddr] = Value;
                    Cycles--;
                } break;
                case INS_ASL_ABS: {
                    Word AbsAddr;
                    Byte Value = LoadAbsolute( Cycles, memory, AbsAddr );
                    ASL( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles--;
                } break;
                case INS_ASL_ABX: {
                    Word AbsAddr;
                    Byte Value = LoadAbsoluteX( Cycles, memory, AbsAddr, false );
                    ASL( Value, Cycles );
                    memory[AbsAddr] = Value;
                    Cycles -= 2;
                } break;

                // Branch
                case INS_BCC: {
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( !C ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; } // Increment PC if failure, otherwise it tries to parse argument as instruction
                } break;
                case INS_BCS: {
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( C ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;
                case INS_BEQ: {
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( Z ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;
                case INS_BMI: { // Untested
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( N ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;
                case INS_BNE: { // Untested
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( !Z ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;
                case INS_BPL: { // Untested
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( !N ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;
                case INS_BVC: { // Untested
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( !V ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;
                case INS_BVS: { // Untested
                    SByte Offset = FetchSByte( Cycles, memory, false );
                    if ( V ) {
                        Cycles--;
                        Word OldPC = PC;
                        PC += Offset;
                        // Check if page is crossed
                        if ( ( PC & 0xFF00 ) > ( OldPC & 0xFF00 ) ) {
                            Cycles -= 2;
                        }
                    }
                    else { PC++; }
                } break;


                // BIT
                case INS_BIT_ZP: {
                    Byte Value = LoadZeroPage( Cycles, memory );
                    BITSsetStatus( Value );
                } break;
                case INS_BIT_ABS: {
                    Byte Value = LoadAbsolute( Cycles, memory );
                    BITSsetStatus( Value );
                } break;

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

                // STA
                case INS_STA_ZP: { // None of the these affect flags
                    WriteZeroPage( A, Cycles, memory );
                } break;
                case INS_STA_ZPX: {
                    WriteZeroPageX( A, Cycles, memory );
                } break;
                case INS_STA_ABS: {
                    WriteAbsolute( A, Cycles, memory );
                } break;
                case INS_STA_ABX: {
                    WriteAbsoluteX( A, Cycles, memory );
                } break;
                case INS_STA_ABY: {
                    WriteAbsoluteY( A, Cycles, memory );
                } break;
                case INS_STA_IX: {
                    WriteIndirectX( A, Cycles, memory );
                } break;
                case INS_STA_IY: {
                    WriteIndirectY( A, Cycles, memory );
                } break;

                // STX - untested
                case INS_STX_ZP: {
                    WriteZeroPage( X, Cycles, memory );
                } break;
                case INS_STX_ZPY: {
                    WriteZeroPageY( X, Cycles, memory );
                } break;
                case INS_STX_ABS: {
                    WriteAbsolute( X, Cycles, memory );
                } break;

                // STY - untested
                case INS_STY_ZP: {
                    WriteZeroPage( Y, Cycles, memory );
                } break;
                case INS_STY_ZPX: {
                    WriteZeroPageX( Y, Cycles, memory );
                } break;
                case INS_STY_ABS: {
                    WriteAbsolute( Y, Cycles, memory );
                } break;

                
                // Implied instructions
                case INS_PHA: {
                    Push( A, memory );
                    Cycles -= 2;
                } break;
                case INS_PHP: {
                    Byte status = CombineFlags();
                    Push( status, memory );
                    Cycles -= 2;
                } break;
                case INS_PLA: {
                    A = Pull( memory );  
                    Cycles -= 3;
                    SetGenericStatus( A );
                } break;
                case INS_PLP: {
                    SeparateFlags(Pull( memory ));   
                    Cycles -= 3;
                } break;
                case INS_RTS: {
                    PC = Pull( Cycles, memory );
                    Cycles -= 3;
                } break;

                // Jump
                case INS_JMP_ABS: { // Untested
                    PC = FetchWord( Cycles, memory );
                } break;
                case INS_JMP_IND: { // Untested
                    Word IndirectAddr = FetchWord( Cycles, memory );
                    PC = ReadWord( IndirectAddr, Cycles, memory );
                } break;
                case INS_JSR: { // Review, not done.
                    Word SubAddr = FetchWord( Cycles, memory );
                    Push( PC, Cycles, memory );
                    PC = SubAddr;
                    Cycles--;
                } break;

                default: {
                    if ( debug ) { printf("Instruction not handled 0x%x\n", Ins); }
                } break;
            }
        }
    // }
    void printRegFlags() {
        printf("A = 0x%x\n", A);
        printf("X = 0x%x\n", X);
        printf("Y = 0x%x\n\n", Y);
        printf("C = %d\n", C);
        printf("Z = %d\n", Z);
        printf("I = %d\n", I);
        printf("D = %d\n", D);
        printf("B = %d\n", B);
        printf("V = %d\n", V);
        printf("N = %d\n", N);
    }
};


// int main() {
//     Mem mem;
//     CPU cpu;
//     Printer printer;
//     cpu.Reset( mem );

//     // start - inline cheat code
//     // cpu.X = 0x02;
//     // mem[0x0012] = 0b11100101;
    

//     // mem[0x0200] = CPU::INS_ASL_ABX;
//     // mem[0x0201] = 0x00;
//     // mem[0x0202] = 0x65;

//     // end - inline cheat code
//     mem.LoadFile("test.bin");
//     cpu.Execute( 500, mem, printer, true );

//     printf("A = 0x%x\n", cpu.A);
//     printf("C = %d\n", cpu.C);
//     printf("Z = %d\n", cpu.Z);
//     printf("I = %d\n", cpu.I);
//     printf("D = %d\n", cpu.D);
//     printf("B = %d\n", cpu.B);
//     printf("V = %d\n", cpu.V);
//     printf("N = %d\n", cpu.N);

//     printf("0x%x\n", mem[0x10]);
//     printf("0x%x\n", mem[0x11]);
//     printf("0x%x\n", mem[0x12]);

//     // Byte a = -16;
//     // printf("%x\n", a);

//     printer.PowerOff();

//     return 0;
// }

#endif