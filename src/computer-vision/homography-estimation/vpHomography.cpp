/****************************************************************************
 *
 * $Id: vpHomography.cpp,v 1.8 2007-04-20 14:22:15 asaunier Exp $
 *
 * Copyright (C) 1998-2006 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit.
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
 * Homography transformation.
 *
 * Authors:
 * Muriel Pressigout
 *
 *****************************************************************************/


/*!
  \file vpHomography.cpp
  \brief D�finition de la classe vpHomography. Class that consider
  the particular case of homography
*/

#include <visp/vpDebug.h>
#include <visp/vpMatrix.h>
#include <visp/vpHomography.h>

// Exception
#include <visp/vpException.h>
#include <visp/vpMatrixException.h>

// Debug trace
#include <visp/vpDebug.h>

#include <stdio.h>



/*!
  \brief initialiaze a 4x4 matrix as identity
*/

void
vpHomography::init()
{
  int i,j ;

  try {
    vpMatrix::resize(3,3) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    throw ;
  }


  for (i=0 ; i < 3 ; i++)
    for (j=0 ; j < 3; j++)
      if (i==j)
	(*this)[i][j] = 1.0 ;
      else
 	(*this)[i][j] = 0.0;

}

/*!
  \brief initialize an homography as Identity
*/
vpHomography::vpHomography() : vpMatrix()
{
  init() ;
}


/*!
  \brief initialize an homography from another homography
*/

vpHomography::vpHomography(const vpHomography &aHb) : vpMatrix()
{
  init() ;
  *this = aHb ;
}

/*!
  \brief initialize an homography from another homography
*/

vpHomography::vpHomography(const vpHomogeneousMatrix &aMb,
			   const vpPlane &_bP) : vpMatrix()
{


  init() ;

  buildFrom(aMb,_bP) ;


}

vpHomography::vpHomography(const vpThetaUVector &tu,
			   const vpTranslationVector &aTb,
			   const vpPlane &_bP) : vpMatrix()
{
  init() ;
  buildFrom(tu,aTb,_bP) ;
}

vpHomography::vpHomography(const vpRotationMatrix &aRb,
			   const vpTranslationVector &aTb,
			   const vpPlane &_bP) : vpMatrix()
{
  init() ;
  buildFrom(aRb,aTb,_bP) ;
 }

vpHomography::vpHomography(const vpPoseVector &arb,
			   const vpPlane &_bP) : vpMatrix()
{

  init() ;
  buildFrom(arb,_bP) ;
}



void
vpHomography::buildFrom(const vpHomogeneousMatrix &aMb,
			const vpPlane &_bP)
{


  insert(aMb) ;
  insert(_bP) ;
  build() ;


}

void
vpHomography::buildFrom(const vpThetaUVector &tu,
			const vpTranslationVector &aTb,
			const vpPlane &_bP)
{

  insert(tu) ;
  insert(aTb) ;
  insert(_bP) ;
  build() ;
}

void
vpHomography::buildFrom(const vpRotationMatrix &aRb,
			const vpTranslationVector &aTb,
			const vpPlane &_bP)
{
  init() ;
  insert(aRb) ;
  insert(aTb) ;
  insert(_bP) ;
  build() ;
}

void
vpHomography::buildFrom(const vpPoseVector &arb,
			const vpPlane &_bP)
{

  aMb.buildFrom(arb[0],arb[1],arb[2],arb[3],arb[4],arb[5]) ;
  insert(_bP) ;
  build() ;
}



/*********************************************************************/


/*!
  \brief insert the rotational component and
  recompute the homography
*/
void
vpHomography::insert(const vpRotationMatrix &aRb)
{
  aMb.insert(aRb) ;
  build() ;
}
/*!
  \brief insert the rotational component and
  recompute the homography
*/
void
vpHomography::insert(const vpHomogeneousMatrix &_aMb)
{

  aMb = _aMb ;
  build() ;
}


/*!  \brief insert the rotational component, insert a
  theta u vector (transformation into a rotation matrix) and
  recompute the homography

*/
void
vpHomography::insert(const vpThetaUVector &tu)
{
  vpRotationMatrix aRb(tu) ;
  aMb.insert(aRb) ;
  build() ;
}


/*!
  \brief  insert the translational component in a homography and
  recompute the homography
*/
void
vpHomography::insert(const vpTranslationVector &aTb)
{
  aMb.insert(aTb) ;
  build() ;
}

/*!
  \brief  insert the reference plane and
  recompute the homography
*/
void
vpHomography::insert(const vpPlane &_bP)
{

  bP= _bP ;
  build() ;
}


/*!
  \relates  vpHomography
  \brief invert the homography


  \return   [H]^-1
*/
vpHomography
vpHomography::inverse() const
{
  vpHomography bHa ;


  vpMatrix::pseudoInverse(bHa,1e-16) ;

  return  bHa;
}

/*!
  \relates vpHomography
  \brief invert the homography


  \param bHa : [H]^-1
*/
void
vpHomography::inverse(vpHomography &bHa) const
{
  bHa = inverse() ;
}


void
vpHomography::save(std::ofstream &f) const
{
  if (f != NULL)
  {
    f << *this ;
  }
  else
  {
    vpERROR_TRACE("\t\t file not open " );
    throw(vpException(vpException::ioError, "\t\t file not open")) ;
  }
}


/*!
  Read an homography in a file, verify if it is really an homogeneous
  matrix.

  \param f : the file.
*/
void
vpHomography::load(std::ifstream &f)
{
  if (f != NULL)
  {
    for (int i=0 ; i < 3 ; i++)
      for (int j=0 ; j < 3 ; j++)
      {
	f>>   (*this)[i][j] ;
      }
  }
  else
  {
    vpERROR_TRACE("\t\t file not open " );
    throw(vpException(vpException::ioError, "\t\t file not open")) ;
  }
}



//! Print the matrix as a vector [T thetaU]
void
vpHomography::print()
{
  std::cout <<*this << std::endl ;
}

/*!
  \brief Compute aHb such that

  \f[  ^a{\bf H}_b = ^a{\bf R}_b + \frac{^a{\bf t}_b}{^bd}
  { ^b{\bf n}^T}
  \f]
*/
void
vpHomography::build()
{
  int i,j ;

  vpColVector n(3) ;
  vpColVector aTb(3) ;
  for (i=0 ; i < 3 ; i++)
  {
    aTb[i] = aMb[i][3] ;
    for (j=0 ; j < 3 ; j++) (*this)[i][j] = aMb[i][j];
  }

  bP.getNormal(n) ;

  double d = bP.getD() ;
  *this += aTb*n.t()/d ;

}

/*!
  \brief Compute aHb such that

  \f[  ^a{\bf H}_b = ^a{\bf R}_b + \frac{^a{\bf t}_b}{^bd}
  { ^b{\bf n}^T}
  \f]
  //note d => -d verifier
*/
void
vpHomography::build(vpHomography &aHb,
		    const vpHomogeneousMatrix &aMb,
		    const vpPlane &bP)
{
  int i,j ;

  vpColVector n(3) ;
  vpColVector aTb(3) ;
  for (i=0 ; i < 3 ; i++)
  {
    aTb[i] = aMb[i][3] ;
    for (j=0 ; j < 3 ; j++) aHb[i][j] = aMb[i][j];
  }

  bP.getNormal(n) ;

  double d = bP.getD() ;
  aHb += aTb*n.t()/d ;

}





#undef DEBUG_LEVEL1

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
