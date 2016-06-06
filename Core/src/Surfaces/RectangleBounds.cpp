// This file is part of the ACTS project.
//
// Copyright (C) 2016 ACTS project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

///////////////////////////////////////////////////////////////////
// RectangleBounds.cpp, ACTS project
///////////////////////////////////////////////////////////////////

#include "ACTS/Surfaces/RectangleBounds.hpp"
// STD/STL
#include <iomanip>
#include <iostream>

// default constructor
Acts::RectangleBounds::RectangleBounds()
  : Acts::PlanarBounds(), m_boundValues(RectangleBounds::bv_length, 0.)
{
}

// rectangle constructor
Acts::RectangleBounds::RectangleBounds(double halex, double haley)
  : Acts::PlanarBounds(), m_boundValues(RectangleBounds::bv_length, 0.)
{
  m_boundValues.at(RectangleBounds::bv_halfX) = halex;
  m_boundValues.at(RectangleBounds::bv_halfY) = haley;
}

// copy constructor
Acts::RectangleBounds::RectangleBounds(const RectangleBounds& recbo)
  : Acts::PlanarBounds(), m_boundValues(recbo.m_boundValues)
{
}

// destructor
Acts::RectangleBounds::~RectangleBounds()
{
}

Acts::RectangleBounds&
Acts::RectangleBounds::operator=(const RectangleBounds& recbo)
{
  if (this != &recbo) m_boundValues = recbo.m_boundValues;
  return *this;
}

bool
Acts::RectangleBounds::operator==(const Acts::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Acts::RectangleBounds* recbo
      = dynamic_cast<const Acts::RectangleBounds*>(&sbo);
  if (!recbo) return false;
  return (m_boundValues == recbo->m_boundValues);
}

double
Acts::RectangleBounds::minDistance(const Acts::Vector2D& pos) const
{
  double dx = fabs(pos[0]) - m_boundValues.at(RectangleBounds::bv_halfX);
  double dy = fabs(pos[1]) - m_boundValues.at(RectangleBounds::bv_halfY);

  if (dx <= 0. || dy <= 0.) {
    if (dx > dy)
      return dx;
    else
      return dy;
  }
  return sqrt(dx * dx + dy * dy);
}

// ostream operator overload
std::ostream&
Acts::RectangleBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Acts::RectangleBounds:  (halflenghtX, halflengthY) = "
     << "(" << m_boundValues.at(RectangleBounds::bv_halfX) << ", "
     << m_boundValues.at(RectangleBounds::bv_halfY) << ")";
  sl << std::setprecision(-1);
  return sl;
}