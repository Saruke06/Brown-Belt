#include "Common.h"

using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`

namespace Shape {
class Rectangle : public IShape {
private:
	shared_ptr<ITexture> texture = nullptr;
	Point position = {0, 0};
	Size size = {0, 0};
public:
	  Rectangle() = default;
	  Rectangle(shared_ptr<ITexture> txtr, Point pos, Size s)
	  : texture(txtr)
	  , position(pos)
	  , size(s)
	  {}

	  // Возвращает точную копию фигуры.
	  // Если фигура содержит текстуру, то созданная копия содержит ту же самую
	  // текстуру. Фигура и её копия совместно владеют этой текстурой.
	  unique_ptr<IShape> Clone() const override {
		  return make_unique<Rectangle>(texture, position, size);
	  }

	  void SetPosition(Point p) override {
		  position = p;
	  }

	  Point GetPosition() const override {
		  return position;
	  }

	  void SetSize(Size s) override {
		  size = s;
	  }

	  Size GetSize() const override {
		  return size;
	  }

	  void SetTexture(std::shared_ptr<ITexture> txtr) override {
		  texture = txtr;
	  }
	  ITexture* GetTexture() const override {
		  return texture.get();
	  }

	  // Рисует фигуру на указанном изображении
	  void Draw(Image& image) const override {
		  Size tex_size = {0, 0};
		  Image tex_image = {};
		  if (texture != nullptr) {
			  tex_size = texture->GetSize();
			  tex_image = texture->GetImage();
		  }

		  for (int i = 0; i < size.height; ++i) {
			  for (int j = 0; j < size.width; ++j) {
				  // Рисуем только если не выходит за рамки изображения
				  if ((position.y + i) < image.size() && (position.x + j) < image[0].size()) {
					  // Покрываем текстурой только в рамках текстуры
					  if (i < tex_size.height && j < tex_size.width) {
						  image[position.y + i][position.x + j] = tex_image[i][j];
					  }
					  // Иначе не покрываем
					  else {
					      image[position.y + i][position.x + j] = '.';
					  }
				  }
			  }
		  }
	  }
};

class Ellipse : public IShape {
private:
	shared_ptr<ITexture> texture = nullptr;
	Point position = {0, 0};
	Size size = {0, 0};
public:
	  Ellipse() = default;
	  Ellipse(shared_ptr<ITexture> txtr, Point pos, Size s)
	  : texture(txtr)
	  , position(pos)
	  , size(s)
	  {}
	  // Возвращает точную копию фигуры.
	  // Если фигура содержит текстуру, то созданная копия содержит ту же самую
	  // текстуру. Фигура и её копия совместно владеют этой текстурой.
	  unique_ptr<IShape> Clone() const override {
		  return make_unique<Ellipse>(texture, position, size);
	  }

	  void SetPosition(Point p) override {
		  position = p;
	  }
	  Point GetPosition() const override {
		  return position;
	  }

	  void SetSize(Size s) override {
		  size = s;
	  }
	  Size GetSize() const override {
		  return size;
	  }

	  void SetTexture(std::shared_ptr<ITexture> txtr) override {
		  texture = txtr;
	  }
	  ITexture* GetTexture() const override {
		  return texture.get();
	  }

	  // Рисует фигуру на указанном изображении
	  void Draw(Image& image) const override {
		  Size tex_size = {0, 0};
		  Image tex_image = {};
		  if(texture != nullptr) {
			  tex_size = texture->GetSize();
			  tex_image = texture->GetImage();
		  }

		  for (int i = 0; i < size.height; ++i) {
			  for (int j = 0; j < size.width; ++j) {
				  // Рисуем только если не выходит за рамки изображения
				  if ((position.y + i) < image.size() && (position.x + j) < image[0].size()) {
					  // Покрываем пикселями только в рамках эллипса
					  if (IsPointInEllipse(Point{j, i}, size)) {
						  // Покрываем текстурой только в рамках текстуры
						  if (i < tex_size.height && j < tex_size.width) {
							  image[position.y + i][position.x + j] = tex_image[i][j];
						  }
						  // Иначе не покрываем
						  else {
						      image[position.y + i][position.x + j] = '.';
						  }
					  }
				  }
			  }
		  }

	  }
};

}


// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
	switch (shape_type) {
	case ShapeType::Rectangle:
		return make_unique<Shape::Rectangle>();
	case ShapeType::Ellipse:
		return make_unique<Shape::Ellipse>();
	default:
		return nullptr;
	}
}
