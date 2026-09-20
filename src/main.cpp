import std;
import derive;
import soa;
import nanopass;

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
  std::println("points[0] = {}", std::as_const(points)[0]);
  std::println("points[1] = {}", std::as_const(points)[1]);
  points[0].y = 10;
  std::println("points[0] = {}", std::as_const(points)[0]);
  points[0] = Point{.x = 'g', .y = 12, .tags = {4, 5, 6}};
  std::println("points[0] = {}", std::as_const(points)[0]);
  std::println("points[0] = {}", points[0]);
  testCompile();
}