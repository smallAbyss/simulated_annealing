
from pathlib import Path
import csv

# Configuration — edit these manually
TASK = "SA_34"

INPUT_FILE = Path("stats/ans_parallel.txt")
OUTPUT_FILE = Path("reports") / f"{TASK}.md"

DIF_THRESHOLD = 0.00001
POINT_THRESHOLD = 0.0001


def parse_vector(value: str) -> list[float]:
    """Parse a vector such as '-0.59, 0.17, -0.02'."""
    return [float(x.strip()) for x in value.strip().split(",")]


def main() -> None:
    total = 0
    dif_matches = 0
    point_matches = 0
    malformed = 0

    with INPUT_FILE.open("r", encoding="utf-8") as file:
        reader = csv.reader(file, delimiter=";")

        for row in reader:
            # Skip empty lines
            if not row or not any(field.strip() for field in row):
                continue

            # Skip the header, if present
            if row[0].strip().lower() == "tasknum":
                continue

            # Expected columns:
            # 0: TaskNum, 1: SA_Func, 2: OPT_FUNC, 3: dif,
            # 4: SA_Point, 5: OPT_Point
            if len(row) != 6:
                malformed += 1
                continue

            try:
                dif = float(row[3].strip())
                sa_point = parse_vector(row[4])
                opt_point = parse_vector(row[5])

                if len(sa_point) != len(opt_point):
                    raise ValueError("Vector dimensions differ")

                # Sum of absolute coordinate differences (L1 distance)
                point_diff = sum(
                    abs(sa - opt)
                    for sa, opt in zip(sa_point, opt_point)
                )

            except ValueError:
                malformed += 1
                continue

            total += 1

            if dif < DIF_THRESHOLD:
                dif_matches += 1

            if point_diff < POINT_THRESHOLD:
                point_matches += 1

    # Create the output directory if it doesn't exist
    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)

    report = f"""# Analysis Report: {TASK}

## Summary

- **Total valid entries:** {total}
- **Malformed entries skipped:** {malformed}

## 1. Function value difference

- **Condition:** `dif < {DIF_THRESHOLD}`
- **Matching entries:** {dif_matches}
- **Percentage:** {dif_matches / total * 100:.2f}%
""" if total else f"""# Analysis Report: {TASK}

No valid entries found.

- Malformed entries skipped: {malformed}
"""

    if total:
        report += f"""
## 2. Point distance

- **Distance:** `sum(abs(SA_Point[i] - OPT_Point[i]))`
- **Condition:** `distance < {POINT_THRESHOLD}`
- **Matching entries:** {point_matches}
- **Percentage:** {point_matches / total * 100:.2f}%
"""

    OUTPUT_FILE.write_text(report, encoding="utf-8")
    print(f"Report saved to: {OUTPUT_FILE}")
    print(f"Valid entries: {total}")
    print(f"dif < {DIF_THRESHOLD}: {dif_matches}")
    print(f"point distance < {POINT_THRESHOLD}: {point_matches}")


if __name__ == "__main__":
    main()
