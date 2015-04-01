// This file is part of Dust Racing 2D.
// Copyright (C) 2014 Jussi Lind <jussi.lind@iki.fi>
//
// Dust Racing 2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// Dust Racing 2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dust Racing 2D. If not, see <http://www.gnu.org/licenses/>.

#include "tire.hpp"
#include "car.hpp"

#include <MCAssetManager>
#include <MCMathUtil>
#include <MCSurface>
#include <MCTrigonom>

Tire::Tire(Car & car, MCFloat friction, MCFloat offTrackFriction)
    : MCObject(MCAssetManager::surfaceManager().surface("frontTire"), "Tire")
    , m_isOffTrack(false)
    , m_friction(friction)
    , m_offTrackFriction(offTrackFriction)
    , m_car(car)
{
    setBypassCollisions(true);
}

void Tire::setIsOffTrack(bool flag)
{
    m_isOffTrack = flag;
}

void Tire::stepTime(MCFloat)
{
    if (velocity().lengthFast() > 0)
    {
        const MCFloat tireNormalAngle = angle() + 90;
        const MCVector2d<MCFloat> tire(
            MCTrigonom::cos(tireNormalAngle), MCTrigonom::sin(tireNormalAngle));
        MCVector2d<MCFloat> v = velocity();
        v.clampFast(0.999); // Clamp instead of normalizing to avoid artifacts on small values
        MCVector2d<MCFloat> impulse =
            MCMathUtil::projection(v, tire) *
                (m_isOffTrack ? m_offTrackFriction : m_friction) * -MCWorld::instance().gravity().k() * parent().mass();
        impulse.clampFast(parent().mass() * 7.0f * m_car.tireWearLevel());
        parent().addForce(-impulse, location());
    }
}
