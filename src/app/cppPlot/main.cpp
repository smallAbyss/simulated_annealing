#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <locale>


#include "MathVec.h"
#include "GKLS/GKLSProblem.hpp"


namespace fs = std::filesystem;

constexpr int DIM = 2;

/**
 * Генерирует равномерную сетку для GKLS-функции
 * и сохраняет её в CSV-файл.
 *
 * Формат CSV: x1,x2,y
 */
void saveGrid(int task_num, int nx, int ny, const fs::path& output_dir) {
    TGKLSProblem task(task_num, DIM);

    // Получаем границы области определения
    std::vector<double> low_bounds;
    std::vector<double> upper_bounds;

    task.GetBounds(low_bounds, upper_bounds);

    if (low_bounds.size() != DIM || upper_bounds.size() != DIM) {
        throw std::runtime_error("Invalid bounds dimensions");
    }

    const double x1_min = low_bounds[0];
    const double x1_max = upper_bounds[0];

    const double x2_min = low_bounds[1];
    const double x2_max = upper_bounds[1];

    // Функция вычисления значения GKLS
    auto EnergyCalc = [&task](double x1, double x2) {
        MathVec<double> x(std::vector<double>{x1, x2});
        return task.ComputeFunction({ x.GetRawVector() });
    };

    // Создаём директорию, если её ещё нет
    fs::create_directories(output_dir);

    // Формируем имя файла
    const fs::path filename =
        output_dir / ("gkls_" + std::to_string(task_num) + ".csv");

    std::ofstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Cannot open file: " + filename.string()
        );
    }

    // Независимость от локали системы:
    // десятичный разделитель всегда точка
    file.imbue(std::locale::classic());

    // Точность, достаточная для восстановления double
    file << std::setprecision(
        std::numeric_limits<double>::max_digits10
    );

    file << "x1,x2,y\n";

    // Сначала перебираем x2, затем x1.
    // Поэтому x1 изменяется быстрее и образует столбцы сетки.
    for (int j = 0; j < ny; ++j) {
        const double x2 = x2_min +
            (x2_max - x2_min) * j / (ny - 1);

        for (int i = 0; i < nx; ++i) {
            const double x1 = x1_min +
                (x1_max - x1_min) * i / (nx - 1);

            const double y = EnergyCalc(x1, x2);

            file << x1 << ','
                 << x2 << ','
                 << y << '\n';
        }
    }

    if (!file) {
        throw std::runtime_error(
            "Error while writing file: " + filename.string()
        );
    }

    std::cout << "Saved: " << filename << '\n';
}


int main(int argc, char* argv[]) {
    if (argc < 5 || 6 > argc) {
        std::cerr
            << "Usage: " << argv[0]
            << " <first_task> <last_task> <nx> <ny> [output_dir]\n";
        return 1;
    }

    try {
        const int first_task = std::stoi(argv[1]);
        const int last_task  = std::stoi(argv[2]);

        const int nx = std::stoi(argv[3]);
        const int ny = std::stoi(argv[4]);

        const fs::path output_dir =
            (argc == 6) ? argv[5] : "gkls_grids";

        if (first_task > last_task || nx < 2 || ny < 2) {
            throw std::invalid_argument("Invalid arguments");
        }

        for (int task_num = first_task; task_num <= last_task; ++task_num) {
            saveGrid(task_num, nx, ny, output_dir);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}