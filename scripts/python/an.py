#!/usr/bin/env python3
"""
Columns:  #;SA_E;OPT_E;dif;time_s;SA_Point;OPT_Point

    python an.py                       # reads ./stats/ans.txt
    python an.py ./stats/leOMP/ans_5d_5b_200k.csv --out-dir py_reports --show
"""
import argparse
import re
import sys
from pathlib import Path

import numpy as np
import pandas as pd

THRESHOLDS = [1e-1, 1e-2, 1e-3, 1e-4]

NUM_RE = re.compile(r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?|[-+]?(?:nan|inf)", re.I)


def parse_point(text) -> np.ndarray:
    return np.array([float(t) for t in NUM_RE.findall(str(text))], dtype=float)


def point_distance(sa: np.ndarray, opt: np.ndarray, norm: str) -> float:
    if sa.size == 0 or sa.shape != opt.shape:
        return float("nan")
    d = np.abs(sa - opt)
    if norm == "l1":
        return float(d.sum())
    if norm == "l2":
        return float(np.sqrt((d ** 2).sum()))
    return float(d.max())  # linf


def load(path: Path, norm: str) -> pd.DataFrame:
    # index_col=False: old-format rows end with ';' (one more field than the header)
    df = pd.read_csv(path, sep=";", skipinitialspace=True, index_col=False)
    df.columns = [c.strip() for c in df.columns]

    required = ["#", "dif", "SA_Point", "OPT_Point"]
    missing = [c for c in required if c not in df.columns]
    if missing:
        sys.exit(f"{path}: missing columns {missing}; found {list(df.columns)}")

    out = pd.DataFrame({"task": df["#"], "dif": pd.to_numeric(df["dif"], errors="coerce")})

    dists, bad = [], []
    for task, sa, opt in zip(df["#"], df["SA_Point"], df["OPT_Point"]):
        d = point_distance(parse_point(sa), parse_point(opt), norm)
        if np.isnan(d):
            bad.append(task)
        dists.append(d)
    out["point_dist"] = dists

    if bad:
        print(f"warning: could not compare points for tasks {bad} (parse error or size mismatch)",
              file=sys.stderr)
    return out


def count_below(values: pd.Series, thresholds):
    return [int((values < t).sum()) for t in thresholds]  # NaN < t is False


def label(t: float) -> str:
    return f"< {t:g}"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("path", nargs="?", default="./stats/ans.txt")
    ap.add_argument("--out-dir", default=None, help="where to save plot/csv (default: next to ans.txt)")
    ap.add_argument("--norm", choices=["l1", "l2", "linf"], default="l1",
                    help="point distance: l1 = sum of |differences| (default)")
    ap.add_argument("--show", action="store_true", help="open the plot window")
    args = ap.parse_args()

    path = Path(args.path)
    out_dir = Path(args.out_dir) if args.out_dir else path.parent
    out_dir.mkdir(parents=True, exist_ok=True)

    res = load(path, args.norm)
    n_total = len(res)
    n_dif = int(res["dif"].notna().sum())
    n_pt = int(res["point_dist"].notna().sum())

    c_dif = count_below(res["dif"], THRESHOLDS)
    c_pt = count_below(res["point_dist"], THRESHOLDS)

    # ---- table -----------------------------------------------------------
    summary = pd.DataFrame({
        "threshold": THRESHOLDS,
        "dif_count": c_dif,
        "dif_pct": [100.0 * c / n_total for c in c_dif],
        f"point_{args.norm}_count": c_pt,
        f"point_{args.norm}_pct": [100.0 * c / n_total for c in c_pt],
    })
    print(f"{path}: {n_total} tasks ({n_dif} with valid dif, {n_pt} with comparable points)\n")
    print(summary.to_string(index=False, float_format=lambda v: f"{v:.4g}"))

    worst = res[~(res["dif"] < THRESHOLDS[0])]
    if len(worst):
        print(f"\nTasks with dif >= {THRESHOLDS[0]:g} (or NaN): {worst['task'].tolist()}")

    summary.to_csv(out_dir / "threshold_summary.csv", index=False)
    res.to_csv(out_dir / "per_task_metrics.csv", index=False)

    # ---- plot (1): tasks solved per dif threshold -----------------------
    import matplotlib
    if not args.show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(7, 4.5))
    labels = [label(t) for t in THRESHOLDS]
    bars = ax.bar(labels, c_dif, color="#4c78a8")
    for bar, c in zip(bars, c_dif):
        ax.annotate(f"{c}  ({100.0 * c / n_total:.0f}%)",
                    (bar.get_x() + bar.get_width() / 2, bar.get_height()),
                    ha="center", va="bottom", fontsize=9, xytext=(0, 3), textcoords="offset points")
    ax.set_ylim(0, n_total * 1.12)
    ax.axhline(n_total, color="gray", lw=0.8, ls="--")
    ax.set_xlabel("|SA_E - OPT_E|")
    ax.set_ylabel("number of tasks")
    ax.set_title(f"Tasks solved within threshold (total {n_total})")
    fig.tight_layout()

    png = out_dir / "dif_thresholds.png"
    fig.savefig(png, dpi=150)
    print(f"\nsaved: {png}, {out_dir / 'threshold_summary.csv'}, {out_dir / 'per_task_metrics.csv'}")

    if args.show:
        plt.show()


if __name__ == "__main__":
    main()