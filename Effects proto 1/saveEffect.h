#pragma once
#include "IEffect.h"
#include <fstream>
#include <string>

template <typename T>
class saveEffect :
	public IEffect<T>
{
private:
	std::string file_name;
	std::ofstream out_file_stream;
public:
	saveEffect();
	saveEffect(std::string file_name);
	void apply(T* buffer, size_t buffer_len);
};

template<typename T>
inline saveEffect<T>::saveEffect()
{
	file_name = "buff_raw_data.txt";
	out_file_stream.open(file_name, std::ostream::out | std::ofstream::app);
	if (!out_file_stream.is_open())
	{
		std::cerr << "COULD NOT OPEN FILE: " << file_name << "\n";
	}
}

template<typename T>
inline saveEffect<T>::saveEffect(std::string new_file_name)
{
	file_name = new_file_name;
	out_file_stream.open(file_name, std::ostream::out | std::ofstream::app);
	if (!out_file_stream.is_open())
	{
		std::cerr << "COULD NOT OPEN FILE: " << file_name << "\n";
	}
}

template<typename T>
inline void saveEffect<T>::apply(T* buffer, size_t buffer_len)
{
	for (size_t i = 0; i < buffer_len; ++i)
	{
		out_file_stream << buffer[i] << " ";
	}
	out_file_stream << "\n";
	//out_file_stream.flush();
}