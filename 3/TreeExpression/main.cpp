#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

namespace Expr {
class Value : public Expression {
private:
	int value;
public:
	Value(int a) : value(a) {}

	int Evaluate() const override {
		return value;
	}

	string ToString() const override {
		return to_string(value);
	}
};

class Sum : public Expression {
private:
	ExpressionPtr lhs;
	ExpressionPtr rhs;
public:
	Sum(ExpressionPtr lhs_, ExpressionPtr rhs_) : lhs(move(lhs_)), rhs(move(rhs_)) {}

	int Evaluate() const override {
		return lhs->Evaluate() + rhs->Evaluate();
	}

	string ToString() const override {
		return "(" + lhs->ToString() + ")+(" + rhs->ToString() + ")";
	}
};

class Product : public Expression {
private:
	ExpressionPtr lhs;
	ExpressionPtr rhs;
public:
	Product(ExpressionPtr lhs_, ExpressionPtr rhs_) : lhs(move(lhs_)), rhs(move(rhs_)) {}

	int Evaluate() const override {
		return lhs->Evaluate() * rhs->Evaluate();
	}

	string ToString() const override {
		return "(" + lhs->ToString() + ")*(" + rhs->ToString() + ")";
	}
};
}


ExpressionPtr Value(int value) {
	return make_unique<Expr::Value>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
	return make_unique<Expr::Sum>(move(left), move(right));
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
	return make_unique<Expr::Product>(move(left), move(right));
}


string Print(const Expression* e) {
  if (!e) {
    return "Null expression provided";
  }
  stringstream output;
  output << e->ToString() << " = " << e->Evaluate();
  return output.str();
}

void Test() {
  ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
  ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

  ExpressionPtr e2 = Sum(move(e1), Value(5));
  ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

  ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, Test);
  return 0;
}
