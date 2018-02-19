#ifndef _SCISSOR_H_
#define _SCISSOR_H_

#include "define.h"

using namespace std;
using namespace cv;

class Scissor
{
    private:
        bool snap;

        Matrixf         cost;
        Path            path;
        Matrix<Point>   link;

        Mat original;
        Mat visual;
        Mat render;
        Mat canvas;
        Mat edge;

        int NeighborCost(Point q, Point r);
        Point Snap(Point cursor);

    public:
        Scissor(const Mat& image);

        bool GetLock() const;
        const Mat& GetEdge() const;
        const Mat& GetVisual() const;

        Mat Crop();
        void OnClick();
        void PopSeed();
        void ToggleSnap();
        void SaveContour(const char* filename);

        Mat Blur(const Mat& original, int degree);
        bool Visualize();

        void Cost();
        bool Wire(const Point& seed);
        void Trace(Point seed, Point cursor);

        void Draw(Mat& canvas);

        void MouseCallback(int event, int x, int y);
        void ButtonCallback(int state);
};

struct CallbackParam
{
    Scissor* scissor;
    bool click;
};


#endif
