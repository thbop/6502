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
    'IM': {
        'lda': 'A9'
    },
    'ZP': {
        'lda': 'A5'
    },
    'ZPX': {
        'lda': 'B5'
    },
    'ZPY': {}
}

out = ''

# Assemble
def get_opcode(address_mode, args):
    return address_mode_opcodes[address_mode][args[0].lower()]

def parse_addressing_mode(args):
    # Some crude parsing...
    global out
    if args[1][0] == '#': # Immediate
        out += get_opcode('IM', args) + '\n' + args[1][1:]

    elif args[1][0] == '$':
        if len(args[1]) == 3: # Zero Page
            out += get_opcode('ZP', args) + '\n' + args[1][1:]
        
        elif len(args[1]) == 4: # Zero Page X or Y
            out += get_opcode('ZP'+args[2][0], args) + '\n' + args[1][1:-1]



for l in s:
    gotoNext = False
    args = l.split(' ') # Separate arguments
    # Ignore comments
    if args[0][0] == ';':
        gotoNext = True

    if not gotoNext:
        for i, arg in enumerate(args):
            if arg[0] == ';':
                args = args[:i]
        
        # Handle individual commands
        if args[0].lower() == 'lda':
            parse_addressing_mode(args)


# An extra \n char may be stored in out
if out[-1] == '\n':
    out = out[:-1]

# Output file
with open(base_file_name + '.bin', 'w') as f:
    f.write(out)
