/*  _________________________________________________________________________
 *
 *  MTGL: The MultiThreaded Graph Library
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top MTGL directory.
 *  _________________________________________________________________________
 */

/****************************************************************************/
/*! \file test_xmt_hash_set.cpp

    \author Greg Mackey (gemacke@sandia.gov)

    \brief Tests the functionality of the xmt_hash_set.

    \date 4/1/2010
*/
/****************************************************************************/

#include <mtgl/xmt_hash_set.hpp>
#include <mtgl/dynamic_array.hpp>
#include <mtgl/util.hpp>

using namespace mtgl;

typedef xmt_hash_set<int> hash_set_type;

#define MAX_LOOP 10000000
#define OPEN_ADDRESS_HASH_SIZE 20000000

template <typename HS>
class set_visitor {
public:
  set_visitor() : sum(0) {}

  void operator()(typename HS::value_type* k) { mt_incr(sum, *k); }

  int sum;
};

int main()
{
//  int data;
  bool truth;
  set_visitor<hash_set_type> sv1;

  dynamic_array<int> arr(MAX_LOOP);

  mt_timer timer;

  #pragma mta fence
  timer.start();
  hash_set_type xhs(OPEN_ADDRESS_HASH_SIZE);
  #pragma mta fence
  timer.stop();

  printf("                 MAX_LOOP: %9d\n", MAX_LOOP);
  printf("               capacity(): %9lu\n\n", xhs.capacity());

  printf("           Initialization: %9.6lf\n", timer.getElapsedSeconds());

  #pragma mta fence
  timer.start();
  #pragma mta assert parallel
  #pragma mta block schedule
  for (int i = 0; i < MAX_LOOP; ++i)
  {
//    if (!xhs.insert(i).second)  printf("Couldn't insert %d.\n", i);
//    xhs.insert(i);
//    hash_set_type::value_type* ptr = xhs.insert(i).first;
//    arr[*ptr] = *ptr;
    int v = *(xhs.insert(i).first);
    arr[v] = v;
  }
  #pragma mta fence
  timer.stop();

  printf("                Insertion: %9.6lf\n", timer.getElapsedSeconds());

  #pragma mta fence
  timer.start();
  hash_set_type xhs2(xhs);
  #pragma mta fence
  timer.stop();

  printf("         Copy constructor: %9.6lf\n", timer.getElapsedSeconds());

  #pragma mta fence
  timer.start();
  xhs2 = xhs;
  #pragma mta fence
  timer.stop();

  printf("      Assignment operator: %9.6lf\n", timer.getElapsedSeconds());

//  if (xhs2.lookup(20, data))  printf("Found: 20 -> %d\n", data);
//  else  printf("Value 20 not found.\n");

  #pragma mta fence
  timer.start();
  xhs2.visit(sv1);
  #pragma mta fence
  timer.stop();

//  if (xhs2.lookup(20, data))  printf("Found: 20 -> %d\n", data);
//  else  printf("Value 20 not found.\n");

  printf("                    Visit: %9.6lf        %d\n",
         timer.getElapsedSeconds(), sv1.sum);

  #pragma mta fence
  timer.start();
  xhs2.clear();
  #pragma mta fence
  timer.stop();

  printf("                    Clear: %9.6lf\n", timer.getElapsedSeconds());

  #pragma mta fence
  timer.start();
  xhs2.resize(10000);
  #pragma mta fence
  timer.stop();

  printf("             Empty Resize: %9.6lf\n", timer.getElapsedSeconds());

  #pragma mta fence
  timer.start();
  #pragma mta block schedule
  for (int i = 0; i < MAX_LOOP; ++i)
  {
//    if (!xhs.member(i))  printf("Didn't find %d.\n", i);
    truth = xhs.member(i);
  }
  #pragma mta fence
  timer.stop();

  printf("          Membership test: %9.6lf        %d\n",
         timer.getElapsedSeconds(), truth);

  #pragma mta fence
  timer.start();
  get_keys(xhs, arr.get_data());
  #pragma mta fence
  timer.stop();

  printf("                 Get keys: %9.6lf\n", timer.getElapsedSeconds());

  #pragma mta fence
  timer.start();
  xhs.resize(MAX_LOOP);
  #pragma mta fence
  timer.stop();

  printf("              Full Resize: %9.6lf\n", timer.getElapsedSeconds());

//  if (xhs.lookup(20, data))  printf("Found: 20 -> %d\n", data);

  #pragma mta fence
  timer.start();
  #pragma mta block schedule
  #pragma mta assert parallel
  for (int i = 0; i < MAX_LOOP; i += 2)
  {
//    if (!xhs.erase(i))  printf("Didn't find %d.\n", i);
    xhs.erase(i);
  }
  #pragma mta fence
  timer.stop();

  printf("                    Erase: %9.6lf\n", timer.getElapsedSeconds());

//  if (xhs.lookup(20, data))  printf("Found: 20 -> %d\n", data);
//  else  printf("Value 20 not found.\n");
//  if (xhs.lookup(139, data))  printf("Found: 139 -> %d\n", data);
//  else  printf("Value 139 not found.\n");

  printf("\n");
  printf("             Num Elements: %9lu\n", xhs.size());
  printf("               capacity(): %9lu\n\n", xhs.capacity());

  return 0;
}
