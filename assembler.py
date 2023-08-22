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

out = b''

# Assemble
for l in s:
    args = l.split(' ') # Separate arguments
    # Ignore comments
    for i, arg in enumerate(args):
        if arg[0] == ';':
            args = args[ : i ]
    
    # Handle individual commands
    if args[0].lower() == 'lda':
        print('Good')

# Output file
with open(base_file_name + '.bin', 'wb') as f:
    f.write(out)
