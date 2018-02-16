#include "define.h"

using namespace cv;
using namespace std;

Mat blur(const Mat& original, int degree)
{
    if(degree % 2)
    {
        Mat image;
        GaussianBlur(original, image, Size(degree, degree), 0);
        return image;
    }
}

Point snap(const Mat& original, const Point& cursor)
{
    Mat image;
    cvtColor(original, image, CV_RGB2GRAY);
    image = blur(image, 3);
    Canny(image, image, 40, 180);
    Mat img;
    original.copyTo(img, image);
    cvtColor(img, img, CV_RGB2GRAY);
    imshow("", img);
    waitKey(0);
}

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
                //D color equation from project page
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

    //The maxD - D equation from project page
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
            //Image pixel in the middle, surrounded by cost values
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
    //simple mapping for i-1, j-1 etc to array index
    if(q == r)
        return -1;
    int mapping[3][3] = {{3,  2, 1},
        {4, -1, 0},
        {5,  6, 7}};
    int x = r.x - q.x + 1;
    int y = r.y - q.y + 1;
    return cost.get(q.y, q.x, mapping[y][x]);
}

Matrix<Point> wire(const Point& seed, const Matrixf& cost)
{
    int h = cost.h;
    int w = cost.w;
    float buf[8];
    FibHeap pq;

    //Construct Node structure for shortest path tree calc
    vector<Pnode *> nodes;
    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
            nodes.push_back(new Pnode(Point(j, i), cost.toArray(buf, i, j)));

    //Init seed node
    nodes[seed.y*w + seed.x]->totalCost = 0.0f;
    pq.Insert(nodes[seed.y * w + seed.x]);

    Pnode* q = NULL, *r = NULL;
    while((q = (Pnode*)pq.ExtractMin()) != NULL)
    {
        q->state = EXPANDED;

        for(int i = -1; i < 2; i++)
            for(int j = -1; j < 2; j++)
                if(i != 0 || j != 0)
                {
                    //Get neighbor
                    int ci = q->pt.y + i;
                    int cj = q->pt.x + j;
                    if(ci >= 0 && ci < h && cj >= 0 && cj < w)
                    {
                        //if r is initial state
                        r = nodes[ci*w + cj];
                        if(r->state == INITIAL)
                        {
                            r->prevNode = q;
                            r->totalCost = q->totalCost + neighborCost(cost, q->pt, r->pt);
                            r->state = ACTIVE;
                            pq.Insert(r);
                        }
                        //if r is already in queue
                        else if(r->state == ACTIVE)
                        {
                            float c = q->totalCost + neighborCost(cost, q->pt, r->pt);
                            if(c < r->totalCost)
                            {
                                r->prevNode = q;
                                r->totalCost = c;
                            }
                        }
                    }
                }
    }

    Matrix<Point> path(h, w);
    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
        {
            //Extract prevNodes and add it to matrix
            path(i, j) = Point(j, i) == seed? seed: nodes[i * w + j]->prevNode->pt;
            delete nodes[i * w + j];
        }
    return path;
}

void draw(Mat& canvas, const Mat& image, Path& p, const Matrix<Point>& link)
{
    p.lock = true;
    canvas = image.clone();

    //draw lines for all previously recorded seed point to seed point trails
    for(int i = 0; i < p.trail.size(); i++)
        for(int j = 0; j < (int)p.trail[i].size() - 1; j++)
            line(canvas, p.trail[i][j], p.trail[i][j+1], Scalar(0, 0, 255), 2);

    if(p.seeds.size() > 0)
    {
        Point seed = p.seeds.back();
        Point curr = p.cursor;
        //re-initialize last-seed-point to cursor trail
        p.mouse.clear();
        while(curr != seed)
        {
            p.mouse.push_back(curr);
            //path(i, j) is the next pixel position to link to from Point(j, i) ps. Point uses (x, y) so has to flip ij
            Point prev = link.get(curr.y, curr.x);
            line(canvas, curr, prev, Scalar(0, 0, 255), 2);
            curr = prev;
        }

        //Put emphasis on seed points by drawing green dots
        for(int i = 0; i < p.seeds.size(); i++)
            circle(canvas, p.seeds[i], 3, Scalar(0, 255, 0), -1);
    }
    p.lock = false;
}

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    MouseParam* mp = (MouseParam*)userdata;
    while(mp->p->lock);
    if(event == EVENT_LBUTTONUP)
    {
        //Set seed point to trigger path tree compute
        mp->p->seeds.push_back(Point(x, y));
        mp->click = true;
    }
    else if(event == EVENT_MBUTTONUP)
    {
        //Reset all points when middle mouse click
        mp->p->seeds.clear();
        mp->p->trail.clear();
        mp->p->mouse.clear();
    }
    else if(event == EVENT_MOUSEMOVE)
        mp->p->cursor = Point(x, y);
}

int main(int argc, char** argv)
{
    Path p;
    MouseParam mp = {&p, false};

    namedWindow("Image", WINDOW_AUTOSIZE);
    moveWindow("Image", 1000, 0);
    setMouseCallback("Image", mouseCallback, (void*)&mp);

    Mat canvas, image = imread(argc == 1? "curless.png": argv[1], CV_LOAD_IMAGE_COLOR);
    resize(image, image, Size(), argc < 3? 0.5: atoi(argv[2]), argc < 3? 0.5: atoi(argv[2]));

    snap(image, Point(0, 0));
    image = blur(image, 15);
    Matrixf c = cost(image);

    Mat vis = visualize(image, c);

    Matrix<Point> link(image.rows, image.cols);


    while(1)
    {
        if(mp.click)
        {
            //Clicking means confirming the cursor position as seed point, therefore add the entire seed-to-cursor path to trail
            reverse(p.mouse.begin(), p.mouse.end());
            p.trail.push_back(p.mouse);
            if(p.seeds.size() > 1 && abs(p.mouse.back().x - p.seeds[0].x) + abs(p.mouse.back().y - p.seeds[0].y) < 10)
            {
                p.trail.back().push_back(p.seeds[0]);
                p.seeds.clear();
            }
            else
                link = wire(p.seeds.back(), c);
            mp.click = false;
        }

        //Continous drawing of lasso lines
        draw(canvas, image, p, link);

        imshow("Image", canvas);
        char key = (char)waitKey(1);
        if(key == 'q')
            break;
        else if(key == 8)
        {
            //Back space pops the last seed point and re-draws the lasso
            if(p.seeds.size() > 0)
            {
                p.seeds.pop_back();
                p.trail.pop_back();
                link = wire(p.seeds.back(), c);
            }
        }
        else if(key == 's')
        {
            //Print out all the contour pixels, can be used for file print 
            for(int i = 0; i < p.trail.size(); i++)
                for(int j = 0; j < p.trail[i].size(); j++)
                    cout << p.trail[i][j] << " ";
            cout << endl;
        }
        else if(key == 'c')
        {
            Mat cropped, mask = Mat::zeros(image.size(), CV_8UC3);

            //Set up contours
            vector<vector<Point> > contours(1);
            for(int i = 0; i < p.trail.size(); i++)
                contours[0].insert(contours[0].end(), p.trail[i].begin(), p.trail[i].end());
            drawContours(mask, contours, -1, Scalar(255, 255, 255), -1);

            Rect bound = boundingRect(contours[0]);

            //Mask original image by contour and crop by bounding rect
            image.copyTo(cropped, mask);
            cropped = cropped(bound);

            imshow("Cropped", cropped);
            waitKey(0);
            destroyWindow("Cropped");
        }
    }

    return 0;
}
