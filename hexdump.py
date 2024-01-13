import argparse

# Parse command line arguments
parser = argparse.ArgumentParser(description='A simple hexdump program')

parser.add_argument('-f', type=str, help='The required file specifier')

parser.add_argument('-s', type=str, required=False, help='Optional export mode; saves export to file.')

args = parser.parse_args()

with open(args.f, 'rb') as f:
    data = f.read().hex()

def padded_hex(value, padding=8):
    h = hex(value)[2:].upper() # Converts to a hex string, removes 0x, and capitalizes symbols.
    return ('0' * (padding - len(h))) + h # Adds padding and returns result.


for j in range( 0, len(data), 32 ):
    print(end=padded_hex( int(j/2) ) + '  ') # Prints line number
    ascii_chars = ' |'
    for i in range(0, 32, 2):
        if j+i < len(data):
            character = data[j+i] + data[j+i+1]
            print(character, end=' ' if i != 14 else '  ')
            
            # Build the ascii representation on the side
            dec_char = int(character, 16)
            ascii_char = chr(dec_char if dec_char > 32 and dec_char < 127 else 46)
            ascii_chars += ascii_char
    print(ascii_chars + '|')



# Save in an easy format
if args.s != None:
    out_file = ''
    for i in range( 0, len(data), 2 ):
        out_file += data[i] + data[i+1] + '\n'
    out_file = out_file.rstrip()
    with open(args.s, 'w') as f:
        f.write(out_file)