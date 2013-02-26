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
/*! \file generate_mesh_graph.hpp

    \brief Generates a 3-D mesh graph.

    \author William McLendon (wcmclen@sandia.gov)
    \author Greg Mackey (gemacke@sandia.gov)

    \date 8/11/2008
*/
/****************************************************************************/

#ifndef MTGL_GENERATE_MESH_GRAPH_HPP
#define MTGL_GENERATE_MESH_GRAPH_HPP

#include <cstdio>
#include <cassert>
#include <iostream>

#include <mtgl/util.hpp>
#include <mtgl/random.hpp>
#include <mtgl/mtgl_adapter.hpp>

namespace mtgl {

/****************************************************************************/
/*! \brief Generates a 3-D mesh graph with numX vertices in the x direction,
           numY vertices in the y direction, and numZ vertices in the z
           direction.

    \author William McLendon (wcmclen@sandia.gov)
    \author Greg Mackey (gemacke@sandia.gov)
*/
/****************************************************************************/
template <typename graph_adapter>
void generate_mesh_graph(graph_adapter& g, int numX, int numY, int numZ,
                         int numSCC = 0)
{
  typedef typename graph_traits<graph_adapter>::size_type size_type;

#ifdef MTGL_TRACE_GENERATOR
  std::cout << "3D mesh graph generator started." << std::endl;
#endif

  // If a dimenzion is zero, or if all dimensions are 1...

  if ((numX <= 0 || numY <= 0 || numZ <= 0) ||
      (numX == 1 && numY == 1 && numZ == 1))
  {
    size_type num_verts   = 1;
    size_type num_edges   = 0;
    size_type srcs[1]     = { 0 };
    size_type dests[1]    = { 0 };

    init(num_verts, num_edges, srcs, dests, g);

    if (numX <= 0 || numY <= 0 || numZ <= 0)
    {
      std::cerr << "X,Y, or Z dimension is zet to zero. All dimensions must "
                << "be 1 or more." << std::endl;
    }

    return;
  }

  // Otherwise, all dimensions are >= 1, so we can generate something
  // interesting ...

//    if (numSCC > (numX - 1) * (numY - 1) * (numZ - 1))
//    {
//      numSCC = (numX - 1) * (numY - 1) * (numZ - 1);
//    }

  if (numZ == 1)
  {
    if (numSCC > ((numX - 1) * numY + (numY - 1) * numX))
    {
      numSCC = ((numX - 1) * numY + (numY - 1) * numX);
    }
  }
  else if (numZ > 1)
  {
    if (numSCC > ((numX - 1) * numY + (numY - 1) * numX) * (numZ - 1))
    {
      numSCC = ((numX - 1) * numY + (numY - 1) * numX) * (numZ - 1);
    }
  }

  size_type num_verts = numX * numY * numZ;

  size_type num_edges = ((numX - 1) * numY + numX * (numY - 1)) * numZ +\
                        numX * numY * (numZ - 1) + numSCC;

#ifdef MTGL_TRACE_GENERATOR
  std::cout << "Allocating " << num_verts << " vertices." << std::endl;
  std::cout << "Allocating " << num_edges << " edges." << std::endl;
  std::cout << "\tnumSCC = " << numSCC << std::endl;
#endif

  size_type* srcs = (size_type*) malloc((num_edges) * sizeof(size_type));
  size_type* dests = (size_type*) malloc((num_edges) * sizeof(size_type));

#ifdef MTGL_TRACE_GENERATOR
  std::cout << "Generating edges on the X dimension." << std::endl;
#endif

  #pragma mta assert parallel
  for (int zi = 0; zi < numZ; ++zi)
  {
    #pragma mta assert parallel
    for (int xi = 0; xi < numX; ++xi)
    {
      #pragma mta assert parallel
      for (int yi = 0; yi < numY - 1; ++yi)
      {
        int eid = yi + (xi * (numY - 1)) + (zi * numX * (numY - 1));
        int col = xi + (zi * numX * numY);
        int src = col + yi * numX;
        int tgt = src + numX;

#ifdef MTGL_ASSERT_RANGE_CHECK
        assert(eid >= 0 && eid < num_edges);
        assert(src >= 0 && src < num_verts);
        assert(tgt >= 0 && tgt < num_verts);
#endif

        srcs[eid] = src;
        dests[eid] = tgt;

#ifdef MTGL_TRACE_GENERATOR
        std::cout << "\t[" << eid << "] a " << srcs[eid] << " "
                  << dests[eid] << std::endl;
#endif
      }
    }
  }

#ifdef MTGL_TRACE_GENERATOR
  std::cout << "Generating edges on the Y dimension." << std::endl;
#endif

  #pragma mta assert parallel
  for (int zi = 0; zi < numZ; ++zi)
  {
    #pragma mta assert parallel
    for (int yi = 0; yi < numY; ++yi)
    {
      #pragma mta assert parallel
      for (int xi = 0; xi < numX - 1; ++xi)
      {
        int eid = numX * numZ * (numY - 1) +\
                  xi + (yi * (numX - 1)) + (zi * numY * (numX - 1));

        int src = xi + (yi * numX) + (zi * numX * numY);
        int tgt = src + 1;

#ifdef MTGL_ASSERT_RANGE_CHECK
        assert(eid >= 0 && eid < num_edges);
        assert(src >= 0 && src < num_verts);
        assert(tgt >= 0 && tgt < num_verts);
#endif

        srcs[eid] = src;
        dests[eid] = tgt;

#ifdef MTGL_TRACE_GENERATOR
        std::cout << "\t[" << eid << "] a " << srcs[eid] << " "
                  << dests[eid] << std::endl;
#endif
      }
    }
  }

#ifdef MTGL_TRACE_GENERATOR
  std::cout << "Generating edges on the Z dimension." << std::endl;
#endif

  #pragma mta assert parallel
  for (int zi = 0; zi < numZ - 1; ++zi)
  {
    #pragma mta assert parallel
    for (int yi = 0; yi < numY; ++yi)
    {
      #pragma mta assert parallel
      for (int xi = 0; xi < numX; ++xi)
      {

        int eid = numX * numZ * (numY - 1) +\
                  numZ * numY * (numX - 1) +\
                  xi + (yi * numX) + (zi * numX * numY);

        int src = zi * (numX * numY) + yi * numX + xi;
        int tgt = src + (numX * numY);

#ifdef MTGL_ASSERT_RANGE_CHECK
        assert(eid >= 0 && eid < num_edges);
        assert(src >= 0 && src < num_verts);
        assert(tgt >= 0 && tgt < num_verts);
#endif

        srcs[eid] = src;
        dests[eid] = tgt;

#ifdef MTGL_TRACE_GENERATOR
        std::cout << "\t[" << eid << "] a " << srcs[eid] << " "
                  << dests[eid] << std::endl;
#endif
      }
    }
  }

  unsigned long edge_idx = (numX - 1) * numY * numZ +\
                           numX * (numY - 1) * numZ +\
                           numX * numY * (numZ - 1);

  lrand48_generator rv_scc(numSCC);

#ifdef MTGL_TRACE_GENERATOR
  if (numSCC > 0) std::cout << "Generating SCC edges." << std::endl;
#endif

  #pragma mta assert parallel
  for (int i = 0; i < numSCC; ++i)
  {
    unsigned long next_eid = mt_incr(edge_idx, 1);
    int eid_to_flip = rv_scc[i] % (num_edges - numSCC);
    size_type src = srcs[eid_to_flip];
    size_type trg = dests[eid_to_flip];

#ifdef MTGL_TRACE_GENERATOR
    std::cout << "\t[" << next_eid << "] = " << trg << "->" << src << std::endl;
#endif

    srcs[next_eid] = trg;
    dests[next_eid] = src;
  }

  init(num_verts, num_edges, srcs, dests, g);

  free(srcs);
  free(dests);
}

}

#endif
