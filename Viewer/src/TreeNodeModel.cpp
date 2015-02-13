//============================================================================
// Copyright 2014 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//============================================================================

#include "TreeNodeModel.hpp"

#include <QDebug>

#include "ChangeMgrSingleton.hpp"

#include "NodeModelData.hpp"
#include "ServerFilter.hpp"
#include "ServerHandler.hpp"
#include "VFilter.hpp"
#include "VNState.hpp"
#include "VSState.hpp"
#include "VAttribute.hpp"
#include "VIcon.hpp"

//=======================================
//
// TreeNodeModel
//
//=======================================

TreeNodeModel::TreeNodeModel(NodeModelDataHandler *data,AttributeFilter *atts,IconFilter* icons,QObject *parent) :
   AbstractNodeModel(data,icons,parent),
   atts_(atts)

{
	//Attribute filter changes
	connect(atts_,SIGNAL(changed()),
				this,SIGNAL(filterChanged()));

	//Icon filter changes
	connect(icons_,SIGNAL(changed()),
			this,SLOT(slotIconFilterChanged()));


	//When the underlying data changes

	//Filter changed
	connect(data_,SIGNAL(filterChanged()),
				this,SIGNAL(filterChanged()));

	//Server added
	connect(data_,SIGNAL(serverAddBegin(int)),
				this,SLOT(slotServerAddBegin(int)));

	connect(data_,SIGNAL(serverAddEnd()),
								this,SLOT(slotServerAddEnd()));

	//Server removed
	connect(data_,SIGNAL(serverRemoveBegin(int)),
						this,SLOT(slotServerRemoveBegin(int)));

	connect(data_,SIGNAL(serverRemoveEnd()),
							this,SLOT(slotServerRemoveEnd()));

	//Node changes
	connect(data_,SIGNAL(dataChanged(ServerHandler*)),
			this,SLOT(slotDataChanged(ServerHandler*)));

	connect(data_,SIGNAL(dataChanged(ServerHandler*,Node*)),
				this,SLOT(slotDataChanged(ServerHandler*,Node*)));

}

int TreeNodeModel::columnCount( const QModelIndex& /*parent */ ) const
{
   	 return 1;
}

int TreeNodeModel::rowCount( const QModelIndex& parent) const
{
	//qDebug() << "rowCount" << parent;

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
	//Parent is the root
	else if(!parent.isValid())
	{
		//qDebug() << "rowCount" << parent << servers_.count();
		return data_->count();
	}
	//The parent is a server
	else if(isServer(parent))
	{
		if(ServerHandler *server=indexToServer(parent))
		{
			//We show the whole tree for the server
			//qDebug() << "  -->suiteNum" <<server->numSuites();
			return server->numSuites();
		}
	}
	//The parent is a node
	else if(Node* parentNode=indexToNode(parent))
	{
		int num=VAttribute::totalNum(parentNode);
		//qDebug() << "  -->node" << parentNode->name().c_str() << num << ServerHandler::numOfImmediateChildren(parentNode);
		return num+ServerHandler::numOfImmediateChildren(parentNode);
	}

	return 0;
}

Qt::ItemFlags TreeNodeModel::flags ( const QModelIndex & index) const
{
	Qt::ItemFlags defaultFlags;

	defaultFlags=Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	return defaultFlags;
}

QVariant TreeNodeModel::data( const QModelIndex& index, int role ) const
{
	//Data lookup can be costly so we immediately return a default value for all
	//the cases where the default should be used.
	if( !index.isValid() ||
	   (role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::BackgroundRole &&
	    role != FilterRole && role != IconRole && role != ServerRole))
    {
		return QVariant();
	}

	//Identifies server
	if(role == ServerRole)
	{
		if(isServer(index))
			return 0;
		else
			return -1;
	}
	//Server
	int id;
	if(isServer(index))
	{
		return serverData(index,role);
	}
	else if(isNode(index))
		return nodeData(index,role);
	else if(isAttribute(index))
		return attributesData(index,role);

	return QVariant();
}

QVariant TreeNodeModel::serverData(const QModelIndex& index,int role) const
{
	if(role == IconRole)
			return QVariant();

	if(role == FilterRole)
		return true;

	else if(index.column() == 0 && role == Qt::BackgroundRole)
	{
		if(ServerHandler *server=indexToServer(index))
				return VSState::toColour(server);
		else
		    return Qt::gray;
	}

	else if(index.column() == 0 || index.column() == 1)
	{
		if(role == Qt::DisplayRole)
		{
			if(ServerHandler *server=indexToServer(index))
			{
				if(index.column() == 0)
					return QString::fromStdString(server->longName());
				else if(index.column() == 1)
					return VSState::toName(server);
			}
		}
	}
	return QVariant();
}

QVariant TreeNodeModel::nodeData(const QModelIndex& index, int role) const
{
	Node* node=indexToNode(index);
	if(!node)
		return QVariant();

	if(role == Qt::DisplayRole)
	{
		switch(index.column())
		{
		case 0:	 return QString::fromStdString(node->name());
		case 1: return VNState::toName(node);
		case 2: return QString::fromStdString(node->absNodePath()); //QString().sprintf("%08p", node); //QString::fromStdString(node->absNodePath());
		default: return QVariant();
		}
	}
	else if(role == FilterRole)
	{
		return QVariant(data_->isFiltered(node));
	}
	else if(index.column() == 0 && role == Qt::BackgroundRole)
	{
		return VNState::toColour(node);
	}
	else if(index.column() == 0 && role == IconRole)
	{
		if(icons_->isEmpty())
			return QVariant();
		else
			return VIcon::pixmapList(node,icons_);
	}
	else if(index.column() == 0 && role == Qt::ToolTipRole)
	{
		return VNState::toName(node);
	}


	return QVariant();
}

//=======================================================================
//
// Attributes data
//
//=======================================================================

QVariant TreeNodeModel::attributesData(const QModelIndex& index, int role) const
{
	if(role == IconRole)
			return QVariant();

	if(index.column()!=0)
		return QVariant();

	if(role != Qt::BackgroundRole && role != FilterRole && role != Qt::DisplayRole)
		return QVariant();

	if(role == Qt::BackgroundRole)
		return QColor(220,220,220);


	if(role == FilterRole)
	{
		if(atts_->isEmpty())
			return QVariant(false);
	}

	//Here we have to be sure this is an attribute!
	Node *node=static_cast<Node*>(index.internalPointer());
	if(!node)
		return QVariant();

	QStringList attrData;
	//VParam::Type type;
	VAttribute* type;
	VAttribute::getData(node,index.row(),&type,attrData);

	if(role == FilterRole)
	{
		if(atts_->isSet(type))
		{
			return QVariant(true);
		}
		return QVariant(false);
	}
	else if(role == Qt::DisplayRole)
	{
		return attrData;
	}

	return QVariant();
}

QVariant TreeNodeModel::headerData( const int section, const Qt::Orientation orient , const int role ) const
{
	//No header!!!
	return QVariant();

	if ( orient != Qt::Horizontal || role != Qt::DisplayRole )
      		  return QAbstractItemModel::headerData( section, orient, role );

   	switch ( section )
	{
   	case 0: return tr("Node");
   	case 1: return tr("Status");
   	case 2: return tr("Path");
   	default: return QVariant();
   	}

    return QVariant();
}

QModelIndex TreeNodeModel::index( int row, int column, const QModelIndex & parent ) const
{
	if(!hasData() || row < 0 || column < 0)
	{
		return QModelIndex();
	}

	//When parent is the root this index refers to a server
	if(!parent.isValid())
	{
		//For the server the internal pointer is NULL
		if(row < data_->count())
		{
			void* p=NULL;
			//qDebug() << "SERVER" << parent;
			return createIndex(row,column,(void*)NULL);
		}
	}

	//Here we are under one of the servers
	else
	{
		//The parent is a server: this index refers to a suite. We set the
		//server as an internal pointer
		if(ServerHandler* server=indexToServer(parent))
		{
			//qDebug() << "NODE1" << parent << server->name().c_str();
			return createIndex(row,column,server);
		}
		//Parent is not the server: the parent must be another node
		else if(Node* parentNode=indexToNode(parent))
		{
			//qDebug() << "NODE2" << parent << parentNode->name().c_str() << VAttribute::totalNum(parentNode);
			return createIndex(row,column,parentNode);
		}

		//qDebug() << "BAD" << parent;
	}

	//qDebug() << "EMPTY" << parent;
	return QModelIndex();

}

QModelIndex TreeNodeModel::parent(const QModelIndex &child) const
{
	//If the child is a server the parent is the root
	if(isServer(child))
		return QModelIndex();


	//Child is a suite. Its internal pointer points to a server.
	if(ServerHandler *s=data_->server(child.internalPointer()))
	{
		//The parent is a server
		int serverIdx=data_->indexOf(s);
		return createIndex(serverIdx,0,(void*)NULL);
	}

	//Child is a non-suite node or an attribute. Its internal pointer points to the parent node.
	else if(Node *parentNode=static_cast<Node*>(child.internalPointer()))
	{
		//The parent is a suite: its internal pointer is the server
		if(parentNode->isSuite())
		{
			if(ServerHandler* server=ServerHandler::find(parentNode))
			{
				return createIndex(server->indexOfSuite(parentNode),0,server);
			}
		}
		//The parent is a non-suite node: its internal pointer
		//is its parent (i.e.. the grandparent)
		else if(Node *grandParentNode=parentNode->parent())
		{
			size_t pos=parentNode->position();
			if(pos != std::numeric_limits<std::size_t>::max())
				return createIndex(pos+VAttribute::totalNum(grandParentNode),0,grandParentNode);
		}
	}

	return QModelIndex();
}

//----------------------------------------------
//
// Server to index mapping and lookup
//
//----------------------------------------------

bool TreeNodeModel::isServer(const QModelIndex & index) const
{
	return (index.isValid() && index.internalPointer() == NULL);
}

bool TreeNodeModel::isNode(const QModelIndex & index) const
{
	return (indexToNode(index) != NULL);
}

bool TreeNodeModel::isAttribute(const QModelIndex & index) const
{
	return (index.isValid() && !isNode(index) && !isServer(index));
}

ServerHandler* TreeNodeModel::indexToServer(const QModelIndex & index) const
{
	//For servers the internal id is a nul pointer
	if(index.isValid())
	{
		if(index.internalPointer() == NULL)
			return data_->server(index.row());
	}

	return NULL;
}

QModelIndex TreeNodeModel::serverToIndex(ServerHandler* server) const
{
	//For servers the internal id is set to their position in servers_ + 1
	int i;
	if((i=data_->indexOf(server))!= -1)
			return createIndex(i,0,(void*)NULL);

	return QModelIndex();
}

Node* TreeNodeModel::indexToNode( const QModelIndex & index) const
{
	if(index.isValid() && !isServer(index))
	{
		//If suite (the internal pointer points to a server id pointer
		if(ServerHandler *s=data_->server(index.internalPointer()))
		{
			return s->suiteAt(index.row()).get();
		}
		//Other nodes
		else if(Node *parentNode=static_cast<Node*>(index.internalPointer()))
		{
			int attNum=VAttribute::totalNum(parentNode);
			if(index.row() >= attNum)
			{
				return ServerHandler::immediateChildAt(parentNode,index.row()-attNum);
			}
		}
	}

	return NULL;
}

//Find the index for the node!
QModelIndex TreeNodeModel::nodeToIndex(Node* node, int column) const
{
	if(!node)
		return QModelIndex();

	//If the node is a suite
	if(node->isSuite())
	{
		if(ServerHandler* server=ServerHandler::find(node))
		{
				int row=server->indexOfSuite(node);
				if(row != -1)
						return createIndex(row,column,server);
		}
	}

	//Other nodes
	else if(Node *parentNode=node->parent())
	{
		int row=ServerHandler::indexOfImmediateChild(node);
		if(row != -1)
		{
			int attNum=VAttribute::totalNum(parentNode);
			row+=attNum;
			return createIndex(row,column,parentNode);
		}
	}

	return QModelIndex();
}

//Find the index for the node when we know what the server is!
QModelIndex TreeNodeModel::nodeToIndex(ServerHandler* server,Node* node, int column) const
{
	if(!node)
		return QModelIndex();

	//If the node is a suite
	if(node->isSuite())
	{
		if(server)
		{
				int row=server->indexOfSuite(node);
				if(row != -1)
						return createIndex(row,column,server);
		}
	}

	//Other nodes
	else if(Node *parentNode=node->parent())
	{
		int row=ServerHandler::indexOfImmediateChild(node);
		if(row != -1)
		{
			int attNum=VAttribute::totalNum(parentNode);
			row+=attNum;
			return createIndex(row,column,parentNode);
		}
	}

	return QModelIndex();
}

//------------------------------------------------------------------
// Create info object to index. It is used to identify nodes in
// the tree all over in the programme outside the view.
//------------------------------------------------------------------

VInfo_ptr TreeNodeModel::nodeInfo(const QModelIndex& index)
{
	//For invalid index no info is created.
	if(!index.isValid())
	{
		VInfo_ptr res;
		return res;
	}

	//Check if the node is a server
	if(ServerHandler *s=indexToServer(index))
	{
		VInfo_ptr res(VInfo::make(s));
		return res;
	}

    //Check if the node is a suite (the internal pointer points to a server id pointer)
    if(ServerHandler *s=data_->server(index.internalPointer()))
	{
		Node* node=s->suiteAt(index.row()).get();
		VInfo_ptr res(VInfo::make(node));
		return res;
	}
	//Other nodes
	else if(Node *parentNode=static_cast<Node*>(index.internalPointer()))
	{
		int attNum=VAttribute::totalNum(parentNode);
		if(index.row() >= attNum)
		{
			Node *node=ServerHandler::immediateChildAt(parentNode,index.row()-attNum);
			VInfo_ptr res(VInfo::make(node));
			return res;
		}
		//It is an attribute
		else
		{
			//VParam::Type type;
			/*VAttribute* type;
			VAttribute::getData(parentNode,index.row(),&type,&row);
			VInfo_ptr res(new VInfo::make(node,type,row));
			return res;*/
		}
	}

    VInfo_ptr res;
	return res;
}

//----------------------------------------
// Filter
//----------------------------------------

//This slot is called when the icon filter changes
void TreeNodeModel::slotIconFilterChanged()
{
	//This should trigger the re-rendering of the whole view according to
	//some blogs. However, this does not work!!!!!
	//Q_EMIT dataChanged(QModelIndex(), QModelIndex());

	//This is very expensive!! There should be a better way!!!
	Q_EMIT filterChanged();
}

//Server is about to be added
void TreeNodeModel::slotServerAddBegin(int row)
{
	beginInsertRows(QModelIndex(),row,row);
}

//Addition of the new server has finished
void TreeNodeModel::slotServerAddEnd()
{
	endInsertRows();
}

//Server is about to be removed
void TreeNodeModel::slotServerRemoveBegin(int row)
{
	beginRemoveRows(QModelIndex(),row,row);
}

//Removal of the server has finished
void TreeNodeModel::slotServerRemoveEnd()
{
	endRemoveRows();
}

void TreeNodeModel::slotDataChanged(ServerHandler* server)
{

}


void TreeNodeModel::slotDataChanged(ServerHandler* server,Node* node)
{
	if(!node)
		return;

	qDebug() << "observer is called" << QString::fromStdString(node->name());
	//for(unsigned int i=0; i < types.size(); i++)
	//	qDebug() << "  type:" << types.at(i);

	QModelIndex index=nodeToIndex(node,0);

	if(!index.isValid())
		return;

	/*Node *nd1=indexToNode(index1);
	Node *nd2=indexToNode(index2);

	if(!nd1 || !nd2)
		return;

	//qDebug() << "indexes" << index1 << index2;
	//qDebug() << "index pointers " << index1.internalPointer() << index2.internalPointer();
	qDebug() << "    --->" << QString::fromStdString(nd1->name()) << QString::fromStdString(nd2->name());*/

	Q_EMIT dataChanged(index,index);


}






