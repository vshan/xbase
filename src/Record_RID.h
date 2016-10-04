#include "Record_Internal.h"

class Record_RID()
{
public:
  static const int NULL_PAGE = -1;
  static const int NULL_SLOT = -1;

  Record_RID() : page(NULL_PAGE), slot(NULL_SLOT) {}
  Record_RID(PageNum pageNum, SlotNum slotNum) : page(pageNum), slot(slotNum) {}
  ~RID(){}

  ErrCode getPageNum(int &pageNum) const          // Return page number
  { pageNum = page; return 0; }

  ErrCode getSlotNum(int &slotNum) const         // Return slot number
  { slotNum = slot; return 0; }

  int returnPage() const          // Return page number
  { return page; }
  
  int returnSlot() const          // Return slot number
  { return slot; }

private:
  int page;
  int slot;
};
