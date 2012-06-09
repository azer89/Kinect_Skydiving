
#ifndef _MyMotionState_h_
#define _MyMotionState_h_

#include "OgreToBulletMesh.h"

class MyMotionState : public btDefaultMotionState
{
protected:
	Ogre::SceneNode * mVisibleObj;
	btTransform mTransform;
	btTransform mCOM;

public:
	MyMotionState(const btTransform & initialPos, Ogre::SceneNode * node)
	{
		mVisibleObj = node;
		mTransform = initialPos;
		mCOM = btTransform::getIdentity();
	}

	MyMotionState(const btTransform & initialPos)
	{
		mVisibleObj = NULL;
		mTransform = initialPos;
	}

	~MyMotionState() {}

	void setNode(Ogre::SceneNode * node)
	{
		mVisibleObj = node;
	}

	btTransform getWorldTransform() const
	{
		return mCOM.inverse() * mTransform;
	}

	void getWorldTransform(btTransform & worldTrans) const
	{
		worldTrans = mCOM.inverse() * mTransform;
	}

	void setWorldTransform(const btTransform & worldTrans)
	{
		if (mVisibleObj == NULL)
			return;

		mTransform = worldTrans;
		btTransform transform = mTransform * mCOM;
		btQuaternion rot = transform.getRotation();
		btVector3 pos = transform.getOrigin();
		mVisibleObj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
		mVisibleObj->setPosition(pos.x(), pos.y(), pos.z());
	}
};

class MyPhysics
{
private:
	btAlignedObjectArray<btCollisionShape*> mCollisionShapes;
	btBroadphaseInterface * mBroadphase;
	btCollisionDispatcher * mDispatcher;
	btConstraintSolver * mSolver;
	btDefaultCollisionConfiguration * mColConfig;
	btDiscreteDynamicsWorld * mWorld;
	Ogre::SceneNode * mRootSceneNode;

public:
	MyPhysics()
	{
		mColConfig = new btDefaultCollisionConfiguration;
		mDispatcher = new btCollisionDispatcher(mColConfig);
		mBroadphase = new btDbvtBroadphase;
		mSolver = new btSequentialImpulseConstraintSolver;
		mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mColConfig);

		mWorld->setGravity(btVector3(0, -10, 0));
		mRootSceneNode = 0;
	}

	~MyPhysics()
	{
		for (int i = mWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject * obj = mWorld->getCollisionObjectArray()[i];
			btRigidBody * body = btRigidBody::upcast(obj);

			if (body && body->getMotionState())
				delete body->getMotionState();

			mWorld->removeCollisionObject(obj);

			delete obj;
		}

		delete mWorld;
		delete mSolver;
		delete mBroadphase;
		delete mDispatcher;
		delete mColConfig;
	}

	void addStaticPlane(Ogre::SceneNode * node)
	{
		btCollisionShape * groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 2);
		mCollisionShapes.push_back(groundShape);
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, 0, 0));

		MyMotionState * motionState = new MyMotionState(groundTransform, node);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, groundShape, btVector3(0, 0, 0));
		btRigidBody * body = new btRigidBody(rbInfo);

		mWorld->addRigidBody(body);
	}

	void addStaticPlane2(Ogre::SceneNode * node)
	{
		Ogre::Vector3 size =node->getAttachedObject(0)->getBoundingBox().getHalfSize();
		btCollisionShape * groundShape = new btBoxShape(btVector3(size.x, 1, size.z));
		mCollisionShapes.push_back(groundShape);
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, 0, 0));

		float yaw = node->getOrientation().getYaw().valueRadians();
		float pitch = node->getOrientation().getPitch().valueRadians();
		float roll = node->getOrientation().getRoll().valueRadians();
		groundTransform.setRotation(btQuaternion(yaw,pitch,roll));

		MyMotionState * motionState = new MyMotionState(groundTransform, node);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, groundShape, btVector3(0, 0, 0));
		btRigidBody * body = new btRigidBody(rbInfo);

		mWorld->addRigidBody(body);
	}
	btTriangleMesh * createTriangleMesh(Ogre::Entity* ent)
	{
		OgreToBulletMesh* ogreBulletMesh = new OgreToBulletMesh();
		btTriangleMesh* triMesh = ogreBulletMesh->convertMesh(ent->getMesh());
		return triMesh;
	}
	btRigidBody * addStaticTriangleMesh(Ogre::SceneNode* node, btTriangleMesh* triMesh)
	{
		OgreToBulletMesh* ogreBulletMesh = new OgreToBulletMesh();
		//btTriangleMesh* triMesh = ogreBulletMesh->convertMesh(dynamic_cast<Ogre::Entity*>(node->getAttachedObject(0))->getMesh());
		//triMesh->setScaling(btVector3(node->getScale().x,node->getScale().y,node->getScale().z));
		btBvhTriangleMeshShape* collisionShape = new btBvhTriangleMeshShape(triMesh,true);
		mCollisionShapes.push_back(collisionShape);
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(node->getPosition().x,node->getPosition().y,node->getPosition().z));

		float yaw = node->getOrientation().getYaw().valueRadians();
		float pitch = node->getOrientation().getPitch().valueRadians();
		float roll = node->getOrientation().getRoll().valueRadians();

		printf("p: %.2f, y: %.2f, r: %.2f\n",pitch,yaw,roll);
		//groundTransform.setRotation(btQuaternion(yaw,pitch,roll));
		groundTransform.setRotation(btQuaternion(node->getOrientation().x,node->getOrientation().y,node->getOrientation().z,node->getOrientation().w));

		MyMotionState * motionState = new MyMotionState(groundTransform, node);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, collisionShape, btVector3(0, 0, 0));
		btRigidBody * body = new btRigidBody(rbInfo);
		mWorld->addRigidBody(body);
		return body;
	}

	btRigidBody * addDynamicBox(Ogre::SceneNode * node, float m = 1.0f)
	{
		btCollisionShape * colShape = new btBoxShape(btVector3(1, 1, 1));
		mCollisionShapes.push_back(colShape);
		btTransform boxTransform;
		boxTransform.setIdentity();

		btScalar mass(m);
		btVector3 localInertia(0, 0, 0);

		colShape->calculateLocalInertia(mass, localInertia);

		boxTransform.setOrigin(btVector3(node->getPosition().x, node->getPosition().y, node->getPosition().z));

		MyMotionState * motionState = new MyMotionState(boxTransform, node);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, colShape, localInertia);
		btRigidBody * body = new btRigidBody(rbInfo);

		mWorld->addRigidBody(body);

		return body;
	}

	btRigidBody * addRigidBody(const btTransform &transform, btCollisionShape * shape, btScalar mass, Ogre::SceneNode * node = NULL)
	{
		mCollisionShapes.push_back(shape);
		btVector3 localInertia(0, 0, 0);

		shape->calculateLocalInertia(mass, localInertia);
		btDefaultMotionState *motionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
		btRigidBody * body = new btRigidBody(rbInfo);

		mWorld->addRigidBody(body);

		return body;
	}

	void addCollisionShape(btCollisionShape * colShape)
	{
		mCollisionShapes.push_back(colShape);
	}

	btDiscreteDynamicsWorld * getDynamicsWorld()
	{
		return mWorld;
	}

	btCollisionWorld * getCollisionWorld()
	{
		return mWorld->getCollisionWorld();
	}

	btBroadphaseInterface * getBroadphase()
	{
		return mBroadphase;
	}

	void setRootSceneNode(Ogre::SceneNode * node)
	{
		mRootSceneNode = node;
	}

	btVector3 toBullet(const Ogre::Vector3 & vec) const
	{
		return btVector3(vec.x, vec.y, vec.z);
	}

	void shootBox(const Ogre::Vector3 & camPosition)
	{
		if (mRootSceneNode)
		{
			Ogre::SceneNode * node = mRootSceneNode->createChildSceneNode(camPosition+Ogre::Vector3(0,5,0));
			btRigidBody * box = addDynamicBox(node);
			box->applyCentralImpulse(btVector3(1, 0, 0));
		}
	}

	static void debugBtVector3(const btVector3 & vec, const char * str = 0)
	{
		std::cout << str << " x: " << vec.x() << "; y: " << vec.y() << "; z: " << vec.z() << std::endl;
	}
};

#endif

