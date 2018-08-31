//============================================================================
// Copyright 2009-2017 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//============================================================================

#include "TableNodeModel.hpp"

#include <QMetaMethod>

#include "ModelColumn.hpp"
#include "ServerHandler.hpp"
#include "UiLog.hpp"
#include "VAttribute.hpp"
#include "VAttributeType.hpp"
#include "VFilter.hpp"
#include "VIcon.hpp"
#include "VModelData.hpp"
#include "VNode.hpp"
#include "VNState.hpp"

//#define _UI_TABLENODEMODEL_DEBUG

//static int hitCount=0;

static std::map<TableNodeModel::ColumnType,VAttributeType*> attrTypes;
static VAttributeType* columnToAttrType(TableNodeModel::ColumnType ct);

VAttributeType* columnToAttrType(TableNodeModel::ColumnType ct)
{
    std::map<TableNodeModel::ColumnType,VAttributeType*>::const_iterator it=
       attrTypes.find(ct);
    return (it != attrTypes.end())?it->second:0;
}


//=======================================================
//
// TableNodeModel
//
//=======================================================


TableNodeModel::TableNodeModel(ServerFilter* serverFilter,NodeFilterDef* filterDef,QObject *parent) :
	AbstractNodeModel(parent),
	data_(nullptr),
	columns_(nullptr)
{
    columns_=ModelColumn::def("table_columns");

    Q_ASSERT(columns_);

    //Check the mapping between the enum and column ids
    Q_ASSERT(columns_->id(PathColumn) == "path");
    Q_ASSERT(columns_->id(StatusColumn) == "status");
    Q_ASSERT(columns_->id(TypeColumn) == "type");
    Q_ASSERT(columns_->id(TriggerColumn) == "trigger");
    Q_ASSERT(columns_->id(LabelColumn) == "label");
    Q_ASSERT(columns_->id(EventColumn) == "event");
    Q_ASSERT(columns_->id(MeterColumn) == "meter");
    Q_ASSERT(columns_->id(StatusChangeColumn) == "statusChange");
    // Q_ASSERT(columns_->id(PreviousStart) == "previousStart");
    // Q_ASSERT(columns_->id(PreviousStop) == "previousStop");
    // Q_ASSERT(columns_->id(Rid) == "rid");

    if(attrTypes.empty())
    {
        QList<ColumnType> ctLst;
        ctLst << TriggerColumn << LabelColumn << EventColumn << MeterColumn;
        Q_FOREACH(ColumnType ct,ctLst)
        {
            VAttributeType* t=VAttributeType::find(columns_->id(ct).toStdString());
            Q_ASSERT(t);
            attrTypes[ct]=t;
        }
    }

	//Create the data handler for the model.
	data_=new VTableModelData(filterDef,this);

	data_->reset(serverFilter);
}

VModelData* TableNodeModel::data() const
{
	return data_;
}

int TableNodeModel::columnCount( const QModelIndex& /*parent */ ) const
{
   	 return columns_->count();
}

int TableNodeModel::rowCount( const QModelIndex& parent) const
{
#ifdef _UI_TABLENODEMODEL_DEBUG
    UiLog().dbg() << "rowCount=" << parent;
#endif

	//There are no servers
	if(!hasData())
	{
		return 0;
	}
	//We use only column 0
	else if(parent.column() > 0)
	{
		return 0;
	}
	//"parent" is the root
	else if(!parent.isValid())
	{
		//The total number of nodes displayed
		int cnt=0;
		for(int i=0; i < data_->count(); i++)
		{
            if(!data_->server(i)->inScan())
                cnt+=data_->numOfNodes(i);
		}
#ifdef _UI_TABLENODEMODEL_DEBUG
        //UiLog().dbg() << "table count " << cnt;
#endif
		return cnt;
	}

	return 0;
}


QVariant TableNodeModel::data( const QModelIndex& index, int role ) const
{
	//Data lookup can be costly so we immediately return a default value for all
	//the cases where the default should be used.
	if( !index.isValid() ||
       (role != Qt::DisplayRole && role != Qt::ToolTipRole &&
        role != Qt::BackgroundRole && role != IconRole && role != SortRole))
    {
		return QVariant();
	}

	//We only display nodes!!
	return nodeData(index,role);
}

QVariant TableNodeModel::nodeData(const QModelIndex& index, int role) const
{
	VNode* vnode=indexToNode(index);
	if(!vnode || !vnode->node())
		return QVariant();

    auto id=static_cast<ColumnType>(index.column());

	if(role == Qt::DisplayRole)
	{
        //QString id=columns_->id(index.column());

        if(id == PathColumn)
        {
            return QString::fromStdString(vnode->absNodePath());
        }
        else if(id == StatusColumn)
			return vnode->stateName();
        else if(id == TypeColumn)
			return QString::fromStdString(vnode->nodeType());

		//Attributes
        else if(id == EventColumn || id == LabelColumn || id == MeterColumn ||
                id == TriggerColumn)
		{
            if(VAttribute* a=vnode->attributeForType(0,columnToAttrType(id)))
                return a->data(true);
            else
                return QVariant();
		}

        else if(id == StatusChangeColumn)
        {
            QString s;
            vnode->statusChangeTime(s);
            return s;
        }

        else if(id == PreviousStart)
        {
	  std::string s = vnode->findVariable("ECF_JOB");
	  QString out = s.c_str();
	  return out;
        }

        else if(id == PreviousStop)
        {
	  std::string s = vnode->findVariable("ECF_JOBOUT");
	  QString out = s.c_str();
	  return out;
        }

        else if(id == Rid)
        {
	  std::string s = vnode->findVariable("ECF_RID");
	  QString out = s.c_str();
	  return out;
        }

        //else if(id == "icon")
        //    return VIcon::pixmapList(vnode,0);
	}
	else if(role == Qt::BackgroundRole)
	{
		return vnode->stateColour();
	}
	else if(role == IconRole)
	{
        if(id == PathColumn)
			return VIcon::pixmapList(vnode,nullptr);
		else
			return QVariant();
	}
    else if(role == SortRole)
    {
        if(id == StatusChangeColumn)
        {
            return vnode->statusChangeTime();
        }
    }

	return QVariant();
}

QVariant TableNodeModel::headerData( const int section, const Qt::Orientation orient , const int role ) const
{
	if ( orient != Qt::Horizontal || (role != Qt::DisplayRole && role != Qt::UserRole ))
      		  return QAbstractItemModel::headerData( section, orient, role );

	if (section < 0 || section >= columns_->count())  // this can happen during a server reset
			return QVariant();

	if(role == Qt::DisplayRole)
		return columns_->label(section);
	else if(role == Qt::UserRole)
		return columns_->id(section);

	return QVariant();
}


QModelIndex TableNodeModel::index( int row, int column, const QModelIndex & parent ) const
{
    if(!hasData() || row < 0 || column < 0 || parent.isValid())
	{
		return {};
	}

    if(VNode *node=data_->nodeAt(row))
	{
		return createIndex(row,column,node);
	}

	return QModelIndex();
}

QModelIndex TableNodeModel::parent(const QModelIndex& /*child*/) const
{
	//Parent is always the root!!!
	return {};
}

//----------------------------------------------
//
// Server to index mapping and lookup
//
//----------------------------------------------

VNode* TableNodeModel::indexToNode( const QModelIndex & index) const
{
	if(index.isValid())
	{
		return static_cast<VNode*>(index.internalPointer());
	}
	return nullptr;
}

QModelIndex TableNodeModel::nodeToIndex(const VNode* node, int column) const
{
	if(!node)
		return {};

	int row=0;
    if((row=data_->position(node)) != -1)
	{
		return createIndex(row,column,const_cast<VNode*>(node));
	}
	return QModelIndex();
}


//Find the index for the node when we know what the server is!
QModelIndex TableNodeModel::nodeToIndex(VTableServer* server,const VNode* node, int column) const
{
	if(!node)
		return {};

	int row=0;
    if((row=data_->position(server,node)) != -1)
	{
		return createIndex(row,column,const_cast<VNode*>(node));
	}
	return QModelIndex();
}

QModelIndex TableNodeModel::attributeToIndex(const VAttribute* a, int column) const
{
    if(!a)
        return {};

    VNode* node=a->parent();
    if(!node)
        return QModelIndex();

    int row=0;
    if((row=data_->position(node)) != -1)
    {
        return createIndex(row,column,const_cast<VNode*>(node));
    }
    return QModelIndex();
}

void TableNodeModel::selectionChanged(QModelIndexList lst)
{
#if 0
    Q_FOREACH(QModelIndex idx,lst)
    {
        VInfo_ptr info=nodeInfo(idx);

        for(int i=0; i < data_->count(); i++)
        {
           VTableServer *ts=data_->server(i)->tableServer();
           Q_ASSERT(ts);
           ts->clearForceShow(info->item());
        }
    }
#endif
}

VInfo_ptr TableNodeModel::nodeInfo(const QModelIndex& index)
{
	VNode *n=indexToNode(index);
	if(n)
	{
		return VInfoNode::create(n);
	}

	VInfo_ptr info;
	return info;
}

//Server is about to be added
void TableNodeModel::slotServerAddBegin(int /*row*/)
{
}

//Addition of the new server has finished
void TableNodeModel::slotServerAddEnd()
{
}

//Server is about to be removed
void TableNodeModel::slotServerRemoveBegin(VModelServer* server,int num)
{
    Q_ASSERT(active_ == true);
    Q_ASSERT(server);

    if(num >0)
    {
        int start=-1;
        int count=-1;
        data_->position(server->tableServer(),start,count);

        Q_ASSERT(start >=0);
        Q_ASSERT(count == num);

        beginRemoveRows(QModelIndex(),start,start+count-1);
    }
}

//Removal of the server has finished
void TableNodeModel::slotServerRemoveEnd(int num)
{
    assert(active_ == true);

    if(num >0)
        endRemoveRows();
}

//The node changed (it status etc)
void TableNodeModel::slotNodeChanged(VTableServer* server,const VNode* node)
{
	if(!node)
		return;

	QModelIndex index=nodeToIndex(server,node,0);

	if(!index.isValid())
		return;

	Q_EMIT dataChanged(index,index);
}

void TableNodeModel::slotBeginServerScan(VModelServer* server,int num)
{
    Q_ASSERT(active_ == true);
    Q_ASSERT(server);

#ifdef _UI_TABLENODEMODEL_DEBUG
     UiLog().dbg() << "TableNodeModel::slotBeginServerScan --> " << server->realServer()->name() <<
                      " " << num;
#endif

	if(num >0)
	{
		int count=num;
        int start=data_->position(server->tableServer());
        beginInsertRows(QModelIndex(),start,start+count-1);
	}
}

void TableNodeModel::slotEndServerScan(VModelServer* server,int num)
{
	assert(active_ == true);

#ifdef _UI_TABLENODEMODEL_DEBUG
     UiLog().dbg() << "TableNodeModel::slotEndServerScan --> " << server->realServer()->name() <<
                      " " << num;
     QTime t;
     t.start();
#endif

	if(num >0)
		endInsertRows();

#ifdef _UI_TABLENODEMODEL_DEBUG
     UiLog().dbg() << "  elapsed: " << t.elapsed() << " ms";
     UiLog().dbg() << "<-- slotEndServerScan";
#endif
}

void TableNodeModel::slotBeginServerClear(VModelServer* server,int num)
{
    Q_ASSERT(active_ == true);
    Q_ASSERT(server);

	if(num >0)
	{
		int start=-1;
		int count=-1;
        data_->position(server->tableServer(),start,count);

        Q_ASSERT(start >=0);
        Q_ASSERT(count == num);

        beginRemoveRows(QModelIndex(),start,start+count-1);
	}
}

void TableNodeModel::slotEndServerClear(VModelServer* server,int num)
{
	assert(active_ == true);

	if(num >0)
		endRemoveRows();
}
