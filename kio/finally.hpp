// Copyright (c) 2019 - 2021 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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



