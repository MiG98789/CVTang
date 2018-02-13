#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <typeinfo>
#include <cstdlib>
#include <vector>
#include <cmath>

#include "../FibonacciHeap/fibheap.h"

using namespace cv;
using namespace std;

template <typename T>
class Matrix
{
    public:
        int h, w, c;
        vector<T> data;

        Matrix(int h, int w, int c = 1): h(h), w(w), c(c), data(h * w * c) {}

        T& operator()(int x, int y, int z)
        { return data[x*w*c + y*c + z]; }

};

class Matrixf
{
    public:
        int h, w, c;
        vector<float> data;

        Matrixf(int h, int w, int c = 1): h(h), w(w), c(c), data(h * w * c, 0.0f) {}

        Matrixf(const Mat& img): h(img.rows), w(img.cols), c(img.channels()), data(h * w * c, 0.0f)
        { 
            for(int i = 0; i < h; i++)
                for(int j = 0; j < w; j++)
                {
                    Vec3b color = img.at<Vec3b>(i, j);
                    for(int k = 0; k < c; k++)
                        data[i * w * c + j * c + k] = static_cast<float>(color[k]);
                }
        }

        float* toArray(float out[8], int i, int j) const
        {
            for(int l = 0; l < 8; l++)
                out[l] = data[i * w * c + j * c + l];
            return out;
        }

        bool inbound(int x, int y, int z = 0) const
        { return !(x < 0 || y < 0 || z < 0 || x >=h || y >= w || z >= c); }

        float& operator()(int x, int y, int z)
        { return data[x*w*c + y*c + z]; }

        float get(int x, int y, int z) const
        { return this->inbound(x, y, z)? data[x*w*c + y*c + z]: 0.0f; }

        float& operator()(int x, int y)
        { return data[x*w + y]; }

        float get(int x, int y) const
        { return this->inbound(x, y, 0)? data[x*w + y]: 0.0f; }
};

const int INITIAL = 0, ACTIVE = 1, EXPANDED = 2;
class Pnode: public FibHeapNode
{
    public:
        Point pt;
        int state;
        double totalCost; 
        float linkCost[8]; 
        Pnode *prevNode;

        Pnode(Point p, const float c[8]): FibHeapNode(), pt(p), state(INITIAL), totalCost(0.0f), prevNode(NULL)
        { for(int i = 0; i < 8; i++) linkCost[i] = c[i]; }

        virtual void operator=(FibHeapNode& RHS)
        {
            FHN_Assign(RHS);
            Pnode pnode = ((Pnode&)RHS);
            pt = pnode.pt;
            state = pnode.state;
            totalCost = pnode.totalCost;
            for(int i = 0; i < 8; i++)
                linkCost[i] = pnode.linkCost[i];
            prevNode = pnode.prevNode;
        }

        virtual int operator==(FibHeapNode& RHS)
        {
            if(FHN_Cmp(RHS)) return 0;
            return totalCost == ((Pnode&)RHS).totalCost;
        }

        virtual int operator<(FibHeapNode& RHS)
        {
            int X;
            if((X = FHN_Cmp(RHS)) != 0)
                return X < 0? 1 : 0;
            return totalCost < ((Pnode&)RHS).totalCost;
        }
};

#endif
