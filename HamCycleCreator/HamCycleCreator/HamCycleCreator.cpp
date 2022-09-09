/******************************************************************************
 Author: Thomas Eberhart (ThomasEEEEEEEE)
 Description: This program contains a simple path drawing program designed to
              draw and create hamiltonian cycles. This program was designed to
              be used in tandem with a Snake AI program written alongside it.

              The controls for the program are as follows:
              Left Click + Drag: Draw path
              Right Click:       Reset path to point
              Enter:             Export path to file

              The format of the output text files simply consists of two numbers
              stating the width and the height of the grid, followed by the numbers
              in the cycle itself.
******************************************************************************/

#define OLC_PGE_APPLICATION
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "olcPixelGameEngine.h"
using namespace olc;
using std::vector;
using std::ofstream;
using std::string;

class CycleCreator : public PixelGameEngine
{
public:
    int GridWidth;
    int GridHeight;
    int TileSize;
    bool Drawing;
    double DrawMessageTimer = 0.0;
    string Message;

    vector<int> Cycle;

    CycleCreator(int width, int height)
    {
        sAppName = "Hamiltonian Cycle Creator";

        GridWidth = width;
        GridHeight = height;
    }

    bool OnUserCreate()
    {
        GridWidth;
        GridHeight;
        TileSize = ScreenWidth() / std::max(GridWidth, GridHeight);
        Drawing = false;

        Cycle.push_back(0);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime)
    {

        if (GetMouse(0).bPressed)
        {
            int mx = GetMousePos().x / TileSize;
            int my = GetMousePos().y / TileSize;
            if (Cycle.size() == 0 || Cycle.back() == my * GridWidth + mx)
            {
                Drawing = true;
            }
        }
        if (GetMouse(0).bReleased)
        {
            Drawing = false;
        }
        if (GetMouse(0).bHeld && Drawing)
        {
            int mx = GetMousePos().x / TileSize;
            int my = GetMousePos().y / TileSize;

            if (Cycle.size() == 0 || (std::find(Cycle.begin(), Cycle.end(), my * GridWidth + mx) == Cycle.end() && Neighbors(Cycle.back(), my * GridWidth + mx)))
            {
                Cycle.push_back(my * GridWidth + mx);
            }
        }
        if (GetMouse(1).bPressed)
        {
            int mx = GetMousePos().x / TileSize;
            int my = GetMousePos().y / TileSize;
            if (std::find(Cycle.begin(), Cycle.end(), my * GridWidth + mx) != Cycle.end())
            {
                int itr = Cycle.back();
                while (itr != my * GridWidth + mx)
                {
                    Cycle.pop_back();
                    itr = Cycle.back();
                }
            }
        }

        //Draw
        Clear(BLACK);
        for (int x = 0; x < GridWidth; ++x)
        {
            for (int y = 0; y < GridHeight; ++y)
            {
                FillRect(x * TileSize, y * TileSize, TileSize, TileSize, CYAN);
                DrawRect(x * TileSize, y * TileSize, TileSize, TileSize, BLACK);
            }
        }

        for (int i = 0; i < Cycle.size(); ++i)
        {
            int cx = Cycle[i] % GridWidth;
            int cy = Cycle[i] / GridWidth;
            FillRect(cx * TileSize, cy * TileSize, TileSize, TileSize, DARK_BLUE);
            DrawRect(cx * TileSize, cy * TileSize, TileSize, TileSize, BLACK);
        }

        DisplayCycle();

        if (GetKey(ENTER).bPressed)
        {
            if (Cycle.size() == GridWidth * GridHeight && Neighbors(Cycle.front(), Cycle.back()))
            {
                ExportPath();

                Message = "Cycle Exported";
                DrawMessageTimer = 3;
            }
            else if (Cycle.size() != GridWidth * GridHeight)
            {
                Message = "Cycle not complete";
                DrawMessageTimer = 3;
            }
            else if (!Neighbors(Cycle.front(), Cycle.back()))
            {
                Message = "Cycle not a circuit";
                DrawMessageTimer = 3;
            }
        }

        if (DrawMessageTimer > 0)
        {
            DrawString(5, 5, Message, olc::WHITE, 3);
            DrawMessageTimer -= fElapsedTime;
        }

        return true;
    }

    //Returns if two positions are neighbors on the grid
    bool Neighbors(int pos1, int pos2)
    {
        if (pos1 == pos2 + 1 && pos1 % GridWidth != 0)
            return true;
        if (pos1 == pos2 - 1 && pos2 % GridWidth != 0)
            return true;
        if (pos1 == pos2 - GridWidth && pos2 < GridWidth * GridHeight)
            return true;
        if (pos1 == pos2 + GridWidth && pos1 > 0)
            return true;

        return false;
    }

    void DisplayCycle()
    {
        for (int i = 0; i < Cycle.size(); ++i)
        {
            int p = Cycle[i];
            int pn;
            if (i == Cycle.size() - 1)
                pn = -100;
            else
                pn = Cycle[i + 1];

            int x = (p % GridWidth) * TileSize + (TileSize / 2);
            int y = (p / GridWidth) * TileSize + (TileSize / 2);

            if (p + 1 == pn)
                DrawLine(x, y, x + TileSize, y, olc::WHITE);
            else if (p - 1 == pn)
                DrawLine(x, y, x - TileSize, y, olc::WHITE);
            else if (p - GridWidth == pn)
                DrawLine(x, y, x, y - TileSize, olc::WHITE);
            else if (p + GridWidth == pn)
                DrawLine(x, y, x, y + TileSize, olc::WHITE);
            else //Should only happen with the "head" of the path
                FillCircle(x, y, 3, olc::WHITE);
        }
    }

    void ExportPath()
    {
        ofstream file;
        file.open("HamCycle.txt");

        file << GridWidth << " " << GridHeight << " ";

        for (int c : Cycle)
        {
            file << c << " ";
        }

        file.close();
    }

    float Map(float num, int from_min, int from_max, int to_min, int to_max)
    {
        return (num - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
    }
};

int main()
{
    int width = 0;
    int height = 0;

    while (width <= 1 || width > 80)
    {
        std::cout << "Enter width: ";
        std::cin >> width;

        if (width <= 0 && width > 80)
            std::cout << "Invalid Width: Must be between 2 and 80\n";
    }
    while (height <= 1 || height > 80)
    {
        std::cout << "\nEnter height: ";
        std::cin >> height;

        if (height <= 1 && height > 80)
            std::cout << "Invalid Height: Must be between 2 and 80\n";
    }

    CycleCreator c(width, height);
    if (c.Construct(800, 800, 1, 1))
        c.Start();
}