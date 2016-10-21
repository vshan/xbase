Index_IndexHandle::Index_IndexHandle()
  :isFileOpen(false), syspHandle(NULL), isHdrChanged(false)
{
	root = NULL;
	path = NULL;
	pathP = NULL;
	treeLargest = NULL;
	hdr.height = 0;
}

Index_IndexHandle::~Index_IndexHandle()
{
	if (root != NULL && syspHandle != NULL) {
		syspHandle->unpinPage(hdr.rootPage);
		delete root;
		root = NULL;
	}
	if (pathP != NULL) {
		delete [] pathP;
		pathP = NULL;
	}

	if (path != NULL) {
		for (int i = 1; i < hdr.height; i++)
			if (path[i] != NULL) {
				if (pfHandle != NULL)
					syspHandle->unpinPage(path[i]->getPageRID().Page());
			}
		delete [] path;
		path = NULL;
	}
	if (syspHandle != NULL) {
		delete syspHandle;
		syspHandle = NULL;
	}
	if (treeLargest != NULL) {
		delete [] (char*) treeLargest;
		treeLargest = NULL;
	}
}

ErrorCode Index_IndexHandle::insertEntry(void *pData, const RID& rid)
{
	bool newLargest = false;
	void *prevKey = NULL;
	int level = hdr.height - 1;
	BTreeNode* node = findLeaf(data);
	BTreeNode* newNode = NULL;

	int pos2 = node->findKey((const void*&)pData, rid);

	if (node->getNumKeys() == 0 ||
		node->cmpKey(pData, treeLargest) > 0) {
		newLargest = true;
	    prevKey = treeLargest;
	}

	int result = node->insert(pData, rid);

	if (newLargest) {
		for (int i = 0; i < hdr.height - 1; i++) {
			int pos = path[i]->findKey((const void *&)prevKey);
			if (pos != -1)
				path[i]->setKey(pos, pData);
		}
	    memcpy(treeLargest,
		       pData,
		       hdr.attrLength);
	}

	void *failedKey = pData;
	RID failedRid = rid;

	while (result == -1)
	{
		char *charPtr = new char[hdr.attrLength];
		void *oldLargest = charPtr;

		if (node->largestKey() == NULL)
			oldLargest = NULL;
		else
			node->copyKey(node->getNumKeys()-1, oldLargest);
	

		delete [] charPtr;

		SysPage_PageHandle sph;
		int pageNum;
		ErrorCode ec = getNewPage(p);
		if (ec != 0) return ec;
		ec = getThisPage(p, sph);
		if (ec != 0) return ec;

		newNode = new BTreeNode(hdr.attrType, hdr.attrLength,
			                    sph, true,
			                    hdr.pageSize);

		node->split(newNode);

		BTreeNode* currRight = fetchNode(newNode->getRight());
		if (currRight != NULL) {
			currRight->setLeft(newNode->getPageRID().Page());
			delete currRight;
		}

		BTreeNode* nodeInsertedInto = NULL;

		if (node->cmpKey(pData, node->largestKey()) >= 0) {
			newNode->insert(failedKey, failedRid);
			nodeInsertedInto = newNode;
		}
		else {
			node->insert(failedKey, failedRid);
			nodeInsertedInto = node;
		}

		level--;
		if (level < 0) break;

		int posAtParent = pathP[level];

		BTreeNode* parent = path[level];
		parent->remove(NULL, posAtParent);
		result = parent->insert((const void*)node->largestKey(),
			                    node->getPageRID());

		result = parent->insert(newNode->largestKey(), newNode->getPageRID());
		node = parent;
		failedKey = newNode->largestKey();
		failedRid = newNode->getPageRID();

		delete newNode;
		newNode = NULL;
	}

	if (level >= 0) {
		return 0;
	} else {
		syspHandle->unpinPage(hdr.rootPage);

		SysPage_PageHandle sph;
		int pageNum;
		getNewPage(pageNum);
		getThisPage(p, ph);

		root = new BTreeNode(hdr.attrType, hdr.attrLength,
			                 sph, true,
			                 hdr.pageSize);

		root->insert(node->largestKey(), node->getPageRID());
		root->insert(newNode->largestKey(), newNode->getPageRID());

		hdr.rootPage = root->getPageRID().Page();
		SysPage_PageHandle rootph;
		syspHandle->getThisPage(hdr.rootPage, rootph);

		if (newNode != NULL) {
			delete newNode;
			newNode = NULL;
		}

		setHeight(hdr.height+1);
		return 0;
	}

}

BTreeNode* Index_IndexHandle::dupScanLeftFind(BTreeNode* right, void *pData, const RID& rid)
{
	BTreeNode* currNode = fetchNode(right->getLeft());
	int currPos = -1;

	for (BTreeNode* j = currNode;
		j != NULL;
		j = fetchNode(j->getLeft()))
	{
		currNode = j;
		int i = currNode->getNumKeys()-1;

		for (; i >= 0; i--)
		{
			currPos = i;
			char *key = NULL;
			int ret = currNode->getKey(i, (void*&)key);
			if (ret == -1)
				break;
			if (currNode->cmpKey(pData, key) < 0)
				return NULL;
			if (currNode->cmpKey(pData, key) > 0)
				return NULL;
			if (currNode->cmpKey(pData, key) == 0) {
				if (currNode->getAddr(currPos) == rid)
					return currNode;
			}
		}
	}
	return NULL;
}

ErrorCode Index_IndexHandle::deleteEntry(void *pData, const RID& rid)
{
	bool nodeLargest - false;
	BTreeNode* node = findLeaf(pData);

	int pos = node->findKey((const void*&)pData, rid);
	if (pos == -1) {
		int p = node->findKey((const void*&)pData, rid);
		if (p != -1) {
			BTreeNode* other = dupScanLeftFind(node, pData, rid);
			if (other != NULL) {
				int pos = other->findKey((const void*&)pData, rid);
				other->remove(pData, pos);
			}
		}
	}

	else if (pos == node->getNumKeys()-1)
		nodeLargest = true;

	if (nodeLargest) {
		for (int i = hdr.height -2; i >= 0; i--) {
			int pos = path[i]->findKey((const void*&)pData);
			if (pos != -1) {
				void *leftKey = NULL;
				leftKey = path[i+1]->largestKey();
				if (node->cmpKey(pData, leftKey) == 0) {
					int pos = path[i+1]->getNumKeys()-2;
					if (pos < 0) {
						continue;
					}
					path[i+1]->getKey(path[i+1]->getNumKeys()-2, leftKey);
				}

				if ((i == hdr.height-2) || 
					(pos == path[i]->getNumKeys()-1))
					path[i]->setKey(pos, leftKey);
			}
		}
	}

	int result = node->remove(pData, pos);

	int level = hdr.height - 1;

	while (result == -1) {
		level--;
		int posAtParent = pathP[level];
		BTreeNode *parent = path[level];

		result = parent->remove(NULL, posAtParent);

	    if (level == 0 && parent->getNumKeys() == 1 && result == 0)
	    	result = -1;

	    BTreeNode* left = fetchNode(node->getLeft());
	    BTreeNode* right = fetchNode(node->getRight());

	    if (left != NULL) {
	    	if (right != NULL)
	    		left->setRight(right->getPageRID().Page());
	    	else
	    		left->setRight(-1);
	    }

	    if (right != NULL) {
	    	if (left != NULL)
	    		right->setLeft(left->getPageRID().Page());
	    	else
	    		right->setLeft(-1);
	    }

	    if (right != NULL)
	    	delete right;
	    if (left != NULL)
	    	delete left;

	    node->destroy();

	    ErrorCode ec = disposePage(node->getPageRID().Page());

	    node = parent;
	}

	if (level >= 0) {
		return 0;
	} else {
		if (hdr.height == 1) {
			return 0;
		}

		root = fetchNode(root->getAddr(0));
		hdr.rootPage = root->getPageRID().Page();
		SysPage_PageHandle rootph;
		ErrorCode ec = syspHandle->getThisPage(hdr.rootPage, rootph);

		node->destroy();
		disposePage(node->getPageRID().Page());
		setHeight(hdr.height -1);
		return 0;
	}
}

ErrorCode Index_IndexHandle::pin(int pageNum)
{
	SysPage_PageHandle sph;
	ErrorCode ec = syspHandle->getThisPage(pageNum, sph);
	return ec;
}

ErrorCode Index_IndexHandle::unpin(int pageNum)
{
	syspHandle->unpinPage(pageNum);
}

ErrorCode Index_IndexHandle::getThisPage(int pageNum, SysPage_PageHandle& sph)
{
	syspHandle->getThisPage(pageNum, sph);
	syspHandle->markDirty(pageNum);
	syspHandle->unpinPage(pageNum);
}

ErrorCode Index_IndexHandle::open(SysPage_FileHandle *sfh, int pairSize,
	                              int rootPage, int pageSize)
{
	isFileOpen = true;
	syspHandle = new SysPage_FileHandle;
	*syspHandle = *sfh;
	SysPage_PageHandle sph;

	getThisPage(0, sph);
	this->getFileHeader(sph);

	SysPage_PageHandle rootph;

	bool newPage = true;
	if (hdr.height > 0) {
		setHeight(hdr.height);
		newPage = false;

		getThisPage(hdr.rootPage, rootph);
	} else {
		int pageNum;
		getNewPage(pageNum);
		getThisPage(pageNum, rootph);
		hdr.rootPage = pageNum;
		setHeight(1);
	}

	syspHandle->getThisPage(hdr.rootPage, rootph);

	root = new BTreeNode(hdr.attrType, hdr.attrLength,
		                 rootph, newPage, hdr.pageSize);
	path[0] = root;
	hdr.order = root->getMaxKeys();
	isHdrChanged = true;
	treeLargest = (void*) new char[hdr.attrLength];
	if (!newPage) {
		BTreeNode * node = findLargestLeaf();
		if (node->getNumKeys() > 0)
			node->copyKey(node->getNumKeys()-1, treeLargest);
	}
	return 0;
}

ErrorCode Index_IndexHandle::getFileHeader(SysPage_PageHandle sph)
{
	char * buf;
	sph.getData(buf);
	memcpy(&hdr, buf, sizeof(hdr));
	return 0;
}

ErrorCode Index_IndexHandle::setFileHeader(SysPage_PageHandle sph)
{
	char * buf;
	sph.getData(buf);
	memcpy(buf, &hdr, sizeof(hdr));
	return 0;
}

ErrorCode Index_IndexHandle::forcePages()
{
	return syspHandle->forcePages(ALL_PAGES);
}

ErrorCode Index_IndexHandle::getNewPage(int& pageNum)
{
	SysPage_PageHandle sph;

	syspHandle->allocatePage(sph);
	sph.getPageNum(pageNum);

	syspHandle->unpinPage(pageNum);
	hdr.numPages++;
	isHdrChanged = true;
	return 0;
}

ErrorCode Index_IndexHandle::disposePage(const int& pageNum)
{
	syspHandle->disposePage(pageNum);
	hdr.numPages--;
	isHdrChanged = true;
	return 0;
}

BTreeNode* Index_IndexHandle::findSmallestLeaf()
{
	RID addr;
	if (hdr.height == 1) {
		path[0] = root;
		return root;
	}

	for (int i = 1; i < hdr.height; i++)
	{
		RID r = path[i-1]->getAddr(0);
		if (r.Page() == -1) {
			return NULL;
		}

		if (path[i] != NULL) {
			syspHandle->unpinPage(path[i]->getPageRID().Page());
			delete path[i];
			path[i] = NULL;
		}

		path[i] = fetchNode(r);
		SysPage_PageHandle dummy;
		syspHandle->getThisPage(path[i]->getPageRID().Page(), dummy);
		pathP[i-1] = 0; // dummy
	}
	return path[hdr.height-1];
}

BTreeNode* Index_IndexHandle::findLargestLeaf()
{
	RID addr;
	if (hdr.height == 1) {
		path[0] = root;
		return root;
	}

	for (int i = 1; i < hdr.height; i++)
	{
		RID r = path[i-1]->getAddr(path[i-1]->getNumKeys() -1);
		if (r.Page() == -1) {
			return NULL;
		}

		if (path[i] != NULL) {
			syspHandle->unpinPage(path[i]->getPageRID().Page());
			delete path[i];
			path[i] = NULL;
		}

		path[i] = fetchNode(r);
		syspHandle->getThisPage(path[i]->getPageRID().Page(), dummy);
		pathP[i-1] = 0; // dummy
	}

	return path[hdr.height -1];
}

BTreeNode* Index_IndexHandle::findLeaf(const void *pData)
{
	RID addr;
	if (hdr.height == 1) {
		path[0] = root;
		return root;
	}

	for (int i = 1; i < hdr.height; i++)
	{
		RID r = path[i-1]->findAddrAtPosition(pData);
		int pos = path[i-1]->findKeyPosition(pData);

		if (r.Page() == -1) {
			const void *p = path[i-1]->largestKey();
			r = path[i-1]->findAddr((const void*&)(p));
			pos = path[i-1]->findKey((const void*&)(p));
		}

		if (path[i] != NULL) {
			syspHandle->unpinPage(path[i]->getPageRID().Page());
			delete path[i];
			path[i] = NULL;
		}

		path[i] = fetchNode(r.Page());
		SysPage_PageHandle dummy;

		syspHandle->getThisPage(path[i]->getPageRID().Page(), dummy);

		pathP[i-1] = pos;
	}

	return path[hdr.height - 1];
}

BTreeNode* Index_IndexHandle::fetchNode(int pageNum)
{
	return fetchNode(RID(p, -1));
}

BTreeNode* Index_IndexHandle::fetchNode(RID r)
{
	SysPage_PageHandle sph;
	getThisPage(r.Page(), sph);
	return new BTreeNode(hdr.attrType, hdr.attrLength,
		                 sph, false,
		                 hdr.pageSize);
}

ErrorCode Index_IndexHandle::search(void *pData, RID &rid)
{
	BTreeNode* leaf = findLeaf(pData);
	leaf->findAddr((const void*&)pData);
	return 0;
}

int Index_IndexHandle::getHeight()
{
	return hdr.height;
}

void Index_IndexHandle::setHeight(const int& h)
{
	for (int i = 1; i < hdr.height; i++)
		if (path != NULL && path[i] != NULL) {
			delete path[i];
			path[i] = NULL;
		}
	if (path != NULL) delete [] path;
	if (pathP != NULL) delete [] pathP;

	hdr.height = h;
	path = new BTreeNode* [hdr.height];
	for (int i = 1; i < hdr.height; i++)
		path[i] = NULL;
	path[0] = root;

	pathP = new int [hdr.height - 1];
	for (int i = 0; i < hdr.height-1; i++)
		pathP[i] = -1;
}

BTreeNode * Index_IndexHandle::getRoot()
{
	return root;
}

BTreeNode* Index_IndexHandle::findLeafForceRight(const void* pData)
{
	findLeaf(pData);
	if (path[hdr.height - 1]->getRight() != -1) {
		int pos = path[hdr.height-1]->findKey(pData);
		if ( pos != -1 &&
			 pos == path[hdr.height -1]->getNumKeys() - 1) {
			BTreeNode* r = fetchNode(path[hdr.height - 1]->getRight());
		    if (r != NULL) {
		    	void *k = NULL;
		    	r->getKey(0, k);
		    	if (r->cmpKey(k, pData) == 0) {
		    		syspHandle->unpinPage(path[hdr.height-1]->getPageRID().Page());
		    		delete path[hdr.height - 1];
		    		path[hdr.height - 1] = fetchNode(r->getPageRID());
		    		pathP[hdr.height - 2]++;
		    	}
		    }
		}
	}
	return path[hdr.height - 1];
}