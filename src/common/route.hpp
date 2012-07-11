// This file is part of Dust Racing (DustRAC).
// Copyright (C) 2011 Jussi Lind <jussi.lind@iki.fi>
//
// DustRAC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// DustRAC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DustRAC. If not, see <http://www.gnu.org/licenses/>.

#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <vector>

class TrackTileBase;
class TargetNodeBase;

//! Route is used to define the race route as a sequence
//! of TrackTiles.
class Route
{
public:

    //! Clear the current route.
    void clear();

    //! Push new target to the route and return true
    //! if the route got closed.
    bool push(TargetNodeBase & target);

    //! Build route from an (unordered) vector of Targets.
    //! Will be sorted with respect to their indices.
    void buildFromVector(std::vector<TargetNodeBase *> & routeVector);

    //! Return number of target nodes.
    unsigned int numNodes() const;

    //! Return Target for the given index.
    TargetNodeBase & get(unsigned int index) const;

    //! Get all nodes.
    void getAll(std::vector<TargetNodeBase *> & routeVector) const;

    //! Return length based on target node locations.
    unsigned int geometricLength() const;

private:

    bool isClosed() const;

    std::vector<TargetNodeBase *> m_route;
};

#endif // ROUTE_HPP
