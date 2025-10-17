#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

struct Point {
    double x;
    double y;
    auto operator<=>(const Point&) const = default;
};

static auto ParseFile(const std::filesystem::path& path) -> std::vector<Point>;

static auto CrossProduct(const Point& p, const Point& a, const Point& b);
static void OtoczkaHelper(std::vector<Point>& set, const Point& start, const Point& end, std::vector<Point>& result);
static auto Otoczka(std::vector<Point> points) -> std::vector<Point>;

static void Proste(std::vector<Point>& points);

static void NajblizszePunkty(std::vector<Point>& points);

auto main(int argc, const char** argv) -> int {
    std::string filename;
    if (argc > 1) {
        std::span args{ std::next(argv), static_cast<size_t>(argc - 1) };
        filename = args.front();
    }
    else {
        std::cout << "Nazwa pliku: ";
        std::cin >> filename;
    }
    auto points = ParseFile(filename);

    auto convexHullPoints = Otoczka(points);
    Proste(convexHullPoints);
    NajblizszePunkty(points);

    return 0;
}

/*************************   NAJBLIZSZE PUNKTY   ******************************/
/******************************************************************************/

static auto distance(const Point& p1, const Point& p2) -> double {
    return std::sqrt(((p2.x - p1.x) * (p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
}

static auto NajblizszePunktyHelper(std::vector<Point>& pointsX, std::vector<Point>& pointsY, size_t start, size_t end)
    -> std::pair<Point, Point> {

    // ilosc punktow
    size_t n = end - start;

    // dla malej ilosc punktow brute-force
    if (n <= 3) {
        double min = DBL_MAX;
        std::pair<Point, Point> closest;

        for (size_t i = start; i < end; ++i) {
            for (size_t j = i + 1; j < end; ++j) {
                double d = distance(pointsX[i], pointsX[j]);
                if (d < min) {
                    min = d;
                    closest = { pointsX[i], pointsX[j] };
                }
            }
        }
        return closest;
    }

    // srodkowy punkt
    size_t mid = start + (n / 2);

    // znajdowanie rekurencyjnie najblizszych punktow w obu czesciach
    std::pair<Point, Point> closestLeft = NajblizszePunktyHelper(pointsX, pointsY, start, mid);
    std::pair<Point, Point> closestRight = NajblizszePunktyHelper(pointsX, pointsY, mid, end);

    double leftDist = distance(closestLeft.first, closestLeft.second);
    double rightDist = distance(closestRight.first, closestRight.second);

    // wybieranie najblizszej pary z dwoch
    std::pair<Point, Point> closest = (leftDist < rightDist) ? closestLeft : closestRight;

    // porownywanie punktow blisko krawedzi przeciwnych stron
    // tylko w odleglosci mniejszej niz odleglosc dotychczas
    // najblizszej pary
    std::vector<Point> inbetween;
    double minDist = std::min(leftDist, rightDist);
    Point midPoint = pointsX[mid];

    for (const auto& p : pointsY) {
        if (std::abs(p.x - midPoint.x) < minDist) {
            inbetween.push_back(p);
        }
    }

    // brute-force zebranych punktow
    for (size_t i = 0; i < inbetween.size(); ++i) {
        for (size_t j = i + 1; j < inbetween.size() && (inbetween[j].y - inbetween[i].y) < minDist; ++j) {
            double dist = distance(inbetween[i], inbetween[j]);
            if (dist < minDist) {
                minDist = dist;
                closest = { inbetween[i], inbetween[j] };
            }
        }
    }

    return closest;
}

static void NajblizszePunkty(std::vector<Point>& points) {
    if (points.size() < 2) {
        return;
    }

    // sortowanie punktow po x
    std::vector<Point> pointsX = points;
    std::ranges::sort(pointsX, [](const Point& a, const Point& b) { return a.x < b.x; });

    // sortowanie punktow po y
    std::vector<Point> pointsY = points;
    std::ranges::sort(pointsY, [](const Point& a, const Point& b) { return a.y < b.y; });

    auto result = NajblizszePunktyHelper(pointsX, pointsY, 0, points.size());

    std::cout << "Najblizsze Punkty: [(" << result.first.x << ", " << result.first.y << "),(";
    std::cout << result.second.x << ", " << result.second.y << ")] d=";
    std::cout << distance(result.first, result.second);
}

/*******************************   OTOCZKA   **********************************/
/******************************************************************************/

// jak daleko i po ktorej stronie jest punkt p wzgledem linii ab
static auto CrossProduct(const Point& p, const Point& a, const Point& b) {
    return ((b.x - a.x) * (p.y - a.y)) - ((b.y - a.y) * (p.x - a.x));
}

static void OtoczkaHelper(std::vector<Point>& set, const Point& start, const Point& end, std::vector<Point>& result) {
    if (set.empty()) {
        return;
    }

    // najdalszy punkt od prostej
    auto furthest = std::ranges::max(set, [&start, &end](const Point& a, const Point& b) {
        return CrossProduct(a, start, end) < CrossProduct(b, start, end);
    });

    // skoro jest najdalszy to musi lezec na otoczce
    result.push_back(furthest);

    // tyko punkty lezace "na zewnatrz" linii tworzonej
    // przez start/end i furthest sa brane pod uwage
    std::vector<Point> startFurthest;
    std::vector<Point> furthestEnd;
    for (const auto& p : set) {
        if (CrossProduct(p, start, furthest) > 0) {
            startFurthest.push_back(p);
        }
        if (CrossProduct(p, furthest, end) > 0) {
            furthestEnd.push_back(p);
        }
    }
    // powtarzamy do momentu az nie ma punktow "na zewnatrz"
    OtoczkaHelper(startFurthest, start, furthest, result);
    OtoczkaHelper(furthestEnd, furthest, end, result);
}

static auto Otoczka(std::vector<Point> points) -> std::vector<Point> {
    // 3 punkty to minimum by stworzyc 2D ksztalt
    if (points.size() < 3) {
        return points;
    }

    std::ranges::sort(points);

    // pierwszy/ostatni element posortowanych punktow to min/max
    auto minX = points.front();
    auto maxX = points.back();
    // minX i maxX sa na pewno w otoczce wiec je usuwamy
    points.erase(points.begin());
    points.pop_back();

    // w result beda znajdowac sie nasze punkty tworzace otoczke
    std::vector<Point> result;
    result.push_back(maxX);
    result.push_back(minX);

    // minX i maxX dziela punkty na dwie czesci horyzontalnie
    std::vector<Point> above;
    for (const auto& p : points) {
        if (CrossProduct(p, minX, maxX) > 0) {
            above.push_back(p);
        }
    }
    std::vector<Point> below;
    for (const auto& p : points) {
        if (CrossProduct(p, minX, maxX) < 0) {
            below.push_back(p);
        }
    }

    // rekurencyjnie rozwiazywanie gornej czesci
    OtoczkaHelper(above, minX, maxX, result);
    // rekurencyjnie rozwiazywanie dolnej czesci
    OtoczkaHelper(below, maxX, minX, result);

    std::cout << "Otoczka: ";
    for (const auto& p : result) {
        std::cout << "(" << p.x << ", " << p.y << "), ";
    }
    std::cout << "\n";

    return result;
}

/*******************************   PROSTE   ***********************************/
/******************************************************************************/

static void Proste(std::vector<Point>& points) {
    Point center = std::accumulate(points.begin(), points.end(), Point{}, [](Point acc, const Point& p) {
        return Point{
            .x = acc.x + p.x,
            .y = acc.y + p.y,
        };
    });
    center.x /= static_cast<double>(points.size());
    center.y /= static_cast<double>(points.size());

    // sortowanie punktow otoczki przeciwnie(chyba) do wskazowek zegara
    std::ranges::sort(points, [&center](const Point& lhs, const Point& rhs) {
        return std::atan2(lhs.y - center.y, lhs.x - center.x) < std::atan2(rhs.y - center.y, rhs.x - center.x);
    });

    struct Line { // ax + by + c = 0
        double a;
        double b;
        double c;
    };

    double result{ std::numeric_limits<double>::max() };

    // szuka najdalszego punktu od prostej ab i liczy odleglosc tego punktu
    // do tej prostej
    auto lineDistance = [&points](const Point& a, const Point& b) {
        Line l1{
            .a = a.y - b.y,
            .b = b.x - a.x,
            .c = (a.x * b.y) - (b.x * a.y),
        };

        const Point* furthest{};
        double distance{};
        for (auto& point : points) {
            if (point == a || point == b) {
                continue;
            }
            double currD =
                std::abs((l1.a * point.x) + (l1.b * point.y) + l1.c) / std::sqrt((l1.a * l1.a) + (l1.b * l1.b));
            if (currD > distance) {
                distance = currD;
                furthest = &point;
            }
        }
        if (!furthest) {
            return 0.;
        }
        return std::abs((l1.a * furthest->x) + (l1.b * furthest->y) + l1.c) / std::sqrt((l1.a * l1.a) + (l1.b * l1.b));
    };

    // dla kazdych dwoch punktow bierzemy najdalszy i rozpatrujemy odleglosci
    // najkrotsza jest wynikiem
    for (auto it = points.begin(); it != std::prev(points.end()); ++it) {
        double distance = lineDistance(*it, *std::next(it));
        result = std::min(distance, result);
    }
    // pierwszy i ostatni punkt ktore sa pomijane przez petle
    double distance = lineDistance(points.back(), points.front());
    result = std::min(distance, result);

    std::cout << "Proste: d=" << result << "\n";
}

////////////////////////////////// MISC ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static auto ParseFile(const std::filesystem::path& path) -> std::vector<Point> {
    if (!std::filesystem::exists(path)) {
        std::cout << "There is no file named " << path.filename() << " in " << path.parent_path() << " directory\n";
        return {};
    }

    std::ifstream file;
    try {
        file.open(path);
    } catch (...) {
        std::cout << "Unexpected error\n";
    }

    size_t size{};
    file >> size;

    std::vector<Point> retval;
    retval.reserve(size);

    double x{};
    double y{};
    while (file >> x >> y) {
        retval.emplace_back(x, y);
    }

    file.close();

    return retval;
}
