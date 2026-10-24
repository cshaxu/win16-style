"""Exercise the real test launcher, including parallel directory ownership."""
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

RUNNER = Path(__file__).resolve().parents[2] / 'tools/run_test.py'


class RunnerContract(unittest.TestCase):
    def launch(self, root, code, *arguments):
        environment = os.environ.copy()
        environment['WIN16STYLE_CAPTURE_DIR'] = str(root)
        return subprocess.run([sys.executable, str(RUNNER), '--name', 'fixture/shared', '--',
                               sys.executable, '-c', code, *arguments],
                              env=environment, capture_output=True, text=True, timeout=10)

    def test_parallel_children_have_distinct_owned_directories(self):
        with tempfile.TemporaryDirectory(dir=os.environ['WIN16STYLE_CAPTURE_DIR']) as root:
            code = 'import os,json; print(json.dumps([os.getcwd(),os.environ["WIN16STYLE_CAPTURE_DIR"]]))'
            with ThreadPoolExecutor(max_workers=2) as pool:
                results = list(pool.map(lambda _: self.launch(root, code), range(2)))
            paths = []
            for result in results:
                self.assertEqual(result.returncode, 0, result.stderr)
                cwd, capture = json.loads(result.stdout)
                self.assertEqual(Path(cwd), Path(capture))
                self.assertEqual(Path(cwd).parent, Path(root).resolve())
                self.assertEqual(json.loads((Path(cwd)/'runner-result.json').read_text())['exit_code'], 0)
                paths.append(cwd)
            self.assertNotEqual(*paths)

    def test_preserves_empty_arguments_and_skip(self):
        with tempfile.TemporaryDirectory(dir=os.environ['WIN16STYLE_CAPTURE_DIR']) as root:
            result = self.launch(root, 'import sys,json;print(json.dumps(sys.argv[1:]));sys.exit(77)', '', 'two words')
            self.assertEqual(result.returncode, 77)
            self.assertEqual(json.loads(result.stdout), ['', 'two words'])

    def test_missing_capture_rejected_before_child_execution(self):
        environment = os.environ.copy()
        environment.pop('WIN16STYLE_CAPTURE_DIR', None)
        result = subprocess.run([sys.executable, str(RUNNER), '--name', 'fixture', '--',
                                 sys.executable, '-c', 'print("must-not-run")'],
                                env=environment, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 2)
        self.assertNotIn('must-not-run', result.stdout)

    def test_measure_preserves_failure_and_working_directory(self):
        with tempfile.TemporaryDirectory(dir=os.environ['WIN16STYLE_CAPTURE_DIR']) as root:
            output = Path(root)/'time.json'
            result = subprocess.run([sys.executable, str(RUNNER), '--measure', str(output), '--',
                                     sys.executable, '-c', 'import os,sys;print(os.getcwd());sys.exit(42)'],
                                    cwd=root, capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 42)
            self.assertEqual(Path(result.stdout.strip()), Path(root))
            data = json.loads(output.read_text())
            self.assertEqual(data['exit_code'], 42)
            self.assertGreater(data['seconds'], 0)


if __name__ == '__main__':
    unittest.main()
