/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * z(n+1) = z(n) * abs(z(n)) ^ p-1

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TransfRPowerIteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfRPowerIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL rp = native_powr(aux->r, fractal->bulb.power - 1.0f);
	aux->r_dz = mad(rp * aux->r_dz, fractal->bulb.power, 1.0f);
	z *= rp;
	aux->DE *= rp;
	return z;
}