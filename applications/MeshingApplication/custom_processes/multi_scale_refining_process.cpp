//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Miguel Maso Sotomayor
//


// System includes


// External includes


// Project includes
#include "custom_processes/multi_scale_refining_process.h"
#include "geometries/point.h"
#include "processes/fast_transfer_between_model_parts_process.h"
#include "utilities/sub_model_parts_list_utility.h"
#include "custom_utilities/meshing_flags.h"

namespace Kratos
{

MultiScaleRefiningProcess::MultiScaleRefiningProcess(
    ModelPart& rThisCoarseModelPart,
    ModelPart& rThisRefinedModelPart,
    ModelPart& rThisVisualizationModelPart,
    Parameters ThisParameters)
    : mrCoarseModelPart(rThisCoarseModelPart)
    , mrRefinedModelPart(rThisRefinedModelPart)
    , mrVisualizationModelPart(rThisVisualizationModelPart)
    , mParameters(ThisParameters)
    , mUniformRefinement(mrRefinedModelPart)
{
    Parameters DefaultParameters = Parameters(R"(
    {
        "number_of_divisions_at_subscale"     : 2,
        "echo_level"                          : 0,
        "subscale_interface_base_name"        : "refined_interface",
        "subscale_boundary_condition"         : "Condition2D2N"
    }
    )");

    mParameters.ValidateAndAssignDefaults(DefaultParameters);

    mDivisionsAtSubscale = mParameters["number_of_divisions_at_subscale"].GetInt();
    mEchoLevel = mParameters["echo_level"].GetInt();

    std::string interface_base_name = mParameters["subscale_interface_base_name"].GetString();
    mRefinedInterfaceName = interface_base_name + "_" + std::to_string(mrCoarseModelPart.GetValue(SUBSCALE_INDEX) + 1);
    mInterfaceConditionName = mParameters["subscale_boundary_condition"].GetString();

    if (mEchoLevel > 1) KRATOS_WATCH(mParameters);

    Check();

    // Initialize the coarse model part
    InitializeCoarseModelPart();

    // Get the model part hierarchy
    StringVectorType sub_model_parts_names;
    sub_model_parts_names = mrCoarseModelPart.GetSubModelPartNames();
    // sub_model_parts_names = RecursiveGetSubModelPartNames(mrCoarseModelPart);

    // Initialize the refined model part
    InitializeRefinedModelPart(sub_model_parts_names);

    // Copy all the entities to the visualization model part
    InitializeVisualizationModelPart(sub_model_parts_names);
}


void MultiScaleRefiningProcess::Check()
{
    KRATOS_TRY

    KRATOS_CHECK(KratosComponents<Condition>::Has(mInterfaceConditionName));

    KRATOS_CHECK_NOT_EQUAL(mDivisionsAtSubscale, 0);

    KRATOS_CATCH("")
}


void MultiScaleRefiningProcess::ExecuteRefinement()
{
    // Initialize the maps
    IndexIndexMapType node_tag, elem_tag, cond_tag;
    SubModelPartsListUtility model_part_collection(mrCoarseModelPart);
    model_part_collection.ComputeSubModelPartsList(node_tag, cond_tag, elem_tag, mCollections);

    // Get the Id's
    IndexType node_id;
    IndexType elem_id;
    IndexType cond_id;
    GetLastId(node_id, elem_id, cond_id);

    // Clone the nodes and set the nodal flags
    CloneNodesToRefine(node_id);

    // Set the elements and conditions flags
    MarkElementsFromNodalFlag();
    MarkConditionsFromNodalFlag();

    // Check and prepare the interface
    IdentifyRefiningInterface(cond_id);

    // Create the auxiliary entities
    CreateElementsToRefine(elem_id, elem_tag);
    CreateConditionsToRefine(cond_id, cond_tag);

    // Execute the refinement
    int divisions = mrRefinedModelPart.GetValue(SUBSCALE_INDEX) * mDivisionsAtSubscale;
    mUniformRefinement.SetCustomIds(node_id, elem_id, cond_id);
    mUniformRefinement.Refine(divisions);
    mUniformRefinement.GetLastCreatedIds(node_id, elem_id, cond_id);

    // Update the visualization model part
    UpdateVisualizationAfterRefinement();

    // Reset the flags
    FinalizeRefinement();
}


void MultiScaleRefiningProcess::ExecuteCoarsening()
{
    IdentifyParentNodesToErase();
    IdentifyElementsToErase();
    IdentifyConditionsToErase();
    IdentifyRefinedNodesToErase();

    ClearInterfaceSet();

    mUniformRefinement.RemoveRefinedEntities(TO_ERASE);

    FinalizeCoarsening();
}


MultiScaleRefiningProcess::StringVectorType MultiScaleRefiningProcess::RecursiveGetSubModelPartNames(
    ModelPart& rThisModelPart,
    std::string Prefix
    )
{
    StringVectorType names = rThisModelPart.GetSubModelPartNames();
    if (!Prefix.empty())
        Prefix += ".";
    
    for (auto& name : names)
    {
        ModelPart& sub_model_part = rThisModelPart.GetSubModelPart(name);
        auto sub_names = this->RecursiveGetSubModelPartNames(sub_model_part, Prefix + name);
        name.insert(0, Prefix);
        for (auto sub_name : sub_names)
            names.push_back(sub_name);
    }

    return names;
}


ModelPart& MultiScaleRefiningProcess::RecursiveGetSubModelPart(ModelPart& rThisModelPart, std::string FullName)
{
    std::istringstream iss(FullName);
    std::string token;
    if (std::getline(iss, token, '.'))
    {
        ModelPart& aux_model_part = rThisModelPart.GetSubModelPart(token);
        return RecursiveGetSubModelPart(aux_model_part, iss.str());
    }
    return rThisModelPart;
}


void MultiScaleRefiningProcess::InitializeNewModelPart(ModelPart& rReferenceModelPart, ModelPart& rNewModelPart)
{
    // Copy all the tables and properties
    AddAllTablesToModelPart(rReferenceModelPart, rNewModelPart);
    AddAllPropertiesToModelPart(rReferenceModelPart, rNewModelPart);

    // Get the model part hierarchy
    StringVectorType sub_model_parts_names;
    sub_model_parts_names = rReferenceModelPart.GetSubModelPartNames();
    // sub_model_parts_names = RecursiveGetSubModelPartNames(mrCoarseModelPart);

    // Copy the hierarchy to the refined model part
    for (auto name : sub_model_parts_names)
    {
        ModelPart& sub_model_part = rNewModelPart.CreateSubModelPart(name);

        ModelPart& origin_model_part = rReferenceModelPart.GetSubModelPart(name);

        // Copy all the tables and properties
        AddAllTablesToModelPart(origin_model_part, sub_model_part);
        AddAllPropertiesToModelPart(origin_model_part, sub_model_part);
    }
}


void MultiScaleRefiningProcess::InitializeCoarseModelPart()
{
    // Create a model part to store the interface boundary conditions
    if (mrCoarseModelPart.HasSubModelPart(mRefinedInterfaceName))
    {
        mrCoarseModelPart.RemoveNodesFromAllLevels();
        mrCoarseModelPart.RemoveElementsFromAllLevels();
        mrCoarseModelPart.RemoveConditionsFromAllLevels();
    }
    else
        mrCoarseModelPart.CreateSubModelPart(mRefinedInterfaceName);
}


void MultiScaleRefiningProcess::InitializeRefinedModelPart(const StringVectorType& rNames)
{
    // Increase the refinement level
    int subscale_index = mrCoarseModelPart.GetValue(SUBSCALE_INDEX);
    mrRefinedModelPart.SetValue(SUBSCALE_INDEX, ++subscale_index);

    // Copy the variables
    AddVariablesToRefinedModelPart();

    // Create a model part to store the interface boundary conditions
    if (mrRefinedModelPart.HasSubModelPart(mRefinedInterfaceName))
    {
        mrRefinedModelPart.RemoveNodesFromAllLevels();
        mrRefinedModelPart.RemoveElementsFromAllLevels();
        mrRefinedModelPart.RemoveConditionsFromAllLevels();
    }
    else
        mrRefinedModelPart.CreateSubModelPart(mRefinedInterfaceName);
}


void MultiScaleRefiningProcess::InitializeVisualizationModelPart(const StringVectorType& rNames)
{
    // Create a model part to store the interface boundary conditions
    mrVisualizationModelPart.CreateSubModelPart(mRefinedInterfaceName);

    // Add the entities to the root model part
    FastTransferBetweenModelPartsProcess(mrVisualizationModelPart, mrCoarseModelPart)();

    // Add the entities to the submodel parts
    for (auto name : rNames)
    {
        ModelPart& destination = mrVisualizationModelPart.GetSubModelPart(name);
        ModelPart& origin = mrCoarseModelPart.GetSubModelPart(name);
        FastTransferBetweenModelPartsProcess(destination, origin)();
    }        
}


void MultiScaleRefiningProcess::UpdateVisualizationAfterRefinement()
{
    // Remove the refined elements and conditions to substitute them by the refined ones
    mrVisualizationModelPart.RemoveElementsFromAllLevels(MeshingFlags::REFINED);
    mrVisualizationModelPart.RemoveConditionsFromAllLevels(MeshingFlags::REFINED);

    // Remove the refined nodes which are not interface
    const int nnodes = static_cast<int>(mrCoarseModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator nodes_begin = mrCoarseModelPart.NodesBegin();
    
    for (int i = 0; i < nnodes; i++)
    {
        auto coarse_node = nodes_begin + i;
        if (coarse_node->Is((MeshingFlags::REFINED)) && (coarse_node->IsNot(INTERFACE)))
            coarse_node->Set(INSIDE, true);
    }
    mrVisualizationModelPart.RemoveNodesFromAllLevels(INSIDE);

    // Add the new entities which are refined
    FastTransferBetweenModelPartsProcess(mrVisualizationModelPart, mrRefinedModelPart,
        FastTransferBetweenModelPartsProcess::EntityTransfered::ALL, NEW_ENTITY)();
}


void MultiScaleRefiningProcess::AddVariablesToRefinedModelPart()
{
    auto variables_list = mrCoarseModelPart.GetNodalSolutionStepVariablesList();

    for (auto variable : variables_list)
    {
        // mrRefinedModelPart.AddNodalSolutionStepVariable(variable);
    }
}


void MultiScaleRefiningProcess::AddAllPropertiesToModelPart(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart)
{
    const IndexType nprop = rOriginModelPart.NumberOfProperties();
    ModelPart::PropertiesContainerType::iterator prop_begin = rOriginModelPart.PropertiesBegin();

    for (IndexType i = 0; i < nprop; i++)
    {
        auto prop = prop_begin + i;
        rDestinationModelPart.AddProperties(*prop.base());
    }
}


void MultiScaleRefiningProcess::AddAllTablesToModelPart(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart)
{
    const IndexType ntables = rOriginModelPart.NumberOfTables();
    ModelPart::TablesContainerType::iterator table_begin = rOriginModelPart.TablesBegin();

    for (IndexType i = 0; i < ntables; i++)
    {
        auto table = table_begin + i;
        rDestinationModelPart.AddTable(table.base()->first, table.base()->second);
    }
}


void MultiScaleRefiningProcess::MarkElementsFromNodalFlag()
{
    const int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrCoarseModelPart.ElementsBegin();

    // We assume all the elements have the same number of nodes
    const IndexType number_of_nodes = elem_begin->GetGeometry().size();

    // We will refine the elements which:
    // 1. all the nodes are to refine
    // 2. at least, one node is marked as new entity
    // NEW_ENTITY flag is used to avoid duplication of refined entities on previous steps
    #pragma omp parallel for
    for (int i = 0; i < nelems; i++)
    {
        auto elem = elem_begin + i;
        bool to_refine = true;
        bool new_entity = false;
        for (IndexType node = 0; node < number_of_nodes; node++)
        {
            if (elem->GetGeometry()[node].IsNot(TO_REFINE))
                to_refine = false;
            
            if (elem->GetGeometry()[node].Is(NEW_ENTITY))
                new_entity = true;
        }
        elem->Set(TO_REFINE, (to_refine && new_entity));
    }
}


void MultiScaleRefiningProcess::MarkConditionsFromNodalFlag()
{
    const int nconds = static_cast<int>(mrCoarseModelPart.Conditions().size());
    ModelPart::ConditionsContainerType::iterator cond_begin = mrCoarseModelPart.ConditionsBegin();

    // We assume all the conditions have the same number of nodes
    const IndexType number_of_nodes = cond_begin->GetGeometry().size();

    // We will refine the conditions which:
    // 1. all the nodes are to refine
    // 2. at least, one node is marked as new entity
    // NEW_ENTITY flag is used to avoid duplication of refined entities on previous steps
    #pragma omp parallel for
    for (int i = 0; i < nconds; i++)
    {
        auto cond = cond_begin + i;
        bool to_refine = true;
        bool new_entity = false;
        for (IndexType node = 0; node < number_of_nodes; node++)
        {
            if (cond->GetGeometry()[node].IsNot(TO_REFINE))
                to_refine = false;
            
            if (cond->GetGeometry()[node].Is(NEW_ENTITY))
                new_entity = true;
        }
        cond->Set(TO_REFINE, (to_refine && new_entity));
    }
}


void MultiScaleRefiningProcess::CloneNodesToRefine(IndexType& rNodeId)
{
    const int nnodes = static_cast<int>(mrCoarseModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator nodes_begin = mrCoarseModelPart.NodesBegin();

    // Adding the nodes to the refined model part
    for (int i = 0; i < nnodes; i++)
    {
        auto coarse_node = nodes_begin + i;
        if (coarse_node->Is(TO_REFINE))
        {
            auto search = mCoarseToRefinedNodesMap.find(coarse_node->Id());
            if (search == mCoarseToRefinedNodesMap.end())
            {
                NodeType::Pointer new_node = mrRefinedModelPart.CreateNewNode(++rNodeId, *coarse_node);
                mCoarseToRefinedNodesMap[coarse_node->Id()] = new_node;
                mRefinedToCoarseNodesMap[rNodeId] = *coarse_node.base();
                new_node->Set(NEW_ENTITY, true);
                coarse_node->Set(NEW_ENTITY, true);
                coarse_node->Set(MeshingFlags::REFINED, true);
            }
        }
    }

    // Adding the nodes to the refined sub model parts
    StringVectorType sub_model_part_names = mrCoarseModelPart.GetSubModelPartNames();
    for (auto name : sub_model_part_names)
    {
        ModelPart& coarse_sub_model_part = mrCoarseModelPart.GetSubModelPart(name);
        ModelPart& refined_sub_model_part = mrRefinedModelPart.GetSubModelPart(name);

        const int nnodes = static_cast<int>(coarse_sub_model_part.Nodes().size());
        ModelPart::NodesContainerType::iterator nodes_begin = coarse_sub_model_part.NodesBegin();

        for (int i = 0; i < nnodes; i++)
        {
            auto coarse_node = nodes_begin + i;
            if (coarse_node->Is(NEW_ENTITY))
                refined_sub_model_part.AddNode(mCoarseToRefinedNodesMap[coarse_node->Id()]);
        }
    }
}


void MultiScaleRefiningProcess::IdentifyParentNodesToErase()
{
    const int nnodes = static_cast<int>(mrCoarseModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator nodes_begin = mrCoarseModelPart.NodesBegin();

    // Identify the nodes to remove
    for (int i = 0; i < nnodes; i++)
    {
        auto coarse_node = nodes_begin + i;
        if (coarse_node->IsNot(TO_REFINE))
        {
            auto search = mCoarseToRefinedNodesMap.find(coarse_node->Id());
            if (search != mCoarseToRefinedNodesMap.end())
            {
                // We need to ensure the refined mesh does not has dependencies
                if (search->second->IsNot(MeshingFlags::REFINED))
                {
                    coarse_node->Set(MeshingFlags::TO_COARSEN, true);
                    mCoarseToRefinedNodesMap.erase(search);
                    mRefinedToCoarseNodesMap.erase(search->second->Id());
                }
            }
        }
    }
}


void MultiScaleRefiningProcess::IdentifyElementsToErase()
{
    // Identify the parent elements to coarse
    const int nelems_coarse = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator coarse_elem_begin = mrCoarseModelPart.ElementsBegin();

    // The number of nodes of the elements
    const IndexType element_nodes = coarse_elem_begin->GetGeometry().size();

    #pragma omp parallel for
    for (int i = 0; i < nelems_coarse; i++)
    {
        auto coarse_elem = coarse_elem_begin + i;
        bool to_coarse = false;
        for (IndexType inode = 0; inode < element_nodes; inode++)
        {
            if (coarse_elem->GetGeometry()[inode].Is(MeshingFlags::TO_COARSEN))
                to_coarse = true;
        }
        if (to_coarse)
            coarse_elem->Set(MeshingFlags::TO_COARSEN, true);
    }

    // Identify the refined elements to remove
    const int nelems_ref = static_cast<int>(mrRefinedModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator ref_elem_begin = mrRefinedModelPart.ElementsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nelems_ref; i++)
    {
        auto refined_elem = ref_elem_begin + i;
        if ((refined_elem->GetValue(FATHER_ELEMENT))->Is(MeshingFlags::TO_COARSEN))
            refined_elem->Set(TO_ERASE, true);
    }
}


void MultiScaleRefiningProcess::IdentifyConditionsToErase()
{
    // Identify the parent conditions  to coarse
    const int nconds_coarse = static_cast<int>(mrCoarseModelPart.Conditions().size());
    ModelPart::ConditionsContainerType::iterator coarse_cond_begin = mrCoarseModelPart.ConditionsBegin();

    // The number of nodes of the conditions
    const IndexType condition_nodes = coarse_cond_begin->GetGeometry().size();

    #pragma omp parallel for
    for (int i = 0; i < nconds_coarse; i++)
    {
        auto coarse_cond = coarse_cond_begin + i;
        bool to_coarse = false;
        for (IndexType inode = 0; inode < condition_nodes; inode++)
        {
            if (coarse_cond->GetGeometry()[inode].Is(MeshingFlags::TO_COARSEN))
                to_coarse = true;
        }
        if (to_coarse)
            coarse_cond->Set(MeshingFlags::TO_COARSEN, true);
    }

    // Identify the refined conditions to remove
    const int nconds_ref = static_cast<int>(mrRefinedModelPart.Conditions().size());
    ModelPart::ConditionsContainerType::iterator ref_cond_begin = mrRefinedModelPart.ConditionsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nconds_ref; i++)
    {
        auto refined_cond = ref_cond_begin + i;
        if ((refined_cond->GetValue(FATHER_CONDITION))->Is(MeshingFlags::TO_COARSEN))
            refined_cond->Set(TO_ERASE, true);
    }
}


void MultiScaleRefiningProcess::IdentifyRefinedNodesToErase()
{
    const IndexType nelems = mrRefinedModelPart.Elements().size();
    if (nelems != 0) // just avoiding segfault in case of an empty coarse model part
    {
        ModelPart::ElementsContainerType::iterator element_begin = mrRefinedModelPart.ElementsBegin();
        const IndexType element_nodes = element_begin->GetGeometry().size();

        for (IndexType i = 0; i < nelems; i++)
        {
            auto elem = element_begin + i;
            if (elem->Is(TO_ERASE))
            {
                for (IndexType inode = 0; inode < element_nodes; inode++)
                {
                    if ((elem->GetGeometry()[inode]).IsNot(TO_REFINE))
                        (elem->GetGeometry()[inode]).Set(TO_ERASE, true);
                }
            }
        }
    }
}


void MultiScaleRefiningProcess::CreateElementsToRefine(IndexType& rElemId, IndexIndexMapType& rElemTag)
{
    const int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elements_begin = mrCoarseModelPart.ElementsBegin();

    // We assume all the elements have the same number of nodes
    const IndexType number_of_nodes = elements_begin->GetGeometry().size();

    // The map to add the elements to the sub model parts
    IndexVectorMapType tag_elems_map;

    // #pragma omp parallel for
    for (int i = 0; i < nelems; i++)
    {
        auto coarse_elem = elements_begin + i;
        if (coarse_elem->Is(TO_REFINE))
        {
            Geometry<NodeType>::PointsArrayType p_elem_nodes;
            for (IndexType node = 0; node < number_of_nodes; node++)
            {
                IndexType node_id = coarse_elem->GetGeometry()[node].Id();
                p_elem_nodes.push_back(mCoarseToRefinedNodesMap[node_id]);
            }

            Element::Pointer aux_elem = coarse_elem->Clone(++rElemId, p_elem_nodes);
            mrRefinedModelPart.AddElement(aux_elem);
            
            aux_elem->SetValue(FATHER_ELEMENT, *coarse_elem.base());
            aux_elem->Set(NEW_ENTITY, true);

            IndexType tag = rElemTag[coarse_elem->Id()];
            tag_elems_map[tag].push_back(rElemId);

            coarse_elem->Set(MeshingFlags::REFINED, true);
        }
    }

    // Loop the sub model parts and add the new elements to it
    for (auto& collection : mCollections)
    {
        const auto tag = collection.first;
        if (tag != 0)
        {
            for (auto name : collection.second)
            {
                ModelPart& sub_model_part = mrRefinedModelPart.GetSubModelPart(name);
                sub_model_part.AddElements(tag_elems_map[tag]);
            }
        }
    }
}


void MultiScaleRefiningProcess::CreateConditionsToRefine(IndexType& rCondId, IndexIndexMapType& rCondTag)
{
    const int nconds = static_cast<int>(mrCoarseModelPart.Conditions().size());
    ModelPart::ConditionsContainerType::iterator conditions_begin = mrCoarseModelPart.ConditionsBegin();

    // We assume all the conditions have the same number of nodes
    const IndexType number_of_nodes = conditions_begin->GetGeometry().size();

    // The map to add the conditions to the sub model parts
    IndexVectorMapType tag_conds_map;

    // #pragma omp parallel for
    for (int i = 0; i < nconds; i++)
    {
        auto coarse_cond = conditions_begin + i;
        if (coarse_cond->Is(TO_REFINE))
        {
            Geometry<NodeType>::PointsArrayType p_cond_nodes;
            for (IndexType node = 0; node < number_of_nodes; node++)
            {
                IndexType node_id = coarse_cond->GetGeometry()[node].Id();
                p_cond_nodes.push_back(mCoarseToRefinedNodesMap[node_id]);
            }

            Condition::Pointer aux_cond = coarse_cond->Clone(++rCondId, p_cond_nodes);
            mrRefinedModelPart.AddCondition(aux_cond);
            
            aux_cond->SetValue(FATHER_CONDITION, *coarse_cond.base());
            aux_cond->Set(NEW_ENTITY, true);

            IndexType tag = rCondTag[coarse_cond->Id()];
            tag_conds_map[tag].push_back(rCondId);

            coarse_cond->Set(MeshingFlags::REFINED, true);
        }
    }

    // Loop the sub model parts and add the new conditions to it
    for (auto& collection : mCollections)
    {
        const auto tag = collection.first;
        if (tag != 0)
        {
            for (auto name : collection.second)
            {
                ModelPart& sub_model_part = mrRefinedModelPart.GetSubModelPart(name);
                sub_model_part.AddConditions(tag_conds_map[tag]);
            }
        }
    }
}


void MultiScaleRefiningProcess::FinalizeRefinement()
{
    // Resetting the nodes flags
    int nnodes = static_cast<int>(mrCoarseModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator nodes_begin = mrCoarseModelPart.NodesBegin();

    #pragma omp parallel for
    for (int i = 0; i < nnodes; i++)
    {
        auto node = nodes_begin + i;
        node->Set(NEW_ENTITY, false);
        node->Set(INTERFACE, false);
    }
}

void MultiScaleRefiningProcess::FinalizeCoarsening()
{
    // Resetting the nodes flags
    int nnodes = static_cast<int>(mrCoarseModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator nodes_begin = mrCoarseModelPart.NodesBegin();

    #pragma omp parallel for
    for (int i = 0; i < nnodes; i++)
    {
        auto node = nodes_begin + i;
        node->Set(MeshingFlags::TO_COARSEN, false);
    }

    // Resetting the elements flags
    int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elements_begin = mrCoarseModelPart.ElementsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nelems; i++)
    {
        auto elem = elements_begin + i;
        elem->Set(MeshingFlags::TO_COARSEN, false);
    }

    // Resetting the conditions flags
    int nconds = static_cast<int>(mrCoarseModelPart.Conditions().size());
    ModelPart::ConditionsContainerType::iterator conditions_begin = mrCoarseModelPart.ConditionsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nconds; i++)
    {
        auto cond = conditions_begin + i;
        cond->Set(MeshingFlags::TO_COARSEN, false);
    }
}


void MultiScaleRefiningProcess::IdentifyRefiningInterface(IndexType& rCondId)
{
    ModelPart& coarse_interface = mrCoarseModelPart.GetSubModelPart(mRefinedInterfaceName);
    Properties::Pointer property = mrCoarseModelPart.ElementsBegin()->pGetProperties();
    // 0. Reset the flags
    
    // 1. Identify the nodes which define the boundary
    const int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrCoarseModelPart.ElementsBegin();

    // The number of nodes of the elements
    const IndexType element_nodes = elem_begin->GetGeometry().size();

    // The number of edges or faces
    IndexType cond_nodes;
    const IndexType dimension = elem_begin->GetGeometry().Dimension();
    if (dimension == 2)
        cond_nodes = elem_begin->GetGeometry().Edges()[0].PointsNumber();
    else
        cond_nodes = elem_begin->GetGeometry().Faces()[0].PointsNumber();

    // Identify the current interface: set the nodes and create the base conditions
    // Look for the elements which are not to refine and have some nodes to refine
    for (int i = 0; i < nelems; i++)
    {
        auto elem = elem_begin + i;
        if (elem->IsNot(MeshingFlags::REFINED))  // TODO: set the flag off before coarsening the element
        {
            // set the nodal flags
            for (IndexType node = 0; node < element_nodes; node++)
            {
                if (elem->GetGeometry()[node].Is(MeshingFlags::REFINED))
                    elem->GetGeometry()[node].Set(INTERFACE, true);
            }
            
            // Create the condition if needed
            for (auto edge : elem->GetGeometry().Edges())
            {
                bool is_interface = true;
                IndexVectorType interface_key(cond_nodes);
                for (IndexType i = 0; i < cond_nodes; i++)
                {
                    if (edge[i].IsNot(INTERFACE))
                        is_interface = false;
                    interface_key[i] = edge[i].Id();
                }
                if (is_interface)
                {
                    // Create the condition if it does not exist
                    std::sort(interface_key.begin(), interface_key.end());
                    auto search = mCoarseInterfacesSet.find(interface_key);
                    if (search == mCoarseInterfacesSet.end())
                    {                        
                        // Condition creation
                        auto aux_cond = coarse_interface.CreateNewCondition(
                            mInterfaceConditionName,
                            ++rCondId,
                            edge,
                            property);
                        aux_cond->Set(TO_REFINE, true);

                        // Storing the condition key
                        mCoarseInterfacesSet.insert(interface_key);
                    }
                }
            }
        }
    }

    // Identify the old interface and delete it
    /* do some stuff here */
    //IndexType nconds 
}


void MultiScaleRefiningProcess::ClearInterfaceSet()
{
    ModelPart& coarse_interface = mrCoarseModelPart.GetSubModelPart(mRefinedInterfaceName);

    if (coarse_interface.Conditions().size() > 0) // just avoiding segfault in case of an empty interface model part
    {
        const IndexType num_nodes = coarse_interface.ConditionsBegin()->GetGeometry().PointsNumber();

        for (ModelPart::ConditionIterator cond = coarse_interface.ConditionsBegin(); cond < coarse_interface.ConditionsEnd(); cond++)
        {
            if (cond->Is(TO_ERASE))
            {
                // Get the key and remove it from the set
                IndexVectorType interface_key(num_nodes);
                for (IndexType i = 0; i < num_nodes; i++)
                    interface_key[i] = cond->GetGeometry()[i].Id();
                
                std::sort(interface_key.begin(), interface_key.end());
                auto search = mCoarseInterfacesSet.find(interface_key);
                if (search != mCoarseInterfacesSet.end())
                    mCoarseInterfacesSet.erase(search);
            }
        }
    }
}


void MultiScaleRefiningProcess::GetLastId(
    IndexType& rNodesId,
    IndexType& rElemsId,
    IndexType& rCondsId)
{
    // Initialize the output
    rNodesId = 0;
    rElemsId = 0;
    rCondsId = 0;

    // Get the absolute root model part
    ModelPart& root_model_part = mrVisualizationModelPart.GetRootModelPart();

    // Get the maximum node id
    const IndexType nnodes = root_model_part.Nodes().size();
    ModelPart::NodesContainerType::iterator nodes_begin = root_model_part.NodesBegin();
    for (IndexType i = 0; i < nnodes; i++)
    {
        auto inode = nodes_begin + i;
        if (rNodesId < inode->Id())
            rNodesId = inode->Id();
    }

    // Get the maximum element id
    const IndexType nelems = root_model_part.Elements().size();
    ModelPart::ElementsContainerType::iterator elements_begin = root_model_part.ElementsBegin();
    for (IndexType i = 0; i < nelems; i++)
    {
        auto elem = elements_begin + i;
        if (rElemsId < elem->Id())
            rElemsId = elem->Id();
    }

    // Get the maximum condition id
    const IndexType nconds = root_model_part.Conditions().size();
    ModelPart::ConditionsContainerType::iterator conditions_begin = root_model_part.ConditionsBegin();
    for (IndexType i = 0; i < nconds; i++)
    {
        auto cond = conditions_begin + i;
        if (rCondsId < cond->Id())
            rCondsId = cond->Id();
    }

    // // Get the number of threads
    // const int num_threads = OpenMPUtils::GetNumThreads();

    // // Get the maximum node id
    // const int nnodes = static_cast<int>(root_model_part.Nodes().size());
    // ModelPart::NodesContainerType::iterator nodes_begin = root_model_part.NodesBegin();
    // std::vector<IndexType> nodes_id(num_threads);
    // #pragma omp parallel
    // {
    //     const int thread_id = OpenMPUtils::ThisThread();

    //     #pragma omp for
    //     for (int i = 0; i < nnodes; i++)
    //     {
    //         auto node = nodes_begin + i;
    //         if (nodes_id[thread_id] < node->Id())
    //             nodes_id[thread_id] = node->Id();
    //     }

    //     #pragma omp single
    //     {
    //         rNodesId = *std::max_element(nodes_id.begin(), nodes_id.end());
    //     }
    // }

    // // Get the maximum element id
    // const int nelems = static_cast<int>(root_model_part.Nodes().size());
    // ModelPart::ElementsContainerType::iterator elements_begin = root_model_part.ElementsBegin();
    // std::vector<IndexType> elems_id(num_threads);
    // #pragma omp parallel
    // {
    //     const int thread_id = OpenMPUtils::ThisThread();

    //     #pragma omp for
    //     for (int i = 0; i < nelems; i++)
    //     {   /* WARNING: HERE IS SOME SEGMENTATION FAULT */
    //         auto elem = elements_begin + i;
    //         if (elems_id[thread_id] < elem->Id())
    //             elems_id[thread_id] = elem->Id();
    //     }

    //     #pragma omp single
    //     {
    //         rElemsId = *(std::max_element(elems_id.begin(), elems_id.end()));
    //     }
    // }
}


} // namespace Kratos