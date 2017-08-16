#pragma once

#include <vector>

#include "Point.h"
#include "TreeElement.h"

struct Vertex;
// Dvojno povezan seznam - povezave med toèkami v G in vozlišèa v T.
struct Node
{
public:
	// Vertex na katerega kaže ta Node. 
	Vertex* Vtx;
	Node* Next;
	Node* Previous;
	// Naklon med lastnikom tega Node in Vertexom Vtx. 
	double Slope;
	// Binarna koda za Node. 
	TTreeNode TreeIdx;

	Node(Vertex* Vtx, double Slope, TTreeNode TreeIdx) :
		Vtx(Vtx),
		Slope(Slope),
		Next(nullptr),
		Previous(nullptr),
		TreeIdx(TreeIdx)
	{
	}

	Node(const Node& N) = default;
	Node(Node&& N) = default;
	Node& operator=(const Node&) = default;
};

// Toèke grafa G. 
struct Vertex
{
	Point Loc;
	Node* LeftTopEdge;
	Node* RightTopEdge;

	Vertex(Point P, TTreeNode);
	Vertex(const Vertex& V) = delete;
	Vertex(Vertex&& V) = delete;
	Vertex& operator=(const Vertex&) = delete;
	~Vertex();

	// Povezava dveh Vertexov tako, da se drug k drugemu vstavita v seznam sosedov.
	void Connect(Vertex* Other, TTreeNode TreeIdx);
	void RemoveVertexOnHull();

	TTreeNode TreeIdx;
	// Kazalec na isti Vertex v drugem grafu (G ali G'). 
	Vertex* OtherSide;

	std::vector<Node*> MergeLists() const;

	// Pove kateri Node ima prednost pri združevanju seznamov. 
	bool LeftPrecedesRight(Node* Left, Node* Right) const
	{
		return Left && (!Right || (TreeIdx.Compare(Left->TreeIdx) > TreeIdx.Compare(Right->TreeIdx)));
	}
};

class ConvexLayers
{
public:
	ConvexLayers();
	virtual ~ConvexLayers();

	std::vector<std::vector<Point>> GetConvexLayers(std::vector<Point> Points);

	std::vector<Vertex*> GetHullGraph(std::vector<Point> CPoints);
	// Ta funkcija se mora klicati vsakiè po GetHullGraph
	void ClearHullGraph(std::vector<Vertex*> HG);

private:
	std::vector<Point> Points;
	std::vector<Vertex*> GUpper;
	std::vector<Vertex*> GLower;

	std::vector<Point> ConvexLayer();

	// Izvedba korakov algoritma za konstrukcijo vgnezdenih konveksnih lupin. 
	void Construct();

	void Sort();
	// Izraèun grafa G, rezultat se shrani v polje Vertices. 
	void ComputeHullGraph(int Depth, int LowerRowCount, bool bLower);
	// Izraèun tangente med dvema upper hull-oma, ki morata biti sortirana po X. 
	std::vector<Vertex*> AddTangent(std::vector<Vertex*> U, std::vector<Vertex*> V, TTreeNode TreeIdx);
	// Pretvorba iz vektorja Pointov v vektor Vertexov. 
	void PointsToVertices(std::vector<Point> Points, int Depth, int LowerRowCount);

	void DirectDeleteHull(std::vector<Vertex*> Hull);
	void CrossDeleteHull(std::vector<Vertex*> Hull);
	void DirectDelete(Vertex* P);
	void CrossDelete(Vertex* P);

	double IntersectionY(Point PY, Point PD1, Point PD2) const;
	void Wrap(Vertex* P, Vertex* A, Vertex* B, Vertex* C, TTreeNode TreeIdx, bool bFormPQ);

	int TreeDepth(int count) const;
};