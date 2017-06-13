/****************************************************************************
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2017 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Tukey M-estimator
 *
 * Authors:
 * Souriya Trinh
 *
 *****************************************************************************/

#ifndef __vpMbtTukeyEstimator_h_
#define __vpMbtTukeyEstimator_h_

#include <vector>
#include <visp3/core/vpColVector.h>

template <typename T>
class VISP_EXPORT vpMbtTukeyEstimator {
public:
  T getMedian(std::vector<T> &vec);

  void MEstimator(const std::vector<T> &residues, std::vector<T> &weights, const T NoiseThreshold);
  void MEstimator(const vpColVector &residues, vpColVector &weights, const double NoiseThreshold);

  void psiTukey(const T sig, std::vector<T> &x, std::vector<T> &weights);
  void psiTukey(const double sig, std::vector<double> &x, vpColVector &weights);

private:
  std::vector<T> m_normres;
  std::vector<T> m_residues;
};
#endif
