#!/usr/bin/env bash
# Sweep the raytracer across np in {1, 2, 4, 8, 10, 12}, take the best of 3 runs
# at each np, and write a CSV with wall time + speedup + parallel efficiency
# relative to the np=1 baseline.
#
# Override defaults via env vars: W (width), H (height), BIN (binary path),
# RUNS (repeats), OUT (csv path).
set -euo pipefail

# Force C locale so awk's printf "%.2f" emits dots, not locale-specific commas
# (otherwise the CSV would have "1,81" inside a comma-separated row).
export LC_ALL=C

BIN="${BIN:-./build/raytracer}"
W="${W:-3840}"
H="${H:-2160}"
RUNS="${RUNS:-3}"
OUT="${OUT:-stats.csv}"
NP_LIST=(1 2 3 4 5 6 7 8 9 10 11 12)

if [[ ! -x "$BIN" ]]; then
    echo "error: binary not found or not executable at $BIN" >&2
    echo "       build it first: cmake --build build -j" >&2
    exit 1
fi
if ! command -v mpirun >/dev/null 2>&1; then
    echo "error: mpirun not on PATH. Did you build OpenMPI and export PATH?" >&2
    exit 1
fi

echo "np,time_s,speedup,efficiency" > "$OUT"
base=""
printf "%-4s %-10s %-10s %-12s\n" "np" "time(s)" "speedup" "efficiency"

for NP in "${NP_LIST[@]}"; do
    best=""
    for i in $(seq 1 "$RUNS"); do
        # Allow oversubscription past hw threads so np=10/12 still launch
        # on machines whose mpirun considers the threading model conservatively.
        line=$(mpirun --oversubscribe -np "$NP" "$BIN" --w "$W" --h "$H" 2>/dev/null \
               | awk -F'=' '/^render_time=/ {print $2}') || true
        if [[ -z "$line" ]]; then
            # Some OpenMPI builds reject --oversubscribe; retry without it.
            line=$(mpirun -np "$NP" "$BIN" --w "$W" --h "$H" 2>/dev/null \
                   | awk -F'=' '/^render_time=/ {print $2}')
        fi
        t="$line"
        if [[ -z "$best" || $(awk "BEGIN{print ($t<$best)}") -eq 1 ]]; then
            best="$t"
        fi
    done
    if [[ -z "$base" ]]; then base="$best"; fi
    speedup=$(awk "BEGIN{printf \"%.2f\", $base/$best}")
    eff=$(awk "BEGIN{printf \"%.2f\", ($base/$best)/$NP}")
    printf "%-4s %-10s %-10s %-12s\n" "$NP" "$best" "$speedup" "$eff"
    echo "$NP,$best,$speedup,$eff" >> "$OUT"
done

echo "wrote $OUT"
