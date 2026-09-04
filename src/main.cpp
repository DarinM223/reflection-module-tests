import std;
import derive;
import soa;

struct[[= derive<Debug>]] Point {
  char x;
  int y;
  std::vector<int> tags;
};

int main() {
  SoaVector<Point> points;
  points.push_back(Point{.x = 'e', .y = 4});
  points.push_back(Point{.x = 'f', .y = 7});
  std::println("x = {} y = {}", std::span(points.pointers_.x, points.size_),
               std::span(points.pointers_.y, points.size_));
  std::println("My point: {}", Point{.x = 'a', .y = 2, .tags = {1, 2, 3}});
}