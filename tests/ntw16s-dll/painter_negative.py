"""The painter's exact owned-object oracle must reject its deliberate leak."""
import re
import subprocess
import sys

result = subprocess.run([sys.argv[1], '--leak-negative'], capture_output=True, timeout=30)
output = result.stdout+result.stderr
assert result.returncode == 1 and re.search(rb'INJECTED PAINTER LEAK live=[1-9][0-9]*', output) and b'CONTROL PRESENT FAIL line=' in output, output.decode(errors='replace')
print('PAINTER NEGATIVE PASS deliberate owned bitmap leak rejected by the actual instrumented painter')
