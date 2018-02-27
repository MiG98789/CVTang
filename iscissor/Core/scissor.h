#ifndef _SCISSOR_H_
#define _SCISSOR_H_

#include "define.h"

using namespace std;
using namespace cv;

class Scissor
{
    private:
        bool snap;
        bool hide;
        bool tree;

        vector<Point> explore;

        Matrixf         cost;
        Path            path;
        Matrix<Point>   link;

        Mat original;
        Mat finalize;
        Mat minpath;
        Mat visual;
        Mat canvas;
        Mat pixel;
        Mat mask;
        Mat edge;

        int NeighborCost(Point q, Point r);
        Point Snap(Point cursor);
        Point Tree(Point point);

    public:
        Scissor(const Mat& image);

        bool GetLock() const;
        const Mat& GetEdge() const;
        const Mat& GetVisual() const;
        const Mat& GetPixel() const;

        Mat Crop(bool isInverse);
        void Reset();
        void OnClick();
        void PopSeed();
        void ToggleSnap();
        void ToggleTree();
        void ToggleHide();
        void CloseContour();
        void SetBlur(int degree);

        void SaveContour(const char* filename);
        void SaveMask(const char* filename);
        void SaveLasso(const char* filename);

        Mat Blur(const Mat& original, int degree);

        bool Visualize();
        void Pixelize();
        
        void Cost();
        bool Wire(const Point& seed);
        vector<Point> Trace(Point seed, Point cursor);

        void Draw(Mat& canvas);
        void PathTree(Mat& tree, int nodes);
        void ImagelessPathTree(Mat& tree, int nodes);
        void MouseCallback(int event, int x, int y);
};

struct CallbackParam
{
    Scissor* scissor;
    bool click;
    int nodes;
};


#endif
