/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_PRIMARY_TYPES_H_
#define HELICS_PRIMARY_TYPES_H_
#pragma once

#include <boost/variant.hpp>
#include <string>
#include <complex>
#include <vector>
#include <cstdint>

namespace helics
{
	using defV = boost::variant< double, int64_t, std::string, std::complex<double>, std::vector<double>,std::vector<std::complex<double>>>;

	enum typeLocation
	{
		
		doubleLoc = 0,
		intLoc = 1,
		stringLoc = 2,
		complexLoc = 3,
		vectorLoc = 4,
		complexVectorLoc=5,
	};


} //namespace helics

#endif //HELICS_PRIMARY_TYPES_H_
