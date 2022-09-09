#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <iterator>
#include <fstream>
#include <string>
using std::vector;
using std::ifstream;
using std::string;

static class HamiltonianCycle
{
public:
	//This is the most basic cycle possible as it simply goes left to right
	//Only works for when Width and Height are both even
	static vector<int> HardCodedCycle(int Width, int Height)
	{
		vector<int> cycle;
		cycle.push_back(0);

		if (Height % 2 == 0)
		{
			for (int i = 1; i < Width; ++i)
				cycle.push_back(i);
		}

		for (int i = 1; i < Height; ++i)
		{
			if (i % 2 == 1)
			{
				for (int j = Width - 1; j > 0; --j)
					cycle.push_back(i * Width + j);
			}
			else
			{
				for (int j = 1; j < Width; ++j)
					cycle.push_back(i * Width + j);
			}
		}

		for (int i = Height - 1; i > 0; --i)
		{
			cycle.push_back(i * Width);
		}
		return cycle;
	}

	//Reads a cycle from a file
	//The first two inputs contain the width and height of the grid. The cycle is everything afterwards.
	static vector<int> CycleFromFile(string filename, int & Width, int & Height)
	{
		std::ifstream t(filename);
		std::stringstream buffer;
		buffer << t.rdbuf();


		std::istream_iterator<string> begin(buffer);
		std::istream_iterator<string> end;
		vector<std::string> vstrings(begin, end);

		//Grab the width and height and then remove them from the vector
		Width = stoi(vstrings[0]);
		Height = stoi(vstrings[1]);
		vstrings.erase(vstrings.begin(), vstrings.begin() + 2);

		vector<int> ret;
		for (string str : vstrings)
		{
			ret.push_back(stoi(str));
		}

		return ret;
	}
};