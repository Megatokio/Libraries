#pragma once
// Copyright (c) 2007 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	used in vipsi.Compiler.cpp
*/

template<class T>
class Stack
{
	uint size;
	uint used;
	T*	 liste;

	void Resize();

public:
	Stack()
	{
		size  = 0;
		used  = 0;
		liste = NULL;
	}
	~Stack() { delete[] liste; }
	void Push(const T& item)
	{
		if (used == size) Resize();
		liste[used++] = item;
	}
	T&	 Pop() { return liste[--used]; }
	T&	 Top() { return liste[used - 1]; }
	T&	 operator[](int i) { return liste[i]; }
	uint Count() { return used; }
	bool IsEmpty() { return used == 0; }
	bool IsNotEmpty() { return used != 0; }
	T*	 get_start() { return liste; }
	T*	 get_end() { return liste + used; }
};


template<class T>
void Stack<T>::Resize()
{
	T* alt = liste;
	liste  = new T[(size = size * 3 / 2 + 10)];
	for (uint i = 0; i < used; i++) liste[i] = alt[i];
	delete[] alt;
}
