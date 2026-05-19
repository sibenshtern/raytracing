#!/usr/bin/env python3
import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt

csv_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("stats.csv")
out_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("speedup.png")

np_list, speedup, efficiency = [], [], []
with csv_path.open() as f:
    for row in csv.DictReader(f):
        np_list.append(int(row["np"]))
        speedup.append(float(row["speedup"]))
        efficiency.append(float(row["efficiency"]))

fig, (ax_s, ax_e) = plt.subplots(1, 2, figsize=(11, 4.2))

ax_s.plot(np_list, speedup, "o-", color="#1f77b4", linewidth=2, markersize=7, label="Измеренное ускорение")
for x, y in zip(np_list, speedup):
    ax_s.annotate(f"{y:.2f}", (x, y), textcoords="offset points", xytext=(6, 6), fontsize=9)
ax_s.set_xlabel("Число рангов")
ax_s.set_ylabel("Ускорение  S(np) = T(1) / T(np)")
ax_s.set_title("Ускорение (speedup)")
ax_s.set_xticks(np_list)
ax_s.grid(True, alpha=0.3)
ax_s.legend(loc="upper left")

ax_e.plot(np_list, efficiency, "s-", color="#d62728", linewidth=2, markersize=7, label="Измеренная эффективность")
for x, y in zip(np_list, efficiency):
    ax_e.annotate(f"{y:.2f}", (x, y), textcoords="offset points", xytext=(6, -14), fontsize=9)
ax_e.set_xlabel("Число рангов")
ax_e.set_ylabel("Параллельная эффективность  E = S(np) / np")
ax_e.set_title("Параллельная эффективность")
ax_e.set_xticks(np_list)
ax_e.set_ylim(0, 1.1)
ax_e.grid(True, alpha=0.3)
ax_e.legend(loc="lower left")

fig.suptitle(f"Raytracing @ 3840x2160   (data: {csv_path.name})", fontsize=11)
fig.tight_layout()
fig.savefig(out_path, dpi=300)
print(f"wrote {out_path}")
