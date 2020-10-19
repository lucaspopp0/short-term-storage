import argparse
import os
import datetime

parser = argparse.ArgumentParser(description='Set up STS on a folder')

parser.add_argument('-f', '--folder', required=True, help='The folder to set up STS on')
parser.add_argument('-t', '--timeout', required=True, type=int, help='The max time a file can be in the folder for')

args = parser.parse_args()

contents = os.listdir(args.folder)
now = datetime.datetime.now()

for obj in contents:
    absPath = os.path.join(args.folder, obj)
    atime = os.path.getatime(absPath)
    parsed = datetime.datetime.fromtimestamp(atime)
    diff = now - parsed
    total = diff.total_seconds()
    if total > args.timeout:
        print(obj)
