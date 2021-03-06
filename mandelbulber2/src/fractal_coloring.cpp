/**
 * Mandelbulber v2, a 3D fractal generator       ,=#MKNmMMKmmßMNWy,
 *                                             ,B" ]L,,p%%%,,,§;, "K
 * Copyright (C) 2017-18 Mandelbulber Team     §R-==%w["'~5]m%=L.=~5N
 *                                        ,=mm=§M ]=4 yJKA"/-Nsaj  "Bw,==,,
 * This file is part of Mandelbulber.    §R.r= jw",M  Km .mM  FW ",§=ß., ,TN
 *                                     ,4R =%["w[N=7]J '"5=],""]]M,w,-; T=]M
 * Mandelbulber is free software:     §R.ß~-Q/M=,=5"v"]=Qf,'§"M= =,M.§ Rz]M"Kw
 * you can redistribute it and/or     §w "xDY.J ' -"m=====WeC=\ ""%""y=%"]"" §
 * modify it under the terms of the    "§M=M =D=4"N #"%==A%p M§ M6  R' #"=~.4M
 * GNU General Public License as        §W =, ][T"]C  §  § '§ e===~ U  !§[Z ]N
 * published by the                    4M",,Jm=,"=e~  §  §  j]]""N  BmM"py=ßM
 * Free Software Foundation,          ]§ T,M=& 'YmMMpM9MMM%=w=,,=MT]M m§;'§,
 * either version 3 of the License,    TWw [.j"5=~N[=§%=%W,T ]R,"=="Y[LFT ]N
 * or (at your option)                   TW=,-#"%=;[  =Q:["V""  ],,M.m == ]N
 * any later version.                      J§"mr"] ,=,," =="""J]= M"M"]==ß"
 *                                          §= "=C=4 §"eM "=B:m|4"]#F,§~
 * Mandelbulber is distributed in            "9w=,,]w em%wJ '"~" ,=,,ß"
 * the hope that it will be useful,                 . "K=  ,=RMMMßM"""
 * but WITHOUT ANY WARRANTY;                            .'''
 * without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * ###########################################################################
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com)
 *
 * CalculateColorIndex(...) - Calculation of the color index of a point
 * based on miscellaneous criteria
 */
#include "fractal_coloring.hpp"

#include "fractal.h"

using namespace fractal;

sFractalColoring::sFractalColoring()
{
	sphereRadius = 0.0;
	coloringAlgorithm = fractalColoring_None;

	extraColorEnabledFalse = false;
	initialColorValue = 0.0;
	initCondFalse = false;
	icRadFalse = false;
	icXYZFalse = false;
	icFabsFalse = false;
	icRadWeight = 1.0;

	orbitTrapTrue = true;
	orbitTrapWeight = 1.0;
	initialMinimumR = 100.0;

	auxColorFalse = false;
	auxColorWeight = 1.0;
	auxColorHybridWeight = 0.0;

	radFalse = false;
	radWeight = 1.0;
	radSquaredFalse = false;
	radDiv1e13False = false;
	radDivDeFalse = false;
	radDivLogDeFalse = false;
	radDivDeWeight = 1.0;
	radDivDeSquaredFalse = false;

	xyzBiasEnabledFalse = false;
	xyzIterScale = 0.0;
	xyzXSqrdFalse = false;
	xyzYSqrdFalse = false;
	xyzZSqrdFalse = false;
	xyzFabsFalse = false;
	xyzDiv1e13False = false;

	iterGroupFalse = false;
	iterAddScaleTrue = true;
	iterScaleFalse = false;
	iterAddScale = 1.0;
	iterScale = 1.0;
	iStartValue = 0;

	// global palette controls
	globalPaletteFalse = false;

	addEnabledFalse = false;
	addMax = 1.0;
	addSpread = 1.0;
	addStartValue = 0.0;

	parabEnabledFalse = false;
	parabScale = 1.0;
	parabStartValue = 0.0;

	cosEnabledFalse = false;
	cosPeriod = 1.0;
	cosAdd = 1.0;
	cosStartValue = 0.0;

	roundEnabledFalse = false;
	roundScale = 1.0;

	maxColorValue = 100000.0;
	minColorValue = 0.0;
}

double CalculateColorIndex(bool isHybrid, double r, CVector4 z, double minimumR,
	const sExtendedAux &extendedAux, const sFractalColoring &fractalColoring,
	fractal::enumColoringFunction coloringFunction, const sFractal *defaultFractal)
{
	double colorIndex = 0.0;

	double minR1000 = minimumR * 1000.0;								 // limited at 100,000 hybrid mode
	double minR5000 = minimumR * 5000.0;								 // DEFAULT
	double auxColorValue100 = extendedAux.color * 100.0; // limited at 100,000,
	double radDE5000 = 0.0;
	double rad1000 = 0.0;
	double colorValue = 0.0;

	// HYBRID MODE
	if (isHybrid)
	{
		//*new hybrid*
		if (fractalColoring.extraColorEnabledFalse)
		{
			colorValue = fractalColoring.initialColorValue;
			if (fractalColoring.initCondFalse)
			{
				double initColorValue = 0.0;
				CVector3 xyzC = CVector3(extendedAux.c.x, extendedAux.c.y, extendedAux.c.z);
				if (fractalColoring.icRadFalse)
					initColorValue = xyzC.Length() * fractalColoring.icRadWeight;

				if (fractalColoring.icXYZFalse)
				{
					if (fractalColoring.icFabsFalse)
					{
						xyzC = xyzC * fractalColoring.xyzC111;
					}
					else
					{
						xyzC = fabs(xyzC) * fractalColoring.xyzC111;
					}
					initColorValue += xyzC.x + xyzC.y + xyzC.z;
				}
				colorValue += initColorValue;
			}

			if (fractalColoring.orbitTrapTrue) colorValue += minimumR * fractalColoring.orbitTrapWeight;

			if (fractalColoring.auxColorFalse)
				colorValue += extendedAux.color * fractalColoring.auxColorWeight // aux.color
											+ extendedAux.colorHybrid // transf_hybrid_color inputs
													* fractalColoring.auxColorHybridWeight;

			if (fractalColoring.radFalse)
			{
				rad1000 = r;
				if (fractalColoring.radDiv1e13False) rad1000 /= 1e13;
				if (fractalColoring.radSquaredFalse) rad1000 = r * r;

				colorValue += rad1000 * fractalColoring.radWeight;
			}

			if (fractalColoring.radDivDeFalse)
			{
				double distEst = fabs(extendedAux.DE);
				if (fractalColoring.radDivLogDeFalse) distEst = fabs(extendedAux.r_dz);
				radDE5000 = r; // r /DE // was named r2
				if (fractalColoring.radDivDeSquaredFalse) radDE5000 = r * r;
				colorValue += radDE5000 * fractalColoring.radDivDeWeight / distEst;
			}
			double addValue = 0.0;

			// example of a basic input
			double xyzValue = 0.0;
			if (fractalColoring.xyzBiasEnabledFalse)
			{

				CVector3 xyzAxis = CVector3(z.x, z.y, z.z);
				if (fractalColoring.xyzDiv1e13False) xyzAxis /= 1e13; // mult rounding error ?

				if (fractalColoring.xyzFabsFalse)
				{
					xyzAxis = xyzAxis * fractalColoring.xyz000;
				}
				else
				{
					xyzAxis = fabs(xyzAxis) * fractalColoring.xyz000;
				}
				if (fractalColoring.xyzXSqrdFalse) xyzAxis.x *= xyzAxis.x;
				if (fractalColoring.xyzYSqrdFalse) xyzAxis.y *= xyzAxis.y;
				if (fractalColoring.xyzZSqrdFalse) xyzAxis.z *= xyzAxis.z;

				xyzValue = (xyzAxis.x + xyzAxis.y + xyzAxis.z)
									 * (1.0 + (fractalColoring.xyzIterScale * extendedAux.i));
			}

			addValue += xyzValue; // addValue accumulates outputs

			colorValue += addValue; // all extra inputs

			if (fractalColoring.iterGroupFalse)
			{
				// Iter ADD,  this allows the input to be influenced by iteration number
				if (fractalColoring.iterAddScaleTrue && extendedAux.i > fractalColoring.iStartValue)
				{
					int iUse = extendedAux.i - fractalColoring.iStartValue;
					colorValue += fractalColoring.iterAddScale * iUse;
				}

				// Iter SCALE,
				if (fractalColoring.iterScaleFalse && extendedAux.i >= fractalColoring.iStartValue)
				{
					int iUse = extendedAux.i - fractalColoring.iStartValue;
					colorValue *= (iUse * fractalColoring.iterScale) + 1.0;
				}
			}
			// "pseudo" global palette controls
			// colorValue /= 256.0;

			if (fractalColoring.globalPaletteFalse)
			{
				if (fractalColoring.addEnabledFalse)
				{ // add curve inv
					if (colorValue > fractalColoring.addStartValue)
					{
						colorValue += (1.0
														- 1.0 / (1.0
																			+ (colorValue - fractalColoring.addStartValue)
																					/ fractalColoring.addSpread))
													* fractalColoring.addMax;
					}
				}

				if (fractalColoring.parabEnabledFalse)
				{
					if (colorValue > fractalColoring.parabStartValue)
					{
						double parab = colorValue - fractalColoring.cosStartValue;
						parab = parab * parab * fractalColoring.parabScale;
						colorValue += parab;
					}
				}

				if (fractalColoring.cosEnabledFalse)
				{ // trig palette
					if (colorValue > fractalColoring.cosStartValue)
					{
						double trig = (0.5
														- 0.5 * cos((colorValue - fractalColoring.cosStartValue) * M_PI
																				/ (fractalColoring.cosPeriod * 2.0)))
													* fractalColoring.cosAdd;
						colorValue += trig;
					}
				}

				if (fractalColoring.roundEnabledFalse)
				{
					double roundScale = fractalColoring.roundScale;
					colorValue /= roundScale;
					colorValue = round(colorValue) * roundScale;
				}
			}

			double minCV = fractalColoring.minColorValue;
			double maxCV = fractalColoring.maxColorValue;
			if (colorValue < minCV) colorValue = minCV;
			if (colorValue > maxCV) colorValue = maxCV;

			colorIndex = colorValue * 256.0; // convert to colorValue units
		}
		else
		{
			//*old hybrid*
			minR1000 = min(minR1000, 1e5);
			// if (minR1000 > 100000.0) minR1000 = 100000.0; // limit is only in old hybrid mode?
			if (auxColorValue100 > 1e5) auxColorValue100 = 1e5; // limit
			radDE5000 = 5000.0 * r / fabs(extendedAux.DE);			// was named r2
			if (radDE5000 > 1e5) radDE5000 = 1e5;
			double oldHybridValue = (minR1000 + auxColorValue100 + radDE5000); // old hybrid code
			colorIndex = oldHybridValue;
		}
	}
	// NORMAL MODE
	else
	{
		switch (coloringFunction)
		{
			case coloringFunctionABox:
				colorIndex =
					auxColorValue100																		 // aux.color
					+ r * defaultFractal->mandelbox.color.factorR / 1e13 // radius scale
					+ ((fractalColoring.coloringAlgorithm != fractalColoring_Standard) ? minR1000 : 0.0);
				break;

			case coloringFunctionIFS: colorIndex = minR1000; break;

			case coloringFunctionAmazingSurf: colorIndex = minR1000 * 0.2; break;

			case coloringFunctionABox2:
				radDE5000 = 5000.0 * r / fabs(extendedAux.DE);					 // was named r2
				colorIndex = minR1000 * extendedAux.minRFactor					 // orbit trap DEFAULT
										 + auxColorValue100 * extendedAux.foldFactor // aux.color
										 + r * extendedAux.radiusFactor / 1e13			 // radius// this may be replaced
										 + radDE5000 * extendedAux.scaleFactor; // r/DE  for backwards compatibility
				// + addValue;													 // all extra inputs

				/*+ ((fractalColoring.coloringAlgorithm != fractalColoring_Standard)
								? minimumR * extendedAux.minRFactor * 1000.0
								: 0.0);*/ // temp
				// removed
				break;

			case coloringFunctionDonut: colorIndex = auxColorValue100 * 20.0 / (extendedAux.i + 1); break;

			case coloringFunctionDefault: colorIndex = minR5000; break;

			case coloringFunctionGeneral:
				if (fractalColoring.extraColorEnabledFalse)
				{
					colorValue += fractalColoring.initialColorValue;
					if (fractalColoring.initCondFalse)
					{
						double initColorValue = 0.0;
						CVector3 xyzC = CVector3(extendedAux.c.x, extendedAux.c.y, extendedAux.c.z);
						if (fractalColoring.icRadFalse)
							initColorValue = xyzC.Length() * fractalColoring.icRadWeight;

						if (fractalColoring.icXYZFalse)
						{

							if (fractalColoring.icFabsFalse)
							{
								xyzC = xyzC * fractalColoring.xyzC111;
							}
							else
							{
								xyzC = fabs(xyzC) * fractalColoring.xyzC111;
							}
							initColorValue += xyzC.x + xyzC.y + xyzC.z;
						}
						colorValue += initColorValue;
					}

					if (fractalColoring.orbitTrapTrue)
						colorValue += minimumR * fractalColoring.orbitTrapWeight;

					if (fractalColoring.auxColorFalse)
						colorValue += extendedAux.color * fractalColoring.auxColorWeight // aux.color
													+ extendedAux.colorHybrid // transf_hybrid_color inputs
															* fractalColoring.auxColorHybridWeight;

					if (fractalColoring.radFalse)
					{
						rad1000 = r;
						if (fractalColoring.radSquaredFalse) rad1000 = r * r;
						colorValue += rad1000 * fractalColoring.radWeight;
					}

					if (fractalColoring.radDivDeFalse)
					{
						double distEst = fabs(extendedAux.DE);
						if (fractalColoring.radDivLogDeFalse) distEst = fabs(extendedAux.r_dz);
						radDE5000 = r; // r /DE // was named r2
						if (fractalColoring.radDivDeSquaredFalse) radDE5000 = r * r;
						colorValue += radDE5000 * fractalColoring.radDivDeWeight / distEst;
					}
					double addValue = 0.0;

					// example of a basic input
					double xyzValue = 0.0;
					if (fractalColoring.xyzBiasEnabledFalse)
					{
						CVector3 xyzAxis;
						xyzAxis = fabs(CVector3(z.x, z.y, z.z)) * fractalColoring.xyz000;

						if (fractalColoring.xyzXSqrdFalse) xyzAxis.x *= xyzAxis.x;
						if (fractalColoring.xyzYSqrdFalse) xyzAxis.y *= xyzAxis.y;
						if (fractalColoring.xyzZSqrdFalse) xyzAxis.z *= xyzAxis.z;

						xyzValue = (xyzAxis.x + xyzAxis.y + xyzAxis.z)
											 * (1.0 + (fractalColoring.xyzIterScale * extendedAux.i));
					}

					addValue += xyzValue; // addValue accumulates outputs

					colorValue += addValue; // all extra inputs

					if (fractalColoring.iterGroupFalse)
					{
						// Iter ADD,  this allows the input to be influenced by iteration number
						if (fractalColoring.iterAddScaleTrue && extendedAux.i > fractalColoring.iStartValue)
						{
							int iUse = extendedAux.i - fractalColoring.iStartValue;
							colorValue += fractalColoring.iterAddScale * iUse;
						}

						// Iter SCALE,
						if (fractalColoring.iterScaleFalse && extendedAux.i >= fractalColoring.iStartValue)
						{
							int iUse = extendedAux.i - fractalColoring.iStartValue;
							colorValue *= (iUse * fractalColoring.iterScale) + 1.0;
						}
					}

					// "pseudo" global palette controls

					if (fractalColoring.globalPaletteFalse)
					{
						if (fractalColoring.addEnabledFalse)
						{ // add curve inv
							if (colorValue > fractalColoring.addStartValue)
							{
								colorValue += (1.0
																- 1.0 / (1.0
																					+ (colorValue - fractalColoring.addStartValue)
																							/ fractalColoring.addSpread))
															* fractalColoring.addMax;
							}
						}

						if (fractalColoring.parabEnabledFalse)
						{
							if (colorValue > fractalColoring.parabStartValue)
							{
								double parab = colorValue - fractalColoring.cosStartValue;
								parab = parab * parab * fractalColoring.parabScale;
								colorValue += parab;
							}
						}

						if (fractalColoring.cosEnabledFalse)
						{ // trig palette
							if (colorValue > fractalColoring.cosStartValue)
							{
								double trig = (0.5
																- 0.5 * cos((colorValue - fractalColoring.cosStartValue) * M_PI
																						/ (fractalColoring.cosPeriod * 2.0)))
															* fractalColoring.cosAdd;
								colorValue += trig;
							}
						}

						if (fractalColoring.roundEnabledFalse)
						{
							double roundScale = fractalColoring.roundScale;
							colorValue /= roundScale;
							colorValue = round(colorValue) * roundScale;
						}
					}

					double minCV = fractalColoring.minColorValue;
					double maxCV = fractalColoring.maxColorValue;
					if (colorValue < minCV) colorValue = minCV;
					if (colorValue > maxCV) colorValue = maxCV;

					colorIndex = colorValue * 256.0; // convert to colorValue units
				}
				else
				{
					colorIndex = minR5000;
				}
				break;
		}
	}

	return colorIndex;
}
