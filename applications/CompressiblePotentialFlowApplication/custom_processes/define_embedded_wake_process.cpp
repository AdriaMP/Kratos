//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:        BSD License
//                  Kratos default license: kratos/license.txt
//
//  Main authors:    Marc Núñez
//


#include "define_embedded_wake_process.h"
#include "move_model_part_process.h"
#include "processes/calculate_discontinuous_distance_to_skin_process.h"
#include "utilities/variable_utils.h"
#include "compressible_potential_flow_application_variables.h"


namespace Kratos
{
// Constructor for DefineEmbeddedWakeProcess Process
DefineEmbeddedWakeProcess::DefineEmbeddedWakeProcess(ModelPart& rModelPart,
                    ModelPart& rWakeModelPart
                ):
    Process(),
    mrModelPart(rModelPart),
    mrWakeModelPart(rWakeModelPart)
{}

void DefineEmbeddedWakeProcess::Execute()
{
    KRATOS_TRY;

    ComputeDistanceToWake();
    MarkWakeElements();
    ComputeTrailingEdgeNode();
    MarkKuttaElements();
    KRATOS_CATCH("");
}


void DefineEmbeddedWakeProcess::ComputeDistanceToWake(){

    CalculateDiscontinuousDistanceToSkinProcess<2> distance_calculator(mrModelPart, mrWakeModelPart);
    distance_calculator.Execute();
    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(mrModelPart.Elements().size()); i++) {
        ModelPart::ElementIterator it_elem = mrModelPart.ElementsBegin() + i;
        auto elemental_distances = it_elem->GetValue(ELEMENTAL_DISTANCES);
        it_elem->SetValue(WAKE_ELEMENTAL_DISTANCES,elemental_distances);
    }
}

void DefineEmbeddedWakeProcess::MarkWakeElements(){

    ModelPart& deactivated_model_part = mrModelPart.CreateSubModelPart("deactivated_model_part");
    std::vector<std::size_t> deactivated_ids;
    // #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(mrModelPart.Elements().size()); i++) {
        ModelPart::ElementIterator it_elem = mrModelPart.ElementsBegin() + i;
        // Check if the element is touching the trailing edge
        // CheckIfTrailingEdgeElement(*it_elem);

        // Elements downstream the trailing edge can be wake elements
        // bool potentially_wake = CheckIfPotentiallyWakeElement(*it_elem);

        // if (potentially_wake) {
            // Compute the nodal distances of the element to the wake
            BoundedVector<double, 3> nodal_distances_to_wake = it_elem->GetValue(WAKE_ELEMENTAL_DISTANCES);

            // Selecting the cut (wake) elements
            bool is_wake_element = CheckIfWakeElement(nodal_distances_to_wake);

            // Mark wake element and save their nodal distances to the wake
            if (is_wake_element) {
                if (it_elem->Is(BOUNDARY)){
                    deactivated_ids.push_back(it_elem->Id());
                    it_elem->Set(ACTIVE, false);
                    it_elem->Set(BOUNDARY, false);
                }
                else{
                    it_elem->SetValue(WAKE, true);
                    // it_elem->SetValue(ELEMENTAL_DISTANCES, nodal_distances_to_wake);
                    // #pragma omp critical
                    // {
                    //     wake_elements_ordered_ids.push_back(it_elem->Id());
                    // }
                    auto r_geometry = it_elem->GetGeometry();
                    for (unsigned int i = 0; i < it_elem->GetGeometry().size(); i++) {
                        r_geometry[i].SetLock();
                        r_geometry[i].SetValue(WAKE_DISTANCE, nodal_distances_to_wake(i));
                        r_geometry[i].UnSetLock();
                    }
                }

            }
    }
    deactivated_model_part.AddElements(deactivated_ids);
}

void DefineEmbeddedWakeProcess::ComputeTrailingEdgeNode(){

    double max_distance = 0.0;
    ModelPart& deactivated_model_part = mrModelPart.GetSubModelPart("deactivated_model_part");
    Node<3>::Pointer p_max_node;
    Element::Pointer p_max_elem;

    auto wake_origin = mrWakeModelPart.pGetNode(5000);
    // #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(deactivated_model_part.Elements().size()); i++) {
        ModelPart::ElementIterator it_elem = deactivated_model_part.ElementsBegin() + i;

        // for (unsigned int i_node= 0; i_node < it_elem->GetGeometry().size(); i_node++) {
        //     // Compute the distance from the trailing edge to the node

        //     Vector distance_vector(2);
        //     distance_vector(0) = wake_origin -> X() - it_elem->GetGeometry()[i_node].X();
        //     distance_vector(1) = wake_origin -> Y() - it_elem->GetGeometry()[i_node].Y();
        //     double norm = norm_2(distance_vector);
        //     KRATOS_WATCH(it_elem->GetGeometry()[i_node].Id())
        //     KRATOS_WATCH(norm)
        //     if(norm>max_distance){
        //         max_distance = norm;
        //         p_max_node = mrModelPart.pGetNode(it_elem->GetGeometry()[i_node].Id());
        //     }
        // }

        Vector distance_vector(2);
        distance_vector(0) = wake_origin -> X() - it_elem->GetGeometry().Center().X();
        distance_vector(1) = wake_origin -> Y() - it_elem->GetGeometry().Center().Y();
        double norm = norm_2(distance_vector);
        KRATOS_WATCH(it_elem->Id())
        KRATOS_WATCH(norm)
        if(norm>max_distance){
            max_distance = norm;
            p_max_elem = mrModelPart.pGetElement(it_elem->Id());
        }
    }
    Vector distance_center_vector(2);
    distance_center_vector(0) = wake_origin -> X() - p_max_elem->GetGeometry().Center().X();
    distance_center_vector(1) = wake_origin -> Y() - p_max_elem->GetGeometry().Center().Y();
    double distance_to_center = norm_2(distance_center_vector);
    for (unsigned int i_node= 0; i_node < p_max_elem->GetGeometry().size(); i_node++) {
        // Vector distance_vector(2);
        // distance_vector(0) = wake_origin -> X() - p_max_elem->GetGeometry()[i_node].X();
        // distance_vector(1) = wake_origin -> Y() - p_max_elem->GetGeometry()[i_node].Y();
        // double distance = norm_2(distance_vector);
        // if (distance > distance_to_center){
        //     p_max_elem->GetGeometry()[i_node].SetValue(TRAILING_EDGE,true);
        // }
        p_max_elem->GetGeometry()[i_node].SetValue(TRAILING_EDGE,true);

    }

    // p_max_node->SetValue(TRAILING_EDGE,true);
    mrModelPart.RemoveSubModelPart("deactivated_model_part");
}

void DefineEmbeddedWakeProcess::MarkKuttaElements(){
    // #pragma omp parallel for
    // mrModelPart.pGetElement(41425)->Set(STRUCTURE);
    for (int i = 0; i < static_cast<int>(mrModelPart.Elements().size()); i++) {
        ModelPart::ElementIterator it_elem = mrModelPart.ElementsBegin() + i;
        if (it_elem->GetValue(WAKE) && it_elem->Is(ACTIVE)){
            for (unsigned int i_node= 0; i_node < it_elem->GetGeometry().size(); i_node++) {
                if(it_elem->GetGeometry()[i_node].GetValue(TRAILING_EDGE)){
                    it_elem->Set(STRUCTURE);
                    KRATOS_WATCH(it_elem->Id())
                }
            }
        }

    }
}
// This function checks whether the element is cut by the wake
const bool DefineEmbeddedWakeProcess::CheckIfWakeElement(const BoundedVector<double, 3>& rNodalDistancesToWake) const
{
    // Initialize counters
    unsigned int number_of_nodes_with_positive_distance = 0;
    unsigned int number_of_nodes_with_negative_distance = 0;

    // Count how many element nodes are above and below the wake
    for (unsigned int i = 0; i < rNodalDistancesToWake.size(); i++) {
        if (rNodalDistancesToWake(i) < 0.0) {
            number_of_nodes_with_negative_distance += 1;
        }
        else {
            number_of_nodes_with_positive_distance += 1;
        }
    }

    // Elements with nodes above and below the wake are wake elements
    return number_of_nodes_with_negative_distance > 0 &&
           number_of_nodes_with_positive_distance > 0;
}

}// Namespace Kratos
