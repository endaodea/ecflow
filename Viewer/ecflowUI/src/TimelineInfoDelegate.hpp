//============================================================================
// Copyright 2009-2018 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
//============================================================================

#ifndef TIMELINEINFODELEGATE_HPP
#define TIMELINEINFODELEGATE_HPP

#include <QBrush>
#include <QDateTime>
#include <QPen>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

#include "NodeViewDelegate.hpp"
#include "VProperty.hpp"

#include <string>

class TimelineInfoDailyModel;

class TimelineInfoDelegate : public NodeViewDelegate
{
public:
    explicit TimelineInfoDelegate(QWidget *parent=0);
    ~TimelineInfoDelegate() override;

    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    void paint(QPainter *painter,const QStyleOptionViewItem &option,
                   const QModelIndex& index) const override;

protected:
    void updateSettings() override;

    QPen borderPen_;

};

#endif // TIMELINEINFODELEGATE_HPP
