#include "define.h"
#include "scissor.h"

Scissor::Scissor(const Mat& image): snap(false), hide(false), tree(false), cost(image.rows, image.cols, 8), link(image.rows, image.cols)
{
    original = image.clone();
    finalize = image.clone();
    cvtColor(finalize, edge, CV_RGB2GRAY);
    edge = Blur(edge, 5);
    Canny(edge, edge, 40, 180);
    rectangle(edge, Point(0, 0), Point(edge.cols-1, edge.rows-1), Scalar(255));

    Cost();
    Visualize();
    Pixelize();
}

void Scissor::Reset()
{
    path.seeds.clear();
    path.trail.clear();
    path.mouse.clear();
}

bool Scissor::GetLock() const
{
    return path.lock;
}

const Mat& Scissor::GetEdge() const
{
    return edge;
}

const Mat& Scissor::GetVisual() const
{
    return visual;
}

const Mat& Scissor::GetPixel() const
{
    return pixel;
}

int Scissor::NeighborCost(Point q, Point r)
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

Mat Scissor::Blur(const Mat& input, int degree)
{
    Mat image;
    if(degree%2)
        GaussianBlur(input, image, Size(degree, degree), 0);
    return image;
}

void Scissor::SetBlur(int degree)
{
    path.seeds.clear();
    path.trail.clear();
    path.mouse.clear();

    finalize = degree? Blur(original, degree): original.clone();
    cvtColor(finalize, edge, CV_RGB2GRAY);
    edge = Blur(edge, 5);
    Canny(edge, edge, 40, 180);
    rectangle(edge, Point(0, 0), Point(edge.cols-1, edge.rows-1), Scalar(255));

    Cost();
    Visualize();
    Pixelize();
}

bool Scissor::Visualize()
{
    if(cost.empty())
        return false;

    int h = finalize.rows;
    int w = finalize.cols;
    visual = Mat::zeros(Size(3 * w, 3 * h), CV_8UC3);

    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++) 
        {
            //Image pixel in the middle, surrounded by cost values
            visual.at<Vec3b>(3*i+1, 3*j+1) = finalize.at<Vec3b>(i, j);
            visual.at<Vec3b>(3*i+1, 3*j+2) = Vec3b(cost.get(i, j, 0), cost.get(i, j, 0), cost.get(i, j, 0));
            visual.at<Vec3b>(3*i  , 3*j+2) = Vec3b(cost.get(i, j, 1), cost.get(i, j, 1), cost.get(i, j, 1));
            visual.at<Vec3b>(3*i  , 3*j+1) = Vec3b(cost.get(i, j, 2), cost.get(i, j, 2), cost.get(i, j, 2));
            visual.at<Vec3b>(3*i  , 3*j  ) = Vec3b(cost.get(i, j, 3), cost.get(i, j, 3), cost.get(i, j, 3));
            visual.at<Vec3b>(3*i+1, 3*j  ) = Vec3b(cost.get(i, j, 4), cost.get(i, j, 4), cost.get(i, j, 4));
            visual.at<Vec3b>(3*i+2, 3*j  ) = Vec3b(cost.get(i, j, 5), cost.get(i, j, 5), cost.get(i, j, 5));
            visual.at<Vec3b>(3*i+2, 3*j+1) = Vec3b(cost.get(i, j, 6), cost.get(i, j, 6), cost.get(i, j, 6));
            visual.at<Vec3b>(3*i+2, 3*j+2) = Vec3b(cost.get(i, j, 7), cost.get(i, j, 7), cost.get(i, j, 7));
        }
    return true;
}

void Scissor::Pixelize()
{
    if(cost.empty())
        return;

    int h = finalize.rows;
    int w = finalize.cols;
    pixel = Mat::zeros(Size(3 * w, 3 * h), CV_8UC3);

    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++) 
            //Image pixel in the middle, surrounded by cost values
            pixel.at<Vec3b>(3*i+1, 3*j+1) = finalize.at<Vec3b>(i, j);
}

void Scissor::PathTree(Mat& tree, int nodes)
{
    int h = finalize.rows;
    int w = finalize.cols;
    tree = Mat::zeros(Size(3 * w, 3 * h), CV_8UC3);

    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
            tree.at<Vec3b>(3*i+1, 3*j+1) = finalize.at<Vec3b>(i, j);

    for(int i = 0; i < nodes && i < explore.size(); i++)
    {
        vector<Point> route = Trace(path.seeds.back(), explore[i]);
        int size = route.size();
        for(int j = 0; j < size - 1; j++)
        {
            Point p = route[size - j - 2], p1 = route[size - j - 1];
            int ox = p1.x - p.x;
            int oy = p1.y - p.y;
            tree.at<Vec3b>(3*p.y+1+oy, 3*p.x+1+ox) = Vec3b(0, min(j*2, 255), min(j*2, 255));
        }
    }
}

void Scissor::ImagelessPathTree(Mat& tree, int nodes)
{
    int h = finalize.rows;
    int w = finalize.cols;
    tree = Mat::zeros(Size(3 * w, 3 * h), CV_8UC3);

    for(int i = 0; i < nodes && i < explore.size(); i++)
    {
        vector<Point> route = Trace(path.seeds.back(), explore[i]);
        int size = route.size();
        for(int j = 0; j < size - 1; j++)
        {
            Point p = route[size - j - 2], p1 = route[size - j - 1];
            int ox = p1.x - p.x;
            int oy = p1.y - p.y;
            tree.at<Vec3b>(3*p.y+1+oy, 3*p.x+1+ox) = Vec3b(0, 255, 0);
        }
    }
}

Point Scissor::Tree(Point point)
{
    return tree? point * 3 + 1: point;
}

Point Scissor::Snap(Point cursor)
{
    int h = edge.rows, w = edge.cols;
    int X = w, Y = h;
    int x = 0, y = 0, dx = 0, dy = -1;
    int t = max(X, Y), maxi = t * t;

    for(int i = 0; i < maxi; i++)
    {
        if((-X/2 < x) && (x <= X/2) && (-Y/2 < y) && (y <= Y/2))
        {
            int cx = cursor.x + x, cy = cursor.y + y;
            if(cx >= 0 && cx < w && cy >= 0 && cy < h)
                if(edge.at<uchar>(cy, cx) == 255)
                    return Point(cx, cy);
        }
        if((x == y) || (x < 0 && x == -y) || (x > 0 && x == 1-y))
        {
            t = dx;
            dx = -dy;
            dy = t;
        }
        x += dx;
        y += dy;
    }
    return Point(-1, -1);
}

void Scissor::Cost()
{
    int h = finalize.rows;
    int w = finalize.cols;
    int c = finalize.channels();
    Matrixf img(finalize);
    Matrixf d_color(4, c);
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
}

bool Scissor::Wire(const Point& seed)
{
    if(cost.empty())
        return false;

    int h = cost.h;
    int w = cost.w;
    float buf[8];
    FibHeap pq;

    explore.clear();

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
                            r->totalCost = q->totalCost + NeighborCost(q->pt, r->pt);
                            r->state = ACTIVE;
                            pq.Insert(r);

                            explore.push_back(r->pt);
                        }
                        //if r is already in queue
                        else if(r->state == ACTIVE)
                        {
                            float c = q->totalCost + NeighborCost(q->pt, r->pt);
                            if(c < r->totalCost)
                            {
                                r->prevNode = q;
                                r->totalCost = c;
                            }
                        }
                    }
                }
    }

    link = Matrix<Point>(h, w);
    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
        {
            //Extract prevNodes and add it to matrix
            link(i, j) = Point(j, i) == seed? seed: nodes[i * w + j]->prevNode->pt;
            delete nodes[i * w + j];
        }
    return true;
}

vector<Point> Scissor::Trace(Point seed, Point cursor)
{
    vector<Point> route;
    while(cursor != seed)
    {
        route.push_back(cursor);
        cursor = link.get(cursor.y, cursor.x);
    }
    return route;
}

void Scissor::Draw(Mat& draw_canvas)
{
    path.lock = true;

    if(tree)
        canvas = minpath.clone();
    else
        canvas = finalize.clone();

    if(!hide)
    {
        //draw lines for all previously recorded seed point to seed point trails
        for(int i = 0; i < path.trail.size(); i++)
            for(int j = 0; j < (int)path.trail[i].size() - 1; j++)
                line(canvas, Tree(path.trail[i][j]), Tree(path.trail[i][j+1]), Scalar(0, 0, 255), 2);

        if(path.seeds.size() > 0)
        {
            path.mouse = Trace(path.seeds.back(), path.cursor);
            for(int i = 0; i < (int)path.mouse.size() - 1; i++)
                line(canvas, Tree(path.mouse[i]), Tree(path.mouse[i+1]), Scalar(0, 0, 255), 2);

            //pathut empathhasis on seed pathoints by drawing green dots
            for(int i = 0; i < path.seeds.size(); i++)
                circle(canvas, Tree(path.seeds[i]), 3, Scalar(0, 255, 0), -1);
        }

        circle(canvas, Tree(path.cursor), 3, snap? Scalar(255, 0, 255): Scalar(0, 255, 0), -1);
    }

    draw_canvas = canvas.clone();
    path.lock = false;
}

void Scissor::OnClick()
{
    if(tree)
        return;

    if(snap)
    {
        path.seeds.back() = Snap(path.seeds.back());

        if(path.seeds.size() > 1)
            path.mouse = Trace(path.seeds.end()[-2], path.seeds.back());
    }
    //Clicking means confirming the cursor position as seed point, therefore add the entire seed-to-cursor path to trail
    if(path.mouse.size() > 0)
    {
        reverse(path.mouse.begin(), path.mouse.end());
        path.trail.push_back(path.mouse);
    }
    if(path.seeds.size() > 1 && abs(path.mouse.back().x - path.seeds[0].x) + abs(path.mouse.back().y - path.seeds[0].y) < 10)
    {
        path.trail.back().push_back(path.seeds[0]);
        path.seeds.clear();
    }
    else
        Wire(path.seeds.back());
}

void Scissor::PopSeed()
{
    if(path.seeds.size() > 1)
    {
        path.seeds.pop_back();
        path.trail.pop_back();
        Wire(path.seeds.back());
    }
    else if(path.seeds.size() == 1)
    {
        path.seeds.clear();
        path.trail.clear();
        path.mouse.clear();
    }
}

void Scissor::CloseContour()
{
    path.mouse = Trace(path.seeds.back(), path.seeds[0]);
    path.trail.back().insert(path.trail.back().end(), path.mouse.rbegin(), path.mouse.rend());
    path.seeds.clear();
}

void Scissor::SaveContour(const char* filename)
{
    while(GetLock());
    imwrite(filename, canvas);
}

void Scissor::SaveMask(const char* filename)
{
    imwrite(filename, mask);
}

void Scissor::SaveLasso(const char* filename)
{
    if(path.trail.size() == 0)
        return;
    ofstream out(filename);
    for(int i = 0; i < path.trail.size(); i++)
        out << path.trail[i] << " ";
    out << endl;
}

void Scissor::ToggleSnap()
{
    snap = !snap;
    cout << "Snap " << (snap? "on": "off") << endl;
}

void Scissor::ToggleHide()
{
    hide = !hide;
    cout << "Hide " << (hide? "on": "off") << endl;
}

void Scissor::ToggleTree()
{
    tree = !tree;
    if(tree)
        ImagelessPathTree(minpath, original.rows * original.cols);
    cout << "Min Path " << (tree? "on": "off") << endl;
}

Mat Scissor::Crop(bool isInverse)
{
    Mat cropped;
    mask = Mat::zeros(finalize.size(), CV_8UC3);

    //Set up contours
    vector<vector<Point> > contours(1);
    for(int i = 0; i < path.trail.size(); i++)
        contours[0].insert(contours[0].end(), path.trail[i].begin(), path.trail[i].end());

    drawContours(mask, contours, -1, Scalar(255, 255, 255), -1);
    Rect bound = boundingRect(contours[0]);

    if (isInverse)
    {
        bitwise_not(mask,mask);   
    }

    //Mask original image by contour and crop by bounding rect
    finalize.copyTo(cropped, mask);

    if (!isInverse)
    {
        cropped = cropped(bound);
    }

    return cropped;
}

void Scissor::MouseCallback(int event, int x, int y)
{
    while(path.lock);
    Point p = tree? Point(x/3, y/3): Point(x, y);
    if(event == EVENT_LBUTTONUP && !tree)
        //Set seed point to trigger path tree compute
        path.seeds.push_back(p);
    else if(event == EVENT_MBUTTONUP)
    {
        //Reset all points when middle mouse click
        path.seeds.clear();
        path.trail.clear();
        path.mouse.clear();
    }
    else if(event == EVENT_MOUSEMOVE)
        path.cursor = snap? Snap(p): p;
}

