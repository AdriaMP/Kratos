//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Inigo Lopez
//
//

// External includes
#include "containers/model.h"

// Project includes
#include "testing/testing.h"
#include "includes/model_part.h"
#include "custom_elements/compressible_potential_flow_element.h"

namespace Kratos {
  namespace Testing {

    typedef ModelPart::IndexType IndexType;
    typedef ModelPart::NodeIterator NodeIteratorType;

    void GenerateCompressibleElement(ModelPart& rModelPart)
    {
      // Variables addition
      rModelPart.AddNodalSolutionStepVariable(VELOCITY_POTENTIAL);
      rModelPart.AddNodalSolutionStepVariable(AUXILIARY_VELOCITY_POTENTIAL);

      // Set the element properties
      Properties::Pointer pElemProp = rModelPart.CreateNewProperties(0);
      BoundedVector<double, 3> v_inf = ZeroVector(3);
      v_inf(0) = 34.0;
      pElemProp->SetValue(VELOCITY_INFINITY,v_inf);
      pElemProp->SetValue(DENSITY_INFINITY,1.225);
      pElemProp->SetValue(MACH_INFINITY,0.1);
      pElemProp->SetValue(HEAT_CAPACITY_RATIO,1.4);
      pElemProp->SetValue(SOUND_VELOCITY,340.0);

      // Geometry creation
      rModelPart.CreateNewNode(1, 0.0, 0.0, 0.0);
      rModelPart.CreateNewNode(2, 1.0, 0.0, 0.0);
      rModelPart.CreateNewNode(3, 1.0, 1.0, 0.0);
      std::vector<ModelPart::IndexType> elemNodes{ 1, 2, 3 };
      rModelPart.CreateNewElement("IncompressiblePotentialFlowElement2D3N", 1, elemNodes, pElemProp);
    }

    /** Checks the IncompressiblePotentialFlowElement element.
     * Checks the LHS and RHS computation.
     */
    KRATOS_TEST_CASE_IN_SUITE(CompressiblePotentialFlowElementRHS, CompressiblePotentialApplicationFastSuite)
    {
      Model this_model;
      ModelPart& model_part = this_model.CreateModelPart("Main", 3);

      GenerateCompressibleElement(model_part);
      Element::Pointer pElement = model_part.pGetElement(1);

      // Define the nodal values
      Vector potential(3);
      potential(0) = 1.0;
      potential(1) = 2.0;
      potential(2) = 3.0;

      for (unsigned int i = 0; i < 3; i++)
        pElement->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) = potential(i);

      // Compute RHS and LHS
      Vector RHS = ZeroVector(3);
      Matrix LHS = ZeroMatrix(3, 3);

      pElement->CalculateLocalSystem(LHS, RHS, model_part.GetProcessInfo());

      // Check the RHS values (the RHS is computed as the LHS x previous_solution,
      // hence, it is assumed that if the RHS is correct, the LHS is correct as well)
      KRATOS_CHECK_NEAR(RHS(0), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(RHS(1), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(RHS(2), -0.6125, 1e-7);
    }

    /** Checks the IncompressiblePotentialFlowElement element.
     * Checks the LHS and RHS computation.
     */
    KRATOS_TEST_CASE_IN_SUITE(CompressiblePotentialFlowElementLHS, CompressiblePotentialApplicationFastSuite)
    {
      Model this_model;
      ModelPart& model_part = this_model.CreateModelPart("Main", 3);

      GenerateCompressibleElement(model_part);
      Element::Pointer pElement = model_part.pGetElement(1);

      // Define the nodal values
      Vector potential(3);
      potential(0) = 1.0;
      potential(1) = 2.0;
      potential(2) = 3.0;

      for (unsigned int i = 0; i < 3; i++)
        pElement->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) = potential(i);

      // Compute RHS and LHS
      Vector RHS = ZeroVector(3);
      Matrix LHS = ZeroMatrix(3, 3);

      pElement->CalculateLocalSystem(LHS, RHS, model_part.GetProcessInfo());

      // Check the RHS values (the RHS is computed as the LHS x previous_solution,
      // hence, it is assumed that if the RHS is correct, the LHS is correct as well)
      KRATOS_CHECK_NEAR(LHS(0,0), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,1), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,2), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,1), 1.225, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,2), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(2,2), 0.6125, 1e-7);
    }

    KRATOS_TEST_CASE_IN_SUITE(CompressiblePotentialFlowElementRHSWake, CompressiblePotentialApplicationFastSuite)
    {
      Model this_model;
      ModelPart& model_part = this_model.CreateModelPart("Main", 3);

      GenerateCompressibleElement(model_part);
      Element::Pointer pElement = model_part.pGetElement(1);

      // Define the nodal values
      Vector potential(3);
      potential(0) = 1.0;
      potential(1) = 2.0;
      potential(2) = 3.0;

      Vector distances(3);
      distances(0) = 1.0;
      distances(1) = -1.0;
      distances(2) = -1.0;

      pElement->GetValue(ELEMENTAL_DISTANCES) = distances;
      pElement->GetValue(WAKE) = true;

      for (unsigned int i = 0; i < 3; i++){
        if (distances(i) > 0.0)
          pElement->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) = potential(i);
        else
          pElement->GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL) = potential(i);
      }
      for (unsigned int i = 0; i < 3; i++){
        if (distances(i) < 0.0)
          pElement->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) = potential(i)+5;
        else
          pElement->GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL) = potential(i)+5;
      }

      // Compute RHS and LHS
      Vector RHS = ZeroVector(6);
      Matrix LHS = ZeroMatrix(6, 6);

      pElement->CalculateLocalSystem(LHS, RHS, model_part.GetProcessInfo());

      // Check the RHS values (the RHS is computed as the LHS x previous_solution,
      // hence, it is assumed that if the RHS is correct, the LHS is correct as well)
      KRATOS_CHECK_NEAR(RHS(0), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(RHS(1), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(RHS(2), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(RHS(3), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(RHS(4), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(RHS(5), -0.6125, 1e-7);
    }

    KRATOS_TEST_CASE_IN_SUITE(CompressiblePotentialFlowElementLHSWake, CompressiblePotentialApplicationFastSuite)
    {
      Model this_model;
      ModelPart& model_part = this_model.CreateModelPart("Main", 3);

      GenerateCompressibleElement(model_part);
      Element::Pointer pElement = model_part.pGetElement(1);

      // Define the nodal values
      Vector potential(3);
      potential(0) = 1.0;
      potential(1) = 2.0;
      potential(2) = 3.0;

      Vector distances(3);
      distances(0) = 1.0;
      distances(1) = -1.0;
      distances(2) = -1.0;

      pElement->GetValue(ELEMENTAL_DISTANCES) = distances;
      pElement->GetValue(WAKE) = true;

      for (unsigned int i = 0; i < 3; i++){
        if (distances(i) > 0.0)
          pElement->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) = potential(i);
        else
          pElement->GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL) = potential(i);
      }
      for (unsigned int i = 0; i < 3; i++){
        if (distances(i) < 0.0)
          pElement->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL) = potential(i)+5;
        else
          pElement->GetGeometry()[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL) = potential(i)+5;
      }

      // Compute RHS and LHS
      Vector RHS = ZeroVector(6);
      Matrix LHS = ZeroMatrix(6, 6);

      pElement->CalculateLocalSystem(LHS, RHS, model_part.GetProcessInfo());

      // Check the RHS values (the RHS is computed as the LHS x previous_solution,
      // hence, it is assumed that if the RHS is correct, the LHS is correct as well)
      KRATOS_CHECK_NEAR(LHS(0,0), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,1), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,2), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,3), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,4), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(0,5), 0.0, 1e-7);

      KRATOS_CHECK_NEAR(LHS(1,0), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,1), 1.225, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,2), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,3), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,4), -1.225, 1e-7);
      KRATOS_CHECK_NEAR(LHS(1,5), 0.6125, 1e-7);

      KRATOS_CHECK_NEAR(LHS(2,0), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(2,1), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(2,2), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(2,3), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(2,4), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(2,5), -0.6125, 1e-7);

      KRATOS_CHECK_NEAR(LHS(3,0), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(3,1), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(3,2), 0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(3,3), 0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(3,4), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(3,5), 0.0, 1e-7);

      KRATOS_CHECK_NEAR(LHS(4,0), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(4,1), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(4,2), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(4,3), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(4,4), 1.225, 1e-7);
      KRATOS_CHECK_NEAR(LHS(4,5), -0.6125, 1e-7);

      KRATOS_CHECK_NEAR(LHS(5,0), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(5,1), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(5,2), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(5,3), 0.0, 1e-7);
      KRATOS_CHECK_NEAR(LHS(5,4), -0.6125, 1e-7);
      KRATOS_CHECK_NEAR(LHS(5,5), 0.6125, 1e-7);
    }

    /** Checks the IncompressiblePotentialFlowElement element.
 * Checks the EquationIdVector.
 */
    KRATOS_TEST_CASE_IN_SUITE(CompressiblePotentialFlowElementEquationIdVector, CompressiblePotentialApplicationFastSuite)
    {

      Model this_model;
      ModelPart& model_part = this_model.CreateModelPart("Main", 3);

      GenerateCompressibleElement(model_part);
      Element::Pointer pElement = model_part.pGetElement(1);

      for (unsigned int i = 0; i < 3; i++)
        pElement->GetGeometry()[i].AddDof(VELOCITY_POTENTIAL);

      Element::DofsVectorType ElementalDofList;
      pElement->GetDofList(ElementalDofList, model_part.GetProcessInfo());

      for (int i = 0; i < 3; i++)
        ElementalDofList[i]->SetEquationId(i);

      Element::EquationIdVectorType EquationIdVector;
      pElement->EquationIdVector(EquationIdVector, model_part.GetProcessInfo());

      // Check the EquationIdVector values
      KRATOS_CHECK(EquationIdVector[0] == 0);
      KRATOS_CHECK(EquationIdVector[1] == 1);
      KRATOS_CHECK(EquationIdVector[2] == 2);
    }

    /** Checks the IncompressiblePotentialFlowElement element.
 * Checks the EquationIdVector for the Wake.
 */
    KRATOS_TEST_CASE_IN_SUITE(CompressiblePotentialFlowElementEquationIdVectorWake, CompressiblePotentialApplicationFastSuite)
    {

      Model this_model;
      ModelPart& model_part = this_model.CreateModelPart("Main", 3);

      GenerateCompressibleElement(model_part);
      Element::Pointer pElement = model_part.pGetElement(1);
      pElement->SetValue(WAKE, true);

      array_1d<double, 3> distances;
      distances[0] = -0.5;
      distances[1] = -0.5;
      distances[2] = 0.5;
      pElement->SetValue(ELEMENTAL_DISTANCES, distances);

      for (unsigned int i = 0; i < 3; i++) {
        pElement->GetGeometry()[i].AddDof(VELOCITY_POTENTIAL);
        pElement->GetGeometry()[i].AddDof(AUXILIARY_VELOCITY_POTENTIAL);
      }

      Element::DofsVectorType ElementalDofList;
      pElement->GetDofList(ElementalDofList, model_part.GetProcessInfo());

      for (int i = 0; i < 6; i++)
        ElementalDofList[i]->SetEquationId(i);

      Element::EquationIdVectorType EquationIdVector;
      pElement->EquationIdVector(EquationIdVector, model_part.GetProcessInfo());

      //Check the EquationIdVector values
      KRATOS_CHECK(EquationIdVector[0] == 0);
      KRATOS_CHECK(EquationIdVector[1] == 1);
      KRATOS_CHECK(EquationIdVector[2] == 2);
      KRATOS_CHECK(EquationIdVector[3] == 3);
      KRATOS_CHECK(EquationIdVector[4] == 4);
      KRATOS_CHECK(EquationIdVector[5] == 5);
    }
  } // namespace Testing
}  // namespace Kratos.
