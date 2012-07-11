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

#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDomElement>

#include "mainwindow.hpp"
#include "object.hpp"
#include "objectfactory.hpp"
#include "objectmodel.hpp"
#include "objectmodelloader.hpp"
#include "targetnode.hpp"
#include "trackdata.hpp"
#include "trackio.hpp"
#include "tracktile.hpp"

#include "../common/config.hpp"
#include "../common/objectbase.hpp"
#include "../common/targetnodebase.hpp"

bool TrackIO::save(const TrackData * trackData, QString path)
{
    // Create content
    QDomDocument doc;
    QDomElement root = doc.createElement("track");
    root.setAttribute("version", Config::Editor::EDITOR_VERSION);
    root.setAttribute("name", trackData->name());
    root.setAttribute("cols", trackData->map().cols());
    root.setAttribute("rows", trackData->map().rows());
    root.setAttribute("lapCount", trackData->lapCount());
    doc.appendChild(root);

    writeTiles(*trackData, root, doc);
    writeObjects(*trackData, root, doc);
    writeTargetNodes(*trackData, root, doc);

    // Save to file
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << doc.toString();
        file.close();
        return true;
    }

    return false;
}

TrackData * TrackIO::open(QString path)
{
    QDomDocument doc;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return nullptr;
    }

    if (!doc.setContent(&file))
    {
        file.close();
        return nullptr;
    }

    file.close();

    const QDomElement  root     = doc.documentElement();
    const QString      version  = root.attribute("version", Config::Editor::EDITOR_VERSION);
    const QString      name     = root.attribute("name", "undefined");
    const unsigned int cols     = root.attribute("cols", "0").toUInt();
    const unsigned int rows     = root.attribute("rows", "0").toUInt();
    const unsigned int lapCount = root.attribute("lapCount", "0").toUInt();

    TrackData * newData = nullptr;
    if (cols && rows)
    {
        newData = new TrackData(name, cols, rows);
        newData->setFileName(path);
        newData->setLapCount(lapCount);

        // Temporary route vector.
        std::vector<TargetNodeBase *> route;

        QDomNode node = root.firstChild();
        while(!node.isNull())
        {
            QDomElement element = node.toElement();
            if(!element.isNull())
            {
                // Read a tile element
                if (element.nodeName() == "tile")
                {
                    readTile(*newData, element);
                }
                // Read an object element
                else if (element.nodeName() == "object")
                {
                    readObject(*newData, element);
                }
                // Read a target node element
                else if (element.nodeName() == "tnode")
                {
                    readTargetNode(route, element);
                }
            }

            node = node.nextSibling();
        }

        // Sort and build route from the temporary vector.
        newData->route().buildFromVector(route);
    }

    return newData;
}

void TrackIO::readTile(TrackData & newData, const QDomElement & element)
{
    const QString       id = element.attribute("type", "clear");
    const unsigned int   i = element.attribute("i", "0").toUInt();
    const unsigned int   j = element.attribute("j", "0").toUInt();
    const int            o = element.attribute("o", "0").toInt();
    const int computerHint = element.attribute("computerHint", "0").toInt();

    // Init a new tile. QGraphicsScene will take
    // the ownership eventually.
    if (TrackTile * tile = static_cast<TrackTile *>(newData.map().getTile(i, j)))
    {
        tile->setRotation(o);
        tile->setTileType(id);
        tile->setPixmap(MainWindow::instance()->objectModelLoader().getPixmapByRole(id));
        tile->setComputerHint(static_cast<TrackTileBase::ComputerHint>(computerHint));
    }
}

void TrackIO::readObject(TrackData & newData, const QDomElement & element)
{
    const QString role     = element.attribute("role", "clear");
    const QString category = element.attribute("category", "clear");
    const int     x        = element.attribute("x", "0").toInt();
    const int     y        = element.attribute("y", "0").toInt();
    const int     o        = element.attribute("o", "0").toInt();

    ObjectModel model = MainWindow::instance()->objectModelLoader().getObjectModelByRole(role);

    // Create a new object. QGraphicsScene will take
    // the ownership eventually.
    Object & object = ObjectFactory::createObject(role);
    object.setLocation(QPointF(x, y));
    object.setRotation(o);
    newData.objects().add(object);
}

void TrackIO::readTargetNode(
   std::vector<TargetNodeBase *> & route, const QDomElement & element)
{
    const int x = element.attribute("x", "0").toInt();
    const int y = element.attribute("y", "0").toInt();
    const int i = element.attribute("i", "0").toInt();

    // Create a new object. QGraphicsScene will take
    // the ownership eventually.
    TargetNode * tnode = new TargetNode;
    tnode->setIndex(i);
    tnode->setLocation(QPointF(x, y));
    route.push_back(tnode);
}

void TrackIO::writeTiles(
    const TrackData & trackData, QDomElement & root, QDomDocument & doc)
{
    for (unsigned int i = 0; i < trackData.map().cols(); i++)
    {
        for (unsigned int j = 0; j < trackData.map().rows(); j++)
        {
            if (TrackTile * tile = static_cast<TrackTile *>(trackData.map().getTile(i, j)))
            {
                QDomElement tileElement = doc.createElement("tile");
                tileElement.setAttribute("type", tile->tileType());
                tileElement.setAttribute("i", i);
                tileElement.setAttribute("j", j);
                tileElement.setAttribute("o", tile->rotation());

                if (tile->computerHint() != TrackTile::CH_NONE)
                {
                    tileElement.setAttribute("computerHint", tile->computerHint());
                }

                root.appendChild(tileElement);
            }
        }
    }
}

void TrackIO::writeObjects(
    const TrackData & trackData, QDomElement & root, QDomDocument & doc)
{
    for (unsigned int i = 0; i < trackData.objects().count(); i++)
    {
        QDomElement objectElement = doc.createElement("object");
        Object    & object        = static_cast<Object &>(trackData.objects().object(i));

        objectElement.setAttribute("category", object.category());
        objectElement.setAttribute("role", object.role());
        objectElement.setAttribute("x", static_cast<int>(object.location().x()));
        objectElement.setAttribute("y", static_cast<int>(object.location().y()));
        objectElement.setAttribute("o", static_cast<int>(object.rotation()));
        root.appendChild(objectElement);
    }
}

void TrackIO::writeTargetNodes(
    const TrackData & trackData, QDomElement & root, QDomDocument & doc)
{
    const Route & route = trackData.route();
    for (unsigned int i = 0; i < route.numNodes(); i++)
    {
        QDomElement      tnodeElement = doc.createElement("tnode");
        TargetNodeBase & tnode        = route.get(i);

        tnodeElement.setAttribute("i", tnode.index());
        tnodeElement.setAttribute("x", static_cast<int>(tnode.location().x()));
        tnodeElement.setAttribute("y", static_cast<int>(tnode.location().y()));
        root.appendChild(tnodeElement);
    }
}
