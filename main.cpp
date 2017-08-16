#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <iterator>
#include <memory>
#include <fstream>

#include "ConvexLayers.h"
#include "TreeElement.h"
#include "QuickHull.h"
#include "Jarvis.h"
#include "Graham.h"

#include <chrono>
#include <ctime>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;
const float POINT_WIDTH = 5;

std::vector<Point> GeneratePoints(int Count);

void Save(std::string Filepath, std::vector<Point> Points);
std::vector<Point> Load(std::string Filepath);

void DrawLine(sf::RenderWindow& Window, Point A, Point B)
{
	const sf::Color LineColor(0, 0, 255);
	sf::Vertex Line[] =
	{
		sf::Vertex(sf::Vector2f((float)(A.X + (POINT_WIDTH - 1) / 2), (float)(WINDOW_HEIGHT - A.Y + (POINT_WIDTH - 1) / 2)), LineColor),
		sf::Vertex(sf::Vector2f((float)(B.X + (POINT_WIDTH - 1) / 2), (float)(WINDOW_HEIGHT - B.Y + (POINT_WIDTH - 1) / 2)), LineColor)
	};

	Window.draw(Line, 2, sf::Lines);
}

void DrawText(sf::RenderWindow& Window, std::string Str, sf::Font font, int Size, sf::Color Color, float X, float Y)
{
	sf::Text Text(Str, font, Size);
	Text.setStyle(sf::Text::Bold);
	Text.setColor(Color);
	Text.setPosition((float)(X), (float)WINDOW_HEIGHT - Y);
	Window.draw(Text);
}

void DrawHullGraph(sf::RenderWindow& Window, sf::Font Font, std::vector<Vertex*> HG, bool bUpper = true)
{
	sf::RectangleShape background(sf::Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
	background.setFillColor(sf::Color(238, 238, 238));
	sf::CircleShape Shape((float)POINT_WIDTH);
	Shape.setFillColor(sf::Color(100, 250, 50));

	Window.clear();
	Window.draw(background);

	std::vector<Vertex*> Vertices = HG;
	for (const auto& V : Vertices)
	{
		double SideMultiplier = bUpper ? 1 : -1;
		double X = V->Loc.X;
		double Y = V->Loc.Y * SideMultiplier;
		
		for (Node* RightEdge = V->RightTopEdge; RightEdge; RightEdge = RightEdge->Next)
		{
			double X2 = RightEdge->Vtx->Loc.X;
			double Y2 = RightEdge->Vtx->Loc.Y * SideMultiplier;
			DrawLine(Window, V->Loc, RightEdge->Vtx->Loc);
			DrawText(Window, RightEdge->TreeIdx.ToString(), Font, 24, sf::Color::Red, (float)(X + X2) / 2, (float)(Y + Y2) / 2);

		}
		DrawText(Window, V->TreeIdx.ToString(), Font, 14, sf::Color::Black, (float)(X), (float)(Y));
		Shape.setPosition((float)(V->Loc.X - POINT_WIDTH), (float)(WINDOW_HEIGHT - V->Loc.Y * SideMultiplier - POINT_WIDTH));
		Window.draw(Shape);
	}

	Window.display();
}
void RefreshScreen(sf::RenderWindow& Window, std::vector<Point> Points)
{
	sf::RectangleShape Background(sf::Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
	Background.setFillColor(sf::Color(238, 238, 238));
	sf::CircleShape Shape(POINT_WIDTH);
	Shape.setFillColor(sf::Color(0, 0, 0));

	Window.clear();
	Window.draw(Background);
	for (const auto& P : Points)
	{
		Shape.setPosition((float)(P.X - POINT_WIDTH / 2), (float)(WINDOW_HEIGHT - P.Y - POINT_WIDTH / 2));
		Window.draw(Shape);
	}
	Window.display();
}

void DrawConvexLayers(sf::RenderWindow& Window, std::vector<std::vector<Point>> Layers)
{
	sf::RectangleShape Background(sf::Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
	Background.setFillColor(sf::Color(238, 238, 238));
	sf::CircleShape Shape(POINT_WIDTH);
	Shape.setFillColor(sf::Color(0, 0, 0));

	Window.clear();
	Window.draw(Background);

	for (const auto& Layer : Layers)
	{
		DrawLine(Window, *Layer.begin(), *(Layer.end() - 1));
		for (auto It = Layer.begin(); It != Layer.end() - 1; ++It) // Nariši daljice.
		{
			DrawLine(Window, *It, *(It + 1));
		}
		for (auto It = Layer.begin(); It != Layer.end(); ++It) // Nariši toèke.
		{
			Shape.setPosition((float)(It->X - POINT_WIDTH / 2), (float)(WINDOW_HEIGHT - It->Y - POINT_WIDTH / 2));
			Window.draw(Shape);
		}
	}
	Window.display();
}

int main()
{
	int Count = 100;
	std::vector<Point> Points = GeneratePoints(Count); 
	sf::ContextSettings settings(0U, 0U, 8);
	sf::RenderWindow Window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Vgnezdene izbocene lupine", sf::Style::Default, settings);
	RefreshScreen(Window, Points);

	auto CL = std::unique_ptr<ConvexLayers>(new ConvexLayers());
	auto QH = std::unique_ptr<QuickHull>(new QuickHull());
	auto J = std::unique_ptr<Jarvis>(new Jarvis());
	auto G = std::unique_ptr<Graham>(new Graham());

	srand(time(NULL)); // randomize seed

	sf::Font Font;
	bool bFontLoaded = !Font.loadFromFile("Font.ttf");
	if (bFontLoaded)
	{
		std::cout << "Error loading 'Font.ttf'." << std::endl;
		std::cout << "Font is required to display the values in the hull graph." << std::endl;
	}

	std::cout << "F - Generate new points" << std::endl;
	std::cout << "G - Clear Points" << std::endl;
	std::cout << "Y - Chazelle" << std::endl;
	std::cout << "X - Jarvis march" << std::endl;
	std::cout << "C - Graham scan" << std::endl;
	std::cout << "V - Quickhull" << std::endl;
	std::cout << "W - Draw hull graph" << std::endl;
	std::cout << "D - Set new number of points" << std::endl;
	while (Window.isOpen())
	{
		sf::Event event;
		while (Window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				Window.close();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Escape: // Zapri program.
						std::cout << "Escape" << std::endl;
						Window.close();
						break;
					case sf::Keyboard::D:
						std::cout << "D" << std::endl;
						std::cout << "Number of poins: ";
						std::cin >> Count;
						// Fall through
					case sf::Keyboard::F: // Generiraj nove toèke.
						std::cout << "F" << std::endl;
						Points = GeneratePoints(Count);
						RefreshScreen(Window, Points);
						Save("Points.txt", Points);
						std::cout << CL->GetConvexLayers(Points).size() << std::endl;
						break;
					case sf::Keyboard::G: 
						std::cout << "G" << std::endl;
						Points = GeneratePoints(0);
						RefreshScreen(Window, Points);
						break;
					case sf::Keyboard::O: // Shrani toèke v datoteko.
						std::cout << "SAVE" << std::endl;
						Save("Points.txt", Points);
						break;
					case sf::Keyboard::P: // Naloži toèke iz datoteke.
						std::cout << "LOAD" << std::endl;
						Points = Load("Points.txt");
						RefreshScreen(Window, Points);
						break;
					case sf::Keyboard::Y:
					{
						std::cout << "CHAZELLE" << std::endl;
						auto begin = clock();
						auto Layers = CL->GetConvexLayers(Points);
						auto end = clock();
						std::cout << (double(end - begin) / CLOCKS_PER_SEC) << " s" << std::endl;
						DrawConvexLayers(Window, Layers);
						break;
					}
					case sf::Keyboard::X:
					{
						std::cout << "JARVIS" << std::endl;
						auto begin = clock();
						auto Layers = J->GetConvexLayers(Points);
						auto end = clock();
						std::cout << (double(end - begin) / CLOCKS_PER_SEC) << " s" << std::endl;
						DrawConvexLayers(Window, Layers);
						break;
					}
					case sf::Keyboard::C:
					{
						std::cout << "GRAHAM" << std::endl;
						auto begin = clock();
						auto Layers = G->GetConvexLayers(Points);
						auto end = clock();
						std::cout << (double(end - begin) / CLOCKS_PER_SEC) << " s" << std::endl;
						DrawConvexLayers(Window, Layers);
						break;
					}
					case sf::Keyboard::V:
					{
						std::cout << "QUICKHULL" << std::endl;
						auto begin = clock();
						auto Layers = QH->GetConvexLayers(Points);
						auto end = clock();
						std::cout << (double(end - begin) / CLOCKS_PER_SEC) << " s" << std::endl;
						DrawConvexLayers(Window, Layers);
						break;
					}
					case sf::Keyboard::W: // Nariši hull graph.
					{
						auto HG = CL->GetHullGraph(Points);
						DrawHullGraph(Window, Font, HG, true);
						CL->ClearHullGraph(HG);
						break;
					}
					default:
						break;
				}
			}
			else if (event.type == sf::Event::MouseButtonPressed) // Z LMB se dodajo nove toèke.
			{
				Points.push_back(Point((float)event.mouseButton.x, (float)(WINDOW_HEIGHT - event.mouseButton.y)));
				RefreshScreen(Window, Points);
			}
		}
	}

	return 0;
}

std::vector<Point> GeneratePoints(int Count)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::vector<Point> Points;

	std::uniform_real_distribution<> dis(0, 1);
	std::uniform_real_distribution<> dis2(0, 2 * std::_Pi);

	int Size = WINDOW_HEIGHT / 2 - 15;

	for (int i = 0; i < Count; i++)
	{
		double Rsqrt = std::sqrt(dis(gen));
		double Theta = dis2(gen);
		double X = Rsqrt * std::cos(Theta);
		double Y = Rsqrt * std::sin(Theta);
		Points.push_back(Point(X* Size + Size + 15, Y* Size + Size + 15));
	}

	return Points;
}

void Save(std::string Filepath, std::vector<Point> Points)
{
	std::ofstream File;
	File.open(Filepath);
	for (const auto &P : Points)
	{
		File << P.X << " " << P.Y << std::endl;
	}
	File.close();
}

std::vector<Point> Load(std::string Filepath)
{
	std::string Line;
	std::ifstream File(Filepath);
	std::vector<Point> Points;
	if (File.is_open())
	{
		while (std::getline(File, Line))
		{
			std::string::size_type sz;     // alias of size_t
			float X = std::stof(Line, &sz);
			float Y = std::stof(Line.substr(sz));
			Points.push_back(Point(X, Y));
		}
		File.close();
	}
	return Points;
}