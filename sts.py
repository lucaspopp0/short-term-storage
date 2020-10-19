import argparse

parser = argparse.ArgumentParser(description='Set up STS on a folder')

parser.add_argument('-f', '--folder', required=True, help='The folder to set up STS on')
parser.add_argument('-t', '--timeout', required=True, type=int, help='The max time a file can be in the folder for')

args = parser.parse_args()
