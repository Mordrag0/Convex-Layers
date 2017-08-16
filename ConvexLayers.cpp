#include "ConvexLayers.h"
#include <algorithm>
#include <tuple>

ConvexLayers::ConvexLayers()
{
}

ConvexLayers::~ConvexLayers()
{
}

void ConvexLayers::Construct()
{
	Sort();

	int Depth = TreeDepth((int)Points.size());
	// LowerRowCount je koliko listov moramo dodati, da lahko zapišemo Count bitov, èe imamo drevo globine Depth-1.
	int LowerRowCount = 0;
	if (Points.size() >= 2)
	{
		LowerRowCount = ((int)Points.size()) - (1 << (Depth - 1));
	}
	PointsToVertices(Points, Depth, LowerRowCount * 2);

	ComputeHullGraph(Depth, LowerRowCount, false);
	ComputeHullGraph(Depth, LowerRowCount, true);
}

void ConvexLayers::Sort()
{
	std::sort(Points.begin(), Points.end());
}

void ConvexLayers::ComputeHullGraph(int Depth, int LowerRowCount, bool bLower)
{
	// Naprej pretvorimo vector Points v Vertices
	std::vector<Vertex*>& Hull = bLower ? GLower : GUpper;
	
	std::vector<std::vector<Vertex*>> UpperHulls;
	std::vector<std::vector<Vertex*>> NewUpperHulls;
	// UpperHulls je na zaèetku seznam seznamov z 1 samim Vertexom.
	for (auto& V : Hull)
	{
		std::vector<Vertex*> VertexVec = { V };
		UpperHulls.push_back(VertexVec);
	}

	// Združuj upper hulle dokler ne ostane en sam za vse toèke.
	while (UpperHulls.size() > 1)
	{
		int TreePath = 0;
		Depth--;
		for (auto It = UpperHulls.begin(); It != UpperHulls.end(); std::advance(It, 2))
		{
			std::vector<Vertex*> JoinedUpperHulls = AddTangent(*It, *(It + 1), TTreeNode(TreePath++, Depth)); // Združenje dveh Upper hullov.
			NewUpperHulls.push_back(JoinedUpperHulls);
			if (LowerRowCount > 0)
			{
				LowerRowCount--;
			}
			if (LowerRowCount == 0)
			{
				NewUpperHulls.insert(NewUpperHulls.end(), It + 2, UpperHulls.end());
				It = UpperHulls.end();
				LowerRowCount = -1;
				break;
			}
		}

		UpperHulls = NewUpperHulls;
		NewUpperHulls.clear();
	}
}

std::vector<Vertex*> ConvexLayers::AddTangent(std::vector<Vertex*> U, std::vector<Vertex*> V, TTreeNode TreeIdx)
{
	Vertex* Ui = U[0];
	Vertex* Vj = V[0];

	// Poišèe vertexa, ki se bosta povezala.
	while (true)
	{
		double Gama = Ui->Loc.Slope(Vj->Loc);

		if ((Vj->RightTopEdge) && (Vj->RightTopEdge->Slope > Gama))
		{
			Vj = Vj->RightTopEdge->Vtx;
		}
		else if ((Ui->RightTopEdge) && (Ui->RightTopEdge->Slope >= Gama))
		{
			Ui = Ui->RightTopEdge->Vtx;
		}
		else
		{
			break;
		}
	}
	Ui->Connect(Vj, TreeIdx);

	// Združi podgrafa.
	U.insert(U.end(), V.begin(), V.end());
	return U;
}

void ConvexLayers::PointsToVertices(std::vector<Point> Points, int Depth, int LowerRowCount)
{
	GUpper.clear();
	GLower.clear();
	int Idx = 0;
	for (const auto& Pt : Points)
	{
		unsigned int PointPath;
		int PointDepth;
		if (Idx < LowerRowCount)
		{
			PointPath = Idx;
			PointDepth = Depth;
		}
		else
		{
			PointPath = ((1 << (Depth - 1)) - 1) - (Points.size() - 1 - Idx);
			PointDepth = Depth - 1;
		}
		TTreeNode TreeIdx(PointPath, PointDepth);
		Vertex* V1 = new Vertex(Pt, TreeIdx);
		Vertex* V2 = new Vertex(Point(Pt.X, -Pt.Y), TreeIdx);
		V1->OtherSide = V2;
		V2->OtherSide = V1;
		GUpper.push_back(V1);
		GLower.push_back(V2);
		++Idx;
	}
}

std::vector<Point> ConvexLayers::ConvexLayer()
{
	// Ker so Vertexi sortirani je prvi vedno na upperhullu.
	Vertex* UVtx = GUpper[0];
	Vertex* LVtx = GLower[0];

	std::vector<Vertex*> UpperHull = { UVtx };
	std::vector<Vertex*> LowerHull = { LVtx };
	// Poišèe zgornjo in spodnjo lupino.
	while (UVtx->RightTopEdge)
	{
		UVtx = UVtx->RightTopEdge->Vtx;
		UpperHull.push_back(UVtx);
	}
	while (LVtx->RightTopEdge)
	{
		LVtx = LVtx->RightTopEdge->Vtx;
		LowerHull.push_back(LVtx);
	}
	if (UpperHull.size() > 1) // Èe še je ostala samo ena toèka potem je brisanje nepotrebno.
	{
		DirectDeleteHull(UpperHull);
		DirectDeleteHull(LowerHull);
		CrossDeleteHull(LowerHull);
		CrossDeleteHull(UpperHull);
	}

	// Odstrani toèke iz G in jih izbriši.
	std::vector<Point> UpperLayer;
	std::vector<Point> LowerLayer;
	auto UpperVtxIt = GUpper.begin();
	auto LowerVtxIt = GLower.begin();
	auto UpperHullIt = UpperHull.begin();
	auto LowerHullIt = LowerHull.begin();

	while ((UpperVtxIt != GUpper.end()) && (LowerVtxIt != GLower.end()) && (UpperHullIt != UpperHull.end()) && (LowerHullIt != LowerHull.end()))
	{
		bool bOnUpper = *UpperVtxIt == *UpperHullIt;
		bool bOnLower = *LowerVtxIt == *LowerHullIt;

		if (bOnUpper || bOnLower)
		{
			if (bOnUpper)
			{
				++UpperHullIt;
				// Dodaj toèko v trenutno konveksno lupino.
				UpperLayer.push_back((*UpperVtxIt)->Loc);
			}
			if (bOnLower)
			{
				++LowerHullIt;
				if (!bOnUpper)
				{
					// Dodaj toèko v trenutno konveksno lupino.
					LowerLayer.push_back((*UpperVtxIt)->Loc);
				}
			}

			delete *UpperVtxIt;
			delete *LowerVtxIt;
			UpperVtxIt = GUpper.erase(UpperVtxIt);
			LowerVtxIt = GLower.erase(LowerVtxIt);
		}
		else
		{
			++UpperVtxIt;
			++LowerVtxIt;
		}
	}

	// Združi zgornjo in spodnjo lupino in vrni celo konveksno lupino.
	std::reverse(LowerLayer.begin(), LowerLayer.end());
	UpperLayer.insert(UpperLayer.end(), LowerLayer.begin(), LowerLayer.end());
	return UpperLayer;
}

std::vector<std::vector<Point>> ConvexLayers::GetConvexLayers(std::vector<Point> CPoints)
{
	Points = CPoints;
	Construct();
	std::vector<std::vector<Point>> Layers;
	int i = 0;
	while (GUpper.size() > 0)
	{
		Layers.push_back(ConvexLayer());
	}

	return Layers;
}

std::vector<Vertex*> ConvexLayers::GetHullGraph(std::vector<Point> CPoints)
{
	Points = CPoints;
	Construct();
	return GUpper;
	/*for (auto& P : GLower)
	{
		P->Loc.Y += 800;
	}
	return GLower;*/
}

void ConvexLayers::ClearHullGraph(std::vector<Vertex*> HG)
{
	GUpper = HG;
	while (GUpper.size() > 0)
	{
		ConvexLayer();
	}
}

void ConvexLayers::DirectDeleteHull(std::vector<Vertex*> Hull)
{
	for (Vertex* Vtx : Hull)
	{
		DirectDelete(Vtx);
	}
}
void ConvexLayers::CrossDeleteHull(std::vector<Vertex*> Hull)
{
	// Prve in zadnje toèke na zgornji in spodnji lupini ne rabimo križno izbrisat, ker smo jih že direktno izbrisali.
	if (Hull.size() > 2)
	{
		for (auto It = Hull.begin() + 1; It != Hull.end() - 1; It++)
		{
			CrossDelete((*It)->OtherSide);
		}
	}
}

void ConvexLayers::DirectDelete(Vertex* P)
{
	std::vector<Node*> Merged = P->MergeLists();
	Vertex* A = nullptr;
	Vertex* B = nullptr;
	Vertex* C = nullptr;

	std::vector<std::tuple<Vertex*, TTreeNode>> TopEdge;
	for (auto Edge : Merged)
	{
		TopEdge.push_back(std::make_tuple(Edge->Vtx, Edge->TreeIdx)); // Shrani Vtx in TreeIdx, ker bo edge v RemoveVertexOnHull izbrisan
	}
	P->RemoveVertexOnHull();
	for (auto Edge : TopEdge) // Dokler so povezave, ki jih je treba zbrisat.
	{
		C = std::get<0>(Edge);
		bool bFormPQ = P->Loc < C->Loc;

		Wrap(P, A, B, C, std::get<1>(Edge), bFormPQ); // Add connection.
		if (bFormPQ)
		{
			B = C;
		}
		else
		{
			A = C;
		}
	}
}

void ConvexLayers::CrossDelete(Vertex* P)
{
	bool bFormPQ = P->LeftTopEdge == nullptr; // Vemo, da ima P povezave samo na eni strani, torej je druga stran nullptr.
	Node* TopEdge = bFormPQ ? P->RightTopEdge : P->LeftTopEdge; // To bo povezava kjer zaènemo (od spodaj navzgor).
	if (!TopEdge)
	{
		return;
	}
	while (TopEdge->Next) // Gremo do zadnjega.
	{
		TopEdge = TopEdge->Next;
	}
	Vertex* AB = nullptr; // CrossDelete je isto kot delete, ampak ima samo A ali B, nikoli oboje.
	Vertex* C = nullptr;

	Node* RealTopEdge = nullptr; // Shranjene povezave od C pred P za posebni primer.
	while (TopEdge) // Dokler so povezave, ki jih je treba zbrisat.
	{
		C = TopEdge->Vtx;
		bool bSpecialCase = !TopEdge->Previous 
			&& !((C->LeftTopEdge && C->LeftTopEdge->Vtx == P) || (C->RightTopEdge && C->RightTopEdge->Vtx == P)); // P ni TopEdge od C;
		if (bSpecialCase) // Poišèi edge, ki ga je potrebno zbrisat in ga shrani.
		{
			Node* FakeTopEdge = bFormPQ ? C->LeftTopEdge : C->RightTopEdge;
			while(FakeTopEdge->Vtx != P)
			{
				FakeTopEdge = FakeTopEdge->Next;
			}
			RealTopEdge = FakeTopEdge->Previous;
			FakeTopEdge->Previous = nullptr;
			RealTopEdge->Next = nullptr;
			if (bFormPQ) // TopEdge v C naj kaže na P, tako da se lahko izvede Wrap enako kot v DirectDelete, ampak je potem treba nazaj dat.
			{
				C->LeftTopEdge = FakeTopEdge;
			}
			else 
			{
				C->RightTopEdge = FakeTopEdge;
			}
		}
		TTreeNode TreeIdx = TopEdge->TreeIdx; // Index povezave, ki se bo spremenila.
		TopEdge = TopEdge->Previous; // Premakni TopEdge.
		if (bFormPQ)
		{
			Node* NodeToDelete = C->LeftTopEdge;
			C->LeftTopEdge = C->LeftTopEdge->Next;
			if (C->LeftTopEdge)
			{
				C->LeftTopEdge->Previous = nullptr;
			}
			delete NodeToDelete;
			Wrap(P, nullptr, AB, C, TreeIdx, bFormPQ); // Add connection.
		}
		else
		{
			Node* NodeToDelete = C->RightTopEdge;
			C->RightTopEdge = C->RightTopEdge->Next;
			if (C->RightTopEdge)
			{
				C->RightTopEdge->Previous = nullptr;
			}
			delete NodeToDelete;
			Wrap(P, AB, nullptr, C, TreeIdx, bFormPQ); // Add connection.
		}
		AB = C;

		if (bSpecialCase && RealTopEdge) // Poveži real top edge pa current top edge.
		{
			Node* TTopEdge = bFormPQ ? C->LeftTopEdge : C->RightTopEdge;
			if (TTopEdge)
			{
				RealTopEdge->Next = TTopEdge;
				TTopEdge->Previous = RealTopEdge;
				while (RealTopEdge->Previous)
				{
					RealTopEdge = RealTopEdge->Previous;
				}
				if (bFormPQ) // Poveži real top edge pa current top edge.
				{
					C->LeftTopEdge = RealTopEdge;
				}
				else
				{
					C->RightTopEdge = RealTopEdge;
				}
			}
			else // C je trenutno brez povezave.
			{
				while (RealTopEdge->Previous)
				{
					RealTopEdge = RealTopEdge->Previous;
				}
				if (bFormPQ)
				{
					C->LeftTopEdge = RealTopEdge;
				}
				else
				{
					C->RightTopEdge = RealTopEdge;
				}
			}
		}
	}
	// Izbris vseh povezav od tega do ostalih Vertex-ov.
	if (bFormPQ)
	{
		while (P->RightTopEdge)
		{
			Node* NodeToDelete = P->RightTopEdge;
			P->RightTopEdge = P->RightTopEdge->Next;
			delete NodeToDelete;
		}
	}
	else
	{
		while (P->LeftTopEdge)
		{
			Node* NodeToDelete = P->LeftTopEdge;
			P->LeftTopEdge = P->LeftTopEdge->Next;
			delete NodeToDelete;
		}
	}
}

double ConvexLayers::IntersectionY(Point PY, Point PD1, Point PD2) const
{
	double X3 = PD1.X;
	double X4 = PD2.X;
	if (std::abs(X3 - X4) < std::numeric_limits<double>::epsilon()*10000)
	{
		return -std::numeric_limits<double>::max();
	}

	double X1 = PY.X;
	double Y1 = PY.Y;
	double X2 = PY.X;
	double Y2 = PY.Y - 1;
	double Y3 = PD1.Y;
	double Y4 = PD2.Y;

	return ((X1 * Y2 - Y1 * X2) * (Y3 - Y4) - (Y1 - Y2) * (X3 * Y4 - Y3 * X4)) 
		 / (-(Y1 - Y2) * (X3 - X4));
}

void ConvexLayers::Wrap(Vertex* P, Vertex* A, Vertex* B, Vertex* C, TTreeNode TreeIdx, bool bFormPQ)
{
	double AY, BY, CY, DY, EY;
	while (true)
	{
		if (A && A->RightTopEdge)
		{
			Vertex* A2 = A->RightTopEdge->Vtx;
			AY = IntersectionY(P->Loc, A->Loc, A2->Loc);
		}
		else
		{
			AY = -std::numeric_limits<double>::infinity();
		}
		if (B && B->LeftTopEdge)
		{
			Vertex* B2 = B->LeftTopEdge->Vtx;
			BY = IntersectionY(P->Loc, B->Loc, B2->Loc);
		}
		else
		{
			BY = -std::numeric_limits<double>::infinity();
		}
		if (C && (bFormPQ && C->LeftTopEdge))
		{
			Vertex* C2 = C->LeftTopEdge->Vtx;
			CY = IntersectionY(P->Loc, C->Loc, C2->Loc);
		}
		else if (C && (!bFormPQ && C->RightTopEdge))
		{
			Vertex* C2 = C->RightTopEdge->Vtx;
			CY = IntersectionY(P->Loc, C->Loc, C2->Loc);
		}
		else
		{
			CY = -std::numeric_limits<double>::infinity();
		}
		if (A && C)
		{
			DY = IntersectionY(P->Loc, A->Loc, C->Loc);
		}
		else
		{
			DY = -std::numeric_limits<double>::infinity();
		}
		if (B && C)
		{
			EY = IntersectionY(P->Loc, B->Loc, C->Loc);
		}
		else
		{
			EY = -std::numeric_limits<double>::infinity();
		}

		double Arr[5] = { AY, BY, CY, DY, EY };

		double* Max = std::max_element(Arr, Arr + 5);
		if (*Max == -std::numeric_limits<double>::infinity())
		{
			break; // Smo konèali.
		}

		int Distance = std::distance(Arr, Max);
		if (Distance == 3)
		{
			C->Connect(A, TreeIdx);
			break; // Konèni primer.
		}
		else if (Distance == 4)
		{
			C->Connect(B, TreeIdx);
			break; // Konèni primer.
		}
		else if (Distance == 2)
		{
			if (bFormPQ)
			{
				C = C->LeftTopEdge->Vtx;
			}
			else
			{
				C = C->RightTopEdge->Vtx;
			}
		}
		else if (Distance == 1)
		{
			B = B->LeftTopEdge->Vtx;
		}
		else // Distance == 0
		{
			A = A->RightTopEdge->Vtx;
		}
	}
}

int ConvexLayers::TreeDepth(int count) const
{
	int Count = (int)Points.size();
	int Depth = 0; // Število bitov potrebnih za zapis Count razliènih števil
	int Bits = 0;
	while (Count > 0)
	{
		Bits += Count & 1;
		Count >>= 1;
		Depth++;
	}
	if (Bits == 1) // Èe je velikiost 2^n, potrebujemo n-1 bitov
	{
		Depth--;
	}
	return Depth;
}

// Vertex

Vertex::Vertex(Point P, TTreeNode TreeIdx) :
	LeftTopEdge(nullptr),
	RightTopEdge(nullptr),
	Loc(P),
	TreeIdx(TreeIdx)
{
}

Vertex::~Vertex()
{
}

void Vertex::Connect(Vertex* Other, TTreeNode TreeIdx)
{
	double Slope = Loc.Slope(Other->Loc);
	Vertex* Left;
	Vertex* Right;

	if(Loc < Other->Loc)
	{
		Left = this;
		Right = Other;
	}
	else 
	{
		Left = Other;
		Right = this;
	}

	Node* NewTopNodeLeft = new Node(Right, Slope, TreeIdx);
	if (!Left->RightTopEdge) 
	{
		Left->RightTopEdge = NewTopNodeLeft;
	}
	else
	{
		Left->RightTopEdge->Previous = NewTopNodeLeft;
		NewTopNodeLeft->Next = Left->RightTopEdge;
		Left->RightTopEdge = NewTopNodeLeft;
	}
	Node* NewTopNodeRight = new Node(Left, Slope, TreeIdx);
	if (!Right->LeftTopEdge)
	{
		Right->LeftTopEdge = NewTopNodeRight;
	}
	else
	{
		Right->LeftTopEdge->Previous = NewTopNodeRight;
		NewTopNodeRight->Next = Right->LeftTopEdge;
		Right->LeftTopEdge = NewTopNodeRight;
	}
}

void Vertex::RemoveVertexOnHull()
{
	auto TopEdge = LeftTopEdge;
	while (TopEdge) // Izbriši vse povezave na PTopEdge
	{
		Node* NodeToDelete = TopEdge->Vtx->RightTopEdge; // Povezava, ki kaže na this Vertex in se bo izbrisala.
		TopEdge->Vtx->RightTopEdge = TopEdge->Vtx->RightTopEdge->Next; // Sosednji Vertex zdaj kaže na naslednjega, èe ga ima.
		if (TopEdge->Vtx->RightTopEdge)
		{
			TopEdge->Vtx->RightTopEdge->Previous = nullptr; // Èe obstaja ta Vertex je novi TopEdge, zato je previous nullptr.
		}
		TopEdge = TopEdge->Next; // Naslednji sosed od this.
		delete NodeToDelete; // Izbrišemo povezavo.
	}
	// Zdaj na levi strani noben Vertex nima veè povezave na this, zato lahko vse leve povezave izbrišemo.
	while (LeftTopEdge)
	{
		Node* Next = LeftTopEdge->Next;
		delete LeftTopEdge;
		LeftTopEdge = Next;
	}
	// Ponovimo vse še za drugo stran.
	TopEdge = RightTopEdge;
	while (TopEdge)
	{
		Node* NodeToDelete = TopEdge->Vtx->LeftTopEdge;
		TopEdge->Vtx->LeftTopEdge = TopEdge->Vtx->LeftTopEdge->Next;
		if (TopEdge->Vtx->LeftTopEdge)
		{
			TopEdge->Vtx->LeftTopEdge->Previous = nullptr;
		}
		TopEdge = TopEdge->Next;
		delete NodeToDelete; // Izbrišemo povezavo.
	}
	while (RightTopEdge)
	{
		Node* Next = RightTopEdge->Next;
		delete RightTopEdge;
		RightTopEdge = Next;
	}
}

std::vector<Node*> Vertex::MergeLists() const
{
	Node* Left = LeftTopEdge;
	Node* Right = RightTopEdge;
	std::vector<Node*> MergedTopEdge;

	while (Left || Right)
	{
		// Èe je desna stran prazna ali pa ni nobena prazna in ima leva prednost
		if (Left && (!Right || !LeftPrecedesRight(Left, Right)))
		{
			MergedTopEdge.push_back(Left);
			Left = Left->Next;
		}
		// Èe je leva stran prazna ali pa ni nobena prazna in ima desna prednost
		else
		{
			MergedTopEdge.push_back(Right);
			Right = Right->Next;
		}
	}
	std::reverse(MergedTopEdge.begin(), MergedTopEdge.end());
	return MergedTopEdge;
}
