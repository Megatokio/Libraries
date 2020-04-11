/*	Copyright  (c)	Günter Woigk 2019 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#pragma once
//#include "kio/kio.h"
#include <bits/move.h>


//
//	use:
//
//	auto z = finally([]{...});
//


template <class F>
class final_act
{
private:
	F	 f_;
	bool invoke_;

public:
	explicit final_act(F f) noexcept
	  : f_(std::move(f)), invoke_(true) {}

	final_act(final_act&& other) noexcept
	 : f_(std::move(other.f_)),
	   invoke_(other.invoke_)
	{
		other.invoke_ = false;
	}

	final_act(const final_act&) = delete;
	final_act& operator=(const final_act&) = delete;

	~final_act() noexcept
	{
		if (invoke_) f_();
	}
};

template <class F>
inline final_act<F> finally(const F& f) noexcept
{
	return final_act<F>(f);
}

template <class F>
inline final_act<F> finally(F&& f) noexcept
{
	return final_act<F>(std::forward<F>(f));
}



