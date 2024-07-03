#include "HSWaveMath.hpp"

double HSWaveMath_Sinc ( double x ){
	if ( x == 0 ) return 1;
	return sin ( x ) / x;
}


