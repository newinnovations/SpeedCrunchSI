// This file is part of the SpeedCrunch project
// Copyright (C) 2004-2006 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2007, 2009 Wolf Lammen
// Copyright (C) 2007-2009, 2013, 2014 Helder Correia <helder.pereira.correia@gmail.com>
// Copyright (C) 2009 Andreas Scherer <andreas_coder@freenet.de>
// Copyright (C) 2011 Enrico Rós <enrico.ros@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "core/functions.h"

#include "core/settings.h"
#include "math/hmath.h"
#include "math/cmath.h"

#include <QCoreApplication>
#include <QHash>

#include <algorithm>
#include <functional>
#include <cfloat>
#include <cmath>
#include <numeric>

#define FUNCTION_INSERT(ID) insert(new Function(#ID, function_ ## ID, this))
#define FUNCTION_USAGE(ID, USAGE) find(#ID)->setUsage(QString::fromLatin1(USAGE));
#define FUNCTION_USAGE_TR(ID, USAGE) find(#ID)->setUsage(USAGE);
#define FUNCTION_NAME(ID, NAME) find(#ID)->setName(NAME)

#define ENSURE_POSITIVE_ARGUMENT_COUNT() \
    if (args.count() < 1) { \
        f->setError(InvalidParamCount); \
        return CMath::nan(InvalidParamCount); \
    }

#define ENSURE_ARGUMENT_COUNT(i) \
    if (args.count() != (i)) { \
        f->setError(InvalidParamCount); \
        return CMath::nan(InvalidParamCount); \
    }

#define ENSURE_EITHER_ARGUMENT_COUNT(i, j) \
    if (args.count() != (i) && args.count() != (j)) { \
        f->setError(InvalidParamCount); \
        return CMath::nan(InvalidParamCount); \
    }

#define ENSURE_SAME_DIMENSION() \
    /*for(int i=0; i<args.count()-1; ++i) { \
        if(!args.at(i).sameDimension(args.at((i)+1))) \
            return CMath::nan(InvalidDimension);\
    }*/

#define ENSURE_REAL_ARGUMENT(i) \
    if (!args[i].isReal()) { \
        f->setError(OutOfDomain); \
        return CMath::nan(); \
    }

#define ENSURE_REAL_ARGUMENTS() \
    for (int i = 0; i < args.count(); i++) { \
        ENSURE_REAL_ARGUMENT(i); \
    }

#define CONVERT_ARGUMENT_ANGLE(angle) \
    if (Settings::instance()->angleUnit == 'd') { \
        if (angle.isReal()) \
            angle = CMath::deg2rad(angle); \
        else { \
            f->setError(OutOfDomain); \
            return CMath::nan(); \
        } \
    }

#define CONVERT_RESULT_ANGLE(result) \
    if (Settings::instance()->angleUnit == 'd') { \
        if (result.isReal()) \
            result = CMath::rad2deg(result); \
	else { \
            f->setError(OutOfDomain); \
            return CMath::nan(); \
        } \
    }

#define CPLX_WRAP_1(fct, arg)			\
  if (Settings::instance()->complexNumbers)	\
    return CMath::fct(arg);			\
  else						\
    return CNumber(HMath::fct(arg.real));

static FunctionRepo* s_FunctionRepoInstance = 0;

// FIXME: destructor seems not to be called
static void s_deleteFunctions()
{
    delete s_FunctionRepoInstance;
}

CNumber Function::exec(const Function::ArgumentList& args)
{
    if (!m_ptr)
        return CMath::nan();
    setError(Success);
    CNumber result = (*m_ptr)(this, args);
    if(result.error())
        setError(result.error());
    return result;
}

CNumber function_abs(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(abs, args.at(0));
}

CNumber function_average(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return std::accumulate(args.begin()+1, args.end(), *args.begin()) / CNumber(args.count());
}

CNumber function_absdev(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    CNumber mean = function_average(f, args);
    if (mean.isNan())
        return mean;   // pass the error along
    CNumber acc = 0;
    for (int i = 0; i < args.count(); ++i)
        acc += CMath::abs(args.at(i) - mean);
    return acc / CNumber(args.count());
}

CNumber function_int(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(integer, args.at(0));
}

CNumber function_trunc(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(1, 2);
    CNumber num = args.at(0);
    if (args.count() == 2) {
        CNumber argprec = args.at(1);
        if (argprec != 0) {
            if (!argprec.isInteger()) {
                f->setError(OutOfDomain);
                return CMath::nan();
            }
            int prec = argprec.toInt();
            if (prec)
                return CMath::trunc(num, prec);
            // The second parameter exceeds the integer limits.
            if (argprec < 0)
                return CNumber(0);
            return num;
        }
    }
    return CMath::trunc(num);
}

CNumber function_frac(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(frac, args.at(0));
}

CNumber function_floor(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(floor, args.at(0));
}

CNumber function_ceil(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(ceil, args.at(0));
}

CNumber function_gcd(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    for (int i = 0; i < args.count(); ++i)
        if (!args.at(i).isInteger()) {
            f->setError(OutOfDomain);
            return CMath::nan();
        }
    return std::accumulate(args.begin() + 1, args.end(), args.at(0), CMath::gcd);
}

CNumber function_round(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(1, 2);
    CNumber num = args.at(0);
    if (args.count() == 2) {
        CNumber argPrecision = args.at(1);
        if (argPrecision != 0) {
            if (!argPrecision.isInteger()) {
                f->setError(OutOfDomain);
                return CMath::nan();
            }
            int prec = argPrecision.toInt();
            if (prec)
                return CMath::round(num, prec);
            // The second parameter exceeds the integer limits.
            if (argPrecision < 0)
                return CNumber(0);
            return num;
        }
    }
    return CMath::round(num);
}

CNumber function_sqrt(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(sqrt, args.at(0));
}

CNumber function_variance(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();

    CNumber mean = function_average(f, args);
    if (mean.isNan())
        return mean;

    CNumber acc = (args.first() - mean);
    acc *= acc;
    for (int i = 1; i < args.count(); ++i)
        acc += (args.at(i) - mean) * (args.at(i) - mean);

    return acc / CNumber(args.count());
}

CNumber function_stddev(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return CMath::sqrt(function_variance(f, args));
}

CNumber function_cbrt(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(cbrt, args.at(0));
}

CNumber function_exp(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(exp, args.at(0));
}

CNumber function_ln(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(ln, args.at(0));
}

CNumber function_lg(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(lg, args.at(0));
}

CNumber function_lb(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(lb, args.at(0));
}

CNumber function_log(Function* f, const Function::ArgumentList& args)
{
     /* TODO : complex mode switch for this function */
     ENSURE_ARGUMENT_COUNT(2);
     return CMath::log(args.at(0), args.at(1));
}

CNumber function_sin(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    CPLX_WRAP_1(sin, angle);
}

CNumber function_cos(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return CMath::cos(angle);
}

CNumber function_tan(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    CPLX_WRAP_1(tan, angle);
}

CNumber function_cot(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    CPLX_WRAP_1(cot, angle);
}

CNumber function_sec(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    CPLX_WRAP_1(sec, angle);
}

CNumber function_csc(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    CPLX_WRAP_1(csc, angle);
}

CNumber function_arcsin(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber result;
    if (Settings::instance()->complexNumbers)
      result = CMath::arcsin(args.at(0));
    else
      result = CNumber( HMath::arcsin( args.at(0).real ) );
    CONVERT_RESULT_ANGLE(result);
    return result;
}

CNumber function_arccos(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber result;
    if (Settings::instance()->complexNumbers)
      result = CMath::arccos(args.at(0));
    else
      result = CNumber( HMath::arccos( args.at(0).real ) );
    CONVERT_RESULT_ANGLE(result);
    return result;
}

CNumber function_arctan(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CNumber result;
    if (Settings::instance()->complexNumbers)
      result = CMath::arctan(args.at(0));
    else
      result = CNumber( HMath::arctan( args.at(0).real ) );
    CONVERT_RESULT_ANGLE(result);
    return result;
}

CNumber function_sinh(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(sinh, args.at(0));
}

CNumber function_cosh(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(cosh, args.at(0));
}

CNumber function_tanh(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(tanh, args.at(0));
}

CNumber function_arsinh(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(arsinh, args.at(0));
}

CNumber function_arcosh(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(arcosh, args.at(0));
}

CNumber function_artanh(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    CPLX_WRAP_1(artanh, args.at(0));
}

CNumber function_erf(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(erf, args.at(0));
}

CNumber function_erfc(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(erfc, args.at(0));
}

CNumber function_gamma(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(gamma, args.at(0));
}

CNumber function_lngamma(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(lnGamma, args.at(0));
}

CNumber function_sgn(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(sgn, args.at(0));
}

CNumber function_ncr(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    ENSURE_REAL_ARGUMENTS();
    return CMath::nCr(args.at(0), args.at(1));
}

CNumber function_npr(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    ENSURE_REAL_ARGUMENTS();
    return CMath::nPr(args.at(0), args.at(1));
}

CNumber function_degrees(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(rad2deg, args.at(0));
}

CNumber function_radians(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    CPLX_WRAP_1(deg2rad, args.at(0));
}

CNumber function_max(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    ENSURE_REAL_ARGUMENTS()
    ENSURE_SAME_DIMENSION();
    return *std::max_element(args.begin(), args.end());
}

CNumber function_median(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    ENSURE_REAL_ARGUMENTS()
    ENSURE_SAME_DIMENSION();

    Function::ArgumentList sortedArgs = args;
    std::sort(sortedArgs.begin(), sortedArgs.end());

    if ((args.count() & 1) == 1)
        return sortedArgs.at((args.count() - 1) / 2);

    const int centerLeft = args.count() / 2 - 1;
    return (sortedArgs.at(centerLeft) + sortedArgs.at(centerLeft + 1)) / CNumber(2);
}

CNumber function_min(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    ENSURE_REAL_ARGUMENTS()
    ENSURE_SAME_DIMENSION();
    return *std::min_element(args.begin(), args.end());
}

CNumber function_sum(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return std::accumulate(args.begin(), args.end(), CNumber(0));
}

CNumber function_product(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return std::accumulate(args.begin(), args.end(), CNumber(1), std::multiplies<CNumber>());
}

CNumber function_geomean(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();

    CNumber result = std::accumulate(args.begin(), args.end(), CNumber(1),
        std::multiplies<CNumber>());

    if (result <= CNumber(0))
        return CNumber("NaN");

    if (args.count() == 1)
        return result;

    if (args.count() == 2)
        return CMath::sqrt(result);

    CNumber nominal = result;
    result = CMath::raise(result/nominal, CNumber(1)/CNumber(args.count()));
    return CMath::exp(CMath::ln(nominal) / CNumber(args.count()))*result;
}

CNumber function_dec(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CNumber(args.at(0)).setFormat('g');
}

CNumber function_hex(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CNumber(args.at(0)).setFormat('h');
}

CNumber function_oct(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CNumber(args.at(0)).setFormat('o');
}

CNumber function_bin(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CNumber(args.at(0)).setFormat('b');
}

CNumber function_binompmf(Function* f, const Function::ArgumentList& args)
{
    ENSURE_ARGUMENT_COUNT(3);
    return CMath::binomialPmf(args.at(0), args.at(1), args.at(2));
}

CNumber function_binomcdf(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(3);
    return CMath::binomialCdf(args.at(0), args.at(1), args.at(2));
}

CNumber function_binommean(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::binomialMean(args.at(0), args.at(1));
}

CNumber function_binomvar(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::binomialVariance(args.at(0), args.at(1));
}

CNumber function_hyperpmf(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(4);
    return CMath::hypergeometricPmf(args.at(0), args.at(1), args.at(2), args.at(3));
}

CNumber function_hypercdf(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(4);
    return CMath::hypergeometricCdf(args.at(0), args.at(1), args.at(2), args.at(3));
}

CNumber function_hypermean(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(3);
    return CMath::hypergeometricMean(args.at(0), args.at(1), args.at(2));
}

CNumber function_hypervar(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(3);
    return CMath::hypergeometricVariance(args.at(0), args.at(1), args.at(2));
}

CNumber function_poipmf(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::poissonPmf(args.at(0), args.at(1));
}

CNumber function_poicdf(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::poissonCdf(args.at(0), args.at(1));
}

CNumber function_poimean(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::poissonMean(args.at(0));
}

CNumber function_poivar(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::poissonVariance(args.at(0));
}

CNumber function_mask(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::mask(args.at(0), args.at(1));
}

CNumber function_unmask(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::sgnext(args.at(0), args.at(1));
}

CNumber function_not(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
	return ~args.at(0);
}

CNumber function_and(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return std::accumulate(args.begin(), args.end(), CNumber(-1),
        std::mem_fun_ref(&CNumber::operator&));
}

CNumber function_or(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return std::accumulate(args.begin(), args.end(), CNumber(0),
        std::mem_fun_ref(&CNumber::operator|));
}

CNumber function_xor(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_POSITIVE_ARGUMENT_COUNT();
    return std::accumulate(args.begin(), args.end(), CNumber(0),
        std::mem_fun_ref(&CNumber::operator^));
}

CNumber function_shl(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::ashr(args.at(0), -args.at(1));
}

CNumber function_shr(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::ashr(args.at(0), args.at(1));
}

CNumber function_idiv(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return CMath::idiv(args.at(0), args.at(1));
}

CNumber function_mod(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return args.at(0) % args.at(1);
}

CNumber function_ieee754_decode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(3, 4);
    if (args.count() == 3) {
        return CMath::decodeIeee754(args.at(0), args.at(1), args.at(2));
    } else {
        return CMath::decodeIeee754(args.at(0), args.at(1), args.at(2), args.at(3));
    }
}

CNumber function_ieee754_encode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(3, 4);
    if (args.count() == 3) {
        return CMath::encodeIeee754(args.at(0), args.at(1), args.at(2));
    } else {
        return CMath::encodeIeee754(args.at(0), args.at(1), args.at(2), args.at(3));
    }
}

CNumber function_ieee754_half_decode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::decodeIeee754(args.at(0), 5, 10);
}

CNumber function_ieee754_half_encode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::encodeIeee754(args.at(0), 5, 10);
}

CNumber function_ieee754_single_decode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::decodeIeee754(args.at(0), 8, 23);
}

CNumber function_ieee754_single_encode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::encodeIeee754(args.at(0), 8, 23);
}

CNumber function_ieee754_double_decode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::decodeIeee754(args.at(0), 11, 52);
}

CNumber function_ieee754_double_encode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::encodeIeee754(args.at(0), 11, 52);
}

CNumber function_ieee754_quad_decode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::decodeIeee754(args.at(0), 15, 112);
}

CNumber function_ieee754_quad_encode(Function* f, const Function::ArgumentList& args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return CMath::encodeIeee754(args.at(0), 15, 112);
}

void FunctionRepo::createFunctions()
{
    // Analysis.
    FUNCTION_INSERT(abs);
    FUNCTION_INSERT(absdev);
    FUNCTION_INSERT(average);
    FUNCTION_INSERT(bin);
    FUNCTION_INSERT(cbrt);
    FUNCTION_INSERT(ceil);
    FUNCTION_INSERT(dec);
    FUNCTION_INSERT(floor);
    FUNCTION_INSERT(frac);
    FUNCTION_INSERT(gamma);
    FUNCTION_INSERT(geomean);
    FUNCTION_INSERT(hex);
    FUNCTION_INSERT(int);
    FUNCTION_INSERT(lngamma);
    FUNCTION_INSERT(max);
    FUNCTION_INSERT(min);
    FUNCTION_INSERT(oct);
    FUNCTION_INSERT(product);
    FUNCTION_INSERT(round);
    FUNCTION_INSERT(sgn);
    FUNCTION_INSERT(sqrt);
    FUNCTION_INSERT(stddev);
    FUNCTION_INSERT(sum);
    FUNCTION_INSERT(trunc);
    FUNCTION_INSERT(variance);

    // Discrete.
    FUNCTION_INSERT(gcd);
    FUNCTION_INSERT(ncr);
    FUNCTION_INSERT(npr);

    // Probability.
    FUNCTION_INSERT(binomcdf);
    FUNCTION_INSERT(binommean);
    FUNCTION_INSERT(binompmf);
    FUNCTION_INSERT(binomvar);
    FUNCTION_INSERT(erf);
    FUNCTION_INSERT(erfc);
    FUNCTION_INSERT(hypercdf);
    FUNCTION_INSERT(hypermean);
    FUNCTION_INSERT(hyperpmf);
    FUNCTION_INSERT(hypervar);
    FUNCTION_INSERT(median);
    FUNCTION_INSERT(poicdf);
    FUNCTION_INSERT(poimean);
    FUNCTION_INSERT(poipmf);
    FUNCTION_INSERT(poivar);

    // Trigonometry.
    FUNCTION_INSERT(arccos);
    FUNCTION_INSERT(arcosh);
    FUNCTION_INSERT(arsinh);
    FUNCTION_INSERT(artanh);
    FUNCTION_INSERT(arcsin);
    FUNCTION_INSERT(arctan);
    FUNCTION_INSERT(cos);
    FUNCTION_INSERT(cosh);
    FUNCTION_INSERT(cot);
    FUNCTION_INSERT(csc);
    FUNCTION_INSERT(degrees);
    FUNCTION_INSERT(exp);
    FUNCTION_INSERT(lb);
    FUNCTION_INSERT(lg);
    FUNCTION_INSERT(ln);
    FUNCTION_INSERT(log);
    FUNCTION_INSERT(radians);
    FUNCTION_INSERT(sec);
    FUNCTION_INSERT(sin);
    FUNCTION_INSERT(sinh);
    FUNCTION_INSERT(tan);
    FUNCTION_INSERT(tanh);

    // Logic.
    FUNCTION_INSERT(mask);
    FUNCTION_INSERT(unmask);
    FUNCTION_INSERT(not);
    FUNCTION_INSERT(and);
    FUNCTION_INSERT(or);
    FUNCTION_INSERT(xor);
    FUNCTION_INSERT(shl);
    FUNCTION_INSERT(shr);
    FUNCTION_INSERT(idiv);
    FUNCTION_INSERT(mod);

    // IEEE-754.
    FUNCTION_INSERT(ieee754_decode);
    FUNCTION_INSERT(ieee754_encode);
    FUNCTION_INSERT(ieee754_half_decode);
    FUNCTION_INSERT(ieee754_half_encode);
    FUNCTION_INSERT(ieee754_single_decode);
    FUNCTION_INSERT(ieee754_single_encode);
    FUNCTION_INSERT(ieee754_double_decode);
    FUNCTION_INSERT(ieee754_double_encode);
    FUNCTION_INSERT(ieee754_quad_decode);
    FUNCTION_INSERT(ieee754_quad_encode);
}

FunctionRepo* FunctionRepo::instance()
{
    if (!s_FunctionRepoInstance) {
        s_FunctionRepoInstance = new FunctionRepo;
        qAddPostRoutine(s_deleteFunctions);
    }
    return s_FunctionRepoInstance;
}

FunctionRepo::FunctionRepo()
{
    createFunctions();
    setNonTranslatableFunctionUsages();
    retranslateText();
}

void FunctionRepo::insert(Function* function)
{
    if (!function)
        return;
    m_functions.insert(function->identifier().toUpper(), function);
}

Function* FunctionRepo::find(const QString& identifier) const
{
    if (identifier.isNull())
        return 0;
    return m_functions.value(identifier.toUpper(), 0);
}

QStringList FunctionRepo::getIdentifiers() const
{
    QStringList result = m_functions.keys();
    std::transform(result.begin(), result.end(), result.begin(), [](const QString& s) { return s.toLower(); });
    return result;
}

void FunctionRepo::setNonTranslatableFunctionUsages()
{
    FUNCTION_USAGE(abs, "x");
    FUNCTION_USAGE(absdev, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(arccos, "x");
    FUNCTION_USAGE(and, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(arcosh, "x");
    FUNCTION_USAGE(arsinh, "x");
    FUNCTION_USAGE(artanh, "x");
    FUNCTION_USAGE(arcsin, "x");
    FUNCTION_USAGE(arctan, "x");
    FUNCTION_USAGE(average, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(bin, "n");
    FUNCTION_USAGE(cbrt, "x");
    FUNCTION_USAGE(ceil, "x");
    FUNCTION_USAGE(cos, "x");
    FUNCTION_USAGE(cosh, "x");
    FUNCTION_USAGE(cot, "x");
    FUNCTION_USAGE(csc, "x");
    FUNCTION_USAGE(dec, "x");
    FUNCTION_USAGE(degrees, "x");
    FUNCTION_USAGE(erf, "x");
    FUNCTION_USAGE(erfc, "x");
    FUNCTION_USAGE(exp, "x");
    FUNCTION_USAGE(floor, "x");
    FUNCTION_USAGE(frac, "x");
    FUNCTION_USAGE(gamma, "x");
    FUNCTION_USAGE(gcd, "n<sub>1</sub>; n<sub>2</sub>; ...");
    FUNCTION_USAGE(geomean, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(hex, "n");
    FUNCTION_USAGE(ieee754_half_decode, "x");
    FUNCTION_USAGE(ieee754_half_encode, "x");
    FUNCTION_USAGE(ieee754_single_decode, "x");
    FUNCTION_USAGE(ieee754_single_encode, "x");
    FUNCTION_USAGE(ieee754_double_decode, "x");
    FUNCTION_USAGE(ieee754_double_encode, "x");
    FUNCTION_USAGE(ieee754_quad_decode, "x");
    FUNCTION_USAGE(ieee754_quad_encode, "x");
    FUNCTION_USAGE(int, "x");
    FUNCTION_USAGE(lb, "x");
    FUNCTION_USAGE(lg, "x");
    FUNCTION_USAGE(ln, "x");
    FUNCTION_USAGE(lngamma, "x");
    FUNCTION_USAGE(max, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(median, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(min, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(ncr, "x<sub>1</sub>; x<sub>2</sub>");
    FUNCTION_USAGE(not, "n");
    FUNCTION_USAGE(npr, "x<sub>1</sub>; x<sub>2</sub>");
    FUNCTION_USAGE(oct, "n");
    FUNCTION_USAGE(or, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(product, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(radians, "x");
    FUNCTION_USAGE(round, "x");
    FUNCTION_USAGE(sec, "x)");
    FUNCTION_USAGE(sgn, "x");
    FUNCTION_USAGE(sin, "x");
    FUNCTION_USAGE(sinh, "x");
    FUNCTION_USAGE(sqrt, "x");
    FUNCTION_USAGE(stddev, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(sum, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(tan, "x");
    FUNCTION_USAGE(tanh, "x");
    FUNCTION_USAGE(trunc, "x");
    FUNCTION_USAGE(variance, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(xor, "x<sub>1</sub>; x<sub>2</sub>; ...");
}

void FunctionRepo::setTranslatableFunctionUsages()
{
    FUNCTION_USAGE_TR(binomcdf, tr("max; trials; probability"));
    FUNCTION_USAGE_TR(binommean, tr("trials; probability"));
    FUNCTION_USAGE_TR(binompmf, tr("hits; trials; probability"));
    FUNCTION_USAGE_TR(binomvar, tr("trials; probability"));
    FUNCTION_USAGE_TR(hypercdf, tr("max; total; hits; trials"));
    FUNCTION_USAGE_TR(hypermean, tr("total; hits; trials"));
    FUNCTION_USAGE_TR(hyperpmf, tr("count; total; hits; trials"));
    FUNCTION_USAGE_TR(hypervar, tr("total; hits; trials"));
    FUNCTION_USAGE_TR(idiv, tr("dividend; divisor"));
    FUNCTION_USAGE_TR(ieee754_decode, tr("x; exponent_bits; significand_bits [; exponent_bias]"));
    FUNCTION_USAGE_TR(ieee754_encode, tr("x; exponent_bits; significand_bits [; exponent_bias]"));
    FUNCTION_USAGE_TR(log, tr("base; x"));
    FUNCTION_USAGE_TR(mask, tr("n; bits"));
    FUNCTION_USAGE_TR(mod, tr("value; modulo"));
    FUNCTION_USAGE_TR(poicdf, tr("events; average_events"));
    FUNCTION_USAGE_TR(poimean, tr("average_events"));
    FUNCTION_USAGE_TR(poipmf, tr("events; average_events"));
    FUNCTION_USAGE_TR(poivar, tr("average_events"));
    FUNCTION_USAGE_TR(shl, tr("n; bits"));
    FUNCTION_USAGE_TR(shr, tr("n; bits"));
    FUNCTION_USAGE_TR(unmask, tr("n; bits"));
}

void FunctionRepo::setFunctionNames()
{
    FUNCTION_NAME(abs, tr("Absolute Value"));
    FUNCTION_NAME(absdev, tr("Absolute Deviation"));
    FUNCTION_NAME(arccos, tr("Arc Cosine"));
    FUNCTION_NAME(and, tr("Logical AND"));
    FUNCTION_NAME(arcosh, tr("Area Hyperbolic Cosine"));
    FUNCTION_NAME(arsinh, tr("Area Hyperbolic Sine"));
    FUNCTION_NAME(artanh, tr("Area Hyperbolic Tangent"));
    FUNCTION_NAME(arcsin, tr("Arc Sine"));
    FUNCTION_NAME(arctan, tr("Arc Tangent"));
    FUNCTION_NAME(average, tr("Average (Arithmetic Mean)"));
    FUNCTION_NAME(bin, tr("Convert to Binary Representation"));
    FUNCTION_NAME(binomcdf, tr("Binomial Cumulative Distribution Function"));
    FUNCTION_NAME(binommean, tr("Binomial Distribution Mean"));
    FUNCTION_NAME(binompmf, tr("Binomial Probability Mass Function"));
    FUNCTION_NAME(binomvar, tr("Binomial Distribution Variance"));
    FUNCTION_NAME(cbrt, tr("Cube Root"));
    FUNCTION_NAME(ceil, tr("Ceiling"));
    FUNCTION_NAME(cos, tr("Cosine"));
    FUNCTION_NAME(cosh, tr("Hyperbolic Cosine"));
    FUNCTION_NAME(cot, tr("Cotangent"));
    FUNCTION_NAME(csc, tr("Cosecant"));
    FUNCTION_NAME(dec, tr("Convert to Decimal Representation"));
    FUNCTION_NAME(degrees, tr("Degrees of Arc"));
    FUNCTION_NAME(erf, tr("Error Function"));
    FUNCTION_NAME(erfc, tr("Complementary Error Function"));
    FUNCTION_NAME(exp, tr("Exponential"));
    FUNCTION_NAME(floor, tr("Floor"));
    FUNCTION_NAME(frac, tr("Fractional Part"));
    FUNCTION_NAME(gamma, tr("Extension of Factorials [= (x-1)!]"));
    FUNCTION_NAME(gcd, tr("Greatest Common Divisor"));
    FUNCTION_NAME(geomean, tr("Geometric Mean"));
    FUNCTION_NAME(hex, tr("Convert to Hexadecimal Representation"));
    FUNCTION_NAME(hypercdf, tr("Hypergeometric Cumulative Distribution Function"));
    FUNCTION_NAME(hypermean, tr("Hypergeometric Distribution Mean"));
    FUNCTION_NAME(hyperpmf, tr("Hypergeometric Probability Mass Function"));
    FUNCTION_NAME(hypervar, tr("Hypergeometric Distribution Variance"));
    FUNCTION_NAME(idiv, tr("Integer Quotient"));
    FUNCTION_NAME(int, tr("Integer Part"));
    FUNCTION_NAME(ieee754_decode, tr("Decode IEEE-754 Binary Value"));
    FUNCTION_NAME(ieee754_encode, tr("Encode IEEE-754 Binary Value"));
    FUNCTION_NAME(ieee754_half_decode, tr("Decode 16-bit Half-Precision Value"));
    FUNCTION_NAME(ieee754_half_encode, tr("Encode 16-bit Half-Precision Value"));
    FUNCTION_NAME(ieee754_single_decode, tr("Decode 32-bit Single-Precision Value"));
    FUNCTION_NAME(ieee754_single_encode, tr("Encode 32-bit Single-Precision Value"));
    FUNCTION_NAME(ieee754_double_decode, tr("Decode 64-bit Double-Precision Value"));
    FUNCTION_NAME(ieee754_double_encode, tr("Encode 64-bit Double-Precision Value"));
    FUNCTION_NAME(ieee754_quad_decode, tr("Decode 128-bit Quad-Precision Value"));
    FUNCTION_NAME(ieee754_quad_encode, tr("Encode 128-bit Quad-Precision Value"));
    FUNCTION_NAME(lb, tr("Binary Logarithm"));
    FUNCTION_NAME(lg, tr("Common Logarithm"));
    FUNCTION_NAME(ln, tr("Natural Logarithm"));
    FUNCTION_NAME(lngamma, "ln(abs(Gamma))");
    FUNCTION_NAME(log, tr("Logarithm to Arbitrary Base"));
    FUNCTION_NAME(mask, tr("Mask to a bit size"));
    FUNCTION_NAME(max, tr("Maximum"));
    FUNCTION_NAME(median, tr("Median Value (50th Percentile)"));
    FUNCTION_NAME(min, tr("Minimum"));
    FUNCTION_NAME(mod, tr("Modulo"));
    FUNCTION_NAME(ncr, tr("Combination (Binomial Coefficient)"));
    FUNCTION_NAME(not, tr("Logical NOT"));
    FUNCTION_NAME(npr, tr("Permutation (Arrangement)"));
    FUNCTION_NAME(oct, tr("Convert to Octal Representation"));
    FUNCTION_NAME(or, tr("Logical OR"));
    FUNCTION_NAME(poicdf, tr("Poissonian Cumulative Distribution Function"));
    FUNCTION_NAME(poimean, tr("Poissonian Distribution Mean"));
    FUNCTION_NAME(poipmf, tr("Poissonian Probability Mass Function"));
    FUNCTION_NAME(poivar, tr("Poissonian Distribution Variance"));
    FUNCTION_NAME(product, tr("Product"));
    FUNCTION_NAME(radians, tr("Radians"));
    FUNCTION_NAME(round, tr("Rounding"));
    FUNCTION_NAME(sec, tr("Secant"));
    FUNCTION_NAME(shl, tr("Arithmetic Shift Left"));
    FUNCTION_NAME(shr, tr("Arithmetic Shift Right"));
    FUNCTION_NAME(sgn, tr("Signum"));
    FUNCTION_NAME(sin, tr("Sine"));
    FUNCTION_NAME(sinh, tr("Hyperbolic Sine"));
    FUNCTION_NAME(sqrt, tr("Square Root"));
    FUNCTION_NAME(stddev, tr("Standard Deviation (Square Root of Variance)"));
    FUNCTION_NAME(sum, tr("Sum"));
    FUNCTION_NAME(tan, tr("Tangent"));
    FUNCTION_NAME(tanh, tr("Hyperbolic Tangent"));
    FUNCTION_NAME(trunc, tr("Truncation"));
    FUNCTION_NAME(unmask, tr("Sign-extend a value"));
    FUNCTION_NAME(variance, tr("Variance"));
    FUNCTION_NAME(xor, tr("Logical XOR"));
}

void FunctionRepo::retranslateText()
{
    setFunctionNames();
    setTranslatableFunctionUsages();
}
