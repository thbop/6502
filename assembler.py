import argparse
from os import getcwd

# Parse command line arguments
parser = argparse.ArgumentParser(description='Simple Assembler for my 6502 emulation project')

parser.add_argument('--file', type=str, help='The required file specifier')

args = parser.parse_args()

try:
    file = open(args.file)
    s = file.readlines()
    file.close()

except FileNotFoundError:
    raise FileNotFoundError(f'The file "{args.file}" could not be found in the current working directory:\n---> "{getcwd()}"')

base_file_name = args.file[ : args.file.find('.s')]

address_mode_opcodes = {
    'ACC': { # Or implied
        'lsr': '4A',
        'nop': 'EA',
        'pha': '48',
        'php': '08',
        'pla': '68',
        'plp': '28',
        'rol': '2A',
        'ror': '6A',
        'asl': '0A'
    },
    'IM': { # Or relative
        'lda': 'A9',
        'ldx': 'A2',
        'ldy': 'A0',
        'ora': '09',
        'adc': '69',
        'and': '29',
        'bcc': '90'
    },
    'ZP': {
        'lda': 'A5',
        'ldx':'A6',
        'ldy': 'A4',
        'lsr': '46',
        'ora': '05',
        'rol': '26',
        'ror': '66',
        'adc': '65',
        'and': '25',
        'asl': '06'
    },
    'ZPX': {
        'lda': 'B5',
        'ldy': 'B4',
        'lsr': '56',
        'ora': '15',
        'rol': '36',
        'ror': '76',
        'adc': '75',
        'and': '35',
        'asl': '16'
    },
    'ZPY': {
        'ldx':'B6'
    },
    'ABS':{
        'lda': 'AD',
        'ldx':'AE',
        'ldy': 'AC',
        'lsr': '4E',
        'ora': '0D',
        'rol': '2E',
        'ror': '6E',
        'adc': '6D',
        'and': '2D',
        'asl': '0E'
    },
    'ABX': {
        'lda':'BD',
        'ldy': 'BC',
        'lsr': '5E',
        'ora': '1D',
        'rol': '3E',
        'ror': '7E',
        'adc': '7D',
        'and': '3D',
        'asl': '1E'
    },
    'ABY': {
        'lda': 'B9',
        'ldx':'BE',
        'ora': '19',
        'and': '39'
    }
}

out = ''

# Assemble
def get_opcode(address_mode, args):
    return address_mode_opcodes[address_mode][args[0].lower().replace('\n', '')]

def parse_addressing_mode(args):
    # Some crude parsing...
    global out
    if len(args) == 1:
        out += get_opcode('ACC', args) + '\n'
    else:
        arg1 = args[1].replace('\n', '')
        if arg1[0] == '#': # Immediate
            out += get_opcode('IM', args) + '\n' + arg1[1:] + '\n'

        elif arg1[0] == '$':
            if len(arg1) == 3: # Zero Page
                out += get_opcode('ZP', args) + '\n' + arg1[1:] + '\n'
            
            elif len(arg1) == 4 and arg1[3] == ',': # Zero Page X or Y
                out += get_opcode('ZP'+args[2][0], args) + '\n' + arg1[1:-1] + '\n'
            
            elif len(arg1) == 5: # Absolute
                out += get_opcode('ABS', args) + '\n' + arg1[1:][2:] + '\n' + arg1[1:][:2] + '\n'
            
            elif len(arg1) == 6 and arg1[5] == ',': # Absolute X or Y
                out += get_opcode('AB'+args[2][0], args) + '\n' + arg1[1:][2:-1] + '\n' + arg1[1:][:2] + '\n'




for l in s:
    gotoNext = False
    args = l.split(' ') # Separate arguments
    # Ignore comments
    if args[0][0] == ';':
        gotoNext = True

    if not gotoNext:
        for i, arg in enumerate(args):
            if arg == '' or arg[0] == ';':
                args = args[:i]
        
        # Parse
        if l != '\n':
            parse_addressing_mode(args)


# An extra \n char may be stored in out
if out[-1] == '\n':
    out = out[:-1]

# Output file
with open(base_file_name + '.bin', 'w') as f:
    f.write(out)
