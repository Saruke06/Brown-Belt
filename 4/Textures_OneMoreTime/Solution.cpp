#include "Common.h"

#include <memory>

using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`

Size GetImageSize(const Image& image) {
    int width = static_cast<int>(image.empty() ? 0 : image[0].size());
    int height = static_cast<int>(image.size());
    return {width, height};
}

// Точка передаётся в локальных координатах
bool IsPointInSize(Point p, Size s) {
    return p.x >= 0 && p.x < s.width && p.y >=0 && p.y < s.height;
}

class Shape : public IShape {
    Point position_;
    Size size_;
    shared_ptr<ITexture> texture_;

    virtual bool IsPointInShape(Point p) const = 0;
public:
    virtual ~Shape() = default;

    void SetPosition(Point new_pos) override {
        position_ = new_pos;
    }

    Point GetPosition() const override {
        return position_;
    }

    void SetSize(Size new_size) override {
        size_ = new_size;
    }

    Size GetSize() const override {
        return size_;
    }

    void SetTexture(shared_ptr<ITexture> new_texture) override {
        texture_ = move(new_texture);
    }

    ITexture* GetTexture() const override {
        return texture_.get();
    }

    void Draw(Image& image) const override {
        Point p;
        Size image_size = GetImageSize(image);
        for (p.y = 0; p.y < size_.height; ++p.y) {
            for(p.x = 0; p.x < size_.width; ++p.x) {
                if (IsPointInShape(p)) {
                    char pixel = '.';
                    if (texture_ && IsPointInSize(p, texture_->GetSize())) {
                        pixel = texture_->GetImage()[p.y][p.x];
                    }
                    Point dp = {position_.x + p.x, position_.y + p.y};
                    if (IsPointInSize(dp, image_size)) {
                        image[dp.y][dp.x] = pixel;
                    }
                }
            }
        }
    }
};

class Rectangle : public Shape {
    ~Rectangle() override = default;

    bool IsPointInShape(Point p) const override {
        return p.x >= 0 && p.x < GetSize().width && p.y >= 0 && p.y < GetSize().height;
    }
public:
    unique_ptr<IShape> Clone() const override {
        return make_unique<Rectangle>(*this);
    }
};

class Ellipse : public Shape {
    ~Ellipse() override = default;

    bool IsPointInShape(Point p) const override {
        return IsPointInEllipse(p, GetSize());
    }
public:
    unique_ptr<IShape> Clone() const override {
        return make_unique<Ellipse>(*this);
    }
};

// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    switch (shape_type) {
        case ShapeType::Rectangle:
            return make_unique<Rectangle>();
        case ShapeType::Ellipse:
            return make_unique<Ellipse>();
    }
    return nullptr;
}