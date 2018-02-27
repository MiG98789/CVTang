#include "define.h"
#include "scissor.h"

void mouseCallback(int event, int x, int y, int flags, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    while(cp->scissor->GetLock());
    if(event == EVENT_LBUTTONUP)
        cp->click = true;
    cp->scissor->MouseCallback(event, x, y);
}

void text(Mat& image, const char texts[11][50])
{
    int h = image.rows, w = image.cols;
    for(int i = 0; i < 11; i++)
        putText(image, texts[10-i], Point(0, h-1 - i*50-20), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
}

void onSaveContour(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->SaveContour("contour.jpg");
    cout << "Contour saved to contour.jpg" << endl;
}

void onSaveMask(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->SaveMask("mask.jpg");
    cout << "Mask saved to mask.jpg" << endl;
}

void onSaveLasso(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->SaveLasso("lasso.txt");
    cout << "Lasso saved to lasso.txt" << endl;
}

void onCrop(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    Mat crop = cp->scissor->Crop(false);
    imshow("Cropped", crop);
}

void onInverseCrop(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    Mat crop = cp->scissor->Crop(true);
    imshow("Cropped", crop);
}

void onPopSeed(int state, void* data)
{
    CallbackParam* cp= (CallbackParam*)data;
    cp->scissor->PopSeed();
}

void onCostGraph(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    Mat visual = cp->scissor->GetVisual();
    imshow("Cost graph", visual);
}

void onEdgeGraph(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    Mat edge = cp->scissor->GetEdge();
    imshow("Edge reference", edge);
}

void onPixel(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    Mat pixel = cp->scissor->GetPixel();
    imshow("Pixel graph", pixel);
}

void onPathTree(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    Mat tree;
    cp->scissor->ImagelessPathTree(tree, cp->nodes);
    imshow("Path tree", tree);
}

void onSnap(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->ToggleSnap();
}

void onHide(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->ToggleHide();
}

void onReset(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->Reset();
}

void onCloseContour(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->CloseContour();
}

void onMinPath(int state, void* data)
{
    CallbackParam* cp = (CallbackParam*)data;
    cp->scissor->ToggleTree();
}

int main(int argc, char** argv)
{
    Mat canvas, image = imread(argc == 1? "curless.png": argv[1], CV_LOAD_IMAGE_COLOR);
    resize(image, image, Size(), argc < 3? 0.5: atof(argv[2]), argc < 3? 0.5: atof(argv[2]));

    Scissor scissor(image);
    CallbackParam cp = {&scissor, false};

    int prev = 0, degree = 0, dummy = 0;
    namedWindow("Canvas", WINDOW_AUTOSIZE);
    moveWindow("Canvas", 1000, 0);
    setMouseCallback("Canvas", mouseCallback, (void*)&cp);
    cvCreateTrackbar("Blur", "Canvas", &degree, 21, NULL);

    createButton("Save Contour", onSaveContour, (void*)&cp);
    createButton("Save Mask", onSaveMask, (void*)&cp);
    createButton("Save Lasso", onSaveLasso, (void*)&cp);
    cvCreateTrackbar("  ", NULL, &dummy, 1, NULL);
    createButton("Crop", onCrop, (void*)&cp);
    createButton("Inverse Crop", onInverseCrop, (void*)&cp);
    cvCreateTrackbar("Nodes", NULL, &cp.nodes, 50000, NULL);
    createButton("Cost Graph", onCostGraph, (void*)&cp);
    createButton("Edge Graph", onEdgeGraph, (void*)&cp);
    createButton("Pixel Graph", onPixel, (void*)&cp);
    createButton("Path Tree", onPathTree, (void*)&cp);
    createButton("Min Path", onMinPath, (void*)&cp);
    cvCreateTrackbar("   ", NULL, &dummy, 1, NULL);
    createButton("Pop Seed", onPopSeed, (void*)&cp);
    createButton("Snapping", onSnap, (void*)&cp);
    createButton("Contour", onHide, (void*)&cp);
    createButton("Close Contour", onCloseContour, (void*)&cp);
    cvCreateTrackbar("    ", NULL, &dummy, 1, NULL);
    createButton("Reset", onReset, (void*)&cp);

    while(true)
    {
        if(cp.click)
        {
            scissor.OnClick();
            cp.click = false;
        }

        if(degree != prev)
        {
            scissor.SetBlur(degree * 2 + 1);
            prev = degree;
        }

        scissor.Draw(canvas);
        imshow("Canvas", canvas);

        char key = (char)waitKey(1);
        if(key == 'q')
            break;
        else if(key == 8)
            scissor.PopSeed();
        else if(key == 's')
            scissor.SaveContour("contour.txt");
        else if(key == 'e')
            scissor.ToggleSnap();
        else if(key == 'c')
        {
            Mat crop = scissor.Crop(false);
            imshow("Cropped", crop);
        }
        else if(key == 'i')
        {
            Mat crop = scissor.Crop(true);
            imshow("Cropped", crop);
        }
        else if(key == 'v')
        {
            Mat edge = scissor.GetEdge();
            Mat visual = scissor.GetVisual();

            imshow("Edge reference", edge);
            imshow("Cost graph", visual);
        }
        else if(key == 'h')
        {
            char texts[11][50] = {
                "Left  Mouse  - insert seed",
                "Mid   Mouse  - reset",
                "<ctrl> + <p> - display toolbox",
                "<q>          - quit",
                "<bkspace>    - remove last seed",
                "<s>          - save contour to file",
                "<e>          - toggle edge snapping",
                "<c>          - crop with contour",
                "<i>          - inverse crop with contour",
                "<v>          - visualize edge and cost graph",
                "<h>          - display this help page"};

            Mat help = Mat::zeros(Size(900, 550), CV_8UC3);
            text(help, texts);
            imshow("HELP" , help);
        }
    }
    return 0;
}

