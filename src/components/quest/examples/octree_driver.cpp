/*
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * All rights reserved.
 *
 * This source code cannot be distributed without permission and further
 * review from Lawrence Livermore National Laboratory.
 */


/*!
 *******************************************************************************
 * \file
 * \brief Driver to get started with in/out octree
 *******************************************************************************
 */

// ATK Toolkit includes
#include "common/ATKMacros.hpp"
#include "common/CommonTypes.hpp"

#include "quest/BoundingBox.hpp"
#include "quest/Mesh.hpp"
#include "quest/Point.hpp"
#include "quest/STLReader.hpp"
#include "quest/SquaredDistance.hpp"
#include "quest/Triangle.hpp"
#include "quest/UniformMesh.hpp"
#include "quest/UnstructuredMesh.hpp"
#include "quest/Point.hpp"
#include "quest/Octree.hpp"

#include "slic/GenericOutputStream.hpp"
#include "slic/slic.hpp"

#include "slam/FileUtilities.hpp"


// C/C++ includes
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <fstream>

using namespace asctoolkit;

typedef meshtk::UnstructuredMesh< meshtk::LINEAR_TRIANGLE > TriangleMesh;

typedef quest::Octree<3> Octree3D;
typedef quest::TopologicalOctree<3,int> TopoOctree3D;

typedef Octree3D::GeometricBoundingBox GeometricBoundingBox;
typedef Octree3D::SpacePt SpacePt;
typedef Octree3D::SpaceVector SpaceVector;
typedef Octree3D::GridPt GridPt;


//------------------------------------------------------------------------------
GeometricBoundingBox compute_bounds( meshtk::Mesh* mesh)
{
   SLIC_ASSERT( mesh != ATK_NULLPTR );

   GeometricBoundingBox meshBB;
   SpacePt pt;

   for ( int i=0; i < mesh->getMeshNumberOfNodes(); ++i )
   {
       mesh->getMeshNode( i, pt.data() );
       meshBB.addPoint( pt );
   } // END for all nodes

   SLIC_ASSERT( meshBB.isValid() );

   return meshBB;
}

void print_surface_stats( meshtk::Mesh* mesh)
{
   SLIC_ASSERT( mesh != ATK_NULLPTR );

   SpacePt pt;

   typedef quest::BoundingBox<double,1> RangeType;
   typedef RangeType::PointType LengthType;

   RangeType edgeRange;
   RangeType areaRange;
   const int ncells = mesh->getMeshNumberOfCells();
   typedef std::set<int> TriIdxSet;
   TriIdxSet badTriangles;

   // simple binning based on the exponent
   typedef std::map<int,int> LogHistogram;
   LogHistogram edgeLenHist;
   LogHistogram areaHist;

   for ( int i=0; i < ncells; ++i )
   {
      GridPt cellids;
      SpacePt triVerts[3];
      mesh->getMeshCell( i, cellids.data() );

      for(int j=0; j<3; ++j)
          mesh->getMeshNode( cellids[j], triVerts[j].data() );

      for(int j=0; j<3; ++j)
      {
          double len = SpaceVector(triVerts[j],triVerts[(j+1)%3]).norm();
          if(len == 0)
          {
              badTriangles.insert(i);
          }
          else
          {
              edgeRange.addPoint( LengthType(len) );
              int exp;
              std::frexp (len, &exp);
              edgeLenHist[exp]++;
          }
      }

      double area = SpaceVector::cross_product(
                  SpaceVector(triVerts[0],triVerts[1])
                  , SpaceVector(triVerts[0],triVerts[2])
                  ).norm();
      if(area == 0.)
          badTriangles.insert(i);
      else
      {
          areaRange.addPoint ( LengthType( area));
          int exp;
          std::frexp (area, &exp);
          areaHist[exp]++;
      }
   }


   std::cout << "\n\tEdge length range is: "  << edgeRange
             << "\n\tTriangle area range is: "  << areaRange
             << std::endl;


   std::cout << "\n  Edge length histogram (lg-arithmic): ";
   for(LogHistogram::const_iterator it = edgeLenHist.begin()
           ; it != edgeLenHist.end()
           ; ++it)
   {
       std::cout <<"\n\t exp: " << it->first <<"\t count: " << it->second;
   }
   std::cout<<std::endl;

   std::cout << "\n  Triangle areas histogram (lg-arithmic): ";
   for(LogHistogram::const_iterator it =areaHist.begin()
           ; it != areaHist.end()
           ; ++it)
   {
       std::cout <<"\n\t exp: " << it->first <<"\t count: " << it->second;
   }
   std::cout<<std::endl;

   if(! badTriangles.empty() )
   {
       std::cout<<"The following triangles have zero area/edge lengths:\n\t";
       for(TriIdxSet::const_iterator it = badTriangles.begin()
               ; it != badTriangles.end()
               ; ++it)
           std::cout << " " << *it;
       std::cout<< std::endl;
   }
}


//------------------------------------------------------------------------------
int main( int argc, char** argv )
{
  // STEP 0: Initialize SLIC Environment
  slic::initialize();
  slic::setLoggingMsgLevel( asctoolkit::slic::message::Debug );
  slic::addStreamToAllMsgLevels( new slic::GenericOutputStream(&std::cout) );

  bool hasInputArgs = argc > 1;

  // STEP 1: get file from user or use default
  std::string inputFile;
  if(hasInputArgs)
  {
      inputFile = std::string( argv[1] );
  }
  else
  {
      const std::string defaultFileName = "plane_simp.stl";
      const std::string defaultDir = "src/components/quest/data/";
      inputFile = defaultDir + defaultFileName;
  }
  std::string stlFile = asctoolkit::slam::util::findFileRecursive(inputFile);


  // STEP 2: read file
  std::cout << "Reading file: " << stlFile << "...";
  std::cout.flush();
  quest::STLReader* reader = new quest::STLReader();
  reader->setFileName( stlFile );
  reader->read();
  std::cout << "[DONE]\n";
  std::cout.flush();


  // STEP 3: get surface mesh
  meshtk::Mesh* surface_mesh = new TriangleMesh( 3 );
  reader-> getMesh( static_cast<TriangleMesh*>( surface_mesh ) );
  // dump mesh info
  std::cout<<"Mesh has "
          << surface_mesh->getMeshNumberOfNodes() << " nodes and "
          << surface_mesh->getMeshNumberOfCells() << " cells."
          << std::endl;

  // STEP 4: Delete the reader
  delete reader;
  reader = ATK_NULLPTR;


  // STEP 5: Compute the bounding box
  GeometricBoundingBox meshBB = compute_bounds( surface_mesh);
  std::cout << "Mesh bounding box: " << meshBB << std::endl;

  print_surface_stats(surface_mesh);

  // STEP 6: Find grid point
  double alpha = 2./3.;

  Octree3D octree(meshBB);
  SpacePt queryPt = SpacePt::lerp(meshBB.getMin(), meshBB.getMax(), alpha);
  std::cout<<"\n"<<"Finding associated grid point for query point: " << queryPt << std::endl;

  for(int lev = 0; lev < Octree3D::MAX_LEV; ++lev)
  {
      GridPt gridPt = octree.findGridCellAtLevel(queryPt, lev);
      std::cout << "\t@level " << lev
              <<":\t" <<  gridPt
              <<"\t[max gridPt: " << octree. maxGridCellAtLevel(lev)
              <<"; spacing" << octree.spacingAtLevel(lev)
              <<"]\n";
  }
  std::cout << std::endl;


  TopoOctree3D topoOctree;




  return 0;
}
