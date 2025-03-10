#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // 1. Загрузка изображения
    Mat originalImage = imread("Testimg.jpg"); 

    if (originalImage.empty()) {
        cout << "Ошибка: не удалось загрузить изображение! Проверьте путь к файлу." << endl;
        return -1;
    }

    imshow("Original Image", originalImage);

    // 2. Переход в различные цветовые пространства
    Mat hsvImage, labImage, yuvImage, xyzImage, grayImage;

    cvtColor(originalImage, hsvImage, COLOR_BGR2HSV);
    cvtColor(originalImage, labImage, COLOR_BGR2Lab);
    cvtColor(originalImage, yuvImage, COLOR_BGR2YUV);
    cvtColor(originalImage, xyzImage, COLOR_BGR2XYZ);
    cvtColor(originalImage, grayImage, COLOR_BGR2GRAY);

    // Вывод результатов цветовых пространств
    imshow("HSV Image", hsvImage);
    imshow("Lab Image", labImage);
    imshow("YUV Image", yuvImage);
    imshow("XYZ Image", xyzImage);
    imshow("Gray Image", grayImage);

    // 3. Преобразование в оттенки серого и применение фильтра Гаусса
    Mat blurredImage;
    GaussianBlur(grayImage, blurredImage, Size(5, 5), 0);
    imshow("Blurred Image", blurredImage);

    // 4. Обнаружение линий с помощью HoughLines
    Mat edges;
    Canny(blurredImage, edges, 50, 150); // Детекция краев

    vector<Vec2f> lines;
    HoughLines(edges, lines, 1, CV_PI / 180, 100);

    Mat houghLinesImage = originalImage.clone();
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(houghLinesImage, pt1, pt2, Scalar(0, 0, 255), 2); // Красные линии
    }
    imshow("Hough Lines", houghLinesImage);

    // 5. Обнаружение окружностей с помощью HoughCircles
    vector<Vec3f> circles;
    HoughCircles(blurredImage, circles, HOUGH_GRADIENT, 1,
        blurredImage.rows / 16, 100, 30, 1, 100);

    Mat houghCirclesImage = originalImage.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        int radius = c[2];
        circle(houghCirclesImage, center, radius, Scalar(0, 255, 0), 2); // Зеленые окружности
        circle(houghCirclesImage, center, 2, Scalar(0, 255, 0), 3); // Центр окружности
    }
    imshow("Hough Circles", houghCirclesImage);

    waitKey(0);
    return 0;
}