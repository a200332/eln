// Items/GfxImageItem.H - This file is part of eln

/* eln is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   eln is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with eln.  If not, see <http://www.gnu.org/licenses/>.
*/

// GfxImageItem.H

#ifndef GFXIMAGEITEM_H

#define GFXIMAGEITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include "Item.h"
#include "Mode.h"
#include "GfxImageData.h"

class GfxImageItem: public Item {
  Q_OBJECT;
public:
  GfxImageItem(GfxImageData *data, Item *parent=0);
  virtual ~GfxImageItem();
  DATAACCESS(GfxImageData);
  QRectF boundingRect() const;
  QRectF imageBoundingRect() const; // w/o margins
  virtual void makeWritable();
  virtual void setScale(double);
  virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
  bool makesOwnNotes() const { return isWritable(); }
  virtual GfxNoteItem *newGfxNote(QPointF p0, QPointF p1);   
  virtual Qt::CursorShape cursorShape(Qt::KeyboardModifiers) const;
  virtual bool changesCursorShape() const;
protected:
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
  virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *);
private:
  enum DragType {
    None,
    Move,
    ResizeTopLeft,
    ResizeTopRight,
    ResizeBottomLeft,
    ResizeBottomRight,
    CropLeft,
    CropRight,
    CropTop,
    CropBottom,
  };
private:
  QPointF moveDelta(QGraphicsSceneMouseEvent *);
  DragType dragTypeForPoint(QPointF) const;
  static Qt::CursorShape cursorForDragType(DragType);
  void startDrag(QGraphicsSceneMouseEvent *);
private slots:
  void setImage(QImage img);
private:
  QImage image; // not cropped
  DragType dragType;
  QPointF dragStart; // in parent block's coordinates
  Qt::CursorShape oldCursor;
  QPointF cursorPos;
  QRectF cropStart; // in parent block's coordinates
  QRectF imStart; // in parent block's coordinates
  QRect dragCrop;
  QGraphicsPixmapItem *pixmap;
};

#endif
