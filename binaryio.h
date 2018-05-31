#ifndef BINARY_IO
#define BINARY_IO

#include "Computer.h"

template<typename T> struct tag {};

template<typename T> void write_binary(T val, ofstream& out)
{
	out.write((const char*)&val, sizeof(T));
}

void write_binary(string str, ofstream& out);
void write_binary(File file, ofstream& out);
string read_binary(tag<string>, ifstream& in);
File read_binary(tag<File>, ifstream& in);

template<typename T> void write_binary(vector<T> vec, ofstream& out)
{
	int len = vec.size();
	out.write((const char*)&len, sizeof(int));
	for (int i = 0; i < len; ++i)
		write_binary(vec[i], out);
}

template<typename T, typename U> void write_binary(map<T, U> m, ofstream& out)
{
	int len = m.size();
	out.write((const char*)&len, sizeof(int));
	for (const auto &pair : m)
	{
		write_binary(pair.first, out);
		write_binary(pair.second, out);
	}
}

template<typename T, typename U> void write_binary(pair<T, U> p, ofstream& out)
{
	write_binary(p.first, out);
	write_binary(p.second, out);
}

template<typename T> T read_binary(ifstream& in)
{
	return read_binary(tag<T>(), in);
}

template<typename T> T read_binary(tag<T>, ifstream& in)
{
	T ret;
	in.read((char*)&ret, sizeof(T));
	return ret;
}

template<typename T> vector<T> read_binary(tag<vector<T>>, ifstream& in)
{
	int len;
	in.read((char*)&len, sizeof(int));
	vector<T> vec;
	for (int i = 0; i < len; ++i)
		vec.push_back(read_binary<T>(in));

	return vec;
}

template<typename T, typename U> map<T, U> read_binary(tag<map<T, U>>, ifstream& in)
{
	int len;
	in.read((char*)&len, sizeof(int));
	map<T, U> m;
	for (int i = 0; i < len; ++i)
	{
		T key = read_binary<T>(in);
		U value = read_binary<U>(in);
		m[key] = value;
	}

	return m;
}

template<typename T, typename U> pair<T, U> read_binary(tag<pair<T, U>>, ifstream& in)
{
	pair<T, U> ret;
	ret.first = read_binary<T>(in);
	ret.second = read_binary<U>(in);
	return ret;
}

#endif