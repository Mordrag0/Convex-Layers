#include "TreeElement.h"
#include <algorithm>


TTreeNode::TTreeNode(unsigned int Path, int Depth) :
	Path(Path),
	Depth(Depth)
{
}

TTreeNode::~TTreeNode()
{
}

int TTreeNode::Compare(const TTreeNode& Other) const
{
	unsigned int  OtherPath = Other.Path;
	int OtherDepth = Other.Depth;

	// Manjša izmed obeh globin.
	int MinDepth = std::min(Depth, OtherDepth);
	int BitsInCommon = 0;
	for (int i = 0; i < MinDepth; i++)
	{
		int Bit = (Path >> (Depth - 1 - i)) & 1;
		int OtherBit = (OtherPath >> (OtherDepth - 1 - i)) & 1;
		if (Bit != OtherBit)
		{
			break;
		}
		BitsInCommon++;
	}
	return BitsInCommon;
}

std::string TTreeNode::ToString() const
{
	if (Depth == 0)
	{
		return "Root";
	}
	else if (Depth < 0)
	{
		return "UNDEFINED";
	}
	else
	{
		std::string Str(Depth, '0');
		for (int i = 0; i < Depth; i++)
		{
			int Bit = (Path >> (Depth - 1 - i)) & 1;
			if (Bit)
			{
				Str[i] = '1';
			}
		}
		return Str;
	}
}

int TTreeNode::ToInt(int Size)
{
	int LowerDepth = 0;
	--Size;
	int Size2 = Size;
	while (Size2 > 0)
	{
		Size2 >>= 1;
		LowerDepth++;
	}
	if (LowerDepth == Depth)
	{
		return Path;
	}
	else
	{
		return Size - ((1 << Depth) - 1 - Path);
	}
	return 0;
}
