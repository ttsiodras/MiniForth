#!/usr/bin/env python3
"""
Usage:
    test_forth.py -p <port>
    test_forth.py (-h | --help)
"""
import sys
import time
import serial

import docopt


def send_cmd_wait_OK(ser, line):
    ser.timeout = 0.1
    resp = ''
    while True:
        it = time.time()
        data = ser.read().decode()
        resp += data
        sys.stdout.write(data)
        sys.stdout.flush()
        if '> ' in resp:
            break
        if time.time() - it > 1:
            print("[x] Timeout waiting for OK")
            sys.exit(1)

    line = line.strip()
    for c in line:
        time.sleep(0.01)
        ser.write(c.encode())
        print(c, end='')
        sys.stdout.flush()
    ser.write('\r'.encode())


def main(args):
    ser = serial.Serial(args.get('<port>'), 115200)
    ser.write('\rreset\r'.encode())
    for line in open("../README.md"):
        if line.startswith('    '):
            send_cmd_wait_OK(ser, line)
            print("\n==> ", end='')
    send_cmd_wait_OK(ser, "\r")


if __name__ == "__main__":
    main(docopt.docopt(__doc__))