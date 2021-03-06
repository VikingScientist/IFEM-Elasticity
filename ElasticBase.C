// $Id$
//==============================================================================
//!
//! \file ElasticBase.C
//!
//! \date Jul 04 2014
//!
//! \author Knut Morten Okstad / SINTEF
//!
//! \brief Base class representing FEM integrands for elasticity problems.
//!
//==============================================================================

#include "ElasticBase.h"
#include "TimeDomain.h"
#include "NewmarkMats.h"


ElasticBase::ElasticBase ()
{
  nSV = 1; // Default number of solution vectors in core

  eM = eKm = eKg = 0;
  eS = iS = 0;

  memset(intPrm,0,sizeof(intPrm));
}


void ElasticBase::setMode (SIM::SolutionMode mode)
{
  m_mode = mode;
  eM = eKm = eKg = 0;
  eS = iS = 0;

  switch (mode)
    {
    case SIM::STATIC:
      eKm = eKg = 1;
      eS  = iS  = 1;
      if (intPrm[3] > 0.0)
        eKg = 0; // Linear analysis, no geometric stiffness
      break;

    case SIM::DYNAMIC:
      eKm = eKg = 3;
      if (intPrm[3] > 0.0)
        eKg = 0; // Linear analysis, no geometric stiffness
      else if (intPrm[3] < 0.0)
        eKg = 4; // Structural damping from material stiffness only
      eM = 2;
      eS = iS = 1;
      if (intPrm[4] == 1.0)
        eS = 3; // Store external and internal forces separately when using HHT
      break;

    case SIM::BUCKLING:
      eKm = 1;
      eKg = 2;
      break;

    case SIM::VIBRATION:
      eM = 2;
    case SIM::STIFF_ONLY:
      eKm = 1;
      break;

    case SIM::MASS_ONLY:
      eM = 1;
      eS = 1;
      break;

    case SIM::RHS_ONLY:
      eS = 1;
    case SIM::INT_FORCES:
      iS = 1;
      break;

    default:
      ;
    }

  switch (mode)
    {
    case SIM::STATIC:
      primsol.resize(nSV);
      break;

    case SIM::DYNAMIC:
      primsol.resize(nSV + (intPrm[4] == 1.0 ? 4 : 2));
      break;

    case SIM::BUCKLING:
    case SIM::RHS_ONLY:
    case SIM::INT_FORCES:
    case SIM::RECOVERY:
      primsol.resize(1);
      break;

    default:
      primsol.clear();
    }
}


void ElasticBase::setIntegrationPrm (unsigned short int i, double prm)
{
  if (i < sizeof(intPrm)/sizeof(double)) intPrm[i] = prm;
}


double ElasticBase::getIntegrationPrm (unsigned short int i) const
{
  if (i < sizeof(intPrm)/sizeof(double) && m_mode == SIM::DYNAMIC)
    return intPrm[i];
  else
    return 0.0;
}


std::string ElasticBase::getField1Name (size_t i, const char* prefix) const
{
  if (i > 6 || i > npv) i = 6;

  static const char* s[7] = { "u_x", "u_y", "u_z", "r_x", "r_y", "r_z",
                              "displacement" };
  if (!prefix) return s[i];

  return prefix + std::string(" ") + s[i];
}


bool ElasticBase::finalizeElement (LocalIntegral& elmInt,
                                   const TimeDomain& time, size_t)
{
  if (m_mode == SIM::DYNAMIC)
    static_cast<NewmarkMats&>(elmInt).setStepSize(time.dt,time.it);

  return true;
}
