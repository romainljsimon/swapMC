/*
 * energy.cpp
 *
 *  Created on: 3 oct. 2022
 *      Author: Romain Simon
 */
#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "util.h"

/*******************************************************************************
 * This function calculates the Lennard-Jones potential energy between two
 * particles seperated by a distance whose square is equal to squareDistance.
 * We consider a cut off whose square is equal to squareRc times the mean diameter
 * of the two particles considered. The potential is equal to 0 for distances
 * greater that the cut off.
 *
 * @param squareDistance Square of the distance separating the two considered
 *                       particles
 *        sigmaA Particle A's diameter
 *        sigmaB Particle B's diameter
 *        squareRc Square of the cut off radius
 *        shift Shifts the Lennard-Jones potential by a constant value
 *
 * @return energy
 ******************************************************************************/
double ljPotential(const double& squareDistance, const double& sigmaA, const double& sigmaB, const double& squareRc, const double& shift)
{

	const double squareSigma { std::pow (((sigmaA + sigmaB) / 2. ), 2.) };
    // We consider a cut-off radius which is the threshold maximum distance of interaction between two particles
	if (squareDistance > squareRc * squareSigma)
	{
		return 0.;
	}

	else
	{
		const double rap_square {std::pow ((squareSigma / squareDistance), 3.)};
		return 4. * rap_square * ( rap_square - 1.) + 4. * shift;
	}
}

/*******************************************************************************
 * This function calculates the FENE (finitely extensible nonlinear elastic) 
 * potential energy between two nearest-neighbor monomers seperated by a 
 * distance whose square is equal to squareDistance. SquareR0 and feneK are two
 * parameters of the potential. SquareRo is the maximum square distance that can
 * separate two neighboring atoms and feneK represents the stiffness of the
 * elastic. They are chosen such as bond crossing doesn't occur.
 *
 * @param squareDistance Square of the distance separating the two considered
 *                       particles
 *        sigmaA particle A's diameter.
 *        sigmaB particle B's diameter.
 *        squareR0 maximum square distance.
 *        feneK stiffness of the elastic.
 *
 * @return energy
 ******************************************************************************/
double fenePotential(const double& squareDistance, const double& sigmaA, const double& sigmaB, double squareR0, double feneK)
{
	const double squareSigma { std::pow (((sigmaA + sigmaB) / 2. ), 2.) };
	squareR0 = squareR0 * squareSigma;
    // We consider a cut-off radius which is the threshold maximum distance of interaction between two particles
    if (squareDistance >= squareR0)
	{
		return std::numeric_limits<double>::infinity();
	}
    else
	{
		feneK = feneK / squareSigma;
		return -0.5 * feneK * squareR0 * std::log(1. - squareDistance / squareR0);
	}
}

/*******************************************************************************
 * This function calculates the potential energy of one particle considering
 * that particles are Lennard-Jones particles.
 *
 * @param indexParticle Considered particle's index in the positionArray and 
 *                      diameterArray arrays.
 *        positionParticle Considered particle's position.
 * 	      positionArray array of particle positions.
 * 	      neighborIList Considered particle's neighbor list.
 *        diameterArray array of particle diameters.
 *        squareRc square of the cut off radius.
 *        lengthCube simulation box length.
 * @return Particle's total energy.
 ******************************************************************************/
double energyParticle(const int& indexParticle, const std::vector<double>& positionParticle,
		      const std::vector<std::vector<double>>& positionArray, const std::vector<int>& neighborIList,
              const std::vector<double>& diameterArray, const double& squareRc, const double& lengthCube,
              const double& halfLengthCube, const int& indexSkip = -1)
{
	double energy { 0. };
	const double particleDiameter = diameterArray[indexParticle];
	const int neighborIListSize { static_cast<int>(neighborIList.size()) };

	for (int i = 0; i < neighborIListSize; i++)
	{
		int realIndex = neighborIList[i];

        if (realIndex != indexSkip)
        {
            if (realIndex == indexParticle)
            {
                continue;
            }

            const double squareDistance{squareDistancePair(positionParticle, positionArray[realIndex], lengthCube, halfLengthCube)};
            energy += ljPotential(squareDistance, particleDiameter, diameterArray[realIndex], squareRc, 0.);
        }
	}
	return energy;
}


/*******************************************************************************
 * This function calculates the total potential energy of the system considering
 * that particles are Lennard-Jones particles.
 *
 * @param positionArray array of particle positions.
 *        diameterArray array of particle diameter.
 *        squareRc square of the cut off radius.
 *        lengthCube simulation box length.
 *
 * @return System's total energy.
 ******************************************************************************/
double energySystem(const std::vector<std::vector<double>>& positionArray, const std::vector<double>& diameterArray,
		            const std::vector<std::vector<int>>& neighborList, const double& squareRc,
					const double& lengthCube, const double& halfLengthCube)
{
	double energy { 0. };
	const int positionArraySize {static_cast<int>( positionArray.size() )};

	for (int i = 0; i < positionArraySize - 1; i++) //Outer loop for rows
    {
    	const std::vector<int>& neighborIList ( neighborList[i] );

    	energy += energyParticle (i, positionArray[i], positionArray, neighborIList,
				  diameterArray, squareRc, lengthCube, halfLengthCube) / 2;

    }
	return energy;
}

double pedersenBonds(const int& indexParticle, const std::vector<double>& positionParticle,
                     const std::vector<std::vector<double>>& positionArray,
                     const std::vector<double>& diameterArray, const std::vector<int>& bondsI,
                     const double& squareRc, const double& lengthCube, const double& halfLengthCube,
                     const double& squareR0, const double& feneK, const int& indexSkip = -1)
{
    double energy { 0. };
    const double particleDiameter = diameterArray[indexParticle];
    const int bondsISize { static_cast<int>(bondsI.size()) };

    for (int i = 0; i < bondsISize; i++)
    {
        const int realIndex = bondsI[i];
        if (realIndex != indexSkip)
        {
            if ((realIndex == indexParticle) || (realIndex == -1))
            {
                continue;
            }
            const double squareDistance{squareDistancePair(positionParticle, positionArray[realIndex],
                                                     lengthCube, halfLengthCube)};

            if ((((indexParticle % 3) == 0) && (realIndex == indexParticle + 2)) ||
                ((indexParticle == realIndex + 2) && ((realIndex % 3) == 0)))
            {
                constexpr double sigma_factor = 1.35;
                energy -= ljPotential(squareDistance, particleDiameter, diameterArray[realIndex], squareRc, 0.25);
                energy += ljPotential(squareDistance, particleDiameter * sigma_factor,
                                      diameterArray[realIndex] * sigma_factor, squareRc, 0.25);
                energy += fenePotential(squareDistance, particleDiameter * sigma_factor,
                                        diameterArray[realIndex] * sigma_factor, squareR0, feneK);
            }
            else
            {
                energy += fenePotential(squareDistance, particleDiameter, diameterArray[realIndex], squareR0, feneK);
            }
        }
    }

    return energy;
}

double flexibleBonds(const int& indexParticle, const std::vector<double>& positionParticle,
                     const std::vector<std::vector<double>>& positionArray,
                     const std::vector<double>& diameterArray, const std::vector<int>& bondsI,
                     const double& lengthCube, const double& halfLengthCube, const double& squareR0,
                     const double& feneK, const int& indexSkip = -1)
{
    double energy { 0. };
    const double particleDiameter = diameterArray[indexParticle];
    const int bondsISize { static_cast<int>(bondsI.size()) };

    for (int i = 0; i < bondsISize; i++)
    {
        const int realIndex = bondsI[i];

        if (realIndex != indexSkip)
        {
            if ((realIndex == indexParticle) || (realIndex == -1))
            {
                continue;
            }

            const double squareDistance{squareDistancePair(positionParticle, positionArray[realIndex], lengthCube, halfLengthCube)};
            energy += fenePotential(squareDistance, particleDiameter, diameterArray[realIndex], squareR0, feneK);
        }
    }
    return energy;
}
/*******************************************************************************
 * This function calculates the potential energy of one monomer considering
 * that it is part of a polymer chain. Every monomer interact via a 
 * Lennard-Jones potential. Nearest-neighbor monomers also interact via a FENE 
 * potential.
 *
 * @param indexParticle Considered monomer's index in the positionArray and 
 *                      diameterArray arrays.
 *        positionArray Array of monomer positions.
 *        neighborIList Considered monomer's neighbor list.
 *        diameterArray Array of particle radii.
 *        bondsI vector that indicates which monomers are linked with the 
 *               considered monomer.
 *        squareRc Square of the cut off radius.
 *        lengthCube Simulation box length.
 *        squareR0 Maximum square distance.
 *        feneK Elastic's stiffness.
 *
 * @return Polymeric particle's total energy.
 ******************************************************************************/
double energyParticlePolymer (const int& indexParticle, const std::vector<double>& positionParticle,
							  const std::vector<std::vector<double>>& positionArray,
                              const std::vector<int>& neighborIList,
							  const std::vector<double>& diameterArray, const std::vector<int>& bondsI,
							  const double& squareRc, const double& lengthCube, const double& halfLengthCube, const double& squareR0,
                              const double& feneK, const std::string& bondType, const int& indexSkip = -1)
{
	double energy { 0. };
	const double particleDiameter = diameterArray[indexParticle];
	const int neighborIListSize { static_cast<int>(neighborIList.size()) };

    for (int i = 0; i < neighborIListSize; i++)
    {
        const int realIndex = neighborIList[i];

        if (realIndex != indexSkip)
        {
            const double squareDistance{squareDistancePair(positionParticle, positionArray[realIndex], lengthCube, halfLengthCube)};
            if (realIndex == indexParticle) {
                continue;
            }

            energy += ljPotential(squareDistance, particleDiameter, diameterArray[realIndex], squareRc,
                                  0.25); //127. / 4096);
        }
    }
    if ( bondType == "flexible")
    {
        energy += flexibleBonds(indexParticle, positionParticle, positionArray, diameterArray, bondsI, lengthCube,
                                halfLengthCube, squareR0, feneK, indexSkip);
    }
    else if ( bondType == "pedersen")
    {
        energy += pedersenBonds(indexParticle, positionParticle, positionArray, diameterArray, bondsI,
                                squareRc, lengthCube, halfLengthCube, squareR0,feneK, indexSkip);
    }
    return energy;
}


/*******************************************************************************
 * This function calculates the total potential energy of the system considering
 * that the system is polymeric. Every monomer interact via a Lennard-Jones
 * potential. Nearest-neighbor monomers also interact via a FENE potential.
 *
 * @param positionArray array of particle positions.
 *        diameterArray array of particle diameters.
 *        bondsMatrix matrix that indicates which monomers are linked together.
 *        squareRc square of the cut off radius.
 *        lengthCube simulation box length.
 *        squareR0 maximum square distance.
 *        feneK stiffness of the elastic
 *
 * @return Polymeric system's total energy.
 ***********************************************************position65000.xyz*******************/
double energySystemPolymer(const std::vector<std::vector<double>>& positionArray, const std::vector<double>& diameterArray,
						   const std::vector<std::vector<int>>& bondsMatrix, const std::vector<std::vector<int>>& neighborList,
						   const double& squareRc, const double& lengthCube, const double& halfLengthCube,const double& squareR0, const double& feneK, const std::string& bondType)
{
	double energy { 0. };
	const int positionArraySize {static_cast<int>(positionArray.size())};

	for (int i = 0; i < positionArraySize; i++) //Outer loop for rows
    {

    	const std::vector<int>& bondsI ( bondsMatrix[i] );
    	const std::vector<int>& neighborIList ( neighborList[i] );

    	energy += energyParticlePolymer (i, positionArray[i], positionArray, neighborIList,
				  diameterArray, bondsI, squareRc, lengthCube, halfLengthCube, squareR0, feneK, bondType) / 2.;

    }
	return energy;
}
