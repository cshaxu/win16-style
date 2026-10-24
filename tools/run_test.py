"""Isolate CTest output and measure real child commands without changing exits."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import time
import uuid


def main():
    parser = argparse.ArgumentParser()
    modes = parser.add_mutually_exclusive_group(required=True)
    modes.add_argument('--name')
    modes.add_argument('--measure', type=Path)
    parser.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command[1:] if args.command[:1] == ['--'] else args.command
    if not command:
        parser.error('a child command is required')
    environment = os.environ.copy()
    directory = None
    result_path = args.measure
    if args.name:
        if not re.fullmatch(r'[A-Za-z0-9_./-]+', args.name) or '..' in args.name.split('/'):
            parser.error('invalid test identity')
        root = environment.get('WIN16STYLE_CAPTURE_DIR')
        if not root:
            parser.error('set WIN16STYLE_CAPTURE_DIR to an owned scratch directory')
        directory = Path(root).resolve(strict=True) / (args.name.replace('/', '-') + '-' + uuid.uuid4().hex[:12])
        directory.mkdir()
        environment['WIN16STYLE_CAPTURE_DIR'] = str(directory)
        result_path = directory / 'runner-result.json'
    started = time.monotonic()
    try:
        code = subprocess.call(command, cwd=directory, env=environment)
    except OSError as error:
        print(f'test command could not start: {error}', flush=True)
        code = 2
    result_path.write_text(json.dumps({'seconds': time.monotonic() - started, 'exit_code': code}) + '\n', encoding='utf-8')
    return code


if __name__ == '__main__':
    raise SystemExit(main())
