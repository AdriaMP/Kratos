//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Miguel Maso Sotomayor
//


// System includes


// External includes


// Project includes
#include "includes/define.h"
#include "includes/variables.h"
#include "uniform_refinement_utility.h"
#include "utilities/sub_model_parts_list_utility.h"


namespace Kratos
{
/// Default constructor
template< unsigned int TDim>
UniformRefinementUtility<TDim>::UniformRefinementUtility(ModelPart& rModelPart) :
    mrModelPart(rModelPart)
{
    // Initialize the member variables storing the Id
    mLastNodeId = 0;
    mLastElemId = 0;
    mLastCondId = 0;

    // Get the last node id
    const IndexType nnodes = mrModelPart.Nodes().size();
    for (IndexType i = 0; i < nnodes; i++)
    {
        ModelPart::NodesContainerType::iterator inode = mrModelPart.NodesBegin() + i;
        if (inode->Id() > mLastNodeId)
            mLastNodeId = inode->Id();
    }

    // Get the elements id
    const IndexType n_elements = mrModelPart.Elements().size();
    for (IndexType i = 0; i < n_elements; i++)
    {
        ModelPart::ElementsContainerType::iterator ielement = mrModelPart.ElementsBegin() + i;
        if (ielement->Id() > mLastElemId)
            mLastElemId = ielement->Id();
    }

    // Get the conditions id
    const IndexType n_conditions = mrModelPart.Conditions().size();
    for (IndexType i = 0; i < n_conditions; i++)
    {
        ModelPart::ConditionsContainerType::iterator icondition = mrModelPart.ConditionsBegin() + i;
        if (icondition->Id() > mLastCondId)
            mLastCondId = icondition->Id();
    }

    mStepDataSize = mrModelPart.GetNodalSolutionStepDataSize();
    mBufferSize = mrModelPart.GetBufferSize();

    // Compute the sub model part maps
    // SubModelPartsListUtility colors_utility(mrModelPart);
    // colors_utility.ComputeSubModelPartsList(mNodesColorMap, mCondColorMap, mElemColorMap, mColors);
}


/// Destructor
template< unsigned int TDim>
UniformRefinementUtility<TDim>::~UniformRefinementUtility() {}


/// Turn back information as a string.
template< unsigned int TDim>
std::string UniformRefinementUtility<TDim>::Info() const {
    return "Uniform refine utility.";
}


/// Print information about this object.
template< unsigned int TDim>
void UniformRefinementUtility<TDim>::PrintInfo(std::ostream& rOStream) const {
    rOStream << "Uniform refine utility.";
}


/// Print object's data.
template< unsigned int TDim>
void UniformRefinementUtility<TDim>::PrintData(std::ostream& rOStream) const {
    rOStream << "Uniform refine utility constructed with:\n";
    rOStream << "   Model part: " << mrModelPart.Info() << "\n";
}


/// Execute the refinement until the final number of divisions level is reached
template< unsigned int TDim>
void UniformRefinementUtility<TDim>::Refine(int& rFinalRefinementLevel)
{
    if (mrModelPart.Nodes().size() == 0)
        KRATOS_WARNING("UniformRefinementUtility") << "Attempting to refine an empty model part" << std::endl;
    else
        mDofs = mrModelPart.NodesBegin()->GetDofs();

    // Get the lowest refinement level
    int minimum_divisions_level = 1e6;
    const IndexType n_elements = mrModelPart.Elements().size();
    for (IndexType i = 0; i < n_elements; i++)
    {
        ModelPart::ElementsContainerType::iterator ielement = mrModelPart.ElementsBegin() + i;
        if (ielement->GetValue(NUMBER_OF_DIVISIONS) < minimum_divisions_level)
            minimum_divisions_level = ielement->GetValue(NUMBER_OF_DIVISIONS);
    }

    // Restart the model part collections utility
    mNodesTags.clear();
    mElementsTags.clear();
    mConditionsTags.clear();
    SubModelPartsListUtility collections_utility(mrModelPart);
    collections_utility.ComputeSubModelPartsList(mNodesTags, mConditionsTags, mElementsTags, mCollections);

    IndexIndexVectorMapType tag_nodes, tag_elements, tag_conditions;

    for (int divisions = minimum_divisions_level; divisions < rFinalRefinementLevel; divisions++)
    {
        ExecuteDivision(divisions, tag_nodes, tag_elements, tag_conditions);
    }

    // Finally, add the new entities to the sub model parts
    for (auto& collection : mCollections)
    {
        const IndexType tag = collection.first;
        if (tag != 0) // NOTE: tag == 0 is the root model part
        {
            for (auto model_part_name : collection.second)
            {
                ModelPart& sub_model_part = mrModelPart.GetSubModelPart(model_part_name);
                sub_model_part.AddNodes(tag_nodes[tag]);
                sub_model_part.AddElements(tag_elements[tag]);
                sub_model_part.AddConditions(tag_conditions[tag]);
            }
        }
    }
}


/// Set the custom ids which will be used to create new entities
template<unsigned int TDim>
void UniformRefinementUtility<TDim>::SetCustomIds(IndexType& rNodeId, IndexType& rElemId, IndexType& rCondId)
{
    // Set the id
    mLastNodeId = rNodeId;
    mLastElemId = rElemId;
    mLastCondId = rCondId;
}


/// Get the last id of the created nodes, elements and conditions
template<unsigned int TDim>
void UniformRefinementUtility<TDim>::GetLastCreatedIds(IndexType& rNodeId, IndexType& rElemId, IndexType& rCondId)
{
    // Get the id
    rNodeId = mLastNodeId;
    rElemId = mLastElemId;
    rCondId = mLastCondId;
}


/// Remove the refined entities
template< unsigned int TDim>
void UniformRefinementUtility<TDim>::RemoveRefinedEntities(Flags ThisFlag)
{
    // Clear the maps
    for (ModelPart::NodeIterator node = mrModelPart.NodesBegin(); node < mrModelPart.NodesEnd(); node++)
    {
        if (node->Is(ThisFlag))
        {
            // auto search = mNodesColorMap.find(node->Id());
            // if (search != mNodesColorMap.end())
            //     mNodesColorMap.erase(search);

            for (NodesInEdgeMapType::iterator pair = mNodesMap.begin(); pair != mNodesMap.end(); pair++)
            {
                if (node->Id() == pair->second)
                    mNodesMap.erase(pair);
            }

            for (NodesInFaceMapType::iterator pair = mNodesInFaceMap.begin(); pair != mNodesInFaceMap.end(); pair++)
            {
                if (node->Id() == pair->second)
                    mNodesInFaceMap.erase(pair);
            }
        }
    }

    // for (ModelPart::ElementIterator elem = mrModelPart.ElementsBegin(); elem < mrModelPart.ElementsEnd(); elem++)
    // {
    //     if (elem->Is(ThisFlag))
    //     {
    //         auto search = mElemColorMap.find(elem->Id());
    //         if (search != mElemColorMap.end())
    //             mElemColorMap.erase(search);
    //     }
    // }

    // for (ModelPart::ConditionIterator cond = mrModelPart.ConditionsBegin(); cond < mrModelPart.ConditionsEnd(); cond++)
    // {
    //     if (cond->Is(ThisFlag))
    //     {
    //     auto search = mCondColorMap.find(cond->Id());
    //     if (search != mCondColorMap.end())
    //         mCondColorMap.erase(search);
    //     }
    // }

    // Remove the entities
    mrModelPart.RemoveNodesFromAllLevels(ThisFlag);
    mrModelPart.RemoveElementsFromAllLevels(ThisFlag);
    mrModelPart.RemoveConditionsFromAllLevels(ThisFlag);
}


/// Execute the refinement once
template< unsigned int TDim>
void UniformRefinementUtility<TDim>::ExecuteDivision(
    const int& rDivision,
    IndexIndexVectorMapType& rTagNodes,
    IndexIndexVectorMapType& rTagElems,
    IndexIndexVectorMapType& rTagConds
)
{
    // Initialize the auxiliary arrays for the elements and conditions to refine
    ElementsArrayType elements_to_refine;
    ConditionsArrayType conditions_to_refine;

    // Restart the elements and conditions maps (since the preexisting elements will be deleted)
    rTagElems.clear();
    rTagConds.clear();

    // Fill the auxiliary array with the elements to refine
    auto all_elem_begin = mrModelPart.ElementsBegin();
    for (int i = 0; i < static_cast<int>(mrModelPart.Elements().size()); i++)
    {
        auto i_element = all_elem_begin + i;

        // Check the divisions level of the origin elements
        if (i_element->GetValue(NUMBER_OF_DIVISIONS) == rDivision)
            elements_to_refine.push_back(*i_element.base());
    }

    // Fill the auxiliary array with the conditions to refine
    auto all_cond_begin = mrModelPart.ConditionsBegin();
    for (int i = 0; i < static_cast<int>(mrModelPart.Conditions().size()); i++)
    {
        auto i_condition = all_cond_begin + i;

        // Check the refinement level of the origin conditions
        if (i_condition->GetValue(NUMBER_OF_DIVISIONS) == rDivision)
            conditions_to_refine.push_back(*i_condition.base());
    }

    // Loop the origin elements
    ElementsArrayType::iterator elements_begin = elements_to_refine.begin();
    for (int i = 0; i < static_cast<int>(elements_to_refine.size()); i++)
    {
        // Get the element iterator
        auto i_element = elements_begin + i;

        // Get the refinement level of the origin element
        int step_divisions_level = rDivision + 1;

        // Get the geometry
        Geometry<NodeType>& geom = i_element->GetGeometry();

        if (geom.GetGeometryType() == GeometryData::KratosGeometryType::Kratos_Triangle2D3)
        {
            // Loop the edges of the father element and get the nodes
            int i_edge = 0;
            std::vector<NodeType::Pointer> middle_nodes(3);
            for (auto edge : geom.Edges())
                middle_nodes[i_edge++] = GetNodeInEdge(edge, step_divisions_level, rTagNodes);
            // AddNodesToSubModelParts(middle_nodes, mElemColorMap[i_element->Id()]);

            // Create the sub elements
            PointerVector<NodeType> sub_element_nodes(3);
            for (int position = 0; position < 4; position++)
            {
                sub_element_nodes = GetSubTriangleNodes(position, geom, middle_nodes);
                CreateElement(i_element, sub_element_nodes, step_divisions_level, rTagElems);
            }
        }
        else if (geom.GetGeometryType() == GeometryData::KratosGeometryType::Kratos_Quadrilateral2D4)
        {
            // Loop the edges of the father element and get the nodes
            int i_edge = 0;
            std::vector<NodeType::Pointer> middle_nodes(5);
            for (auto edge : geom.Edges())
                middle_nodes[i_edge++] = GetNodeInEdge(edge, step_divisions_level, rTagNodes);
            middle_nodes[4] = GetNodeInFace(geom, step_divisions_level, rTagNodes);
            // AddNodesToSubModelParts(middle_nodes, mElemColorMap[i_element->Id()]);

            // Create the sub elements
            PointerVector<NodeType> sub_element_nodes(4);
            for (int position = 0; position < 4; position++)
            {
                sub_element_nodes = GetSubQuadrilateralNodes(position, geom, middle_nodes);
                CreateElement(i_element, sub_element_nodes, step_divisions_level, rTagElems);
            }
        }
        else
        {
            KRATOS_ERROR << "Your geometry contains " << geom.GetGeometryType() << " which cannot be refined" << std::endl;
        }

        // Once we have created all the sub elements, the origin element must be deleted
        i_element->Set(TO_ERASE, true);
    }

    mrModelPart.RemoveElementsFromAllLevels(TO_ERASE);

    // Loop the origin conditions
    ConditionsArrayType::iterator conditions_begin = conditions_to_refine.begin();
    for (int i = 0; i < static_cast<int>(conditions_to_refine.size()); i++)
    {
        // Get the condition iterator
        auto i_condition = conditions_begin + i;

        // Get the refinement level of the origin condition
        int step_divisions_level = rDivision + 1;

        // Get the geometry
        Geometry<NodeType>& geom = i_condition->GetGeometry();

        if (geom.GetGeometryType() == GeometryData::KratosGeometryType::Kratos_Line2D2)
        {
            NodeType::Pointer middle_node = GetNodeInEdge(geom, step_divisions_level, rTagNodes);
            // AddNodeToSubModelParts(middle_node, mCondColorMap[i_condition->Id()]);

            // Create the sub conditions
            PointerVector<NodeType> sub_condition_nodes(2);
            for (int position = 0; position < 2; position++)
            {
                sub_condition_nodes = GetSubLineNodes(position, geom, middle_node);
                CreateCondition(i_condition, sub_condition_nodes, step_divisions_level, rTagConds);
            }
        }
        else
        {
            KRATOS_ERROR << "Your geometry contains " << geom.GetGeometryType() << " which cannot be refined" << std::endl;
        }

        // Once we have created all the sub conditions, the origin conditions must be deleted
        i_condition->Set(TO_ERASE, true);
    }

    mrModelPart.RemoveConditionsFromAllLevels(TO_ERASE);

}


/// Get the middle node on an edge
template <unsigned int TDim>
typename NodeType::Pointer UniformRefinementUtility<TDim>::GetNodeInEdge(
    const EdgeType& rEdge,
    const int& rNumberOfDivisions,
    IndexIndexVectorMapType& rTagNodes
)
{
    // Initialize the output
    NodeType::Pointer middle_node;

    // Get the middle node key
    std::pair<IndexType, IndexType> node_key;
    node_key = std::minmax(rEdge(0)->Id(), rEdge(1)->Id());

    // Check if the node exist
    auto search = mNodesMap.find(node_key);
    if (search != mNodesMap.end() )
    {
        middle_node = mrModelPart.Nodes()(search->second);
    }
    else
    {
        middle_node = CreateNodeInEdge(rEdge, rNumberOfDivisions, node_key, rTagNodes);
    }

    return middle_node;
}


/// Create a middle node on an edge. If the node does not exist, it creates one
template <unsigned int TDim>
typename NodeType::Pointer UniformRefinementUtility<TDim>::CreateNodeInEdge(
    const EdgeType& rEdge,
    const int& rNumberOfDivisions,
    const EdgeKeyType& rNodeKey,
    IndexIndexVectorMapType& rTagNodes
)
{
    // Initialize the output
    NodeType::Pointer middle_node;

    // Create the new node
    const double new_x = 0.5*rEdge(0)->X() + 0.5*rEdge(1)->X();
    const double new_y = 0.5*rEdge(0)->Y() + 0.5*rEdge(1)->Y();
    const double new_z = 0.5*rEdge(0)->Z() + 0.5*rEdge(1)->Z();
    middle_node = mrModelPart.CreateNewNode(++mLastNodeId, new_x, new_y, new_z);

    // Store the node key in the map
    mNodesMap[rNodeKey] = middle_node->Id();

    // interpolate the variables
    CalculateNodalStepData(middle_node, rEdge(0), rEdge(1));

    // Set the number of divisions level
    int& this_node_level = middle_node->GetValue(NUMBER_OF_DIVISIONS);
    this_node_level = rNumberOfDivisions;

    // Set the appropriate flags
    middle_node->Set(NEW_ENTITY, true);

    // Set the DoF's
    for (typename NodeType::DofsContainerType::const_iterator it_dof = mDofs.begin(); it_dof != mDofs.end(); ++it_dof)
        middle_node->pAddDof(*it_dof);

    // Store the created node on the taps map in order to later add it to the sub model parts
    IndexType tag = mNodesTags[rEdge(0)->Id()];
    rTagNodes[tag].push_back(middle_node->Id());
    mNodesTags[middle_node->Id()] = tag;

    return middle_node;
}


/// Get the middle node on a face defined by four nodes. If the node does not exist, it creates one
template< unsigned int TDim>
typename NodeType::Pointer UniformRefinementUtility<TDim>::GetNodeInFace(
    const FaceType& rFace,
    const int& rNumberOfDivisions,
    IndexIndexVectorMapType& rTagNodes
)
{
    // Initialize the output
    NodeType::Pointer middle_node;

    // Get the middle node key
    std::array<IndexType, 4> node_key = {{rFace(0)->Id(), rFace(1)->Id(), rFace(2)->Id(), rFace(3)->Id()}};
    std::sort(node_key.begin(), node_key.end());

    // Check if the node exist
    auto search = mNodesInFaceMap.find(node_key);
    if (search != mNodesInFaceMap.end() )
    {
        middle_node = mrModelPart.Nodes()(search->second);
    }
    else
    {
        middle_node = CreateNodeInFace(rFace, rNumberOfDivisions, node_key, rTagNodes);
    }

    return middle_node;
}


/// Get the middle node on a face defined by four nodes. If the node does not exist, it creates one
template< unsigned int TDim>
typename NodeType::Pointer UniformRefinementUtility<TDim>::CreateNodeInFace(
    const FaceType& rFace,
    const int& rNumberOfDivisions,
    const FaceKeyType& rNodeKey,
    IndexIndexVectorMapType& rTagNodes
)
{
    // Initialize the output
    NodeType::Pointer middle_node;

    // Create the new node
    const double new_x = 0.25*rFace(0)->X() + 0.25*rFace(1)->X() + 0.25*rFace(2)->X() + 0.25*rFace(3)->X();
    const double new_y = 0.25*rFace(0)->Y() + 0.25*rFace(1)->Y() + 0.25*rFace(2)->Y() + 0.25*rFace(3)->Y();
    const double new_z = 0.25*rFace(0)->Z() + 0.25*rFace(1)->Z() + 0.25*rFace(2)->Z() + 0.25*rFace(3)->Z();
    middle_node = mrModelPart.CreateNewNode(++mLastNodeId, new_x, new_y, new_z);

    // Store the node key in the map
    mNodesInFaceMap[rNodeKey] = middle_node->Id();

    // interpolate the variables
    CalculateNodalStepData(middle_node, rFace(0), rFace(1), rFace(2), rFace(3));

    // Set the refinement level
    int& this_node_level = middle_node->GetValue(NUMBER_OF_DIVISIONS);
    this_node_level = rNumberOfDivisions;

    // Set the appropriate flags
    middle_node->Set(NEW_ENTITY, true);

    // Set the DoF's
    for (typename NodeType::DofsContainerType::const_iterator it_dof = mDofs.begin(); it_dof != mDofs.end(); ++it_dof)
        middle_node->pAddDof(*it_dof);

    // Store the created node on the tags map in order to later add it to the sub model parts
    IndexType tag = mNodesTags[rFace(0)->Id()];
    rTagNodes[tag].push_back(middle_node->Id());
    mNodesTags[middle_node->Id()] = tag;

    return middle_node;
}


/// Compute the nodal data of a node
template< unsigned int TDim >
void UniformRefinementUtility<TDim>::CalculateNodalStepData(
    NodeType::Pointer pNewNode,
    const NodeType::Pointer pNode0,
    const NodeType::Pointer pNode1
    )
{
    for (IndexType step = 0; step < mBufferSize; step++)
    {
        double* new_node_data = pNewNode->SolutionStepData().Data(step);

        const double* node_data_0 = pNode0->SolutionStepData().Data(step);
        const double* node_data_1 = pNode1->SolutionStepData().Data(step);

        for (IndexType variable = 0; variable < mStepDataSize; variable++)
            new_node_data[variable] = 0.5 * node_data_0[variable] + 0.5 * node_data_1[variable];
    }

    WeakPointerVector<NodeType>& r_new_father_nodes = pNewNode->GetValue(FATHER_NODES);
    r_new_father_nodes.clear();
    r_new_father_nodes = pNode0->GetValue(FATHER_NODES);

    std::vector<double>& r_new_father_nodes_weights = pNewNode->GetValue(FATHER_NODES_WEIGHTS);
    r_new_father_nodes_weights.clear();
    r_new_father_nodes_weights = pNode0->GetValue(FATHER_NODES_WEIGHTS);

    AddOtherFatherNodes(r_new_father_nodes, r_new_father_nodes_weights,
        pNode1->GetValue(FATHER_NODES), pNode1->GetValue(FATHER_NODES_WEIGHTS));
}


/// Compute the nodal data of a node
template< unsigned int TDim >
void UniformRefinementUtility<TDim>::CalculateNodalStepData(
    NodeType::Pointer pNewNode,
    const NodeType::Pointer pNode0,
    const NodeType::Pointer pNode1,
    const NodeType::Pointer pNode2,
    const NodeType::Pointer pNode3
    )
{
    for (IndexType step = 0; step < mBufferSize; step++)
    {
        double* new_node_data = pNewNode->SolutionStepData().Data(step);

        const double* node_data_0 = pNode0->SolutionStepData().Data(step);
        const double* node_data_1 = pNode1->SolutionStepData().Data(step);
        const double* node_data_2 = pNode2->SolutionStepData().Data(step);
        const double* node_data_3 = pNode3->SolutionStepData().Data(step);

        for (IndexType variable = 0; variable < mStepDataSize; variable++)
            new_node_data[variable] = 0.25 * node_data_0[variable] + 0.25 * node_data_1[variable] +
                                      0.25 * node_data_2[variable] + 0.25 * node_data_3[variable];
    }

    WeakPointerVector<NodeType>& r_new_father_nodes = pNewNode->GetValue(FATHER_NODES);
    r_new_father_nodes.clear();
    r_new_father_nodes = pNode0->GetValue(FATHER_NODES);

    std::vector<double>& r_new_father_nodes_weights = pNewNode->GetValue(FATHER_NODES_WEIGHTS);
    r_new_father_nodes_weights.clear();
    r_new_father_nodes_weights = pNode0->GetValue(FATHER_NODES_WEIGHTS);

    AddOtherFatherNodes(r_new_father_nodes, r_new_father_nodes_weights,
        pNode1->GetValue(FATHER_NODES), pNode1->GetValue(FATHER_NODES_WEIGHTS), 0.5);
    AddOtherFatherNodes(r_new_father_nodes, r_new_father_nodes_weights,
        pNode1->GetValue(FATHER_NODES), pNode2->GetValue(FATHER_NODES_WEIGHTS), 1/3);
    AddOtherFatherNodes(r_new_father_nodes, r_new_father_nodes_weights,
        pNode1->GetValue(FATHER_NODES), pNode3->GetValue(FATHER_NODES_WEIGHTS), 0.25);
}


/// Add the father nodes which does not exist in the current father nodes
template<unsigned int TDim>
void UniformRefinementUtility<TDim>::AddOtherFatherNodes(
    WeakPointerVector<NodeType>& rThisFatherNodes,
    std::vector<double>& rThisFatherWeights,
    WeakPointerVector<NodeType>& rOtherFatherNodes,
    const std::vector<double>& rOtherFatherWeights,
    const double& rWeight
)
{
    for (auto& weight : rThisFatherWeights)
        weight *= (1-rWeight);

    WeakPointerVector<NodeType>::iterator other_nodes_begin = rOtherFatherNodes.begin();
    for (IndexType o = 0; o < rOtherFatherNodes.size(); o++)
    {
        auto other_node = other_nodes_begin + o;
        bool other_not_found = true;

        WeakPointerVector<NodeType>::iterator this_nodes_begin = rThisFatherNodes.begin();
        for (IndexType t = 0; (t < rThisFatherNodes.size()) && (other_not_found); t++)
        {
            auto this_node = this_nodes_begin + t;
            if (other_node->Id() == this_node->Id())
            {
                rThisFatherWeights[t] = rOtherFatherWeights[o] * rWeight;
                other_not_found = false;
            }
        }
        if (other_not_found)
        {
            rThisFatherNodes.push_back(*other_node.base());
            rThisFatherWeights.push_back(rOtherFatherWeights[o] * rWeight);
        }
    }
}


/// Create a sub element
template<unsigned int TDim>
void UniformRefinementUtility<TDim>::CreateElement(
    ElementsArrayType::iterator pOriginElement,
    PointerVector<NodeType>& rThisNodes,
    const int& rNumberOfDivisions,
    IndexIndexVectorMapType& rTagElems
)
{
    Element::Pointer sub_element = pOriginElement->Clone(++mLastElemId, rThisNodes);

    if (sub_element != nullptr)
    {
        // Add the element to the origin model part
        mrModelPart.AddElement(sub_element);

        // Set the refinement level
        int& this_elem_level = sub_element->GetValue(NUMBER_OF_DIVISIONS);
        this_elem_level = rNumberOfDivisions;

        // Store the father element pointer
        sub_element->SetValue(FATHER_ELEMENT, pOriginElement->GetValue(FATHER_ELEMENT));

        // // Add the element to the sub model parts
        // IndexType key = mElemColorMap[pOriginElement->Id()];
        // if (key != 0)  // NOTE: key==0 is the main model part
        // {
        //     for (std::string sub_name : mColors[key])
        //     {
        //         ModelPart& sub_model_part = SubModelPartsListUtility::GetRecursiveSubModelPart(mrModelPart, sub_name);
        //         sub_model_part.AddElement(sub_element);
        //     }
        // }
        // // Store the condition Id on the unique tag map
        // mElemColorMap[sub_element->Id()] = key;

        // Store the created element on the tags map in order to later add it to the sub model parts
        IndexType tag = mElementsTags[pOriginElement->Id()];
        rTagElems[tag].push_back(sub_element->Id());
        mElementsTags[sub_element->Id()] = tag;
    }
}


/// Create a sub condition
template<unsigned int TDim>
void UniformRefinementUtility<TDim>::CreateCondition(
    ConditionsArrayType::iterator pOriginCondition,
    PointerVector<NodeType>& rThisNodes,
    const int& rNumberOfDivisions,
    IndexIndexVectorMapType& rTagConds
    )
{
    Condition::Pointer sub_condition = pOriginCondition->Clone(++mLastCondId, rThisNodes);

    if (sub_condition != nullptr)
    {
        // Add the condition to the origin model part
        mrModelPart.AddCondition(sub_condition);

        // Set the refinement level
        int& this_cond_level = sub_condition->GetValue(NUMBER_OF_DIVISIONS);
        this_cond_level = rNumberOfDivisions;

        // // Add the conditions to the sub model parts
        // IndexType key = mCondColorMap[pOriginCondition->Id()];
        // if (key != 0)  // NOTE: key==0 is the main model part
        // {
        //     for (std::string sub_name : mColors[key])
        //     {
        //         ModelPart& sub_model_part = SubModelPartsListUtility::GetRecursiveSubModelPart(mrModelPart, sub_name);
        //         sub_model_part.AddCondition(sub_condition);
        //     }
        // }
        // // Store the condition Id on the unique tag map
        // mCondColorMap[sub_condition->Id()] = key;

        // Store the created condition on the tags map in order to later add it to the sub model parts
        IndexType tag = mConditionsTags[pOriginCondition->Id()];
        rTagConds[tag].push_back(sub_condition->Id());
        mConditionsTags[sub_condition->Id()] = tag;
    }
}


/// Return the nodes defining the i-subline
template<unsigned int TDim>
PointerVector<NodeType> UniformRefinementUtility<TDim>::GetSubLineNodes(
    const int Position,
    const Geometry<NodeType>& rGeom,
    NodeType::Pointer& rMiddleNode
    )
{
    PointerVector<NodeType> sub_line_nodes(2);

    if (Position == 0)
    {
        // First sub line
        sub_line_nodes(0) = rGeom.pGetPoint(0);
        sub_line_nodes(1) = rMiddleNode;
    }
    else if (Position == 1)
    {
        // second sub line
        sub_line_nodes(0) = rMiddleNode;
        sub_line_nodes(1) = rGeom.pGetPoint(1);
    }
    else
    {
        KRATOS_ERROR << "Attempting to get " << Position << " sub-line inside a line" << std::endl;
    }

    return sub_line_nodes;
}


/// Return the nodes defining the i-subtriangle
template<unsigned int TDim>
PointerVector<NodeType> UniformRefinementUtility<TDim>::GetSubTriangleNodes(
    const int Position,
    const Geometry<NodeType>& rGeom,
    std::vector<NodeType::Pointer>& rMiddleNodes
    )
{
    PointerVector<NodeType> sub_triangle_nodes(3);

    if (Position == 0)
    {
        // First sub triangle
        sub_triangle_nodes(0) = rGeom.pGetPoint(0);
        sub_triangle_nodes(1) = rMiddleNodes[0];
        sub_triangle_nodes(2) = rMiddleNodes[2];
    }
    else if (Position == 1)
    {
        // Second sub triangle
        sub_triangle_nodes(0) = rGeom.pGetPoint(1);
        sub_triangle_nodes(1) = rMiddleNodes[1];
        sub_triangle_nodes(2) = rMiddleNodes[0];
    }
    else if (Position == 2)
    {
        // Third sub triangle
        sub_triangle_nodes(0) = rGeom.pGetPoint(2);
        sub_triangle_nodes(1) = rMiddleNodes[2];
        sub_triangle_nodes(2) = rMiddleNodes[1];
    }
    else if (Position == 3)
    {
        // Fourth sub triangle (inner triangle)
        sub_triangle_nodes(0) = rMiddleNodes[0];
        sub_triangle_nodes(1) = rMiddleNodes[1];
        sub_triangle_nodes(2) = rMiddleNodes[2];
    }
    else
    {
        KRATOS_ERROR << "Attempting to get " << Position << " sub-triangle inside a triangle" << std::endl;
    }

    return sub_triangle_nodes;
}

/// Return the nodes defining the i-subquadrilateral
template<unsigned int TDim>
PointerVector<NodeType> UniformRefinementUtility<TDim>::GetSubQuadrilateralNodes(
    const int Position,
    const Geometry<NodeType>& rGeom,
    std::vector<NodeType::Pointer>& rMiddleNodes
    )
{
    PointerVector<NodeType> sub_quadrilateral_nodes(4);

    if (Position == 0)
    {
        // First sub element
        sub_quadrilateral_nodes(0) = rGeom.pGetPoint(0);
        sub_quadrilateral_nodes(1) = rMiddleNodes[0];
        sub_quadrilateral_nodes(2) = rMiddleNodes[4];
        sub_quadrilateral_nodes(3) = rMiddleNodes[3];
    }
    else if (Position == 1)
    {
        // Second sub element
        sub_quadrilateral_nodes(0) = rGeom.pGetPoint(1);
        sub_quadrilateral_nodes(1) = rMiddleNodes[1];
        sub_quadrilateral_nodes(2) = rMiddleNodes[4];
        sub_quadrilateral_nodes(3) = rMiddleNodes[0];
    }
    else if (Position == 2)
    {
        // Third sub element
        sub_quadrilateral_nodes(0) = rGeom.pGetPoint(2);
        sub_quadrilateral_nodes(1) = rMiddleNodes[2];
        sub_quadrilateral_nodes(2) = rMiddleNodes[4];
        sub_quadrilateral_nodes(3) = rMiddleNodes[1];
    }
    else if (Position == 3)
    {
        // Fourth sub element
        sub_quadrilateral_nodes(0) = rGeom.pGetPoint(3);
        sub_quadrilateral_nodes(1) = rMiddleNodes[3];
        sub_quadrilateral_nodes(2) = rMiddleNodes[4];
        sub_quadrilateral_nodes(3) = rMiddleNodes[2];
    }
    else
    {
        KRATOS_ERROR << "Attempting to get " << Position << " sub-quadrilateral inside a quadrilateral" << std::endl;
    }

    return sub_quadrilateral_nodes;
}


/// Add a node to the sub model parts specified by the tag
// template<unsigned int TDim>
// void UniformRefinementUtility<TDim>::AddNodeToSubModelParts(
//     NodeType::Pointer pNode,
//     IndexType Tag
//     )
// {
//     if (Tag != 0)  // NOTE: Tag==0 is the main model part
//     {
//         for (auto sub_name : mColors[Tag])
//         {
//             ModelPart& sub_model_part = SubModelPartsListUtility::GetRecursiveSubModelPart(mrModelPart, sub_name);
//             sub_model_part.AddNode(pNode);
//         }
//     }
//     // Store the node Id on the unique tag map
//     mNodesColorMap[pNode->Id()] = Tag;
// }


/// Add a node to the sub model parts specified by the tag
// template<unsigned int TDim>
// void UniformRefinementUtility<TDim>::AddNodesToSubModelParts(
//     std::vector<NodeType::Pointer>& rThisNodes,
//     IndexType Tag
//     )
// {
//     if (Tag != 0)  // NOTE: Tag==0 is the main model part
//     {
//         for (auto sub_name : mColors[Tag])
//         {
//             ModelPart& sub_model_part = SubModelPartsListUtility::GetRecursiveSubModelPart(mrModelPart, sub_name);
//             for (auto node : rThisNodes)
//                 sub_model_part.AddNode(node);
//         }
//     }
//     // Store the node Id on the unique tag map
//     for (auto node : rThisNodes)
//         mNodesColorMap[node->Id()] = Tag;
// }


template class UniformRefinementUtility<2>;

}  // namespace Kratos.
