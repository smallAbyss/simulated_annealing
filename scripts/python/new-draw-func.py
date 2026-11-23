from pathlib import Path
import re
import argparse

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


PROJECT_ROOT = Path(__file__).resolve().parents[1].parent
# DEFAULT_STATS = PROJECT_ROOT / "data" / "tmp_run" / "ALL_ans.csv"
DEFAULT_STATS = PROJECT_ROOT / "data" / "tmp_run_10k_0.9995" / "ans.csv"
DEFAULT_POINTS = PROJECT_ROOT / "data" / "tmp_run_10k_0.9995" / "steps_task_42.csv"


def parse_point(value: str) -> tuple[float, float]:
    """
    '0.683108, 0.538363, 0.538363' -> (0.683108, 0.538363).
    Берём первые два числа.
    """
    values = [
        float(part.strip())
        for part in str(value).strip().strip("()[]").split(",")
    ]
    if len(values) < 2:
        raise ValueError(f"Invalid point format: {value}")
    return values[0], values[1]


def load_extra_points(filename: str | Path) -> pd.DataFrame:
    """
    Читает CSV со шагами SA. Возвращает DataFrame с колонками:
        x1, x2, accepted, new_best, step
    Если колонки cur_point нет — пытается взять последнее поле строки.
    """
    df = pd.read_csv(filename, sep=";", skipinitialspace=True)
    df.columns = df.columns.str.strip()

    if "cur_point" in df.columns:
        points = df["cur_point"].astype(str).apply(parse_point)
    else:
        # fallback на «сырой» разбор последнего поля
        points = []
        with open(filename, "r", encoding="utf-8") as f:
            next(f, None)  # пропускаем заголовок
            for line in f:
                line = line.strip()
                if not line:
                    continue
                points.append(parse_point(line.split(";")[-1]))
        points = pd.Series(points)

    df["x1"] = [p[0] for p in points]
    df["x2"] = [p[1] for p in points]
    df["step"] = np.arange(len(df))

    # Приводим флаги к числам, если они строки
    for col in ("accepted", "new_best"):
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")

    return df


def infer_task_num(grid_path: Path) -> int:
    """Извлекает номер задания из имени файла, например gkls_52.csv."""
    match = re.search(r"(\d+)$", grid_path.stem)
    if not match:
        raise ValueError(
            f"Cannot infer task number from filename: {grid_path.name}. "
            "Use --task to specify it explicitly."
        )
    return int(match.group(1))


def load_points(task_num: int, stats_path: Path = DEFAULT_STATS):
    """Читает SA_Point и OPT_Point для указанного задания."""
    stats = pd.read_csv(stats_path, sep=";", dtype=str, skipinitialspace=True)
    stats.columns = stats.columns.str.strip()

    required_columns = {"#", "SA_Point", "OPT_Point"}
    if not required_columns.issubset(stats.columns):
        raise ValueError(
            f"Missing columns in {stats_path}. Expected: {required_columns}"
        )

    rows = stats[stats["#"].str.strip() == str(task_num)]
    if rows.empty:
        raise ValueError(f"Task {task_num} not found in {stats_path}")
    if len(rows) > 1:
        raise ValueError(
            f"Multiple entries found for task {task_num} in {stats_path}"
        )

    row = rows.iloc[0]
    return parse_point(row["SA_Point"]), parse_point(row["OPT_Point"]), row


def plot_contour(
    filename: str | Path,
    stats_path: Path = DEFAULT_STATS,
    points_path: Path = DEFAULT_POINTS,
    task_num: int | None = None,
    show_trace: bool = True,
    color_by_new_best: bool = True,
    output: Path | None = None,
):
    grid_path = Path(filename).resolve()

    if task_num is None:
        task_num = infer_task_num(grid_path)

    # --- Сетка ---
    df = pd.read_csv(grid_path)
    x1 = np.sort(df["x1"].unique())
    x2 = np.sort(df["x2"].unique())
    Z = (
        df.pivot(index="x2", columns="x1", values="y")
        .reindex(index=x2, columns=x1)
        .to_numpy()
    )
    X, Y = np.meshgrid(x1, x2)

    # --- Точки алгоритмов ---
    sa_point, opt_point, stats_row = load_points(task_num, stats_path)

    # --- Точек трассы ---
    trace = load_extra_points(points_path)

    # --- Рисуем ---
    fig, ax = plt.subplots(figsize=(10, 8))

    filled = ax.contourf(X, Y, Z, levels=30, alpha=0.6)
    contours = ax.contour(X, Y, Z, levels=30, linewidths=0.8, colors="black")
    ax.clabel(contours, inline=True, fontsize=8)
    fig.colorbar(filled, ax=ax, label="f(x1, x2)")

    # Трасса
    if len(trace) > 0:
        if show_trace:
            ax.plot(
                trace["x1"], trace["x2"],
                "-", color="dodgerblue", linewidth=0.8,
                alpha=0.5, zorder=7,
            )

        if color_by_new_best and "new_best" in trace.columns:
            is_best = trace["new_best"].fillna(0).astype(bool)
            ax.scatter(
                trace.loc[~is_best, "x1"], trace.loc[~is_best, "x2"],
                color="dodgerblue", marker=".", s=25, alpha=0.6,
                label="cur_point", zorder=8,
            )
            ax.scatter(
                trace.loc[is_best, "x1"], trace.loc[is_best, "x2"],
                color="red", marker=".", s=40, alpha=0.9,
                label="new best", zorder=9,
            )
        else:
            ax.scatter(
                trace["x1"], trace["x2"],
                color="dodgerblue", marker=".", s=25, alpha=0.65,
                label="cur_point", zorder=8,
            )

        # Первая точка
        ax.scatter(
            trace["x1"].iloc[0], trace["x2"].iloc[0],
            facecolors="none", edgecolors="black",
            marker="s", s=90, linewidths=1.5,
            label="start", zorder=12,
        )
        # Последняя точка
        ax.scatter(
            trace["x1"].iloc[-1], trace["x2"].iloc[-1],
            facecolors="none", edgecolors="magenta",
            marker="D", s=130, linewidths=2,
            label="last cur_point", zorder=13,
        )

    # SA / OPT
    ax.scatter(
        sa_point[0], sa_point[1],
        color="red", marker="x", s=120, linewidths=3,
        label=f"SA (E={stats_row['SA_E'].strip()})", zorder=10,
    )
    ax.scatter(
        opt_point[0], opt_point[1],
        facecolors="none", edgecolors="limegreen",
        marker="o", s=180, linewidths=2.5,
        label=f"OPT (E={stats_row['OPT_E'].strip()})", zorder=11,
    )

    ax.set_xlabel("x1")
    ax.set_ylabel("x2")
    ax.set_title(f"GKLS function #{task_num}")
    ax.legend(loc="best")
    ax.grid(alpha=0.2)

    plt.tight_layout()

    if output is None:
        output = Path("data/grids_img") / f"my_plot_{task_num}.png"
    output.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output, dpi=150)
    print(f"Saved: {output}")
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plot GKLS contour with SA, OPT and the SA trace"
    )
    parser.add_argument("grid", type=Path, help="Path to GKLS grid CSV")
    parser.add_argument("--task", type=int, default=None,
                        help="Task number (inferred from filename by default)")
    parser.add_argument("--stats", type=Path, default=DEFAULT_STATS,
                        help="Path to ans.csv")
    parser.add_argument("--points", type=Path, default=DEFAULT_POINTS,
                        help="Path to steps_task_XX.csv")
    parser.add_argument("--no-trace", action="store_true",
                        help="Do not draw the polyline between cur_points")
    parser.add_argument("--no-color", action="store_true",
                        help="Do not color points by new_best")
    parser.add_argument("--out", type=Path, default=None,
                        help="Output PNG path")

    args = parser.parse_args()

    plot_contour(
        args.grid,
        stats_path=args.stats,
        points_path=args.points,
        task_num=args.task,
        show_trace=not args.no_trace,
        color_by_new_best=not args.no_color,
        output=args.out,
    )