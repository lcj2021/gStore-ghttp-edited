#include "JumpingLikeJoin.h"

using namespace std;

TYPE_PREDICATE_ID JumpingLikeJoin::getPreID(string pre)
{
  return this->kvstore->getIDByPredicate(pre);
}

void JumpingLikeJoin::initEdgeTable(TYPE_PREDICATE_ID preid)
{
  unsigned *list;
  unsigned list_size;
  this->kvstore->getsubIDobjIDlistBypreID(preid, list, list_size);

  for (int i = 0; i < list_size; i += 2)
  {
    if (this->edgeTable.find(list[i]) == this->edgeTable.end())
      this->edgeTable[list[i]] = vector<unsigned int>();
    this->edgeTable[list[i]].push_back(list[i + 1]);
  }
}

void JumpingLikeJoin::buildSubTable(TempResultSet *temp)
{
  this->subTable.clear();
  for (int i = 0; i < temp->results[0].result.size(); i++)
  {
    if (this->subTable.find(temp->results[0].result[i].id[0]) == this->subTable.end())
      this->subTable[temp->results[0].result[i].id[0]] = vector<unsigned int *>();
    this->subTable[temp->results[0].result[i].id[0]].push_back(temp->results[0].result[i].id);
  }
}

TempResultSet *JumpingLikeJoin::getEdge3ByEgde1()
{
  TempResultSet *res = new TempResultSet();
  res->results.push_back(TempResult());

  res->results[0].id_varset.addVar("?x");
  res->results[0].id_varset.addVar("?y");

  for (map<unsigned int, vector<unsigned int>>::iterator mit = this->edgeTable.begin();
       mit != this->edgeTable.end(); mit++)
  {
    // iterate vector<unsigned int> in this->edgeTable.
    for (vector<unsigned int>::iterator vit = mit->second.begin();
         vit != mit->second.end(); vit++)
    {
      if (this->edgeTable.find(*vit) != this->edgeTable.end())
      {
        // iterate the edges whose subject is the id
        const vector<unsigned int> &vec_in_edgeTable = this->edgeTable[*vit];
        for (int i = 0; i < vec_in_edgeTable.size(); i++)
        {
          if (this->edgeTable.find(vec_in_edgeTable[i]) != this->edgeTable.end())
          {
            // one edge's obj is egde2's subject
            const vector<unsigned int> &vec_of_last_edge = this->edgeTable[vec_in_edgeTable[i]];
            for (int j = 0; j < vec_of_last_edge.size(); j++)
            {
              res->results[0].result.push_back(TempResult::ResultPair());
              unsigned int *res_id = new unsigned int[2];
              res_id[0] = mit->first;
              res_id[1] = vec_of_last_edge[j];
              res->results[0].result.back().id = res_id;
            }
          }
        }
      }
    }
  }
  return res;
}

TempResultSet *JumpingLikeJoin::intersect(TempResultSet *temp2) // TempResultSet *temp1,
{
  // int temp1_id_num = temp1->results[0].id_varset.getVarsetSize();
  int temp2_id_num = temp2->results[0].id_varset.getVarsetSize();

  TempResultSet *res = new TempResultSet();
  res->results.push_back(TempResult());

  res->results[0].id_varset.addVar(temp2->results[0].id_varset.vars[0]);
  res->results[0].id_varset.addVar(temp2->results[0].id_varset.vars.back());

  // len one join temp2
  // iterate this->edgeTable.
  /*
  for (map<unsigned int, vector<unsigned int> >::iterator mit = this->edgeTable.begin();
  mit != this->edgeTable.end(); mit++)
  {
    // iterate vector<unsigned int> in this->edgeTable.
    for(vector<unsigned int>::iterator vit = mit->second.begin();
    vit != mit->second.end(); vit++)
    {
      // id in vector<unsigned int> can be the subject of edge1
      if(this->edgeTable.find(*vit) != this->edgeTable.end())
      {
        // iterate the edges whose subject is the id
        const vector<unsigned int>& vec_in_edgeTable = this->edgeTable[*vit];
        for (int i = 0; i < vec_in_edgeTable.size(); i++)
        {
          if(this->subTable.find(vec_in_edgeTable[i]) != this->subTable.end())
          {
            // one edge's obj is egde2's subject
            const vector<unsigned int *>& vec_in_subTable = this->subTable[vec_in_edgeTable[i]];
            for (int j = 0; j < vec_in_subTable.size(); j++)
            {
              res->results[0].result.push_back(TempResult::ResultPair());
              unsigned int* res_id = new unsigned int[2];
              res_id[0] = mit->first;
              res_id[1] = vec_in_subTable[j][temp2_id_num - 1];
              res->results[0].result.back().id = res_id;
            }
          }
        }
      }
    }
  }
  */

  // temp2 join len one
  for (int i = 0; i < temp2->results[0].result.size(); i++)
  {
    unsigned *ids_of_two = temp2->results[0].result[i].id;
    if (this->edgeTable.find(ids_of_two[1]) != this->edgeTable.end())
    {
      vector<unsigned int> &vec_in_edgeTable = this->edgeTable[ids_of_two[1]];
      for (vector<unsigned int>::iterator it = vec_in_edgeTable.begin(); it != vec_in_edgeTable.end(); it++)
      {
        if (this->edgeTable.find(*it) != this->edgeTable.end())
        {
          vector<unsigned int> &vec_found = this->edgeTable[*it];
          for (vector<unsigned int>::iterator vfi = vec_found.begin(); vfi != vec_found.end(); vfi++)
          {
            res->results[0].result.push_back(TempResult::ResultPair());
            unsigned int *res_id = new unsigned int[2];
            res_id[0] = ids_of_two[0];
            res_id[1] = *vfi;
            res->results[0].result.back().id = res_id;
          }
        }
      }
    }
  }
  return res;
}

TempResultSet *JumpingLikeJoin::intersect(TempResultSet *temp1, TempResultSet *temp2)
{
  TempResultSet *res = new TempResultSet();
  res->results.push_back(TempResult());

  res->results[0].id_varset.addVar(temp2->results[0].id_varset.vars[0]);
  res->results[0].id_varset.addVar(temp2->results[0].id_varset.vars.back());

  for (int i = 0; i < temp1->results[0].result.size(); i++)
  {
    unsigned int l_id0 = temp1->results[0].result[i].id[0];
    unsigned int l_id1 = temp1->results[0].result[i].id[1];

    map<unsigned int, std::vector<unsigned int>>::iterator it = this->edgeTable.find(l_id1);
    if (it == this->edgeTable.end())
      continue;

    vector<unsigned int>::iterator vit = it->second.begin();
    for (; vit != it->second.end(); vit++)
    {
      map<unsigned int, std::vector<unsigned int *>>::iterator sit = this->subTable.find(*vit);
      if (sit == this->subTable.end())
        continue;
      vector<unsigned int *>::iterator svit = sit->second.begin();
      for (; svit < sit->second.end(); svit++)
      {
        res->results[0].result.push_back(TempResult::ResultPair());
        unsigned int *res_id = new unsigned int[2];
        res_id[0] = l_id0;
        res_id[1] = (*svit)[1];
        res->results[0].result.back().id = res_id;
      }
    }
  }

  return res;
}

void JumpingLikeJoin::initSub2Pos(TYPE_PREDICATE_ID preid)
{
  this->kvstore->getsubIDobjIDlistBypreID(preid, this->sub_obj_list, this->sub_obj_list_size);
  if (this->sub_obj_list_size == 0)
    return;

  // cout<< "----------initSub2Pos print-------------------------" << endl;
  // for (int i = 0; i < this->sub_obj_list_size; i+=2)
  //   cout << this->sub_obj_list[i] << "\t" << this->sub_obj_list[i + 1] << endl;
  // cout<< "----------initSub2Pos print-------------------------" << endl;

  cout << "In initSub2Pos:" << endl;
  this->sub2posOfEdgeOne[this->sub_obj_list[0]] = 0;
  unsigned int prev = this->sub_obj_list[0];
  for (int i = 2; i < this->sub_obj_list_size; i += 2)
  {
    if (this->sub_obj_list[i] == prev)
      continue;

    // if (this->sub2posOfEdgeOne.find(this->sub_obj_list[i])!=this->sub2posOfEdgeOne.end())
    //     cout<< "existed: "<< this->sub_obj_list[i]<<" "<<this->sub2posOfEdgeOne[this->sub_obj_list[i]]<<" "<<i<<endl;

    this->sub2posOfEdgeOne[this->sub_obj_list[i]] = i;
    prev = this->sub_obj_list[i];
  }
}

void JumpingLikeJoin::buildSubTableUsedArray(TempResultSet *temp)
{
  if (temp->results[0].result.size() == 0)
    return;
  cout << "In buildSubTableUsedArray:" << endl;
  unsigned int prev = temp->results[0].result[0].id[0];
  this->sub2posOfEgdeJoined[prev] = 0;
  for (int i = 1; i < temp->results[0].result.size(); i++)
  {
    if (temp->results[0].result[i].id[0] == prev)
      continue;

    // if (this->sub2posOfEgdeJoined.find(temp->results[0].result[i].id[0])!=this->sub2posOfEgdeJoined.end())
    //     cout<< "existed: "<< temp->results[0].result[i].id[0]<<" "<<this->sub2posOfEgdeJoined[temp->results[0].result[i].id[0]]<<" "<<i<<endl;

    this->sub2posOfEgdeJoined[temp->results[0].result[i].id[0]] = i;
    prev = temp->results[0].result[i].id[0];
  }
}

TempResultSet *JumpingLikeJoin::intersectUsedArray(TempResultSet *temp)
{
  int temp2_id_num = temp->results[0].id_varset.getVarsetSize();

  TempResultSet *res = new TempResultSet();
  res->results.push_back(TempResult());

  res->results[0].id_varset.addVar(temp->results[0].id_varset.vars[0]);
  res->results[0].id_varset.addVar(temp->results[0].id_varset.vars.back());

  // one joins two
  for (int i = 0; i < this->sub_obj_list_size; i += 2)
  {
    if (this->sub2posOfEdgeOne.find(this->sub_obj_list[i + 1]) ==
        this->sub2posOfEdgeOne.end())
      continue;

    unsigned int prevInOne = this->sub_obj_list[i + 1];
    for (int j = this->sub2posOfEdgeOne[prevInOne];
         j < this->sub_obj_list_size && this->sub_obj_list[j] == prevInOne; j += 2)
    {
      if (this->sub2posOfEgdeJoined.find(this->sub_obj_list[j + 1]) ==
          this->sub2posOfEgdeJoined.end())
        continue;

      unsigned int prevInJoined = this->sub_obj_list[j + 1];
      for (int k = this->sub2posOfEgdeJoined[this->sub_obj_list[j + 1]];
           k < temp->results[0].result.size() && temp->results[0].result[k].id[0] == prevInJoined; k++)
      {
        res->results[0].result.push_back(TempResult::ResultPair());
        unsigned int *res_id = new unsigned int[2];
        res_id[0] = this->sub_obj_list[i];
        res_id[1] = temp->results[0].result[k].id[1];
        res->results[0].result.back().id = res_id;
      }
    }
  }
  return res;
}

void *JumpingLikeJoin::run(void *args)
{
  JumpingLikeJoin *jumping = (JumpingLikeJoin *)args;
  jumping->initEdgeTable(jumping->getPreID(jumping->pre));

  TempResultSet *edge3 = jumping->getEdge3ByEgde1();

  return (void *)edge3;
}
