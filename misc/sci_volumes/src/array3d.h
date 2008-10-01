// Volume data shader
// Copyright (C) 2008 Christopher J. Foster
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#ifndef ARRAY3D_H_INCLUDED
#define ARRAY3D_H_INCLUDED

#include <vector>
#include <fstream>
#include <stdexcept>
#include <iterator>

template<typename T>
class Array3D
{
	public:
		Array3D(int nx, int ny, int nz, T init = 0);
		Array3D(const char* fileName);
		inline T operator()(int x, int y, int z) const;
		inline T& operator()(int x, int y, int z);
		inline int nx() const;
		inline int ny() const;
		inline int nz() const;
	private:
		int m_nx;
		int m_ny;
		int m_nz;
		std::vector<T> m_data;
};

//------------------------------------------------------------------------------
// Implementation of inline functions
template<typename T>
inline T Array3D<T>::operator()(int x, int y, int z) const
{
	return m_data[(z*m_ny + y)*m_nx + x];
}

template<typename T>
inline T& Array3D<T>::operator()(int x, int y, int z)
{
	return m_data[(z*m_ny + y)*m_nx + x];
}

template<typename T>
inline int Array3D<T>::nx() const
{
	return m_nx;
}

template<typename T>
inline int Array3D<T>::ny() const
{
	return m_ny;
}

template<typename T>
inline int Array3D<T>::nz() const
{
	return m_nz;
}

template<typename T>
Array3D<T>::Array3D(int nx, int ny, int nz, T init)
	: m_nx(nx),
	m_ny(ny),
	m_nz(nz),
	m_data(nx*ny*nz, init)
{ }

template<typename T>
Array3D<T>::Array3D(const char* fileName)
	: m_nx(0),
	m_ny(0),
	m_nz(0),
	m_data()
{
	std::ifstream inFile(fileName);

	if(!inFile)
		throw std::runtime_error("Could not open input density file");

	// Read in the header of dimensions.
	inFile >> m_nx;
	inFile >> m_ny;
	inFile >> m_nz;
	int nTot = m_nx*m_ny*m_nz;
	if(nTot <= 0)
		throw std::runtime_error("Data array contains zero elements");

	// Read data into array
	m_data.reserve(nTot);
	std::copy(std::istream_iterator<float>(inFile), std::istream_iterator<float>(),
			std::back_inserter(m_data));
	if(static_cast<int>(m_data.size()) != nTot)
	{
		throw std::runtime_error("Not enough data in file: " + std::string(fileName));
	}

	// read from a binary file instead...
	/*
    // Read header
    inFile.read(reinterpret_cast<char*>(&m_nx), sizeof(m_nx));
    inFile.read(reinterpret_cast<char*>(&m_ny), sizeof(m_ny));
    inFile.read(reinterpret_cast<char*>(&m_nz), sizeof(m_nz));
	m_data = boost::shared_array<float>
    // Read in data
	int dataSize = m_data.size()*sizeof(float);
    inFile.read(reinterpret_cast<char*>(&m_data[0]), dataSize);
	// check that we've got it all...
	if(inFile.gcount() != dataSize)
		throw std::runtime_error("Could not extract enough data from density file");
    inFile.close();
	*/
}

#endif // ARRAY3D_H_INCLUDED
