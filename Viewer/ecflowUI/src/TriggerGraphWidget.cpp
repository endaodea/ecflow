//============================================================================
// Copyright 2009-2020 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
//============================================================================

#include "TriggerGraphWidget.hpp"

#include "ui_TriggerGraphWidget.h"

#include "TriggerGraphView.hpp"
#include "TriggerItemWidget.hpp"
#include "TriggerGraphModel.hpp"
#include "TriggerGraphDelegate.hpp"
#include "VSettings.hpp"

TriggerGraphWidget::TriggerGraphWidget(QWidget* parent) :
    QWidget(parent),
    ui_(new Ui::TriggerGraphWidget)
{
    ui_->setupUi(this);
    scene_ = new TriggerGraphScene(this);
    ui_->view->setScene(scene_);
    // the bg can only be set correctly when the scene is set on the view
    ui_->view->adjustBackground();

    model_ = new TriggerGraphModel(TriggerGraphModel::TriggerMode,this);
    ui_->view->setModel(model_);

    delegate_ = new TriggerGraphDelegate(this);

    //nodeCollector_=new TriggerTableCollector(false);

    //relay commands
    connect(ui_->view,SIGNAL(infoPanelCommand(VInfo_ptr,QString)),
            this,SIGNAL(infoPanelCommand(VInfo_ptr,QString)));

    connect(ui_->view,SIGNAL(dashboardCommand(VInfo_ptr,QString)),
            this,SIGNAL(dashboardCommand(VInfo_ptr,QString)));
}

TriggerGraphWidget::~TriggerGraphWidget()
{
    clear();
}

void TriggerGraphWidget::clear()
{
    info_.reset();
    model_->clearData();
    scene_->clear();
}

void TriggerGraphWidget::setInfo(VInfo_ptr info)
{
    info_=info;

    //nodeModel_->beginUpdate();
    //nodeCollector_->clear();
    //if(info_)
    //{
    //    nodeCollector_->add(info_->item(),nullptr,TriggerCollector::Normal);
    //}
    //nodeModel_->setTriggerCollector(nodeCollector_);
    //nodeModel_->endUpdate();

    model_->setNode(info);

    //ui_->view->setInfo(info);
}

void TriggerGraphWidget::adjust(VInfo_ptr info, TriggerTableCollector* tc1, TriggerTableCollector* tc2)
{
    if (!info) {
        clear();
    } else if(info_ != info) {
        setInfo(info);
        beginTriggerUpdate();
        setTriggerCollector(tc1,tc2);
        endTriggerUpdate();
    }
}

void TriggerGraphWidget::setTriggerCollector(TriggerTableCollector *tc1,TriggerTableCollector *tc2)
{
    model_->setTriggerCollectors(tc1, tc2);
}


void TriggerGraphWidget::beginTriggerUpdate()
{
    model_->beginUpdate();
}

void TriggerGraphWidget::endTriggerUpdate()
{
    model_->endUpdate();

    TriggerGraphNodeItem* item =new TriggerGraphNodeItem(model_->index(0,0), delegate_, 0, nullptr);
    scene_->addItem(item);

    for(int i=1; i < model_->rowCount(); i++) {
        TriggerGraphNodeItem* itemN =new TriggerGraphNodeItem(model_->index(i,0), delegate_, -1, nullptr);
        scene_->addItem(itemN);

        TriggerGraphEdgeItem* itemE =new TriggerGraphEdgeItem(itemN, item, nullptr);
        scene_->addItem(itemE);
    }
}

void TriggerGraphWidget::nodeChanged(const VNode* node, const std::vector<ecf::Aspect::Type>& aspect)
{
    ui_->view->nodeChanged(node, aspect);
}
