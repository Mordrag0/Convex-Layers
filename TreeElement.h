#pragma once
#include <iostream>

// Razred za vozli��a drevesa T.
class TTreeNode
{
public:
	TTreeNode(unsigned int Path, int Depth);
	~TTreeNode();

	/** Vrne kolik�no pot imata dva elemnta v drevesu skupno. */
	int Compare(const TTreeNode& Other) const;

	unsigned int GetPath() const { return Path; }
	int GetDepth() const { return Depth; }

	std::string ToString() const;

private:
	// Predstavlja pot od korena drevesa proti listom - 0 pomeni levo, 1 pa desno.
	unsigned int Path;
	// Kako globoko v drevesu se element nahaja - uporabno, da vemo pri katerem bitu za�eti brati Path.
	int Depth;

	int ToInt(int Size);
};

