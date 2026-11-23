import csv
import matplotlib.pyplot as plt

CSV_PATH = "data/tmp_run_10k_0.99995/steps_task_50.csv"

xs, ys = [], []

with open(CSV_PATH, newline="") as f:
    reader = csv.DictReader(f, delimiter=";")
    for row in reader:
        point = row.get("cur_point", "").strip()
        if not point:
            continue
        # cur_point хранится как "x, y"
        parts = point.split(",")
        if len(parts) != 2:
            continue
        try:
            x = float(parts[0].strip())
            y = float(parts[1].strip())
        except ValueError:
            continue
        xs.append(x)
        ys.append(y)

print(f"Точек найдено: {len(xs)}")

fig, ax = plt.subplots(figsize=(6, 6))
ax.scatter(xs, ys, s=15, c="tab:blue", alpha=0.8)

# Границы (-1,1) x (-1,1) с небольшим отступом для наглядности
# ax.set_xlim(-1, 1)
# ax.set_ylim(-1, 1)
ax.set_aspect("equal")
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.set_title("cur_point")
ax.autoscale()
ax.grid(True, linestyle="--", alpha=0.4)

plt.tight_layout()
plt.savefig("cur_points.png", dpi=150)
plt.show()