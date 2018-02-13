#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>
#include <iostream>

using namespace cv;
using namespace std;

Mat image;
int radius = 10;

void callback(int event, int x, int y, int flags, void* userdata)
{
    if(event == EVENT_LBUTTONDOWN)
        cout << "LButton Down " << x << " " << y << endl;
    else if(event == EVENT_LBUTTONUP)
        cout << "LButton Up " << x << " " << y << endl;
    else if(event == EVENT_MOUSEMOVE)
    {
        cout << x << " " << y << ": " << image.at<Vec3b>(y, x) << endl;
        circle(image, Point(x, y), radius, Scalar(255, 0, 0), CV_FILLED);
    }
}

int main()
{
    namedWindow("Image", WINDOW_AUTOSIZE);
    moveWindow("Image", 1000, 0);
    setMouseCallback("Image", callback, NULL);

    image = imread("curless.png", CV_LOAD_IMAGE_COLOR);

    while(true)
    {
        imshow("Image", image);
        if(char(waitKey(1)) == 'q')
            break;
    }
    return 0;
}

