/******************************************************************************
 Author: Thomas Eberhart (ThomasEEEEEEEE)
 Description: This program contains an AI snake game solver.
			  
			  The logic for this program was inspired by a Youtube video from 
			  the channel "CodeBullet". He has a set of two videos where 
			  he tackled the challenge of making a perfect Snake AI. His succeeding
			  attempt involving using an algorithm that utilizes a hamiltonian cycle, 
			  which is a fancy sounding word that simply refers to a graph that
			  covers every single node once and then loops on itself. This is the
			  easiest way to create a functioning snake AI, as the snake will
			  guarantee a win by simply following the cycle. However, as CodeBullet
			  elegantly put it, "This is ****ing boring!". So, instead, he used
			  an algorithm that used the hamintonian cycle as a base to build off of.
			  This algorithm involves taking shortcuts across the cycle when
			  the snake can get away with it. This is basically where CodeBullet's
			  explanation ended and I decided to try and recreate this algorithm
			  with no further explanation or examples.

			  I was unfortunately unsuccessful in completely replicating the
			  algorithm. I had managed to create a version that worked in
			  numerous cases. However, it was also succeptible to multiple bugs
			  as well as some edge cases. So, unfortunately, I had to look up
			  the algorithm to find out how to finish it off and clear up these
			  final issues. I am still quite happy with this program and am 
			  satisfied with how much I was able to do with basically no reference.
******************************************************************************/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "HamiltonianCycle.h"
using olc::Sprite;

//Contains 4 possibilities for the state of a tile
enum TileState
{
	EMPTY = 0,
	SNAKE,
	SNAKE_HEAD,
	APPLE
};

class SnakeBot : public olc::PixelGameEngine
{
public:
	int TileSize;
	int GridWidth;
	int GridHeight;
	TileState* Grid;
	double Timer;
	vector<int> HCycle;
	vector<int> OriginalCycle;
	int CurApplePos;
	int CurSnakePos;
	int CurSnakeTailPos;
	vector<int> Snake;
	int MovesPerSecond;
	bool Debug = false;
	bool DisableTimer = false;
	bool GameOver = false;
	string GameOverText = "";

	SnakeBot()
	{
		sAppName = "Snake AI";
	}
	
	bool OnUserCreate() override
	{
		MovesPerSecond = 3;
		Timer = 0;

		OriginalCycle = HamiltonianCycle::CycleFromFile("HamCycle.txt", GridWidth, GridHeight);

		TileSize = std::min(ScreenWidth(), ScreenHeight()) / GridWidth;

		Grid = new TileState[GridWidth * GridHeight];
		srand(time(0));

		for (int i = 0; i < GridWidth * GridHeight; ++i)
		{
			Grid[i] = EMPTY;
		}

		//Start the snake in a random position
		CurSnakePos = rand() % (GridWidth * GridHeight);
		Grid[CurSnakePos] = SNAKE_HEAD;
		CurSnakeTailPos = CurSnakePos;
		Snake.push_back(CurSnakePos);

		//Start the apple in a random position
		CurApplePos = rand() % (GridWidth * GridHeight);
		while (Grid[CurApplePos] != EMPTY)
			CurApplePos = rand() % (GridWidth * GridHeight);
		Grid[CurApplePos] = APPLE;

		//This code reorganizes the hamintonian cycle
		//The original cycle is ordered by the path of the cycle itself
		//For example: 79 may be the 40th element in the cycle and thus HCycle[40] == 79
		//This code changes that so that indexing by an element on the grid will give that place in the cycle i.e HCycle[79] == 40
		//This is primarily done so that the cycle and the Grid array are in sync.
		vector<int> cycle;
		cycle.resize(GridWidth * GridHeight);
		for (int i = 0; i < GridWidth * GridHeight; ++i)
		{
			cycle[OriginalCycle[i]] = i;
		}
		HCycle = cycle;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GameOver)
		{
			DrawString(ScreenWidth() / 10, ScreenHeight() / 4, GameOverText, olc::WHITE, 10);
			return true;
		}

		//Use the numpad + and - buttons to speed up or slow down the run
		if (GetKey(olc::NP_ADD).bPressed)
			MovesPerSecond += 3;
		else if (GetKey(olc::NP_SUB).bPressed && MovesPerSecond > 3)
			MovesPerSecond -= 3;

		//Reset the speed when backspace is pressed
		if (GetKey(olc::BACK).bPressed)
			MovesPerSecond = 3;

		if (GetKey(olc::D).bPressed)
			Debug = !Debug;

		if (GetKey(olc::TAB).bPressed)
			DisableTimer = !DisableTimer;

		Timer += fElapsedTime;

		if (DisableTimer || (!DisableTimer && Timer > (1.0 / (double)MovesPerSecond)))
		{
			Timer = 0;
			
			int move = GetSnakeBestMove();
			if (move == -1)
			{
				GameOver = true;
				GameOverText = "Game\nOver!";
			}
			else
				MoveSnake(move);
		}

		Clear(olc::BLACK);
		//Draw the empty spaces and the apple
		for (int x = 0; x < GridWidth; ++x)
		{
			for (int y = 0; y < GridHeight; ++y)
			{
				switch (Grid[y * GridWidth + x])
				{
				case APPLE:
					FillRect(x * TileSize, y * TileSize, TileSize, TileSize, olc::RED);
					break;
				case EMPTY:
				default:
					FillRect(x * TileSize, y * TileSize, TileSize, TileSize, olc::VERY_DARK_GREY);
					break;
				}
			}
		}

		DisplaySnake();
		if (Debug)
			DisplayDebugInfo();
		return true;
	}

	void MoveSnake(int move)
	{
		auto PlaceNewApple = [&]()
		{
			int EmptyTiles = GridWidth * GridHeight - Snake.size();
			int i = 0;
			int TileCount = 0;
			int RandIndex = EmptyTiles > 1 ? ((rand() % (EmptyTiles - 1)) + 1) : 1;

			while (TileCount < RandIndex)
			{
				if (Grid[i] == EMPTY)
					++TileCount;
				++i;
			}

			CurApplePos = i - 1;
			Grid[CurApplePos] = APPLE;
		};

		if (Snake.size() > 1)
			Grid[CurSnakePos] = SNAKE;
		Grid[move] = SNAKE_HEAD;
		Snake.push_back(move);
		CurSnakePos = move;

		//Eat the apple if the move is the apple position
		if (CurApplePos == move)
		{
			//Place the apple in a new random position
			
			/*CurApplePos = rand() % (GridWidth * GridHeight);
			while (Grid[CurApplePos] != EMPTY)
				CurApplePos = rand() % (GridWidth * GridHeight);
			Grid[CurApplePos] = APPLE;*/
			PlaceNewApple();
		}
		else
		{ //If the apple was not eaten then the tail must move forward
			Grid[CurSnakeTailPos] = EMPTY;
			Snake.erase(Snake.begin());
			CurSnakeTailPos = Snake.front();
		}

		if (Snake.size() == GridWidth * GridHeight - 1)
		{
			GameOver = true;
			GameOverText = "Winner!";
		}
	}

	void DisplayDebugInfo()
	{
		DisplayCycle();

		DrawString(5, 5, "Speed: " + (DisableTimer ? "Max" : std::to_string(MovesPerSecond / 3) + "x"), olc::WHITE, 1);
		DrawString(GetTextSize("Speed: Max").x + 20, 5, std::to_string((int)((Snake.size() + 1) / (double)(GridWidth * GridHeight) * 100)) + "% Covered", olc::WHITE, 1);
		DrawString(GetTextSize("Speed: Max100% Covered").x + 40, 5, "Dist to Apple: " + std::to_string(DistToPoint(CurSnakePos, CurApplePos)), olc::WHITE, 1);
	}

	//Displays the hamiltonian cycle using lines
	void DisplayCycle()
	{
		for (int i = 0; i < GridWidth * GridHeight; ++i)
		{
			int p = OriginalCycle[i];
			int pn;
			if (i == GridWidth * GridHeight - 1)
				pn = OriginalCycle[0];
			else
				pn = OriginalCycle[i + 1];

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
			else //Shouldn't happen with a correct cycle
				FillCircle(x, y, 3, olc::WHITE);
		}
	}

	//Displays the snake using a green color gradient.
	void DisplaySnake()
	{
		auto Map = [&](float num, int from_min, int from_max, int to_min, int to_max)
		{
			return (num - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
		};

		for (int i = 0; i < Snake.size(); ++i)
		{
			int s = Snake[i];
			int sn;
			if (i == Snake.size() - 1)
				sn = Snake[i];
			else
				sn = Snake[i + 1];

			int sx = s % GridWidth * TileSize;
			int sy = s / GridWidth * TileSize;

			olc::Pixel p = olc::Pixel(0, Map(i, 0, Snake.size(), 64, 228), 0);
			FillRect(s % GridWidth * TileSize, s / GridWidth * TileSize, TileSize, TileSize, p);

		}
	}

	//Main logic for moving the snake
	int GetSnakeBestMove()
	{
		int CurDistToApple = DistToPoint(CurSnakePos, CurApplePos);
		int CurDistToTail;
		if (CurSnakeTailPos == CurSnakePos) 
			CurDistToTail = GridWidth * GridHeight - 1;
		else
			CurDistToTail = DistToPoint(CurSnakePos, CurSnakeTailPos);

		int AvailCutAmt = CurDistToTail - 4;
		int EmptySquares = GridWidth * GridHeight - Snake.size() - 2;

		//Just follow the cycle once 75% of the board is covered
		if (EmptySquares < GridWidth * GridHeight / 4)
			AvailCutAmt = 0;
		else if (CurDistToApple < CurDistToTail)
		{
			AvailCutAmt -= 1;

			if ((CurDistToTail - CurDistToApple) * 4 > EmptySquares)
				AvailCutAmt -= 10;
		}

		if (CurDistToApple < AvailCutAmt)
			AvailCutAmt = CurDistToApple;

		if (AvailCutAmt < 0)
			AvailCutAmt = 0;

		int BestMove = -1;
		int BestDist = -1;
		int BackupMove = -1;

		//Left
		if (CurSnakePos % GridWidth != 0)
		{
			int dist = DistToPoint(CurSnakePos, CurSnakePos - 1);
			if (dist <= AvailCutAmt && dist > BestDist)
			{
				BestMove = CurSnakePos - 1;
				BestDist = dist;
			}
			else if (Grid[CurSnakePos - 1] == EMPTY || Grid[CurSnakePos - 1] == APPLE)
				BackupMove = CurSnakePos - 1;
		}
		//Right
		if ((CurSnakePos + 1) % GridWidth != 0)
		{
			int dist = DistToPoint(CurSnakePos, CurSnakePos + 1);
			if (dist <= AvailCutAmt && dist > BestDist)
			{
				BestMove = CurSnakePos + 1;
				BestDist = dist;
			}
			else if (Grid[CurSnakePos + 1] == EMPTY || Grid[CurSnakePos + 1] == APPLE)
				BackupMove = CurSnakePos + 1;
		}
		//Up
		if (CurSnakePos >= GridWidth)
		{
			int dist = DistToPoint(CurSnakePos, CurSnakePos - GridWidth);
			if (dist <= AvailCutAmt && dist > BestDist)
			{
				BestMove = CurSnakePos - GridWidth;
				BestDist = dist;
			}
			else if (Grid[CurSnakePos - GridWidth] == EMPTY || Grid[CurSnakePos - GridWidth] == APPLE)
				BackupMove = CurSnakePos - GridWidth;
		}
		//Down
		if (CurSnakePos < GridWidth * (GridHeight - 1))
		{
			int dist = DistToPoint(CurSnakePos, CurSnakePos + GridWidth);
			if (dist <= AvailCutAmt && dist > BestDist)
			{
				BestMove = CurSnakePos + GridWidth;
				BestDist = dist;
			}
			else if (Grid[CurSnakePos + GridWidth] == EMPTY || Grid[CurSnakePos + GridWidth] == APPLE)
				BackupMove = CurSnakePos + GridWidth;
		}

		if (BestDist >= 0)
			return BestMove;

		return BackupMove;
	}

	//Returns how far away 2 points are from each other on the hamiltonian cycle
	int DistToPoint(int Pos, int Point)
	{
		if (HCycle[Point] < HCycle[Pos])
			return (GridWidth * GridHeight) - HCycle[Pos] + HCycle[Point] - 1;
		else
			return HCycle[Point] - HCycle[Pos] - 1;
	}
};

int main()
{
	SnakeBot sb;
	if (sb.Construct(640, 640, 1, 1))
		sb.Start();
}