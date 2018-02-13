#include "define.h"

using namespace cv;
using namespace std;

Matrixf cost(const Mat& original)
{
    int h = original.rows;
    int w = original.cols;
    int c = original.channels();
    Matrixf img(original);
    Matrixf d_color(4, c);
    Matrixf cost(h, w, 8);
    float maxD = -1.0f;

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
        {
            for (int k = 0; k < c; k++)
            {
                d_color(0, k) = abs( (img.get(i-1, j  , k) + img.get(i-1, j+1, k))/2.0f - (img.get(i+1, j  , k) + img.get(i+1, j+1, k))/2.0f ) / 2.0f;
                d_color(2, k) = abs( (img.get(i-1, j-1, k) + img.get(i  , j-1, k))/2.0f - (img.get(i-1, j+1, k) + img.get(i  , j+1, k))/2.0f ) / 2.0f;
                d_color(1, k) = abs( img.get(i-1, j  , k) - img.get(i  , j+1, k) ) / sqrt(2.0f);
                d_color(3, k) = abs( img.get(i  , j-1, k) - img.get(i-1, j  , k) ) / sqrt(2.0f);
            }

            for (int k = 0; k < 4; k++)
            {
                //Sum of squres of color channels
                cost(i, j, k) = sqrt( (pow(d_color.get(k, 0), 2) + pow(d_color.get(k, 1), 2) + pow(d_color.get(k, 2), 2)) / 3.0f);
                //Get max element using the same for loop
                if (cost.get(i, j, k) > maxD)
                    maxD = cost.get(i, j, k);
            }

            //Cost symmetry
            if(cost.inbound(i  , j+1, 4))
                cost(i  , j+1, 4) = cost.get(i, j, 0);
            if(cost.inbound(i-1, j+1, 5))
                cost(i-1, j+1, 5) = cost.get(i, j, 1);
            if(cost.inbound(i-1, j  , 6))
                cost(i-1, j  , 6) = cost.get(i, j, 2);
            if(cost.inbound(i-1, j-1, 7))
                cost(i-1, j-1, 7) = cost.get(i, j, 3);

        }

    for (int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
            for(int k = 0; k < 8; k++)
                cost(i, j, k) = (maxD - cost.get(i, j, k)) * sqrt(k % 2 + 1) / 2.0f;

    return cost;

}

Mat visualize(const Mat& image, const Matrixf& cost)
{
    int h = image.rows;
    int w = image.cols;
    Mat vis = Mat::zeros(Size(3 * w, 3 * h), CV_8UC3);

    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++) 
        {
            vis.at<Vec3b>(3*i+1, 3*j+1) = image.at<Vec3b>(i, j);
            vis.at<Vec3b>(3*i+1, 3*j+2) = Vec3b(cost.get(i, j, 0), cost.get(i, j, 0), cost.get(i, j, 0));
            vis.at<Vec3b>(3*i  , 3*j+2) = Vec3b(cost.get(i, j, 1), cost.get(i, j, 1), cost.get(i, j, 1));
            vis.at<Vec3b>(3*i  , 3*j+1) = Vec3b(cost.get(i, j, 2), cost.get(i, j, 2), cost.get(i, j, 2));
            vis.at<Vec3b>(3*i  , 3*j  ) = Vec3b(cost.get(i, j, 3), cost.get(i, j, 3), cost.get(i, j, 3));
            vis.at<Vec3b>(3*i+1, 3*j  ) = Vec3b(cost.get(i, j, 4), cost.get(i, j, 4), cost.get(i, j, 4));
            vis.at<Vec3b>(3*i+2, 3*j  ) = Vec3b(cost.get(i, j, 5), cost.get(i, j, 5), cost.get(i, j, 5));
            vis.at<Vec3b>(3*i+2, 3*j+1) = Vec3b(cost.get(i, j, 6), cost.get(i, j, 6), cost.get(i, j, 6));
            vis.at<Vec3b>(3*i+2, 3*j+2) = Vec3b(cost.get(i, j, 7), cost.get(i, j, 7), cost.get(i, j, 7));
        }

    return vis;
}

int neighborCost(const Matrixf& cost, Point q, Point r)
{
    int mapping;
    Point diff(r.x - q.x, r.y - q.y);
    
}

vector<Point> wire(const Point& seed, const Matrixf& cost)
{
    float buf[8];
    FibHeap pq;

    vector<Pnode*> nodes(cost.h * cost.w);
    for(int i = 0; i < cost.h; i++)
        for(int j = 0; j < cost.w; j++)
            nodes.push_back(new Pnode(Point(j, i), cost.toArray(buf, j, i)));

    pq.Insert(nodes[seed.y * cost.h + seed.x]);

    Pnode* q = NULL, *r = NULL;
    while((q = (Pnode*)pq.ExtractMin()) != NULL)
    {
        q->state = EXPANDED;

        for(int i = -1; i < 2; i++)
            for(int j = -1; j < 2; j++)
                if(i != 0 || j != 0)
                {
                    //Get neighbor
                    r = nodes[(q->pt.y + i) * cost.h + q->pt.x + j];
                    if(r->state != EXPANDED)
                    {
                        r->prevNode = q;
                        r->totalCost = q->totalCost;
                    }
                }
    }

    cout << "LEL" << endl;
    vector<Point> v;
    return v;
}

Point prev_pt = Point(-1, -1);
Point seed_pt = Point(-1, -1);
Point move_pt = Point(-1, -1);
void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    if(event == EVENT_LBUTTONUP)
    {
        cout << "LBUTTON UP " << x << " " << y << endl;
        seed_pt = seed_pt.x == -1? Point(x, y): Point(-1, -1);
    }
    else if(event == EVENT_MOUSEMOVE)
        move_pt = Point(x, y);
}

int main()
{
    namedWindow("Image", WINDOW_AUTOSIZE);
    moveWindow("Image", 1000, 0);
    setMouseCallback("Image", mouseCallback, NULL);

    Mat image = imread("../playground/curless.png", CV_LOAD_IMAGE_COLOR);
    resize(image, image, Size(), 0.2, 0.2);
    Matrixf c = cost(image);

    Mat vis = visualize(image, c);

    while(1)
    {
        if(seed_pt.x != -1 && move_pt.x != -1)
            vector<Point> p = wire(seed_pt, c);

        imshow("Image", vis);
        if((char)waitKey(1) == 'q') break;
    }

    return 0;
}
