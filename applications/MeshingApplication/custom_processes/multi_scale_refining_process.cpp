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
#include "utilities/sub_model_parts_list_utility.h"
#include "custom_utilities/uniform_refine_utility.h"

namespace Kratos
{

MultiScaleRefiningProcess::MultiScaleRefiningProcess(
    ModelPart& rThisCoarseModelPart,
    ModelPart& rThisRefinedModelPart,
    Parameters ThisParameters)
    : mrCoarseModelPart(rThisCoarseModelPart)
    , mrRefinedModelPart(rThisRefinedModelPart)
    , mParameters(ThisParameters)
{
    Parameters DefaultParameters = Parameters(R"(
    {
        "element_name"                        : "Element2D3N",
        "condition_name"                      : "Condition2D2N",
        "echo_level"                          : 0,
        "number_of_divisions_at_subscale"     : 2,
        "origin_interface_name"               : "coarse_interface",
        "subscale_interface_name"             : "refined_interface",
        "subscale_boundary_condition"         : "Condition2D2N"
    }
    )");

    mParameters.ValidateAndAssignDefaults(DefaultParameters);

    mEchoLevel = mParameters["echo_level"].GetInt();
    mDivisionsAtSubscale = mParameters["number_of_divisions_at_subscale"].GetInt();

    mElementName = mParameters["element_name"].GetString();
    mConditionName = mParameters["condition_name"].GetString();

    mCoarseInterfaceName = mParameters["origin_interface_name"].GetString();
    mRefinedInterfaceName = mParameters["subscale_interface_name"].GetString();
    mInterfaceConditionName = mParameters["subscale_boundary_condition"].GetString();

    if (mEchoLevel > 1) KRATOS_WATCH(mParameters);

    // Get the model part hierarchy
    StringVectorType sub_model_parts_names;
    sub_model_parts_names = mrCoarseModelPart.GetSubModelPartNames();
    // sub_model_parts_names = RecursiveGetSubModelPartNames(mrCoarseModelPart);

    Check();

    // Initialize the coarse model part
    InitializeCoarseModelPart(sub_model_parts_names);

    // Initialize the refined model part
    InitializeRefinedModelPart(sub_model_parts_names);
}


void MultiScaleRefiningProcess::Check()
{
    KRATOS_TRY

    KRATOS_CHECK(KratosComponents<Element>::Has(mElementName));
    KRATOS_CHECK(KratosComponents<Condition>::Has(mConditionName));
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

    // Create the auxiliary entities
    CreateElementsToRefine(elem_id, elem_tag);
    CreateConditionsToRefine(elem_id, cond_tag);

    // Execute the refinement
    int divisions = mrRefinedModelPart.GetValue(SUBSCALE_INDEX) * mDivisionsAtSubscale;
    auto uniform_refining = UniformRefineUtility<2>(mrRefinedModelPart, divisions);
    uniform_refining.Refine(node_id, elem_id, cond_id);

    // Reset the flags
    FinalizeRefinement();
}


void MultiScaleRefiningProcess::ExecuteCoarsening()
{
    IdentifyParentNodesToErase();
    IdentifyElementsToErase();
    IdentifyConditionsToErase();
    IdentifyRefinedNodesToErase();

    // NOTE: THIS IS TEMPORARY. THE UNIFORM REFINING UTILITY SHOULD BE
    // A MEMBER OBJECT IN CHARGE TO EXECUTE THE REFINEMENT AND THE COARSENING
    mrRefinedModelPart.RemoveNodesFromAllLevels(TO_ERASE);
    mrRefinedModelPart.RemoveElementsFromAllLevels(TO_ERASE);
    mrRefinedModelPart.RemoveConditionsFromAllLevels(TO_ERASE);
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


void MultiScaleRefiningProcess::InitializeCoarseModelPart(const StringVectorType& rNames)
{}


void MultiScaleRefiningProcess::InitializeRefinedModelPart(const StringVectorType& rNames)
{
    // Increase the refinement level
    int subscale_index = mrCoarseModelPart.GetValue(SUBSCALE_INDEX);
    mrRefinedModelPart.SetValue(SUBSCALE_INDEX, ++subscale_index);

    // Copy the variables
    AddVariablesToRefinedModelPart();

    // Copy all the tables and properties
    AddAllTablesToModelPart(mrCoarseModelPart, mrRefinedModelPart);
    AddAllPropertiesToModelPart(mrCoarseModelPart, mrRefinedModelPart);

    // Copy the hierarchy to the refined model part
    for (auto name : rNames)
    {
        ModelPart& sub_model_part = mrRefinedModelPart.CreateSubModelPart(name);

        ModelPart& origin_model_part = mrCoarseModelPart.GetSubModelPart(name);

        // Copy all the tables and properties
        AddAllTablesToModelPart(origin_model_part, sub_model_part);
        AddAllPropertiesToModelPart(origin_model_part, sub_model_part);
    
        // Note: we don't add the nodes, elements and conditions
        // This operation is the refining process itself
    }

    // Create a model part to store the interface boundary conditions
    mrRefinedModelPart.CreateSubModelPart(mRefinedInterfaceName);

    const int subscale_digit = 100000000;
    mMinNodeId = (subscale_index + 1) * subscale_digit;
    mMinElemId = (subscale_index + 1) * subscale_digit;
    mMinCondId = (subscale_index + 1) * subscale_digit;
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

void MultiScaleRefiningProcess::AddAllNodesToModelPart(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart)
{
    const int nnodes = static_cast<int>(rOriginModelPart.Nodes().size());
    IndexVectorType origin_nodes(nnodes);
    ModelPart::NodesContainerType::iterator node_begin = rOriginModelPart.NodesBegin();

    #pragma omp parallel for
    for (int i = 0; i < nnodes; i++)
    {
        auto node = node_begin + i;
        origin_nodes[i] = node->Id();
    }
    rDestinationModelPart.AddNodes(origin_nodes);
}


void MultiScaleRefiningProcess::AddAllElementsToModelPart(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart)
{
    const int nelems = static_cast<int>(rOriginModelPart.Elements().size());
    IndexVectorType origin_elems(nelems);
    ModelPart::ElementsContainerType::iterator elem_begin = rOriginModelPart.ElementsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nelems; i++)
    {
        auto elem = elem_begin + i;
        origin_elems[i] = elem->Id();
    }
    rDestinationModelPart.AddElements(origin_elems);
}


void MultiScaleRefiningProcess::AddAllConditionsToModelPart(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart)
{
    const int nconds = static_cast<int>(rOriginModelPart.Conditions().size());
    IndexVectorType origin_conds(nconds);
    ModelPart::ConditionsContainerType::iterator cond_begin = rOriginModelPart.ConditionsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nconds; i++)
    {
        auto cond = cond_begin + i;
        origin_conds[i] = cond->Id();
    }
    rDestinationModelPart.AddConditions(origin_conds);
}


void MultiScaleRefiningProcess::MarkElementsFromNodalFlag()
{
    const int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrCoarseModelPart.ElementsBegin();

    // We assume all the elements have the same number of nodes
    const IndexType number_of_nodes = elem_begin->GetGeometry().size();

    // We will refine the elements which all the nodes are to refine
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

    const IndexType number_of_nodes = cond_begin->GetGeometry().size();

    // We will refine the conditions which all the nodes are to refine
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
                coarse_node->Set(NEW_ENTITY, true);
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
                coarse_node->Set(OLD_ENTITY, true);
                mCoarseToRefinedNodesMap.erase(search);
                mRefinedToCoarseNodesMap.erase(search->second->Id());
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
        bool old_entity = false;
        for (IndexType inode = 0; inode < element_nodes; inode++)
        {
            if (coarse_elem->GetGeometry()[inode].Is(OLD_ENTITY))
                old_entity = true;
        }
        if (old_entity)
            coarse_elem->Set(OLD_ENTITY, true);
    }

    // Identify the refined elements to remove
    const int nelems_ref = static_cast<int>(mrRefinedModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator ref_elem_begin = mrRefinedModelPart.ElementsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nelems_ref; i++)
    {
        auto refined_elem = ref_elem_begin + i;
        if ((refined_elem->GetValue(FATHER_ELEMENT))->Is(OLD_ENTITY))
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
        bool old_entity = false;
        for (IndexType inode = 0; inode < condition_nodes; inode++)
        {
            if (coarse_cond->GetGeometry()[inode].Is(OLD_ENTITY))
                old_entity = true;
        }
        if (old_entity)
            coarse_cond->Set(OLD_ENTITY, true);
    }

    // Identify the refined conditions to remove
    const int nconds_ref = static_cast<int>(mrRefinedModelPart.Conditions().size());
    ModelPart::ConditionsContainerType::iterator ref_cond_begin = mrRefinedModelPart.ConditionsBegin();

    #pragma omp parallel for
    for (int i = 0; i < nconds_ref; i++)
    {
        auto refined_cond = ref_cond_begin + i;
        if ((refined_cond->GetValue(FATHER_CONDITION))->Is(OLD_ENTITY))
            refined_cond->Set(TO_ERASE, true);
    }
}


void MultiScaleRefiningProcess::IdentifyRefinedNodesToErase()
{
    ModelPart::ElementsContainerType::iterator element_begin = mrRefinedModelPart.ElementsBegin();
    const IndexType element_nodes = element_begin->GetGeometry().size();
    const IndexType nelems = mrRefinedModelPart.Elements().size();

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

            Element::Pointer aux_elem = mrRefinedModelPart.CreateNewElement(
                mElementName,
                ++rElemId,
                p_elem_nodes,
                coarse_elem->pGetProperties());
            
            aux_elem->SetValue(FATHER_ELEMENT, *coarse_elem.base());

            IndexType tag = rElemTag[coarse_elem->Id()];
            tag_elems_map[tag].push_back(rElemId);
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

            Condition::Pointer aux_cond = mrRefinedModelPart.CreateNewCondition(
                mConditionName,
                ++rCondId,
                p_cond_nodes,
                coarse_cond->pGetProperties());
            
            aux_cond->SetValue(FATHER_CONDITION, *coarse_cond.base());

            IndexType tag = rCondTag[coarse_cond->Id()];
            tag_conds_map[tag].push_back(rCondId);
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
    int nnodes = static_cast<int>(mrCoarseModelPart.Nodes().size());
    ModelPart::NodesContainerType::iterator nodes_begin = mrCoarseModelPart.NodesBegin();

    // Reseting the flag
    #pragma omp parallel for
    for (int i = 0; i < nnodes; i++)
    {
        auto node = nodes_begin + i;
        node->Set(NEW_ENTITY, false);
        node->Set(INTERFACE, false);
        node->Set(OLD_ENTITY, false);
    }

    int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elements_begin = mrCoarseModelPart.ElementsBegin();

    // Reseting the flag
    #pragma omp parallel for
    for (int i = 0; i < nelems; i++)
    {
        auto elem = elements_begin + i;
        elem->Set(OLD_ENTITY, false);
    }
}


void MultiScaleRefiningProcess::IdentifyRefiningInterface()
{
    // 0. Reset the flags
    
    // 1. Identify the nodes which define the boundary
    const int nelems = static_cast<int>(mrCoarseModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrCoarseModelPart.ElementsBegin();

    // The number of nodes of the elements
    const IndexType element_nodes = elem_begin->GetGeometry().size();

    // Look for the elements which are not to refine and have some nodes to refine
    for (int i = 0; i < nelems; i++)
    {
        auto elem = elem_begin + i;
        if (elem->IsNot(TO_REFINE))
        {
            for (IndexType node = 0; node < element_nodes; node++)
            {
                if (elem->GetGeometry()[node].Is(TO_REFINE))
                    elem->GetGeometry()[node].Set(INTERFACE, true);
            }
        }
    }

    // TODO: here I need to tranfer the INTERFACE flag from the father nodes to the middle nodes
    /* do some stuff */

    // 2. Remove the old conditions
    ModelPart& interface = mrRefinedModelPart.GetSubModelPart(mRefinedInterfaceName);
    const int nconds = static_cast<int>(interface.Conditions().size());
    ModelPart::ConditionsContainerType::iterator cond_begin = interface.ConditionsBegin();

    // The number of nodes of the conditions
    const IndexType condition_nodes = cond_begin->GetGeometry().size();

    // Find the conditions which are not interface
    #pragma omp parallel for
    for (int i = 0; i < nconds; i++)
    {
        auto cond = cond_begin + i;
        bool to_erase = false;
        for (IndexType node = 0; node < condition_nodes; node++)
        {
            if (cond->GetGeometry()[node].IsNot(INTERFACE))
                to_erase = true;
        }
        cond->Set(TO_ERASE, to_erase);

        // We need to preserve the old interface
        // Warning: what should I do with OMP???????
        // NOTE: I am accessing the nodes twice
        if (!to_erase)
        {
            for (IndexType node = 0; node < condition_nodes; node++)
                cond->GetGeometry()[node].Set(OLD_ENTITY, true);
        }
    }

    // 3. And finally, create the new conditions where needed
    // The condition should inherit the refining level from the coarse element
    /**
     *  loop elements
     *      loop edges
     *          loop nodes
     *              if (all nodes are INTERFACE and are not OLD_ENTITY)
     *                  Create condition
     **/
}


void MultiScaleRefiningProcess::GetLastId(
    IndexType& rNodesId,
    IndexType& rElemsId,
    IndexType& rCondsId)
{
    // Initialize the output
    rNodesId = mMinNodeId;
    rElemsId = mMinElemId;
    rCondsId = mMinCondId;

    // Get the absolute root model part
    ModelPart& root_model_part = mrRefinedModelPart.GetRootModelPart();

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