"""Extract PCM from a WAV file and write raw BIN and Intel HEX files."""

#& 'D:\Python312\python.exe' 'D:\KEIL C51\HAL\FreeRtos\H750_freertos_LVGL_Audio\music\wav_to_flash.py' 'D:\KEIL C51\HAL\FreeRtos\H750_freertos_LVGL_Audio\music\lanhuacao.wav'


import argparse
import wave
from pathlib import Path


DEFAULT_ADDRESS = 0x90100000
MAX_ADDRESS = 0xFFFFFFFF
DEFAULT_RECORD_SIZE = 32


def parse_address(value):
    try:
        address = int(value, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("address must be decimal or 0x-prefixed hex") from error
    if not 0 <= address <= MAX_ADDRESS:
        raise argparse.ArgumentTypeError("address must fit in 32 bits")
    return address


def make_record(address, record_type, data):
    body = bytes((len(data), (address >> 8) & 0xFF, address & 0xFF, record_type)) + data
    checksum = (-sum(body)) & 0xFF
    return ":" + (body + bytes((checksum,))).hex().upper() + "\n"


def write_ihex(path, data, base_address):
    if data and base_address + len(data) - 1 > MAX_ADDRESS:
        raise ValueError("PCM data extends beyond the 32-bit Intel HEX address space")

    with path.open("w", encoding="ascii", newline="\n") as output:
        current_upper = None
        offset = 0

        while offset < len(data):
            address = base_address + offset
            upper = address >> 16
            if upper != current_upper:
                output.write(make_record(0, 0x04, upper.to_bytes(2, "big")))
                current_upper = upper

            record_size = min(
                DEFAULT_RECORD_SIZE,
                len(data) - offset,
                0x10000 - (address & 0xFFFF),
            )
            chunk = data[offset : offset + record_size]
            output.write(make_record(address & 0xFFFF, 0x00, chunk))
            offset += record_size

        output.write(make_record(0, 0x01, b""))


def main():
    parser = argparse.ArgumentParser(
        description="Extract uncompressed PCM from WAV and create BIN plus address-based Intel HEX."
    )
    parser.add_argument("wav", type=Path, help="input uncompressed PCM WAV file")
    parser.add_argument(
        "--address",
        type=parse_address,
        default=DEFAULT_ADDRESS,
        help="Flash address recorded in the HEX file (default: 0x90100000)",
    )
    parser.add_argument("--bin-output", type=Path, help="BIN output path (default: input name with .bin)")
    parser.add_argument("--hex-output", type=Path, help="HEX output path (default: input name with .hex)")
    args = parser.parse_args()

    source = args.wav.resolve()
    bin_path = (args.bin_output or args.wav.with_suffix(".bin")).resolve()
    hex_path = (args.hex_output or args.wav.with_suffix(".hex")).resolve()

    if not source.is_file():
        parser.error(f"input WAV does not exist: {source}")
    if bin_path == hex_path or source in (bin_path, hex_path):
        parser.error("input, BIN output, and HEX output must be different files")
    if not bin_path.parent.is_dir() or not hex_path.parent.is_dir():
        parser.error("output directories must already exist")

    try:
        with wave.open(str(source), "rb") as wav_file:
            params = wav_file.getparams()
            if params.comptype != "NONE":
                parser.error(f"compressed WAV is not supported: {params.compname}")
            pcm = wav_file.readframes(params.nframes)
    except (wave.Error, EOFError) as error:
        parser.error(f"could not read WAV: {error}")

    expected_size = params.nframes * params.nchannels * params.sampwidth
    if len(pcm) != expected_size:
        parser.error(f"incomplete WAV data: expected {expected_size} bytes, got {len(pcm)}")

    try:
        bin_path.write_bytes(pcm)
        write_ihex(hex_path, pcm, args.address)
    except (OSError, ValueError) as error:
        parser.error(str(error))

    print(f"WAV: {source}")
    print(
        f"PCM: {params.framerate} Hz, {params.nchannels} channel(s), "
        f"{params.sampwidth * 8}-bit, {len(pcm)} bytes"
    )
    print(f"BIN: {bin_path}")
    print(f"HEX: {hex_path}, address 0x{args.address:08X}-0x{args.address + len(pcm) - 1:08X}")
    if (params.framerate, params.nchannels, params.sampwidth) != (44100, 2, 2):
        print("Warning: current audio firmware expects 44.1 kHz, 16-bit stereo PCM; no resampling was performed.")


if __name__ == "__main__":
    main()