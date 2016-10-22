Index_IndexScan::Index_IndexScan(): isOpen(false), desc(false), eof(false), lastNode(NULL)
{
	pred = NULL;
	pixh = NULL;
	currNode = NULL;
	currPos = -1;
	currKey = NULL;
	currRid = RID(-1,-1);
	c = EQ_OP;
	value = NULL;
}

Index_IndexScan::~Index_IndexScan()
{
	if (pred != NULL)
		delete pred;

	if (pixh != NULL && pixh->getHeight() > 1) {
		if (currNode != NULL)
			delete currNode;
		if (lastNode != NULL)
			delete lastNode;
	}
}

ErrorCode Index_IndexScan::openScan(const Index_IndexHandle &fileHandle,
	                                CompOp compOp,
	                                void *value,
	                                ClientHint pinHint,
	                                bool desc)
{
	pixh = const_cast<Index_IndexHandle*>(&fileHandle);
	isOpen = true;
	if (desc)
		this->desc = true;
	foundOne = false;

	pred = new Predicate(pixh->getAttrType(),
		                 pixh->getAttrLength(),
		                 0,
		                 compOp,
		                 value_,
		                 pinHint);

	c = compOp;

	// todo deep copy

	return 0;
}

ErrorCode Index_IndexScan::getNextEntry(RID &rid)
{
	void *k = NULL;
	int i = -1;
	return getNextEntry(k, rid, i);
}

ErrorCode Index_IndexScan::getNextEntry(void *& k, RID &rid, int& numScanned)
{
	bool currDeleted = false;

	if (currNode == NULL && currPos == -1) {
		if (!desc) {
			currNode = pixh->fetchNode(pixh->findSmallestLeaf()->getPageRID());
			currPos = -1;
		} else {
			currNode = pixh->fetchNode(pixh->findLargestLeaf()->getPageRID());
			currPos = currNode->getNumKeys();
		}
		currDeleted = false;
	} else {
		if (currKey != NULL && currNode != NULL && currPos != -1) {
			char *key = NULL;
			int ret = currNode->getKey(currPos, (void*&)key);
			if (currNode->cmpKey(key, currKey) != 0) {
				currDeleted = true;
			} else {
				if (!(currRid == currNode->getAddr(currPos)))
					currDeleted = true;
			}
		}
	}

	for (;
		 (currNode != NULL)
		 )
	{
		int i = -1;

		if (!desc) {
			if (!currDeleted) {
				i = currPos + 1;
			} else {
				i = currPos;
				currDeleted = false;
			}

			for (; i < currNode->getNumKeys(); i++)
			{
				char *key = NULL;
				int ret = currNode->getKey(i, (void *&)key);
				numScanned++;

				currPos = -1;
				if (currKey == NULL)
					currKey = (void*) new char[pixh->getAttrLength()];
				memcpy(currKey, key, pixh->getAttrLength());
				currRid = currNode->getAddr(i);

				if (pred->eval(key, pred->initOp())) {
					k = key;
					rid = currNode->getAddr(i);
					foundOne = true;
					return 0;
				} else {
					if (foundOne) {
						// todo
					}
				}
			}

		}

		else {
			if (!currDeleted) {
				i = currPos-1;
			} else {
				i = currPos;
				currDeleted = false;
			}

			for (; i >= 0; i--)
			{
				char *key = NULL;
				int ret = currNode->getKey(i, (void*&)key);
				numScanned++;

				currPos = i;

				if (currKey == NULL)
					currKey = (void*) new char[pixh->getAttrLength()];
				memcpy(currKey, key, pixh->getAttrLength());
				currRid = currNode->getAddr(i);

				if (pred->eval(key, pred->initOp())) {
					k = key;
					rid = currNode->getAddr(i);
					foundOne = true;
					return 0;
				} else {
					if (foundOne) {
						// todo
					}
				}
			}
		}

		if ((lastNode != NULL) &&
			currNode->getPageRID() == lastNode->getPageRID() )
			break;

		if (!desc) {
			int right = currNode->getRight();
			pixh->unpin(currNode->getPageRID().Page());
			delete currNode;
			currNode = NULL;
			currNode = pixh->fetchNode(right);
			if (currNode != NULL)
				pixh->pin(currNode->getPageRID().Page());
			currPos = -1;
		}
		else {
			int left = currNode->getLeft();
			pixh->unpin(currNode->getPageRID().Page());
			delete currNode;
			currNode = NULL;
			currNode = pixh->fetchNode(left);
			if (currNode != NULL) {
				pixh->pin(currNode->getPageRID().Page());
				currPos = currNode->getNumKeys();
			}
		}
	}

	return INDEX_EOF;
}

ErrorCode Index_IndexScan::closeScan()
{
	isOpen = false;
	if (pred != NULL)
		delete pred;
	pred = NULL;
	currNode = NULL;
	currPos = -1
	if (currKey != NULL) {
		delete [] ((char *) currKey);
		currKey = NULL;
	}
	currRid = RID(-1,-1);
	lastNode = NULL;
	eof = false;
	return 0;
}

// todo resetstate, earlyexitoptimize, opoptimize