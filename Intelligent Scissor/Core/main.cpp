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

void text(Mat& image, const char texts[9][50])
{
    int h = image.rows, w = image.cols;
    for(int i = 0; i < 9; i++)
        putText(image, texts[8-i], Point(0, h-1 - i*50-20), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
}

int main(int argc, char** argv)
{
    Mat canvas, image = imread(argc == 1? "curless.png": argv[1], CV_LOAD_IMAGE_COLOR);
    resize(image, image, Size(), argc < 3? 0.5: atof(argv[2]), argc < 3? 0.5: atof(argv[2]));

    Scissor scissor(image);
    CallbackParam cp = {&scissor, false};

    namedWindow("Canvas", WINDOW_AUTOSIZE);
    moveWindow("Canvas", 1000, 0);
    setMouseCallback("Canvas", mouseCallback, (void*)&cp);

    int prev = 0, degree = 0;
    createTrackbar("Blur", "Canvas", &degree, 21);

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
            Mat crop = scissor.Crop();
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
            char texts[9][50] = {
                "Left  Mouse  - insert seed",
                "Mid   Mouse  - reset",
                "<q>          - quit",
                "<bkspace>    - remove last seed",
                "<s>          - save contour to file",
                "<e>          - toggle edge snapping",
                "<c>          - crop with contour",
                "<v>          - visualize edge and cost graph",
                "<h>          - display this help page"};

            Mat help = Mat::zeros(Size(900, 500), CV_8UC3);
            text(help, texts);
            imshow("HELP" , help);
        }
    }
    return 0;
}
