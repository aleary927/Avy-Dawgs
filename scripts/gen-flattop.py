import numpy as np
import scipy.signal.windows as windows
import sys
import argparse

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser("window")
    parser.add_argument("window_size", type=int)
    parser.add_argument("data_format", type=str)
    parser.add_argument("output_file", type=str)
    args = parser.parse_args()
    window_size = args.window_size
    output_file = args.output_file
    data_format = args.data_format
    print(f'Writing flattop window of size {args.window_size} to file {args.output_file}')

    scale_factor = 1
    match data_format: 
        case 'int16': 
            scale_factor = 2**15 - 1
        case 'int32': 
            scale_factor = 2**31 - 1
        case 'float32': 
            scale_factor = 1
        case default: 
            print('Invalid data format')
            sys.exit()

    window = windows.flattop(window_size)
    window_scaled = []
    count = 0
    for w in window:
        match data_format: 
            case 'int16': 
                window_scaled.append(np.int16(w * scale_factor))
            case 'int32': 
                window_scaled.append(np.int32(w * scale_factor))
            case 'float32': 
                window_scaled.append(np.float32(w * scale_factor))


    f = open(output_file, mode='w')
    match data_format: 
        case 'int16':
            f.write('int16_t ')
        case 'int32': 
            f.write('int32_t ') 
        case 'float32': 
            f.write('float ')
    f.write(f'array_name[{window_size}]')
    f.write(' = {\n')
    # f.write('array_name[2048] = {\n')

    for i in range(window_size - 1):
        count = count + 1;
        f.write(str(window_scaled[i]) + ',')
        if count == 4: 
            count = 0;
            f.write('\n')

    f.write(str(window_scaled[window_size - 1]))
    f.write('};')
    f.close()
