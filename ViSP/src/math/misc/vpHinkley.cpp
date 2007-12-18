/****************************************************************************
 *
 * $Id: vpHinkley.cpp,v 1.6 2007-12-18 14:29:03 fspindle Exp $
 *
 * Copyright (C) 1998-2007 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Hinkley's cumulative sum test implementation.
 *
 * Authors:
 * Fabien Spindler
 *
 *****************************************************************************/

/*!

  \file vpHinkley.cpp

  \brief Definition of the vpHinkley class corresponding to the Hinkley's
  cumulative sum test.

*/

/*!
  \class vpHinkley

  \brief This class implements the Hinkley's cumulative sum test.

  \author Fabien Spindler (Fabien.Spindler@irisa.fr), Irisa / Inria Rennes

  The Hinkley's cumulative sum test is designed to detect jump in mean
  of an observed signal \f$ s(t) \f$. It is known to be robust (by
  taking into account all the past of the observed quantity),
  efficient, and inducing a very low computational load. The other
  attractive features of this test are two-fold. First, it can
  straightforwardly and accurately provide the jump instant. Secondly,
  due to its formulation (cumulative sum test), it can simultaneously
  handle both very abrupt and important changes, and gradual smaller
  ones without adapting the involved thresholds.

  Two tests are performed in parallel to look for downwards or upwards
  jumps in \f$ s(t) \f$, respectively defined by:

  \f[ S_k = \sum_{t=0}^{k} (s(t) - m_0 + \frac{\delta}{2}) \f]
  \f[ M_k = \max_{0 \leq i \leq k} S_i\f]
  \f[ T_k = \sum_{t=0}^{k} (s(t) - m_0 - \frac{\delta}{2}) \f]
  \f[ N_k = \max_{0 \leq i \leq k} T_i\f]


  In which \f$m_o\f$ is computed on-line and corresponds to the mean
  of the signal \f$ s(t) \f$ we want to detect a jump. \f$m_o\f$ is
  re-initialized at zero after each jump detection. \f$\delta\f$
  denotes the jump minimal magnitude that we want to detect and
  \f$\alpha\f$ is a predefined threshold. These values are set by
  default to 0.2 in the default constructor vpHinkley(). To modify the
  default values use setAlpha() and setDelta() or the
  vpHinkley(double alpha, double delta) constructor.

  A downward jump is detected if \f$ M_k - S_k > \alpha \f$.
  A upward jump is detected if \f$ T_k - N_k > \alpha \f$.

  To detect only downward jumps in \f$ s(t) \f$ use
  testDownwardJump().To detect only upward jumps in \f$ s(t) \f$ use
  testUpwardJump(). To detect both, downard and upward jumps use
  testDownUpwardJump().

  If a jump is detected, the jump location is given by the last instant
  \f$k^{'}\f$ when \f$ M_{k^{'}} - S_{k^{'}} = 0 \f$, or \f$ T_{k^{'}} -
  N_{k^{'}} = 0 \f$.

*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <visp/vpHinkley.h>
#include <visp/vpDebug.h>
#include <visp/vpIoTools.h>


/* VP_DEBUG_MODE fixed by configure:
   1:
   2:
   3: Print data
*/


/*!

  Constructor.

  Call init() to initialise the Hinkley's test and set \f$\alpha\f$
  and \f$\delta\f$ to default values.

  By default \f$ \delta = 0.2 \f$ and \f$ \alpha = 0.2\f$. Use
  setDelta() and setAlpha() to modify these values.

*/
vpHinkley::vpHinkley()
{
  init();

  setAlpha(0.2);
  setDelta(0.2);
}

/*!

  Constructor.

  Call init() to initialise the Hinkley's test and set \f$\alpha\f$
  and \f$\delta\f$ thresholds.

  \param alpha : \f$\alpha\f$ is a predefined threshold.

  \param delta : \f$\delta\f$ denotes the jump minimal magnitude that
  we want to detect.

  \sa setAlpha(), setDelta()

*/

vpHinkley::vpHinkley(double alpha, double delta)
{
  init();

  setAlpha(alpha);
  setDelta(delta);
}

/*!

  Call init() to initialise the Hinkley's test and set \f$\alpha\f$
  and \f$\delta\f$ thresholds.

  \param alpha : \f$\alpha\f$ is a predefined threshold.

  \param delta : \f$\delta\f$ denotes the jump minimal magnitude that
  we want to detect.

  \sa setAlpha(), setDelta()

*/
void
vpHinkley::init(double alpha, double delta)
{
  init();

  setAlpha(alpha);
  setDelta(delta);
}

/*!

  Destructor.

*/
vpHinkley::~vpHinkley()
{
}

/*!

  Initialise the Hinkley's test by setting the mean signal value
  \f$m_0\f$ to zero as well as \f$S_k, M_k, T_k, N_k\f$.

*/
void vpHinkley::init()
{
  nsignal = 0;
  mean  = 0.0;

  Sk = 0;
  Mk = 0;

  Tk = 0;
  Nk = 0;
}

/*!

  Set the value of \f$\delta\f$, the jump minimal magnetude that we want to
  detect.

  \sa setAlpha()

*/
void vpHinkley::setDelta(double delta)
{
  dmin2 = delta / 2;
}

/*!

  Set the value of \f$\alpha\f$, a predefined threshold.

  \sa setDelta()

*/
void vpHinkley::setAlpha(double alpha)
{
  this->alpha = alpha;
}

/*!

  Perform the Hinkley test. A downward jump is detected if
  \f$ M_k - S_k > \alpha \f$.

  \param signal : Observed signal \f$ s(t) \f$.

  \sa setDelta(), setAlpha(), testUpwardJump()

*/
vpHinkley::vpHinkleyJumpType vpHinkley::testDownwardJump(double signal)
{

  vpHinkleyJumpType jump = noJump;

  nsignal ++; // Signal lenght

  if (nsignal == 1) mean = signal;

  // Calcul des variables cumul�es
  computeSk(signal);

  computeMk();

  vpCDEBUG(2) << "alpha: " << alpha << " dmin2: " << dmin2
	    << " signal: " << signal << " Sk: " << Sk << " Mk: " << Mk;

  // teste si les variables cumul�es exc�dent le seuil
  if ((Mk - Sk) > alpha)
    jump = downwardJump;

#ifdef VP_DEBUG
  if (VP_DEBUG_MODE >=2) {
    switch(jump) {
    case noJump:
      std::cout << "noJump " << std::endl;
     break;
    case downwardJump:
      std::cout << "downWardJump " << std::endl;
      break;
    case upwardJump:
      std::cout << "upwardJump " << std::endl;
      break;
    }
  }
#endif

  computeMean(signal);

  if (jump == downwardJump)  {
    vpCDEBUG(2) << "\n*** Reset the Hinkley test  ***\n";

    Sk = 0; Mk = 0;  nsignal = 0;
  }

  return (jump);
}

/*!

  Perform the Hinkley test. An upward jump is detected if \f$ T_k - N_k >
  \alpha \f$.

  \param signal : Observed signal \f$ s(t) \f$.

  \sa setDelta(), setAlpha(), testDownwardJump()

*/
vpHinkley::vpHinkleyJumpType vpHinkley::testUpwardJump(double signal)
{

  vpHinkleyJumpType jump = noJump;

  nsignal ++; // Signal lenght

  if (nsignal == 1) mean = signal;

  // Calcul des variables cumul�es
  computeTk(signal);

  computeNk();

  vpCDEBUG(2) << "alpha: " << alpha << " dmin2: " << dmin2
	    << " signal: " << signal << " Tk: " << Tk << " Nk: " << Nk;

  // teste si les variables cumul�es exc�dent le seuil
  if ((Tk - Nk) > alpha)
    jump = upwardJump;

#ifdef VP_DEBUG
  if (VP_DEBUG_MODE >= 2) {
    switch(jump) {
    case noJump:
      std::cout << "noJump " << std::endl;
     break;
    case downwardJump:
      std::cout << "downWardJump " << std::endl;
      break;
    case upwardJump:
      std::cout << "upWardJump " << std::endl;
      break;
    }
  }
#endif
  computeMean(signal);

  if (jump == upwardJump)  {
    vpCDEBUG(2) << "\n*** Reset the Hinkley test  ***\n";

    Tk = 0; Nk = 0;  nsignal = 0;
  }

  return (jump);
}

/*!

  Perform the Hinkley test. A downward jump is detected if \f$ M_k - S_k >
  \alpha \f$. An upward jump is detected if \f$ T_k - N_k > \alpha \f$.

  \param signal : Observed signal \f$ s(t) \f$.

  \sa setDelta(), setAlpha(), testDownwardJump(), testUpwardJump()

*/
vpHinkley::vpHinkleyJumpType vpHinkley::testDownUpwardJump(double signal)
{

  vpHinkleyJumpType jump = noJump;

  nsignal ++; // Signal lenght

  if (nsignal == 1) mean = signal;

  // Calcul des variables cumul�es
  computeSk(signal);
  computeTk(signal);

  computeMk();
  computeNk();

  vpCDEBUG(2) << "alpha: " << alpha << " dmin2: " << dmin2
	      << " signal: " << signal
	      << " Sk: " << Sk << " Mk: " << Mk
	      << " Tk: " << Tk << " Nk: " << Nk << std::endl;

  // teste si les variables cumul�es exc�dent le seuil
  if ((Mk - Sk) > alpha)
    jump = downwardJump;
  else if ((Tk - Nk) > alpha)
    jump = upwardJump;

#ifdef VP_DEBUG
  if (VP_DEBUG_MODE >= 2) {
    switch(jump) {
    case noJump:
      std::cout << "noJump " << std::endl;
     break;
    case downwardJump:
      std::cout << "downWardJump " << std::endl;
      break;
    case upwardJump:
      std::cout << "upwardJump " << std::endl;
      break;
    }
  }
#endif

  computeMean(signal);

  if ((jump == upwardJump) || (jump == downwardJump)) {
    vpCDEBUG(2) << "\n*** Reset the Hinkley test  ***\n";

    Sk = 0; Mk = 0; Tk = 0; Nk = 0;  nsignal = 0;
    // Debut modif FS le 03/09/2003
    mean = signal;
    // Fin modif FS le 03/09/2003
  }

  return (jump);
}

/*!

  Compute the mean value \f$m_0\f$ of the signal. The mean value must be
  computed before the jump is estimated on-line.

  \param signal : Observed signal \f$ s(t) \f$.

*/
void vpHinkley::computeMean(double signal)
{
  // Debut modif FS le 03/09/2003
  // Lors d'une chute ou d'une remont�e lente du signal, pariculi�rement
  // apr�s un saut, la moyenne a tendance � "d�river". Pour r�duire ces
  // d�rives de la moyenne, elle n'est remise � jour avec la valeur
  // courante du signal que si un d�but de saut potentiel n'est pas d�tect�.
  if ( ((Mk-Sk) == 0) && ((Tk-Nk) == 0) )
  // Fin modif FS le 03/09/2003

  // Mise a jour de la moyenne.
    mean = (mean * (nsignal - 1) + signal) / (nsignal);

}
/*!

  Compute \f$S_k = \sum_{t=0}^{k} (s(t) - m_0 + \frac{\delta}{2})\f$

  \param signal : Observed signal \f$ s(t) \f$.

*/
void vpHinkley::computeSk(double signal)
{

  // Calcul des variables cumul�es
  Sk += signal - mean + dmin2;
}
/*!

  Compute \f$M_k\f$, the maximum value of \f$S_k\f$.

*/
void vpHinkley::computeMk()
{
  if (Sk > Mk) Mk = Sk;
}
/*!

  Compute \f$T_k = \sum_{t=0}^{k} (s(t) - m_0 - \frac{\delta}{2})\f$

  \param signal : Observed signal \f$ s(t) \f$.
*/
void vpHinkley::computeTk(double signal)
{

  // Calcul des variables cumul�es
  Tk += signal - mean - dmin2;
}
/*!

  Compute \f$N_k\f$, the minimum value of \f$T_k\f$.

*/
void vpHinkley::computeNk()
{
  if (Tk < Nk) Nk = Tk;
}

void vpHinkley::print(vpHinkley::vpHinkleyJumpType jump)
{
  switch(jump)
    {
    case noJump :
      std::cout << " No jump detected " << std::endl ;
      break ;
    case downwardJump :
      std::cout << " Jump downward detected " << std::endl ;
      break ;
    case upwardJump :
      std::cout << " Jump upward detected " << std::endl ;
      break ;
    default:
      std::cout << " Jump  detected " << std::endl ;
      break ;

  }
}

