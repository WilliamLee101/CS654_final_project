import numpy as np
import librosa
import argparse
import os

def mp3_to_8bit_values(input_file, output_file=None, sample_rate=1000):

    print(f"Loading audio file: {input_file}")
    
    if output_file is None:
        base_name = os.path.splitext(input_file)[0]
        output_file = f"{base_name}_8bit.bin"
    
    try:
        y, sr = librosa.load(input_file, sr=sample_rate, mono=True)
        print(f"Loaded audio with sample rate: {sr}Hz")
        print(f"Audio duration: {len(y) / sr:.2f} seconds")
    except Exception as e:
        print(f"Error loading audio file: {e}")
        return None
    
    # Normalize to range -1 to 1
    y = y / np.max(np.abs(y))
    
    # Convert to range 0-255 (unsigned 8-bit)
    y_8bit_unsigned = ((y * 127).astype(np.int16) + 128).astype(np.uint8)
    
    with open(output_file, 'wb') as f:
        f.write(y_8bit_unsigned.tobytes())
    
    print(f"Converted {len(y_8bit_unsigned)} samples to 8-bit values (0-255)")
    print(f"Saved 8-bit data as raw bytes to: {output_file}")
    
    txt_file = f"{os.path.splitext(output_file)[0]}.txt"
    with open(txt_file, 'w') as f:
        for value in y_8bit_unsigned:
            f.write(f"{value}\n")
    
    print(f"Saved values as text to: {txt_file}")
    print("\nSample of 8-bit data (first 20 values):")
    print(y_8bit_unsigned[:20])
    
    return y_8bit_unsigned, sr

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert MP3 to 8-bit values (0-255)")
    parser.add_argument("input_file", help="Path to input MP3 file")
    parser.add_argument("-o", "--output", help="Output file path (optional)")
    parser.add_argument("-r", "--rate", type=int, default=1000, 
                        help="Sample rate in Hz (default: 1000)")

    args = parser.parse_args()
    mp3_to_8bit_values(args.input_file, args.output, args.rate)