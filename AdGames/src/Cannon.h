#pragma once

#include <initializer_list>

namespace Cannon
{
	void Run();
};

template <typename A, typename B, typename C, typename D>
struct Quadruple
{
	A first;
	B second;
	C third;
	D fourth;
	Quadruple(A first, B second, C third, D fourth) : first(first), second(second), third(third), fourth(fourth) {}
	Quadruple(std::initializer_list<A> first, std::initializer_list<B> second, std::initializer_list<C> third, std::initializer_list<D> fourth) : first(*first.begin()), second(*second.begin()), third(*third.begin()), fourth(*fourth.begin()) {};
};
