/*
 * util.cpp
 *
 *  Created on: 13 oct. 2022
 *      Author: Romain Simon
 */

#include <utility>
#include <vector>
#include <numeric>
#include <algorithm>
#include <valarray>


// Vector Operators



double innerProduct(const std::vector<double>& vec1, const std::vector<double>& vec2)
{
    return std::inner_product(vec1.begin(), vec1.end(), vec2.begin(), 0.);
}

double cosAngleVectors(const std::vector<double>& vec1, const std::vector<double>& vec2)
{
    double innerProduct11 {innerProduct(vec1, vec1)};
    double innerProduct22 {innerProduct(vec2, vec2)};
    double innerProduct12 {innerProduct(vec1, vec2)};

    if (innerProduct12 == 0.)
    {
        return 0.;
    }

    else
    {
        return innerProduct12 / (sqrt(innerProduct11) * sqrt(innerProduct22));
    }
}

double meanVector(const std::vector<double>& vec)
{
	double sum { 0 };
	for (auto const &elt: vec)
	{
		sum += elt;
	}
	return sum / static_cast<double>(vec.size());
}

std::vector<double> divideVectorByScalar(std::vector<double> vec, const double& scalar)
{
	for (double & i : vec)
	{
		i = i / scalar;
	}
	return vec;
}

std::vector<double> multiplyVectorByScalar(std::vector<double> vec, const double& scalar)
{
	for (double & i : vec)
	{
		i = i * scalar;
	}
	return vec;
}


std::vector<double> vectorNormalization(const std::vector<double>& vec)
{
	double mean { meanVector(vec) };
	return divideVectorByScalar(vec, mean);
}

std::vector<double> vectorSum(const std::vector<double>& vec1, const std::vector<double>& vec2)
{
    //std::transform (vec1.begin(), vec1.end(), vec2.begin(), vec1.begin(), std::plus<double>());
    int vecSize{ static_cast<int>(vec1.size()) };
    std::vector<double> sumVec(vecSize);
    for (int i=0; i<vecSize; i++)
    {
        sumVec[i] = vec1[i] + vec2[i];
    }
    return sumVec;
}

std::vector<double> vectorDiff(const std::vector<double>& vec1, const std::vector<double>& vec2, int nDims)
{
    std::vector<double> vecDiff(nDims);

    std::transform(vec1.begin(), vec1.begin() + nDims,
                   vec2.begin(), vecDiff.begin(), std::minus<>());
    return vecDiff;
}

double getMaxVector(const std::vector<double>& vec)
{
	double max = vec[0];
	for (auto const &elt: vec)
	{
		max = (max < elt) ? elt: max;
	}
	return max;
}


// matrix operations


std::vector<std::vector<double>> matrixSum(std::vector<std::vector<double>> mat1, const std::vector<std::vector<double>>& mat2)
{
	int matSize{ static_cast<int>(mat1.size()) };

	for (int i = 0; i < matSize; i++)
	{
			mat1[i] = vectorSum(mat1[i], mat2[i]);
	}
	return mat1;

}

std::vector<std::vector<double>> matrixSumWithVector(std::vector<std::vector<double>> mat1, const std::vector<double>& vec1)
{
	int matSize{ static_cast<int>(mat1.size()) };

	for (int i = 0; i < matSize; i++)
	{
			mat1[i] = vectorSum(mat1[i], vec1);
	}
	return mat1;

}
/***
std::vector<std::vector<double>> matrixDiffWithVector(std::vector<std::vector<double>> mat1, const std::vector<double>& vec1)
{
	int matSize{ static_cast<int>(mat1.size()) };

	for (int i = 0; i < matSize; i++)
	{
			mat1[i] = vectorDiff(mat1[i], vec1);
	}
	return mat1;
}
***/

std::vector<std::vector<double>> multiplyMatrixByScalar(std::vector<std::vector<double>> mat, const double& scalar)
{
	for (auto & i : mat)
		{
			i = multiplyVectorByScalar(i, scalar);
		}
	return mat;
}

double getMaxMatrix(const std::vector<std::vector<double>>& mat)
{
	double max { mat[0][0] };

	for (auto const &vec: mat)
	{
		double newMax = getMaxVector(vec);
		max = (newMax > max) ? newMax : max;
	}
	return max;
}

std::vector<std::vector<double>> rescaleMatrix(const std::vector<std::vector<double>>& mat, const double& rescale)
{
	double max { getMaxMatrix(mat) };
	double ratio {rescale / max};
	return multiplyMatrixByScalar(mat, ratio);
}



std::vector<double> meanColumnsMatrix(std::vector<std::vector<double>> mat)
{
	int matSize { static_cast<int>(mat.size()) };
	std::vector<double> mean(3, 0);

	for (int i = 0; i < matSize; i++)
	{
		mean = vectorSum(mean, mat[i]);
	}
	return divideVectorByScalar(mean, matSize);
}

std::vector<int> createSaveTime(const int& max, const int& linear_scalar, const float& log_scalar)
{
	std::vector<int> timeStepArray;

	for (int j = 0; j < max; j += linear_scalar)
	{
		timeStepArray.push_back ( j ) ;
		timeStepArray.push_back ( j + 1 );


		for (int i = static_cast<int>(log_scalar) +1; i < linear_scalar; i = static_cast<int>(i * log_scalar) +1)
		{
			timeStepArray.push_back ( j + i );

		}
	}
	timeStepArray.push_back ( max ) ;
	return timeStepArray;

}
