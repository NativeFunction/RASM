#pragma once

#include <iostream>
#include <Utils/types.h>

std::ostream& black(std::ostream &s);
std::ostream& grey(std::ostream &s);

std::ostream& blue(std::ostream &s);
std::ostream& brightblue(std::ostream &s);

std::ostream& green(std::ostream &s);
std::ostream& brightgreen(std::ostream &s);

std::ostream& aqua(std::ostream &s);
std::ostream& brightaqua(std::ostream &s);

std::ostream& red(std::ostream &s);
std::ostream& brightred(std::ostream &s);

std::ostream& purple(std::ostream &s);
std::ostream& brightpurple(std::ostream &s);

std::ostream& yellow(std::ostream &s);
std::ostream& brightyellow(std::ostream &s);

std::ostream& white(std::ostream &s);
std::ostream& brightwhite(std::ostream &s);

struct color {
	color(uint16_t attribute) :m_color(attribute) {};
	uint16_t m_color;
};

void SetColorGeneric(color& c);

template <class _Elem, class _Traits> std::basic_ostream<_Elem, _Traits>&
operator<<(std::basic_ostream<_Elem, _Traits>& i, color& c)
{
	SetColorGeneric(c);
	return i;
}