#ifndef _JUMPING_LIKE_JOIN
#define _JUMPING_LIKE_JOIN

#include "../KVstore/KVstore.h"
#include "../Util/Util.h"
#include "TempResult.h"

class JumpingLikeJoin
{
private:
  KVstore* kvstore;
  string pre;

  //use array
  unsigned* sub_obj_list;
  unsigned sub_obj_list_size;
  map<unsigned, unsigned> sub2posOfEdgeOne;
  map<unsigned, unsigned> sub2posOfEgdeJoined;


  //based on map.
  std::map<unsigned int, std::vector<unsigned int> > edgeTable;
  std::map<unsigned int, std::vector<unsigned int*> > subTable;

  std::map<unsigned int, TempResultSet*> intermediate;
public:

  JumpingLikeJoin(KVstore* _kvstore,string _pre):kvstore(_kvstore), pre(_pre)
  {}

  TYPE_PREDICATE_ID getPreID(std::string pre);

  //function based on array.
  void initSub2Pos(TYPE_PREDICATE_ID preid);
  void buildSubTableUsedArray(TempResultSet *temp);
  TempResultSet* intersectUsedArray(TempResultSet *temp);

  // functions based on map.
  void initEdgeTable(TYPE_PREDICATE_ID preid);
  void buildSubTable(TempResultSet *temp);
  TempResultSet* getEdge3ByEgde1();
  TempResultSet* intersect(TempResultSet* temp2); // TempResultSet *temp1 is removed
  TempResultSet* intersect(TempResultSet* temp1, TempResultSet* temp2);

  static void *run(void *args);
};

#endif //_JUMPING_LIKE_JOIN