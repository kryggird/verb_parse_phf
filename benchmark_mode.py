#!/usr/bin/env python
from pathlib import Path
import sys

SYS_PSTATE = Path("/sys/devices/system/cpu/intel_pstate/")
# file: (powersave, benchmarking)
CONFIGS = {
    SYS_PSTATE / "no_turbo": {"benchmarking": "1", "powersave": "0"},
    SYS_PSTATE / "min_perf_pct": {"benchmarking": "45", "powersave": "10"},
    SYS_PSTATE / "max_perf_pct": {"benchmarking": "45", "powersave": "100"},
}

if __name__ == "__main__":
    assert len(sys.argv) <= 2
    kind = "powersave" if len(sys.argv) <= 1 else sys.argv[1]
    assert kind in ("powersave", "benchmarking"), "Arg 1 must be in powersave, benchmarking"

    assert all(f.exists() for f in CONFIGS)

    for file in CONFIGS:
        with open(file, "wt") as f:
            f.write(CONFIGS[file][kind]) 
