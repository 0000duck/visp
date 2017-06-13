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
 * Model-based tracker using depth as feature
 *
 * Authors:
 * Souriya Trinh
 *
 *****************************************************************************/

#ifndef __vpMbDepthDenseTracker_h_
#define __vpMbDepthDenseTracker_h_

#include <visp3/core/vpPlane.h>
#include <visp3/mbt/vpMbTracker.h>
#include <visp3/mbt/vpMbtFaceDepthDense.h>
#include <visp3/mbt/vpMbtTukeyEstimator.h>


class VISP_EXPORT vpMbDepthDenseTracker : public virtual vpMbTracker {
public:
  vpMbDepthDenseTracker();
  virtual ~vpMbDepthDenseTracker();

  virtual void display(const vpImage<unsigned char>& I, const vpHomogeneousMatrix &cMo, const vpCameraParameters &cam,
                       const vpColor& col , const unsigned int thickness=1, const bool displayFullModel = false);

  virtual void display(const vpImage<vpRGBa>& I, const vpHomogeneousMatrix &cMo, const vpCameraParameters &cam,
                       const vpColor& col , const unsigned int thickness=1, const bool displayFullModel = false);

  virtual inline vpColVector getError() const {
    return m_error_depthDense;
  }

  virtual inline vpColVector getRobustWeights() const {
    return m_w_depthDense;
  }

  virtual void init(const vpImage<unsigned char>& I);

  virtual void loadConfigFile(const std::string& configFile);

  void reInitModel(const vpImage<unsigned char>& I, const std::string &cad_name, const vpHomogeneousMatrix& cMo_,
                   const bool verbose=false);
#if defined(VISP_HAVE_PCL)
  void reInitModel(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr &point_cloud, const std::string &cad_name, const vpHomogeneousMatrix& cMo_,
                   const bool verbose=false);
#endif

  virtual void resetTracker();

  virtual void setCameraParameters(const vpCameraParameters& camera);

  inline void setDepthDenseSamplingStep(const unsigned int stepX, const unsigned int stepY) {
    m_depthDenseStepX = stepX;
    m_depthDenseStepY = stepY;
  }

  virtual void setOgreVisibilityTest(const bool &v);

  virtual void setPose(const vpImage<unsigned char> &I, const vpHomogeneousMatrix& cdMo);
#ifdef VISP_HAVE_PCL
  virtual void setPose(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr &point_cloud, const vpHomogeneousMatrix& cdMo);
#endif

  virtual void setScanLineVisibilityTest(const bool &v);

  virtual void testTracking();

  virtual void track(const vpImage<unsigned char> &);
#ifdef VISP_HAVE_PCL
  virtual void track(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr &point_cloud);
#endif
  virtual void track(const std::vector<vpColVector> &point_cloud, const unsigned int width, const unsigned int height);


protected:
  //! Set of faces describing the object used only for display with scan line.
  vpMbHiddenFaces<vpMbtPolygon> m_depthDenseHiddenFacesDisplay;
  //! Dummy image used to compute the visibility
  vpImage<unsigned char> m_depthDenseI_dummyVisibility;
  //! List of current active (visible and features extracted) faces
  std::vector<vpMbtFaceDepthDense*> m_depthDenseListOfActiveFaces;
  //! Nb features
  unsigned int m_denseDepthNbFeatures;
  //! List of faces
  std::vector<vpMbtFaceDepthDense*> m_depthDenseNormalFaces;
  //! Sampling step in x-direction
  unsigned int m_depthDenseStepX;
  //! Sampling step in y-direction
  unsigned int m_depthDenseStepY;
  //! (s - s*)
  vpColVector m_error_depthDense;
  //! Interaction matrix
  vpMatrix m_L_depthDense;
  //! Tukey M-Estimator
  vpMbtTukeyEstimator<double> m_robust_depthDense;
  //! Robust weights
  vpColVector m_w_depthDense;
  //! Weighted error
  vpColVector m_weightedError_depthDense;

  //TODO: debug
  bool m_debug;

  void addFace(vpMbtPolygon &polygon, const bool alreadyClose);

  void computeVisibility(const unsigned int width, const unsigned int height);

  void computeVVS();
  virtual void computeVVSInit();
  virtual void computeVVSInteractionMatrixAndResidu();
  virtual void computeVVSWeights();
  using vpMbTracker::computeVVSWeights;

  virtual void initCircle(const vpPoint& p1, const vpPoint &p2, const vpPoint &p3, const double radius,
                          const int idFace=0, const std::string &name="");

  virtual void initCylinder(const vpPoint& p1, const vpPoint &p2, const double radius, const int idFace=0,
                            const std::string &name="");

  virtual void initFaceFromCorners(vpMbtPolygon &polygon);

  virtual void initFaceFromLines(vpMbtPolygon &polygon);

#ifdef VISP_HAVE_PCL
  void segmentPointCloud(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr &point_cloud);
#endif
  void segmentPointCloud(const std::vector<vpColVector> &point_cloud, const unsigned int width, const unsigned int height);
};
#endif
