from pathlib import Path
import re
import argparse

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


PROJECT_ROOT = Path(__file__).resolve().parents[1].parent
DEFAULT_STATS = PROJECT_ROOT / "data" / "stats" / "ans.csv"
POINTS_PATH = PROJECT_ROOT / "data" / "stats" / "steps_task_52.csv"

def load_extra_points(filename):
    points = []

    with open(filename, "r", encoding="utf-8") as file:
        for line_num, line in enumerate(file, start=1):
            if not line.strip():
                continue

            last_field = line.strip().split(";")[-1]

            coordinates = last_field.split(",")

            x1 = float(coordinates[0].strip())
            x2 = float(coordinates[1].strip())

            points.append((x1, x2))

    return np.array(points)


def infer_task_num(grid_path: Path) -> int:
    """Извлекает номер задания из имени файла, например gkls_52.csv."""
    match = re.search(r"(\d+)$", grid_path.stem)

    if not match:
        raise ValueError(
            f"Cannot infer task number from filename: {grid_path.name}. "
            "Use --task to specify it explicitly."
        )

    return int(match.group(1))


def parse_point(value: str) -> tuple[float, float]:
    """
    Преобразует строку вида:
        '0.683108, 0.538363, 0.538363'

    В координаты двумерной точки (x1, x2).

    Используются первые два значения.
    """
    values = [
        float(part.strip())
        for part in str(value).strip().strip("()[]").split(",")
    ]

    if len(values) < 2:
        raise ValueError(f"Invalid point format: {value}")

    return values[0], values[1]


def load_points(
    task_num: int,
    stats_path: Path = DEFAULT_STATS
):
    """Читает SA_Point и OPT_Point для указанного задания."""

    stats = pd.read_csv(
        stats_path,
        sep=";",
        dtype=str,
        skipinitialspace=True
    )

    stats.columns = stats.columns.str.strip()

    required_columns = {"#", "SA_Point", "OPT_Point"}

    if not required_columns.issubset(stats.columns):
        raise ValueError(
            f"Missing columns in {stats_path}. "
            f"Expected: {required_columns}"
        )

    # Ищем строку соответствующего задания
    rows = stats[
        stats["#"].str.strip() == str(task_num)
    ]

    if rows.empty:
        raise ValueError(
            f"Task {task_num} not found in {stats_path}"
        )

    if len(rows) > 1:
        raise ValueError(
            f"Multiple entries found for task {task_num} "
            f"in {stats_path}"
        )

    row = rows.iloc[0]

    sa_point = parse_point(row["SA_Point"])
    opt_point = parse_point(row["OPT_Point"])

    return sa_point, opt_point, row


def plot_contour(
    filename: str | Path,
    stats_path: Path = DEFAULT_STATS,
    task_num: int | None = None
):
    grid_path = Path(filename).resolve()

    if task_num is None:
        task_num = infer_task_num(grid_path)

    # Загружаем сетку
    df = pd.read_csv(grid_path)

    x1 = np.sort(df["x1"].unique())
    x2 = np.sort(df["x2"].unique())

    Z = (
        df.pivot(index="x2", columns="x1", values="y")
        .reindex(index=x2, columns=x1)
        .to_numpy()
    )

    X, Y = np.meshgrid(x1, x2)

    # Загружаем точки алгоритмов
    sa_point, opt_point, stats_row = load_points(
        task_num,
        stats_path
    )

    # Построение графика
    fig, ax = plt.subplots(figsize=(10, 8))

    filled = ax.contourf(
        X, Y, Z,
        levels=30,
        alpha=0.6
    )

    contours = ax.contour(
        X, Y, Z,
        levels=30,
        linewidths=0.8,
        colors="black"
    )

    ax.clabel(contours, inline=True, fontsize=8)

    fig.colorbar(filled, ax=ax, label="f(x1, x2)")

    # SA_Point — найденная точка
    ax.scatter(
        sa_point[0],
        sa_point[1],
        color="red",
        marker="x",
        s=120,
        linewidths=3,
        label=f"SA (E={stats_row['SA_E'].strip()})",
        zorder=10
    )

    # OPT_Point — оптимальная точка
    ax.scatter(
        opt_point[0],
        opt_point[1],
        facecolors="none",
        edgecolors="limegreen",
        marker="o",
        s=180,
        linewidths=2.5,
        label=f"OPT (E={stats_row['OPT_E'].strip()})",
        zorder=11
    )

    extra_points = load_extra_points(POINTS_PATH)
    # TRACE
    if len(extra_points) > 0:
        ax.scatter(
            extra_points[:, 0],
            extra_points[:, 1],
            color="dodgerblue",
            marker=".",
            s=25,
            alpha=0.65,
            label="Points from file",
            zorder=8
        )
    ax.set_xlabel("x1")
    ax.set_ylabel("x2")
    ax.set_title(f"GKLS function #{task_num}")


    ax.legend(loc="best")
    ax.grid(alpha=0.2)

    plt.tight_layout()
    plt.savefig("data/grids_img/my_plot_" + str(task_num) + ".png") 
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plot GKLS contour with SA and OPT points"
    )

    parser.add_argument(
        "grid",
        type=Path,
        help="Path to GKLS grid CSV"
    )

    parser.add_argument(
        "--task",
        type=int,
        default=None,
        help="Task number (inferred from filename by default)"
    )

    parser.add_argument(
        "--stats",
        type=Path,
        default=DEFAULT_STATS,
        help="Path to ans.csv"
    )

    args = parser.parse_args()

    plot_contour(
        args.grid,
        stats_path=args.stats,
        task_num=args.task
    )