template <class T>
bool
Acts::RungeKuttaEngine::propagateRungeKuttaT(ExtrapolationCell<T>& eCell,
                                             PropagationCache&     pCache,
                                             const T&              parametersT,
                                             const Surface& dSurface) const
{
  EX_MSG_VERBOSE(eCell.navigationStep,
                 "propagate",
                 "<T> ",
                 "propagateRungeKuttaT called.");

  // bail out if you can't transform into global frame
  if (!m_rkUtils.transformLocalToGlobal(
          pCache.useJacobian, parametersT, pCache.pVector))
    return false;

  // get the surface transform
  const Transform3D& sTransform = dSurface.transform();

  // now switch the surface type (defines local parameters)
  Surface::SurfaceType sType = dSurface.type();

  // propaate with different surface types
  if (sType == Surface::Plane || sType == Surface::Disc) {
    // (i) planar surface types
    double s[4], d = sTransform(0, 3) * sTransform(0, 2)
        + sTransform(1, 3) * sTransform(1, 2)
        + sTransform(2, 3) * sTransform(2, 2);
    if (d >= 0.) {
      s[0] = sTransform(0, 2);
      s[1] = sTransform(1, 2);
      s[2] = sTransform(2, 2);
      s[3] = d;
    } else {
      s[0] = -sTransform(0, 2);
      s[1] = -sTransform(1, 2);
      s[2] = -sTransform(2, 2);
      s[3] = -d;
    }
    // check for propagation failure
    if (!propagateWithJacobian(eCell.navigationStep, pCache, 1, s))
      return false;
  } else if (sType == Surface::Line || sType == Surface::Perigee) {
    // (ii) line-type surfaces
    double s[6] = {sTransform(0, 3),
                   sTransform(1, 3),
                   sTransform(2, 3),
                   sTransform(0, 2),
                   sTransform(1, 2),
                   sTransform(2, 2)};
    // check for propagation failure
    if (!propagateWithJacobian(eCell.navigationStep, pCache, 0, s))
      return false;
  } else if (sType == Surface::Cylinder) {
    // (iii) cylinder surface:
    // - cast to CylinderSurface for checking the cross point
    const CylinderSurface* cyl = static_cast<const CylinderSurface*>(&dSurface);
    double r0[3] = {pCache.pVector[0], pCache.pVector[1], pCache.pVector[2]};
    double s[9]  = {sTransform(0, 3),
                   sTransform(1, 3),
                   sTransform(2, 3),
                   sTransform(0, 2),
                   sTransform(1, 2),
                   sTransform(2, 2),
                   dSurface.bounds().r(),
                   pCache.direction,
                   0.};
    // check for propagation failure
    if (!propagateWithJacobian(eCell.navigationStep, pCache, 2, s))
      return false;
    // For cylinder we do test for next cross point
    if (cyl->bounds().halfPhiSector() < 3.1
        && newCrossPoint(*cyl, r0, pCache.pVector)) {
      s[8] = 0.;
      // check for propagation failure
      if (!propagateWithJacobian(eCell.navigationStep, pCache, 2, s))
        return false;
    }
  } else if (sType == Surface::Cone) {
    // (iv) cone surface
    // -  need to access the tangent of alpha
    double k = static_cast<const ConeSurface*>(&dSurface)->bounds().tanAlpha();
    k        = k * k + 1.;
    double s[9] = {sTransform(0, 3),
                   sTransform(1, 3),
                   sTransform(2, 3),
                   sTransform(0, 2),
                   sTransform(1, 2),
                   sTransform(2, 2),
                   k,
                   pCache.direction,
                   0.};
    // check for propagation failure
    if (!propagateWithJacobian(eCell.navigationStep, pCache, 3, s))
      return false;
  } else
    return false;  // there was no surface that did fit

  EX_MSG_VERBOSE(eCell.navigationStep,
                 "propagate",
                 "<T> ",
                 "surface type determined and localToGlobal performed.");

  // check if the direction solution was ok
  if (pCache.direction != 0. && (pCache.direction * pCache.step) < 0.)
    return false;

  // Common transformation for all surfaces (angles and momentum)
  if (pCache.useJacobian) {
    double p = 1. / pCache.pVector[6];
    pCache.pVector[35] *= p;
    pCache.pVector[36] *= p;
    pCache.pVector[37] *= p;
    pCache.pVector[38] *= p;
    pCache.pVector[39] *= p;
    pCache.pVector[40] *= p;
  }

  // return curvilinear when the path limit is met
  if (pCache.maxPathLimit) pCache.returnCurvilinear = true;
  // use the jacobian for tranformation
  bool uJ                          = pCache.useJacobian;
  if (pCache.returnCurvilinear) uJ = false;

  // create the return track parameters from Global to Local
  m_rkUtils.transformGlobalToLocal(
      &dSurface, uJ, pCache.pVector, pCache.parameters, pCache.jacobian);

  if (pCache.boundaryCheck) {
    // create a local position and check for inside
    Vector2D lPosition(pCache.parameters[0], pCache.parameters[1]);
    if (!dSurface.insideBounds(lPosition, pCache.boundaryCheck)) return false;
  }

  // Transformation to curvilinear presentation
  if (pCache.returnCurvilinear)
    m_rkUtils.transformGlobalToCurvilinear(
        pCache.useJacobian, pCache.pVector, pCache.parameters, pCache.jacobian);

  if (pCache.useJacobian) {
    // create a new covariance matrix using the utils
    pCache.covariance = m_rkUtils.newCovarianceMatrix(
        pCache.jacobian, *parametersT.covariance());
    auto& cov = (*pCache.covariance);
    // check for positive finite elements
    if (cov(0, 0) <= 0. || cov(1, 1) <= 0. || cov(2, 2) <= 0. || cov(3, 3) <= 0.
        || cov(4, 4) <= 0.) {
      delete pCache.covariance;
      pCache.covariance = nullptr;
      return false;
    }
  }
  // everything worked, return true
  return true;
}