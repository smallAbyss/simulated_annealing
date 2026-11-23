import sys
import numpy as np
import pandas as pd

path = sys.argv[1] if len(sys.argv) > 1 else "results.csv"
THRESHOLDS = [0.01, 0.005, 0.001]

df = pd.read_csv(path, sep=";", skipinitialspace=True)
df = df.rename(columns={df.columns[0]: "task"})     # первый столбец 

def parse_point(s):
    return np.array([float(v) for v in s.split(",")])
    

# расстояние: sum_i |sa_i - opt_i|
df["pdiff"] = [
    np.abs(parse_point(a) - parse_point(b)).sum()
    for a, b in zip(df["SA_Point"], df["OPT_Point"])
]

# 1) min / max по каждой задаче + число запусков под порогами
g = df.groupby("task")["pdiff"]
res = g.agg(runs="count", min="min", max="max")
for t in THRESHOLDS:
    res[f"n<{t}"] = g.apply(lambda s: (s < t).sum())

pd.set_option("display.float_format", lambda v: f"{v:.6g}")
print(res.to_string())

# 2) сводка по порогам
print()
for t in THRESHOLDS:
    n_any = (res[f"n<{t}"] > 0).sum()
    n_all = (res[f"n<{t}"] == res["runs"]).sum()
    n_runs = (df["pdiff"] < t).sum()
    print(f"порог {t}: задач, где хотя бы один запуск попал: {n_any}/{len(res)}; "
          f"задач, где попали все запуски: {n_all}/{len(res)}; "
          f"запусков всего: {n_runs}/{len(df)}")