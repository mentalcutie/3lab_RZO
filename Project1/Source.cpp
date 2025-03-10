#include <iostream>
#include <clocale>
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>

#include "sciplot/Plot2D.hpp"
#include "sciplot/sciplot.hpp"
using namespace sciplot;

// структура для рисования графиков
template <typename CustomDrawFunction, typename CustomCanvasFunction>
struct Drawer
{
    CustomDrawFunction drawFunctions;
    CustomCanvasFunction canvasFunctions;
    Drawer(CustomDrawFunction drawFunctions,
        CustomCanvasFunction canvasFunctions) : drawFunctions{ drawFunctions },
        canvasFunctions{ canvasFunctions } {}
    void draw(bool show = true)
    {
        Plot2D plot;
        drawFunctions(plot);
        plot.legend().atOutsideBottomLeft().displayHorizontal().displayExpandWidthBy(10).displayExpandHeightBy(5);
        Figure fig = { {plot} };
        Canvas canvas = { {fig} };
        canvasFunctions(canvas);
        canvas.size(1024, 600);
        if (show)
        {
            canvas.show();
        }
    }
};

int main(int argc, char** argv)
{
    // стараемся сделать правильным вывод в консоль
    setlocale(LC_ALL, ".UTF8");
    // количество данных и степень полинома для интерполяции
    int N{}, m{};

    std::vector<double> a, praw, x, y;
    std::vector<std::vector<double>> sumx;

    if (argc > 1) {
        // если передали любой параметр при запуске то имеем возможность ввести все данные
        std::cout << u8"Введите число измерений N = ";
        std::cin >> N;
        std::cout << u8"Введите степень аппрокхсимирующего полинома m = ";
        std::cin >> m;
        std::cout << u8"Введите " << N << u8" измерений в виде t ω построчно:" << std::endl;
        x.resize(N);
        y.resize(N);
        for (int i = 0; i < N; i++)
        {
            std::cin >> x[i] >> y[i];
        }
    }
    else {
        // если запускаем без аргументов то берем значения из варианта
        N = 6;
        m = 3;
        x = { 5.7,  9.6,  16.0, 24.4, 29.8, 34.4 };
        y = { 2.60, 2.01,  1.34, 0.94, 1.06, 1.25 };
    }

    // Делаем замену пеменной x=t-15 чтобы аппроксимирующая функция из 
    // задания имела вид а+bx+cxx без всяких (t-15)^2

    for (auto&& xx : x) {
        xx = xx - 15.0f;
    }

    // создаем матрицы коэффициентов 
    a = std::vector<double>(m + 1, 0);
    praw = std::vector<double>(m + 1, 0);
    sumx = std::vector<std::vector<double>>(m + 1, std::vector<double>(m + 1, 0));

    // заполняем массив POWERX по переданным значениям N, m, х_i
    auto powerx = std::vector<double>(2 * (m + 1), 0.f);
    for (int i = 0; i < 2 * (m + 1); i++) {
        for (int j = 0; j < N; j++) {
            powerx[i] += powl(x[j], static_cast<long double>(i));
        }
    }

    {
        int i = 0, j = 0, k = 0;
        // запонялем матрицу SUMX
        for (i = 0; i < m + 1; i++)
        {
            for (j = 0; j < m + 1; j++)
            {
                sumx[i][j] = powerx[i + j];
            }
        }
        sumx[0][0] = N;
        // заполняем столбец правых частей PRAW
        for (i = 0; i < m + 1; i++)
        {
            for (k = 0; k < N; k++)
            {
                praw[i] += powl(x[k], static_cast<long double>(i)) * y[k];
            }
        }
    }

    // метода Гаусса для решения системы SUMX*a=PRAW относительно а 
    // находим нули на диагоналях и меняем местами такие строки с Не нулем в том же месте по столбцу
    {
        int i, j, k;
        double temp = 0;
        for (i = 0; i < m + 1; i++)
        {
            if (sumx[i][i] == 0)
            {
                for (j = 0; j < m + 1; j++)
                {
                    if (j == i)
                        continue;
                    if (sumx[j][i] != 0 && sumx[i][j] != 0)
                    {
                        for (k = 0; k < m + 1; k++)
                        {
                            temp = sumx[j][k];
                            sumx[j][k] = sumx[i][k];
                            sumx[i][k] = temp;
                        }
                        temp = praw[j];
                        praw[j] = praw[i];
                        praw[i] = temp;
                        break;
                    }
                }
            }
        }
    }
    {
        int i = 0, j = 0, k = 0;
        //приводим к нижнетреугольной матрице
        for (k = 0; k < m + 1; k++)
        {
            for (i = k + 1; i < m + 1; i++)
            {
                if (sumx[k][k] == 0)
                {
                    std::cout << u8"Нет решений матрицы!" << std::endl;
                    return -1;
                }
                double M = sumx[i][k] / sumx[k][k];
                for (j = k; j < m + 1; j++)
                {
                    sumx[i][j] -= M * sumx[k][j];
                }
                praw[i] -= M * praw[k];
            }
        }
        // привели к нижнетреугольной, можем вычичсилить каждый а_i явно:
        for (i = static_cast<int>(m + 1) - 1; i >= 0; i--)
        {
            double s = 0;
            for (j = i; j < m + 1; j++)
            {
                s = s + sumx[i][j] * a[j];
            }
            a[i] = (praw[i] - s) / sumx[i][i];
        }
    }

    // считаем среднеквадратичную ошибку
    double error2 = 0.f;
    std::vector<double> new_Y;
    for (size_t i = 0; i < x.size(); i++)
    {
        auto xx = x[i];
        auto yy = y[i];
        // a[0] + a[1] * x + a[2] * x * x+a[3]*x*x*x
        double new_y = 0.f, pow_x = 1.f;
        for (int j = 0; j <= m; j++)
        {
            new_y += a[j] * pow_x;
            pow_x *= xx;
        }
        new_Y.push_back(new_y);
        error2 += (new_y - yy) * (new_y - yy);
    }

    double error = sqrtl(error2 / static_cast<double>(N - m - 1));
    std::cout << u8"Среднеквадратическое отклонение равно: " << error << std::endl;

    // подготавливаем данные для рисования графика 
    // f(x) - с заданной степенью по m  , ω(t) - с квадратичной степенью из задания 
    // точность рисования контролируется шагом precision
    std::vector<double> f_x, ksi_x, new_x;
    constexpr double precision = 0.01;
    for (auto xx = x.front(); xx <= x.back(); xx += precision) {
        new_x.push_back(xx);
        double newy{ 0.f }, powx = 1.f;
        for (int i = 0; i <= m; i++)
        {
            newy += a[i] * powx;
            powx *= xx;
            if (i == 2) {
                ksi_x.push_back(newy);
            }
        }
        f_x.push_back(newy);
    }
    // не забываем сдвинуть графики обратно 
    for (auto&& xx : x) {
        xx += 15.0f;
    }
    for (auto&& xx : new_x) {
        xx += 15.0f;
    }

    std::cout << u8"Искомые коэффициенты в задаче:" << std::endl;
    std::cout << u8" ω_15 = " << a[0] << std::endl;
    std::cout << u8" β = " << a[1] << std::endl;
    std::cout << u8" γ = " << a[2] << std::endl;

    Drawer d{
        [&](Plot2D& plot)
        {
            plot.xlabel("x").fontSize(14);
            plot.ylabel("y").fontSize(14);
            // рисуем точки исходнных данных
            plot.drawPoints(x, y).label("Точки исходных данных");
            // и полученные функции
            plot.drawCurve(new_x, f_x).label("Итог Аппроксимации f(x) co степенью " + std::to_string(m));
            plot.drawCurve(new_x, ksi_x).label("ω(t)");
        },
        [](Canvas& canvas)
        {
            canvas.title("Аппроксимация методом наименьших квадратов");
        } };
    d.draw();
}
