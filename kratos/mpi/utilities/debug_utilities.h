//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main author:     Carlos A. Roig
//                   Riccardo Rossi
//

#pragma once

#include <vector>
#include <sstream>

#include "includes/model_part.h"
#include "includes/parallel_environment.h"
#include "utilities/global_pointer_utilities.h"
#include "utilities/pointer_communicator.h"
#include "utilities/retrieve_global_pointers_by_index_functor.h"
#include "utilities/get_value_functor.h"

namespace Kratos {

class MpiDebugUtilities {
public:

    KRATOS_CLASS_POINTER_DEFINITION(MpiDebugUtilities);

    MpiDebugUtilities() {}

    // This will work with #5091 or when we move to C++17
    // static void CheckNodalHistoricalDatabaseConsistency(ModelPart & rModelPart) { 
    //     // Build the list of indices and the pointer communicator
    //     DataCommunicator& r_default_comm = ParallelEnvironment::GetDefaultDataCommunicator();
    //     std::vector<int> indices;

    //     auto node_list = rModelPart.Nodes();

    //     for(auto& node : node_list) {
    //         indices.push_back(node.Id());
    //     }

    //     auto gp_map  = GlobalPointerUtilities::RetrieveGlobalIndexedPointersMap(node_list, indices, r_default_comm );
    //     auto gp_list = GlobalPointerUtilities::RetrieveGlobalIndexedPointers(node_list, indices, r_default_comm );

    //     GlobalPointerCommunicator<Node<3>> pointer_comm(r_default_comm, gp_list.ptr_begin(), gp_list.ptr_end());

    //     // Note: I think this can be change for an access function with std::tuple of the variables instead of a loop, but not in C++11/14
    //     for(auto & variableData: rModelPart.GetNodalSolutionStepVariablesList()) {
    //         auto & variable = KratosComponents<VariableComponentType>::Get(variableData.Name());
    //         CheckNodalHistoricalVariableConsistency(rModelPart, variable, pointer_comm, gp_map);
    //     }
    // }

    template<class TVarType>
    static void CheckNodalHistoricalVariableConsistency(
        ModelPart & rModelPart,
        const TVarType & rVariable) 
    {
        // Build the list of indices and the pointer communicator
        DataCommunicator& r_default_comm = ParallelEnvironment::GetDefaultDataCommunicator();
        std::vector<int> indices;

        auto node_list = rModelPart.Nodes();

        for(auto& node : node_list) {
            indices.push_back(node.Id());
        }

        auto gp_map  = GlobalPointerUtilities::RetrieveGlobalIndexedPointersMap(node_list, indices, r_default_comm );
        auto gp_list = GlobalPointerUtilities::RetrieveGlobalIndexedPointers(node_list, indices, r_default_comm );

        GlobalPointerCommunicator< Node<3>> pointer_comm(r_default_comm, gp_list.ptr_begin(), gp_list.ptr_end());

        CheckNodalHistoricalVariableConsistency(rModelPart, rVariable, pointer_comm, gp_map);
    }

    template<class TVarType>
    static void CheckNodalHistoricalVariableConsistency(
        ModelPart & rModelPart,
        const TVarType & rVariable,
        GlobalPointerCommunicator<Node<3>> & rPointerCommunicator, 
        std::unordered_map<int, GlobalPointer<Node<3>>> & gp_map) 
    {
        DataCommunicator& r_default_comm = ParallelEnvironment::GetDefaultDataCommunicator();
        
        bool val_error_detected = false;
        bool fix_error_detected = false;

        std::stringstream error_stream;

        // Create the data functior
        auto data_proxy = rPointerCommunicator.Apply(
            [rVariable](GlobalPointer< Node<3> >& gp)->std::pair<typename TVarType::Type, bool> {
                return {gp->FastGetSolutionStepValue(rVariable),gp->IsFixed(rVariable)};
            }
        );

        // Check variable for all nodes.
        for(auto& node : rModelPart.Nodes()) {
            auto& gp = gp_map[node.Id()];

            // Check Variable
            if(data_proxy.Get(gp).first != node.FastGetSolutionStepValue(rVariable)) {
                std::cout << r_default_comm.Rank() << " Inconsistent variable Val for Id: " << node.Id() << " Expected: " << node.FastGetSolutionStepValue(rVariable) << " Obtained " << data_proxy.Get(gp).first << std::endl;
                val_error_detected = true;
            }
        }

        // Check fixity for all nodes.
        for(auto& node : rModelPart.Nodes()) {
            auto& gp = gp_map[node.Id()];

            // Check Fixity
            if(data_proxy.Get(gp).second != node.IsFixed(rVariable)) {
                std::cout << r_default_comm.Rank() << " Inconsistent variable Fix for Id: " << node.Id() << " Expected: " << node.IsFixed(rVariable) << " Obtained " << data_proxy.Get(gp).second << std::endl;
                fix_error_detected = true;
            }
        }

        if(val_error_detected) {
            error_stream << "Value error(s) found" << std::endl;
        }

        if(fix_error_detected) {
            error_stream << "Fixity error(s) found" << std::endl;
        }

        if(error_stream.rdbuf()->in_avail())
        {
            KRATOS_ERROR << error_stream.str() << std::endl;
        }
    }
};
    
} // namespace Kratos